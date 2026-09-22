#!/usr/bin/env python3
"""Build the bounded 2026 skill manifest from decompiled kRO client tables."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path

FIRST_ID = 6606
LAST_ID = 6643
HIDDEN_IDS = {6616, 6617, 6618, 6619, 6620, 6636}
STATUS_GATED_IDS = {6626, 6627, 6628, 6629}
STATUS_NAMES = {
    "EFST_VENOMIGNITION", "EFST_ELEMENTAL_INTEGRATION",
    "EFST_SEVENTH_KICK_SKILLORB", "EFST_SEVENTH_KICK_MAX",
    "EFST_KI_SUL_AND_CHUL_HO", "EFST_KI_SUL_AND_HYUN_ROK",
    "EFST_NOBORU", "EFST_PRIMED_TRAP", "EFST_WERERAPTOR",
    "EFST_ENRAGE_RAPTOR", "EFST_TRUTH_OF_ICE", "EFST_TRUTH_OF_WIND",
    "EFST_TRUTH_OF_EARTH", "EFST_APEX_PHASE", "EFST_RAPTORIAL_INSTINCT",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def table_block(text: str, name: str) -> str | None:
    marker = f"[SKID.{name}] = {{"
    start = text.find(marker)
    if start < 0:
        return None
    brace = text.find("{", start)
    depth = 0
    quoted = False
    escaped = False
    for index in range(brace, len(text)):
        char = text[index]
        if quoted:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                quoted = False
            continue
        if char == '"':
            quoted = True
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise ValueError(f"unterminated table for {name}")


def int_field(block: str | None, field: str) -> int | None:
    match = re.search(rf"\b{re.escape(field)}\s*=\s*(\d+)", block or "")
    return int(match.group(1)) if match else None


def int_array(block: str | None, field: str) -> list[int] | None:
    match = re.search(rf"\b{re.escape(field)}\s*=\s*{{([^}}]*)}}", block or "")
    return [int(value) for value in re.findall(r"\d+", match.group(1))] if match else None


def prerequisites(block: str | None) -> list[dict[str, int | str]]:
    if not block or "_NeedSkillList" not in block:
        return []
    tail = block[block.find("_NeedSkillList"):]
    return [
        {"name": name, "level": int(level)}
        for name, level in re.findall(r"{\s*SKID\.(\w+)\s*,\s*(\d+)\s*}", tail)
    ]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("directory", type=Path)
    args = parser.parse_args()
    root = args.directory.resolve()
    skill = root / "data" / "luafiles514" / "lua files" / "skillinfoz"
    state = root / "data" / "luafiles514" / "lua files" / "stateicon"
    paths = {
        "skillid": skill / "skillid.lub",
        "skillinfolist": skill / "skillinfolist.lub",
        "skilldescript": skill / "skilldescript.lub",
        "skilldelaylist": skill / "skilldelaylist.lub",
        "skilltreeview": skill / "skilltreeview.lub",
        "jobinheritlist": skill / "jobinheritlist.lub",
        "efstids": state / "efstids.lub",
    }
    text = {
        key: path.with_suffix(path.suffix + ".lua").read_text(encoding="utf-8")
        for key, path in paths.items()
    }
    ids = {
        int(skill_id): name
        for name, skill_id in re.findall(r"^\s*(\w+)\s*=\s*(\d+),?\s*$", text["skillid"], re.M)
        if FIRST_ID <= int(skill_id) <= LAST_ID
    }
    if sorted(ids) != list(range(FIRST_ID, LAST_ID + 1)):
        raise ValueError(f"expected contiguous {FIRST_ID}..{LAST_ID}; got {sorted(ids)}")

    skills = []
    for skill_id, name in sorted(ids.items()):
        info = table_block(text["skillinfolist"], name)
        delay = table_block(text["skilldelaylist"], name)
        skills.append({
            "id": skill_id,
            "name": name,
            "role": (
                "hidden_trigger" if skill_id in HIDDEN_IDS else
                "status_gated_learnable" if skill_id in STATUS_GATED_IDS else
                "learnable"
            ),
            "max_level": int_field(info, "MaxLv"),
            "sp": int_array(info, "SpAmount"),
            "ap": int_array(info, "ApAmount"),
            "client_range": int_array(info, "AttackRange"),
            "requirements": prerequisites(info),
            "delay_ms": {
                "global": int_array(delay, "SkillGlobalPostDelay"),
                "fixed_cast": int_array(delay, "SkillCastFixedDelay"),
                "cooldown": int_array(delay, "SkillSinglePostDelay"),
                "variable_cast": int_array(delay, "SkillCastStatDelay"),
            },
            "client_description_present": table_block(text["skilldescript"], name) is not None,
        })

    statuses = [
        {"name": name, "id": int(status_id)}
        for name, status_id in re.findall(r"^\s*(EFST_\w+)\s*=\s*(\d+),?\s*$", text["efstids"], re.M)
        if name in STATUS_NAMES
    ]
    result = {
        "source": {
            "archive": r"Z:\KR_RO1_Live_20260401_155632\data.grf",
            "archive_size": 4925222202,
            "archive_mtime_local": "2026-09-16 17:19:36 +07:00",
            "format": "GRF 0x300 / GPakEx 0x80",
            "key_discovery": "Automatic offline key discovery verified: 4 method keys.",
            "entries": {
                key: {
                    "path": str(path.relative_to(root)).replace("/", "\\"),
                    "size": path.stat().st_size,
                    "sha256": sha256(path),
                }
                for key, path in paths.items()
            },
        },
        "record_count": len(skills),
        "record_range": [FIRST_ID, LAST_ID],
        "scope_note": (
            "Coverage is 38 primary client records: IDs 6606 and 6607 supplied by "
            "rAthena PR 9765 plus 36 records added by this change. Six are hidden "
            "trigger records and four are status-gated Primed Trap attacks; IDs 6644 "
            "and 6645 do not exist in the primary skill table."
        ),
        "skills": skills,
        "statuses": statuses,
    }
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
