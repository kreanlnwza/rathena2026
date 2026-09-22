#!/usr/bin/env python3
"""Validate the 2026 client manifest against rAthena DB and source integration."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

import yaml


FIRST_ID = 6606
LAST_ID = 6643
STATUS_INTEGRATION = {
    "SC_VENOMIGNITION": ("VenomIgnition", "EFST_VENOMIGNITION"),
    "SC_ELEMENTAL_INTEGRATION": ("Elemental_Integration", "EFST_ELEMENTAL_INTEGRATION"),
    "SC_SEVENTH_KICK_SKILLORB": ("Seventh_Kick_SkillOrb", "EFST_SEVENTH_KICK_SKILLORB"),
    "SC_SEVENTH_KICK_MAX": ("Seventh_Kick_Max", "EFST_SEVENTH_KICK_MAX"),
    "SC_KI_SUL_AND_CHUL_HO": ("Ki_Sul_And_Chul_Ho", "EFST_KI_SUL_AND_CHUL_HO"),
    "SC_KI_SUL_AND_HYUN_ROK": ("Ki_Sul_And_Hyun_Rok", "EFST_KI_SUL_AND_HYUN_ROK"),
    "SC_NOBORU": ("Noboru", "EFST_NOBORU"),
    "SC_PRIMED_TRAP": ("Primed_Trap", "EFST_PRIMED_TRAP"),
}


def unique_index(entries: list[dict], id_key: str, name_key: str, label: str) -> dict[int, dict]:
    result: dict[int, dict] = {}
    names: set[str] = set()
    for entry in entries:
        entry_id = entry[id_key]
        name = entry[name_key]
        if entry_id in result:
            raise ValueError(f"{label}: duplicate ID {entry_id}")
        if name in names:
            raise ValueError(f"{label}: duplicate name {name}")
        result[entry_id] = entry
        names.add(name)
    return result


def levels(value, count: int, key: str, default: int = 0) -> list[int]:
    if value is None:
        return [default] * count
    if isinstance(value, int):
        return [value] * count
    result = [default] * count
    current = default
    changes = {entry["Level"]: entry[key] for entry in value}
    for level in range(1, count + 1):
        current = changes.get(level, current)
        result[level - 1] = current
    return result


def missing_status_reference_errors(
    skill_records: list[dict], status_names: set[str]
) -> list[str]:
    errors: list[str] = []
    for skill in skill_records:
        if not FIRST_ID <= skill["Id"] <= LAST_ID:
            continue
        references: list[str] = []
        if isinstance(skill.get("Status"), str):
            references.append(skill["Status"])
        required_statuses = skill.get("Requires", {}).get("Status", {})
        if isinstance(required_statuses, dict):
            references.extend(required_statuses)
        for status_name in references:
            if status_name not in status_names:
                errors.append(
                    f"skill DB {skill['Name']} references missing status {status_name}"
                )
    return errors


def source_integration_errors(
    source_root: Path, manifest: dict, skill_records: list[dict]
) -> list[str]:
    errors: list[str] = []
    paths = {
        "skill enum": source_root / "src/map/skill.hpp",
        "status enum": source_root / "src/map/status.hpp",
        "script constants": source_root / "src/map/script_constants.hpp",
        "status DB": source_root / "db/re/status.yml",
        "MAX_SKILL": source_root / "src/common/mmo.hpp",
    }
    missing = [f"{label}: {path}" for label, path in paths.items() if not path.is_file()]
    if missing:
        return ["missing source integration file " + item for item in missing]

    skill_hpp = paths["skill enum"].read_text(encoding="utf-8")
    status_hpp = paths["status enum"].read_text(encoding="utf-8")
    constants = paths["script constants"].read_text(encoding="utf-8")
    mmo_hpp = paths["MAX_SKILL"].read_text(encoding="utf-8")
    status_body = yaml.safe_load(paths["status DB"].read_text(encoding="utf-8"))["Body"]
    try:
        status_by_name = unique_index(status_body, "Status", "Status", "status DB")
    except ValueError as error:
        errors.append(str(error))
        status_by_name = {}

    expected_skills = sorted(manifest["skills"], key=lambda entry: entry["id"])
    first_name = expected_skills[0]["name"]
    last_name = expected_skills[-1]["name"]
    start = skill_hpp.find(first_name)
    end = skill_hpp.find(last_name, start)
    if start < 0 or end < 0:
        errors.append("skill enum does not contain the complete 6606..6643 range")
    else:
        end = skill_hpp.find("\n", end)
        enum_slice = skill_hpp[start:end]
        enum_values: dict[str, int] = {}
        for index, match in enumerate(re.finditer(
            r"^\s*([A-Z][A-Z0-9_]*)\s*(?:=\s*(\d+))?\s*,", enum_slice, re.M
        )):
            name, explicit = match.groups()
            inferred = FIRST_ID + index
            if explicit is not None and int(explicit) != inferred:
                errors.append(f"skill enum explicit {name}={explicit} != {inferred}")
            enum_values[name] = inferred
        for skill in expected_skills:
            if enum_values.get(skill["name"]) != skill["id"]:
                errors.append(
                    f"skill enum {skill['name']}={enum_values.get(skill['name'])} "
                    f"!= {skill['id']}"
                )

    max_skill_match = re.search(r"^#define\s+MAX_SKILL\s+(\d+)", mmo_hpp, re.M)
    if max_skill_match is None:
        errors.append("MAX_SKILL definition not found")
    else:
        max_skill = int(max_skill_match.group(1))
        if max_skill < len(skill_records):
            errors.append(f"MAX_SKILL {max_skill} < {len(skill_records)} skill DB records")

    efst_ids = {entry["name"]: entry["id"] for entry in manifest["statuses"]}
    for sc_name, (status_name, efst_name) in STATUS_INTEGRATION.items():
        if re.search(rf"^\s*{re.escape(sc_name)}\s*,", status_hpp, re.M) is None:
            errors.append(f"status enum missing {sc_name}")
        if f"export_constant({sc_name});" not in constants:
            errors.append(f"script constants missing {sc_name}")

        efst_id = efst_ids.get(efst_name)
        if efst_id is None:
            errors.append(f"manifest missing {efst_name}")
        elif re.search(
            rf"^\s*{re.escape(efst_name)}\s*=\s*{efst_id}\s*,", status_hpp, re.M
        ) is None:
            errors.append(f"EFST enum missing {efst_name}={efst_id}")
        if f"export_constant({efst_name});" not in constants:
            errors.append(f"script constants missing {efst_name}")

        status_entry = status_by_name.get(status_name)
        if status_entry is None:
            errors.append(f"status DB missing {status_name}")
        elif status_entry.get("Icon") != efst_name:
            errors.append(
                f"status DB {status_name} icon {status_entry.get('Icon')} != {efst_name}"
            )

    errors.extend(missing_status_reference_errors(skill_records, set(status_by_name)))

    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, default=Path("doc/skill_2026/manifest.json"))
    parser.add_argument("--skill-db", type=Path, default=Path("db/re/skill_db.yml"))
    parser.add_argument("--skill-tree", type=Path, default=Path("db/re/skill_tree.yml"))
    parser.add_argument("--source-root", type=Path, default=Path("."))
    args = parser.parse_args()

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    body = yaml.safe_load(args.skill_db.read_text(encoding="utf-8"))["Body"]
    tree = yaml.safe_load(args.skill_tree.read_text(encoding="utf-8"))["Body"]
    errors: list[str] = []
    try:
        all_db = unique_index(body, "Id", "Name", "skill DB")
        db = {
            entry_id: entry
            for entry_id, entry in all_db.items()
            if FIRST_ID <= entry_id <= LAST_ID
        }
        expected = unique_index(manifest["skills"], "id", "name", "manifest")
    except ValueError as error:
        print(error)
        return 1

    if set(db) != set(expected):
        errors.append(f"ID set mismatch: db={sorted(db)} manifest={sorted(expected)}")

    delay_fields = {
        "global": "AfterCastActDelay",
        "fixed_cast": "FixedCastTime",
        "cooldown": "Cooldown",
        "variable_cast": "CastTime",
    }
    for skill_id, source in expected.items():
        target = db.get(skill_id)
        if target is None:
            continue
        count = source["max_level"]
        if target["Name"] != source["name"]:
            errors.append(f"{skill_id}: name {target['Name']} != {source['name']}")
        if target["MaxLevel"] != count:
            errors.append(f"{skill_id}: max level {target['MaxLevel']} != {count}")

        requires = target.get("Requires", {})
        checks = [
            ("SP", source["sp"], levels(requires.get("SpCost"), count, "Amount")),
            ("AP", source["ap"], levels(requires.get("ApCost"), count, "Amount")),
        ]
        for label, client_value, server_value in checks:
            if client_value is not None and client_value != server_value:
                errors.append(f"{skill_id}: {label} {server_value} != {client_value}")

        client_range = source["client_range"]
        server_range = levels(target.get("Range"), count, "Size", default=1)
        if client_range is not None and client_range != server_range:
            errors.append(f"{skill_id}: range {server_range} != {client_range}")

        for client_field, server_field in delay_fields.items():
            client_value = source["delay_ms"][client_field]
            server_value = levels(target.get(server_field), count, "Time")
            if client_value is not None and client_value != server_value:
                errors.append(f"{skill_id}: {server_field} {server_value} != {client_value}")

    tree_entries: dict[str, list[dict]] = {}
    for job in tree:
        for skill in job.get("Tree", []):
            tree_entries.setdefault(skill["Name"], []).append(skill)
    tree_names = set(tree_entries)
    visible = {entry["name"] for entry in expected.values() if entry["role"] != "hidden_trigger"}
    hidden = {entry["name"] for entry in expected.values() if entry["role"] == "hidden_trigger"}
    if missing := visible - tree_names:
        errors.append(f"visible skills missing from tree: {sorted(missing)}")
    if exposed := hidden & tree_names:
        errors.append(f"hidden triggers present in tree: {sorted(exposed)}")
    for source in expected.values():
        if source["role"] == "hidden_trigger":
            continue
        expected_requirements = {
            (entry["name"], entry["level"]) for entry in source["requirements"]
        }
        for entry in tree_entries.get(source["name"], []):
            actual_requirements = {
                (item["Name"], item["Level"]) for item in entry.get("Requires", [])
            }
            if entry["MaxLevel"] != source["max_level"]:
                errors.append(
                    f"{source['name']}: tree max {entry['MaxLevel']} != {source['max_level']}"
                )
            if actual_requirements != expected_requirements:
                errors.append(
                    f"{source['name']}: tree requirements {sorted(actual_requirements)} "
                    f"!= {sorted(expected_requirements)}"
                )

    errors.extend(source_integration_errors(args.source_root.resolve(), manifest, body))
    if errors:
        print("\n".join(errors))
        return 1
    print(
        f"validated {len(expected)} records; {len(visible)} tree-visible; "
        f"{len(hidden)} hidden; source integration complete"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
