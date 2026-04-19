"""
Load item & mob data from rAthena YAML files at startup.
Provides fast in-memory lookups for item names/types and mob drop rates.
"""
import os
import logging
import yaml
import config

logger = logging.getLogger(__name__)

# item_id -> {name, type, aegis}
_items: dict[int, dict] = {}
# aegis_name -> item_id
_aegis_map: dict[str, int] = {}
# item_id -> list of {mob, base_rate}
_mob_drops: dict[int, list[dict]] = {}
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


# ── Loader ────────────────────────────────────────────────────────────────────

async def load_game_data() -> None:
    base = config.RATHENA_PATH

    _load_drop_conf(os.path.join(base, 'conf/battle/drops.conf'))

    for path in [
        os.path.join(base, 'db/re/item_db.yml'),
        os.path.join(base, 'db/re/item_db2.yml'),
        os.path.join(base, 'db/import/item_db.yml'),
        os.path.join(base, 'db/import/item_db2.yml'),
    ]:
        _load_item_yaml(path)

    for path in [
        os.path.join(base, 'db/re/mob_db.yml'),
        os.path.join(base, 'db/re/mob_db2.yml'),
        os.path.join(base, 'db/import/mob_db.yml'),
        os.path.join(base, 'db/import/mob_db2.yml'),
    ]:
        _load_mob_yaml(path)

    logger.info(
        'Game data loaded: %d items, %d item-drop entries',
        len(_items), len(_mob_drops),
    )


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
            data = yaml.safe_load(f)
        if not data or 'Body' not in data:
            return
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
            data = yaml.safe_load(f)
        if not data or 'Body' not in data:
            return
        for mob in data['Body']:
            mob_name = mob.get('Name', mob.get('AegisName', 'Unknown'))
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
