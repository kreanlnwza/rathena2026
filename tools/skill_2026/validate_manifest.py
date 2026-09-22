#!/usr/bin/env python3
"""Validate the 2026 client manifest against the rAthena skill databases."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import yaml


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


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, default=Path("doc/skill_2026/manifest.json"))
    parser.add_argument("--skill-db", type=Path, default=Path("db/re/skill_db.yml"))
    parser.add_argument("--skill-tree", type=Path, default=Path("db/re/skill_tree.yml"))
    args = parser.parse_args()

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    body = yaml.safe_load(args.skill_db.read_text(encoding="utf-8"))["Body"]
    tree = yaml.safe_load(args.skill_tree.read_text(encoding="utf-8"))["Body"]
    db = {entry["Id"]: entry for entry in body if 6606 <= entry["Id"] <= 6643}
    expected = {entry["id"]: entry for entry in manifest["skills"]}
    errors: list[str] = []

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

    if errors:
        print("\n".join(errors))
        return 1
    print(f"validated {len(expected)} records; {len(visible)} tree-visible; {len(hidden)} hidden")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
