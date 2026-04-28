#!/usr/bin/env python3
"""
item_image_writer.py - Read item data from an item card image and write to rAthena item database.

Usage:
    python tools/item_image_writer.py <image_path> [--id <item_id>]

Output:
    Appends the extracted item entry to db/import/item_db.yml
    (automatically loaded by rAthena via db/item_db.yml Footer import)

Requirements:
    pip install anthropic pyyaml
    export ANTHROPIC_API_KEY=<your_key>
"""

import anthropic
import base64
import json
import os
import sys

import yaml

CUSTOM_DB_PATH = "db/import/item_db.yml"
MODEL = "claude-sonnet-4-6"

EXTRACTION_PROMPT = """\
You are an expert at reading Ragnarok Online item cards and converting them to rAthena item database format.

Analyze the item card image carefully and extract ALL item data shown.

Return ONLY a valid JSON object with the following structure. Omit fields that are not visible or not applicable.

{
  "Id": <integer - use the item ID if shown, otherwise use 30000>,
  "AegisName": "<server_name_with_underscores_no_spaces>",
  "Name": "<exact display name from the image including [N] slot notation>",
  "Type": "<one of: Weapon | Armor | Healing | Usable | Etc | Card>",
  "SubType": "<weapon subtype if applicable: Fist|Dagger|1hSword|2hSword|1hSpear|2hSpear|1hAxe|2hAxe|Mace|Staff|Bow|Knuckle|Musical|Whip|Book|Katar|2hStaff>",
  "Buy": <integer NPC buy price if shown>,
  "Weight": <integer - weight value as shown (e.g. if image shows '110' use 110)>,
  "Attack": <integer ATK value for weapons>,
  "MagicAttack": <integer MATK value if shown>,
  "Defense": <integer DEF value for armor>,
  "Range": <integer weapon attack range, usually 1 for melee>,
  "Slots": <integer number of card slots, e.g. 1 if name shows [1]>,
  "Jobs": {
    "<JobName>": true
  },
  "Locations": {
    "<location>": true
  },
  "WeaponLevel": <1-4 for weapons>,
  "ArmorLevel": <1-4 for armor>,
  "EquipLevelMin": <integer minimum level to equip>,
  "Refineable": <true or false>,
  "Flags": {
    "BuyingStore": <true if tradeable in buying stores>
  },
  "Script": "<full rAthena item script — see translation rules below>",
  "EquipScript": "<rAthena script executed on equip, if any>",
  "UnEquipScript": "<rAthena script executed on unequip, if any>"
}

Valid Job names: All, Novice, Swordman, Mage, Archer, Acolyte, Merchant, Thief,
Knight, Priest, Wizard, Blacksmith, Hunter, Assassin, Crusader, Monk, Sage,
Rogue, Alchemist, Bard, Dancer, SuperNovice, Gunslinger, Ninja, RuneKnight,
ArchBishop, Warlock, Ranger, Mechanic, GuillotineCross, RoyalGuard, Sorcerer,
Minstrel, Wanderer, ShadowChaser, Sura, Genetic, StarEmperor, SoulReaper,
Doram

Valid Locations: Head_Top, Head_Mid, Head_Low, Armor, Right_Hand, Left_Hand,
Garment, Shoes, Right_Accessory, Left_Accessory

Script translation rules — convert item description text to rAthena script:
- "Indestructible in battle" → bonus bUnbreakableWeapon;
- "Indestructible" → bonus bUnbreakableWeapon;
- "Reduces damage taken from players by N%" → bonus bSubDmgRaceRate,RC_Player,-N;
- "Reduce damage from players by N%" → bonus bSubDmgRaceRate,RC_Player,-N;
- "ATK +N" or "ATK increases by N" → bonus bBaseAtk,N;
- "MATK +N" → bonus bMatk,N;
- "STR +N" → bonus bStr,N;
- "AGI +N" → bonus bAgi,N;
- "VIT +N" → bonus bVit,N;
- "INT +N" → bonus bInt,N;
- "DEX +N" → bonus bDex,N;
- "LUK +N" → bonus bLuk,N;
- "MaxHP +N" → bonus bMaxHP,N;
- "MaxSP +N" → bonus bMaxSP,N;
- "MaxHP +N%" → bonus bMaxHPrate,N;
- "HIT +N" → bonus bHit,N;
- "FLEE +N" → bonus bFlee,N;
- "CRIT +N" → bonus bCritical,N;
- "DEF +N" → bonus bDef,N;
- "MDEF +N" → bonus bMdef,N;
- "ASPD +N%" → bonus bAspdRate,N;
- "Cast time reduced by N%" → bonus bCastrate,-N;
- "Increases physical damage against all races by N%" → bonus2 bAddRace,RC_All,N;
- "Increases magical damage against all races by N%" → bonus2 bMagicAddRace,RC_All,N;
- "Reduce damage from [Race] by N%" → bonus2 bSubRace,RC_<Race>,N;
- "Increase damage from [Element] property by N%" → bonus2 bMagicAtkEle,Ele_<Element>,N;
- "For every N BaseLevel" → use conditional: if (BaseLevel >= X) { ... }
- "For every upgrade level" → use: .@r = getrefine(); with conditionals
- "When equipped by [job]" → use: if (BaseJob == Job_<JobName>) { ... }
- Write complex conditional scripts using if/else blocks with proper indentation
- Use semicolons at end of each statement

For complex scripts with multiple conditions, write them as proper rAthena script code.
Return ONLY the JSON object, no markdown code blocks, no explanation.
"""


class LiteralStr(str):
    """String subclass that forces YAML literal block scalar style."""
    pass


def _literal_presenter(dumper, data):
    if "\n" in data:
        return dumper.represent_scalar("tag:yaml.org,2002:str", data, style="|")
    return dumper.represent_scalar("tag:yaml.org,2002:str", data)


def _setup_yaml():
    yaml.add_representer(LiteralStr, _literal_presenter)
    yaml.add_representer(str, _literal_presenter)


def _wrap_scripts(item: dict) -> dict:
    """Wrap multi-line script strings in LiteralStr so PyYAML uses block scalar."""
    for key in ("Script", "EquipScript", "UnEquipScript"):
        val = item.get(key)
        if val and "\n" in val:
            item[key] = LiteralStr(val.rstrip() + "\n")
    return item


def load_or_create_db(path: str) -> dict:
    if not os.path.exists(path):
        os.makedirs(os.path.dirname(path), exist_ok=True)
        return {"Header": {"Type": "ITEM_DB", "Version": 3}, "Body": []}
    with open(path, "r", encoding="utf-8") as f:
        data = yaml.safe_load(f)
    if not data:
        return {"Header": {"Type": "ITEM_DB", "Version": 3}, "Body": []}
    if data.get("Body") is None:
        data["Body"] = []
    return data


def save_db(path: str, db: dict) -> None:
    with open(path, "w", encoding="utf-8") as f:
        yaml.dump(db, f, allow_unicode=True, default_flow_style=False,
                  sort_keys=False, indent=2)


def detect_media_type(image_path: str) -> str:
    ext = os.path.splitext(image_path)[1].lower()
    return {
        ".jpg": "image/jpeg",
        ".jpeg": "image/jpeg",
        ".png": "image/png",
        ".gif": "image/gif",
        ".webp": "image/webp",
    }.get(ext, "image/jpeg")


def extract_item_from_image(image_path: str) -> dict:
    client = anthropic.Anthropic()
    with open(image_path, "rb") as f:
        image_data = base64.standard_b64encode(f.read()).decode("utf-8")

    media_type = detect_media_type(image_path)

    message = client.messages.create(
        model=MODEL,
        max_tokens=4096,
        messages=[{
            "role": "user",
            "content": [
                {
                    "type": "image",
                    "source": {
                        "type": "base64",
                        "media_type": media_type,
                        "data": image_data,
                    },
                },
                {"type": "text", "text": EXTRACTION_PROMPT},
            ],
        }],
    )

    raw = message.content[0].text.strip()
    # Strip markdown code fences if Claude wrapped the response
    if raw.startswith("```"):
        raw = raw.split("```")[1]
        if raw.startswith("json"):
            raw = raw[4:]
    return json.loads(raw)


def override_id(item: dict, new_id: int) -> dict:
    item["Id"] = new_id
    return item


def check_duplicate(db: dict, item_id: int) -> bool:
    return any(entry.get("Id") == item_id for entry in (db.get("Body") or []))


def main():
    args = sys.argv[1:]
    if not args or args[0] in ("-h", "--help"):
        print(__doc__)
        sys.exit(0)

    image_path = args[0]
    if not os.path.isfile(image_path):
        print(f"Error: file not found: {image_path}", file=sys.stderr)
        sys.exit(1)

    # Optional --id override
    custom_id = None
    if "--id" in args:
        idx = args.index("--id")
        try:
            custom_id = int(args[idx + 1])
        except (IndexError, ValueError):
            print("Error: --id requires an integer argument", file=sys.stderr)
            sys.exit(1)

    print(f"Extracting item data from: {image_path}")
    item_data = extract_item_from_image(image_path)

    if custom_id is not None:
        item_data = override_id(item_data, custom_id)

    name = item_data.get("Name", "Unknown")
    item_id = item_data.get("Id", "?")
    print(f"  Name : {name}")
    print(f"  ID   : {item_id}")
    print(f"  Type : {item_data.get('Type', '?')}")

    db = load_or_create_db(CUSTOM_DB_PATH)

    if check_duplicate(db, item_id):
        print(f"Warning: item ID {item_id} already exists in {CUSTOM_DB_PATH}.")
        answer = input("Overwrite? [y/N] ").strip().lower()
        if answer != "y":
            print("Aborted.")
            sys.exit(0)
        db["Body"] = [e for e in db["Body"] if e.get("Id") != item_id]

    item_data = _wrap_scripts(item_data)
    db["Body"].append(item_data)

    _setup_yaml()
    save_db(CUSTOM_DB_PATH, db)
    print(f"Written to: {CUSTOM_DB_PATH}")


if __name__ == "__main__":
    main()
