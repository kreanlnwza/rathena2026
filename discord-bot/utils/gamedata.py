"""
Load item & mob data from rAthena YAML files at startup.
Provides fast in-memory lookups for item names/types and mob drop rates.

Performance:
- Pickle cache: parsed data is saved to data/gamedata.pickle and reused
  unless any YAML file is newer (typically loads in <100ms vs 2-5s YAML parse).
- yaml.CSafeLoader (C extension) for the initial parse.
- Runs in a thread pool to avoid blocking the event loop.
"""
import asyncio
import os
import pickle
import logging
import yaml
import config

logger = logging.getLogger(__name__)

# Prefer C-based loader for speed (~5-10× faster than pure Python)
_Loader = getattr(yaml, 'CSafeLoader', yaml.SafeLoader)

_CACHE_FILE = os.path.join(os.path.dirname(__file__), '..', 'data', 'gamedata.pickle')
_CACHE_VERSION = 1  # bump when data schema changes

# item_id -> {name, type, aegis}
_items: dict[int, dict] = {}
# aegis_name -> item_id
_aegis_map: dict[str, int] = {}
# item_id -> list of {mob, base_rate}
_mob_drops: dict[int, list[dict]] = {}
# mob_id -> mob name
_mob_names: dict[int, str] = {}
# drop multipliers from drops.conf  (100 = 1×)
_mult: dict[str, float] = {'common': 1.0, 'equip': 1.0, 'card': 1.0}


# ── Public API ────────────────────────────────────────────────────────────────

def get_item(item_id: int) -> dict | None:
    return _items.get(item_id)


def search_items(keyword: str, limit: int = 10) -> list[dict]:
    kw = keyword.lower()
    results = []
    for iid, data in _items.items():
        if kw in data['name'].lower() or kw in data['aegis'].lower():
            results.append({'id': iid, **data})
            if len(results) >= limit:
                break
    return results


def get_item_drops(item_id: int) -> list[dict]:
    return _mob_drops.get(item_id, [])


def format_rate(base_rate: int, item_type: str) -> str:
    """Return 'X.XX% (×Y)' string."""
    if item_type == 'Card':
        mult = _mult['card']
    elif item_type in ('Weapon', 'Armor'):
        mult = _mult['equip']
    else:
        mult = _mult['common']

    effective = min(base_rate * mult, 10000)
    pct = effective / 100
    base_pct = base_rate / 100
    if mult != 1.0:
        return f'{pct:.4f}% (base {base_pct:.4f}% ×{mult:.1f})'
    return f'{pct:.4f}%'


def get_multipliers() -> dict:
    return dict(_mult)


def item_count() -> int:
    return len(_items)


def mob_drop_count() -> int:
    return len(_mob_drops)


def get_mob_name(mob_id: int) -> str:
    return _mob_names.get(mob_id, f'Monster#{mob_id}')


# ── Loader ────────────────────────────────────────────────────────────────────

async def load_game_data() -> None:
    """Load game data in a thread pool to avoid blocking the event loop."""
    loop = asyncio.get_running_loop()
    await loop.run_in_executor(None, _load_all_sync)


def _load_all_sync() -> None:
    """Synchronous loader — runs inside thread pool."""
    import time
    t0 = time.perf_counter()

    base = config.RATHENA_PATH
    drops_conf = os.path.join(base, 'conf/battle/drops.conf')

    # Resolve file paths for this mode
    re_path = os.path.join(base, 'db/re/item_db.yml')
    if os.path.exists(re_path):
        mode = 're'
    else:
        mode = 'pre-re'

    item_paths = [
        os.path.join(base, f'db/{mode}/item_db.yml'),
        os.path.join(base, f'db/{mode}/item_db2.yml'),
        os.path.join(base, f'db/{mode}/item_db_equip.yml'),
        os.path.join(base, f'db/{mode}/item_db_etc.yml'),
        os.path.join(base, f'db/{mode}/item_db_usable.yml'),
        os.path.join(base, 'db/import/item_db.yml'),
        os.path.join(base, 'db/import/item_db2.yml'),
    ]
    mob_paths = [
        os.path.join(base, f'db/{mode}/mob_db.yml'),
        os.path.join(base, f'db/{mode}/mob_db2.yml'),
        os.path.join(base, 'db/import/mob_db.yml'),
        os.path.join(base, 'db/import/mob_db2.yml'),
    ]
    all_sources = [drops_conf] + item_paths + mob_paths

    # ── Try pickle cache first ────────────────────────────────────────────────
    if _try_load_cache(all_sources):
        elapsed = time.perf_counter() - t0
        logger.info(
            'Game data loaded from cache: %d items, %d item-drop entries (%.2fs)',
            len(_items), len(_mob_drops), elapsed,
        )
        return

    # ── Fall back to YAML parsing ─────────────────────────────────────────────
    _load_drop_conf(drops_conf)

    for path in item_paths:
        _load_item_yaml(path)

    for path in mob_paths:
        _load_mob_yaml(path)

    _save_cache(all_sources)

    elapsed = time.perf_counter() - t0
    logger.info(
        'Game data loaded from YAML: %d items, %d item-drop entries (%.1fs)',
        len(_items), len(_mob_drops), elapsed,
    )


def _try_load_cache(source_paths: list[str]) -> bool:
    """Load from pickle cache if it exists and is newer than all source files."""
    if not os.path.exists(_CACHE_FILE):
        return False
    try:
        cache_mtime = os.path.getmtime(_CACHE_FILE)
        for p in source_paths:
            if os.path.exists(p) and os.path.getmtime(p) > cache_mtime:
                return False  # YAML updated — invalidate cache

        with open(_CACHE_FILE, 'rb') as f:
            data = pickle.load(f)

        if data.get('version') != _CACHE_VERSION:
            return False

        _items.update(data['items'])
        _aegis_map.update(data['aegis'])
        _mob_drops.update(data['drops'])
        _mob_names.update(data['mob_names'])
        _mult.update(data['mult'])
        return True
    except Exception as e:
        logger.warning('Cache load failed (%s) — falling back to YAML', e)
        return False


def _save_cache(source_paths: list[str]) -> None:
    try:
        os.makedirs(os.path.dirname(_CACHE_FILE), exist_ok=True)
        with open(_CACHE_FILE, 'wb') as f:
            pickle.dump({
                'version': _CACHE_VERSION,
                'items': _items,
                'aegis': _aegis_map,
                'drops': _mob_drops,
                'mob_names': _mob_names,
                'mult': _mult,
            }, f, protocol=pickle.HIGHEST_PROTOCOL)
    except Exception as e:
        logger.warning('Cache save failed: %s', e)


# ── Private helpers ───────────────────────────────────────────────────────────

def _load_drop_conf(path: str) -> None:
    if not os.path.exists(path):
        return
    try:
        with open(path, encoding='utf-8') as f:
            for raw in f:
                line = raw.strip()
                if line.startswith('//') or ':' not in line:
                    continue
                key, _, rest = line.partition(':')
                key = key.strip()
                try:
                    val = int(rest.split('//')[0].strip())
                except ValueError:
                    continue
                if key == 'item_drop_common':
                    _mult['common'] = val / 100
                elif key == 'item_drop_equip':
                    _mult['equip'] = val / 100
                elif key == 'item_drop_card':
                    _mult['card'] = val / 100
    except Exception as e:
        logger.warning('drops.conf load failed: %s', e)


def _load_item_yaml(path: str) -> None:
    if not os.path.exists(path):
        return
    try:
        with open(path, encoding='utf-8') as f:
            raw = f.read()

        # Use C loader for speed
        for data in yaml.load_all(raw, Loader=_Loader):
            if not data or 'Body' not in data:
                continue
            for entry in data['Body']:
                iid = entry.get('Id')
                if iid is None:
                    continue
                aegis = entry.get('AegisName', '')
                name = entry.get('Name', aegis)
                itype = entry.get('Type', 'Etc')
                _items[iid] = {'name': name, 'type': itype, 'aegis': aegis}
                if aegis:
                    _aegis_map[aegis] = iid
        logger.debug('Loaded items from %s', path)
    except Exception as e:
        logger.warning('item YAML load failed (%s): %s', path, e)


def _load_mob_yaml(path: str) -> None:
    if not os.path.exists(path):
        return
    try:
        with open(path, encoding='utf-8') as f:
            raw = f.read()

        data = yaml.load(raw, Loader=_Loader)
        if not data or 'Body' not in data:
            return
        for mob in data['Body']:
            mob_id = mob.get('Id')
            mob_name = mob.get('Name', mob.get('AegisName', 'Unknown'))
            if mob_id is not None:
                _mob_names[mob_id] = mob_name
            for drop in list(mob.get('Drops', [])) + list(mob.get('MvpDrops', [])):
                ref = drop.get('Item', '')
                base_rate = int(drop.get('Rate', 0))
                if isinstance(ref, int):
                    item_id = ref
                else:
                    item_id = _aegis_map.get(str(ref))
                if item_id is None:
                    continue
                _mob_drops.setdefault(item_id, []).append(
                    {'mob': mob_name, 'base_rate': base_rate}
                )
        logger.debug('Loaded mob drops from %s', path)
    except Exception as e:
        logger.warning('mob YAML load failed (%s): %s', path, e)
