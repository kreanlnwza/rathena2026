#!/usr/bin/env python3
"""
generate_randomopt_group.py
สร้างไฟล์ db/import/item_randomopt_group.yml
  - 5 กลุ่ม (Id 1-5)
  - Id N มี N slots (Id1=1slot, Id2=2slots, ..., Id5=5slots)
  - แต่ละ slot สุ่มจาก option ทั้งหมดใน item_randomopt_db.yml
  - MinValue = 1 ทุกตัว
  - MaxValue: HP/SP=100, % based=10, integer=10, static=1
  - Chance = 5000 ทุกตัว
"""

import os

# (option_name, type)
# ประเภท: HP_SP, PERCENT, INTEGER, STATIC
OPTIONS = [
    ("VAR_MAXHPAMOUNT", "HP_SP"),
    ("VAR_MAXSPAMOUNT", "HP_SP"),
    ("VAR_STRAMOUNT", "INTEGER"),
    ("VAR_AGIAMOUNT", "INTEGER"),
    ("VAR_VITAMOUNT", "INTEGER"),
    ("VAR_INTAMOUNT", "INTEGER"),
    ("VAR_DEXAMOUNT", "INTEGER"),
    ("VAR_LUKAMOUNT", "INTEGER"),
    ("VAR_MAXHPPERCENT", "PERCENT"),
    ("VAR_MAXSPPERCENT", "PERCENT"),
    ("VAR_HPACCELERATION", "PERCENT"),
    ("VAR_SPACCELERATION", "PERCENT"),
    ("VAR_ATKPERCENT", "PERCENT"),
    ("VAR_MAGICATKPERCENT", "PERCENT"),
    ("VAR_PLUSASPD", "INTEGER"),
    ("VAR_PLUSASPDPERCENT", "PERCENT"),
    ("VAR_ATTPOWER", "INTEGER"),
    ("VAR_HITSUCCESSVALUE", "INTEGER"),
    ("VAR_ATTMPOWER", "INTEGER"),
    ("VAR_ITEMDEFPOWER", "INTEGER"),
    ("VAR_MDEFPOWER", "INTEGER"),
    ("VAR_AVOIDSUCCESSVALUE", "INTEGER"),
    ("VAR_PLUSAVOIDSUCCESSVALUE", "INTEGER"),
    ("VAR_CRITICALSUCCESSVALUE", "INTEGER"),
    ("ATTR_TOLERACE_NOTHING", "INTEGER"),
    ("ATTR_TOLERACE_WATER", "INTEGER"),
    ("ATTR_TOLERACE_GROUND", "INTEGER"),
    ("ATTR_TOLERACE_FIRE", "INTEGER"),
    ("ATTR_TOLERACE_WIND", "INTEGER"),
    ("ATTR_TOLERACE_POISON", "INTEGER"),
    ("ATTR_TOLERACE_SAINT", "INTEGER"),
    ("ATTR_TOLERACE_DARKNESS", "INTEGER"),
    ("ATTR_TOLERACE_TELEKINESIS", "INTEGER"),
    ("ATTR_TOLERACE_UNDEAD", "INTEGER"),
    ("ATTR_TOLERACE_ALL", "INTEGER"),
    ("ATTR_TOLERACE_ALLBUTNOTHING", "INTEGER"),
    ("DAMAGE_PROPERTY_NOTHING_USER", "INTEGER"),
    ("DAMAGE_PROPERTY_NOTHING_TARGET", "INTEGER"),
    ("DAMAGE_PROPERTY_WATER_USER", "INTEGER"),
    ("DAMAGE_PROPERTY_WATER_TARGET", "INTEGER"),
    ("DAMAGE_PROPERTY_GROUND_USER", "INTEGER"),
    ("DAMAGE_PROPERTY_GROUND_TARGET", "INTEGER"),
    ("DAMAGE_PROPERTY_FIRE_USER", "INTEGER"),
    ("DAMAGE_PROPERTY_FIRE_TARGET", "INTEGER"),
    ("DAMAGE_PROPERTY_WIND_USER", "INTEGER"),
    ("DAMAGE_PROPERTY_WIND_TARGET", "INTEGER"),
    ("DAMAGE_PROPERTY_POISON_USER", "INTEGER"),
    ("DAMAGE_PROPERTY_POISON_TARGET", "INTEGER"),
    ("DAMAGE_PROPERTY_SAINT_USER", "INTEGER"),
    ("DAMAGE_PROPERTY_SAINT_TARGET", "INTEGER"),
    ("DAMAGE_PROPERTY_DARKNESS_USER", "INTEGER"),
    ("DAMAGE_PROPERTY_DARKNESS_TARGET", "INTEGER"),
    ("DAMAGE_PROPERTY_TELEKINESIS_USER", "INTEGER"),
    ("DAMAGE_PROPERTY_TELEKINESIS_TARGET", "INTEGER"),
    ("DAMAGE_PROPERTY_UNDEAD_USER", "INTEGER"),
    ("DAMAGE_PROPERTY_UNDEAD_TARGET", "INTEGER"),
    ("MDAMAGE_PROPERTY_NOTHING_USER", "INTEGER"),
    ("MDAMAGE_PROPERTY_NOTHING_TARGET", "INTEGER"),
    ("MDAMAGE_PROPERTY_WATER_USER", "INTEGER"),
    ("MDAMAGE_PROPERTY_WATER_TARGET", "INTEGER"),
    ("MDAMAGE_PROPERTY_GROUND_USER", "INTEGER"),
    ("MDAMAGE_PROPERTY_GROUND_TARGET", "INTEGER"),
    ("MDAMAGE_PROPERTY_FIRE_USER", "INTEGER"),
    ("MDAMAGE_PROPERTY_FIRE_TARGET", "INTEGER"),
    ("MDAMAGE_PROPERTY_WIND_USER", "INTEGER"),
    ("MDAMAGE_PROPERTY_WIND_TARGET", "INTEGER"),
    ("MDAMAGE_PROPERTY_POISON_USER", "INTEGER"),
    ("MDAMAGE_PROPERTY_POISON_TARGET", "INTEGER"),
    ("MDAMAGE_PROPERTY_SAINT_USER", "INTEGER"),
    ("MDAMAGE_PROPERTY_SAINT_TARGET", "INTEGER"),
    ("MDAMAGE_PROPERTY_DARKNESS_USER", "INTEGER"),
    ("MDAMAGE_PROPERTY_DARKNESS_TARGET", "INTEGER"),
    ("MDAMAGE_PROPERTY_TELEKINESIS_USER", "INTEGER"),
    ("MDAMAGE_PROPERTY_TELEKINESIS_TARGET", "INTEGER"),
    ("MDAMAGE_PROPERTY_UNDEAD_USER", "INTEGER"),
    ("MDAMAGE_PROPERTY_UNDEAD_TARGET", "INTEGER"),
    # STATIC: เปลี่ยน element เกราะ (bonus bDefEle) ไม่ใช้ค่าจาก ROA_VALUE
    ("BODY_ATTR_NOTHING", "STATIC"),
    ("BODY_ATTR_WATER", "STATIC"),
    ("BODY_ATTR_GROUND", "STATIC"),
    ("BODY_ATTR_FIRE", "STATIC"),
    ("BODY_ATTR_WIND", "STATIC"),
    ("BODY_ATTR_POISON", "STATIC"),
    ("BODY_ATTR_SAINT", "STATIC"),
    ("BODY_ATTR_DARKNESS", "STATIC"),
    ("BODY_ATTR_TELEKINESIS", "STATIC"),
    ("BODY_ATTR_UNDEAD", "STATIC"),
    ("RACE_TOLERACE_NOTHING", "INTEGER"),
    ("RACE_TOLERACE_UNDEAD", "INTEGER"),
    ("RACE_TOLERACE_ANIMAL", "INTEGER"),
    ("RACE_TOLERACE_PLANT", "INTEGER"),
    ("RACE_TOLERACE_INSECT", "INTEGER"),
    ("RACE_TOLERACE_FISHS", "INTEGER"),
    ("RACE_TOLERACE_DEVIL", "INTEGER"),
    ("RACE_TOLERACE_HUMAN", "INTEGER"),
    ("RACE_TOLERACE_ANGEL", "INTEGER"),
    ("RACE_TOLERACE_DRAGON", "INTEGER"),
    ("RACE_DAMAGE_NOTHING", "INTEGER"),
    ("RACE_DAMAGE_UNDEAD", "INTEGER"),
    ("RACE_DAMAGE_ANIMAL", "INTEGER"),
    ("RACE_DAMAGE_PLANT", "INTEGER"),
    ("RACE_DAMAGE_INSECT", "INTEGER"),
    ("RACE_DAMAGE_FISHS", "INTEGER"),
    ("RACE_DAMAGE_DEVIL", "INTEGER"),
    ("RACE_DAMAGE_HUMAN", "INTEGER"),
    ("RACE_DAMAGE_ANGEL", "INTEGER"),
    ("RACE_DAMAGE_DRAGON", "INTEGER"),
    ("RACE_MDAMAGE_NOTHING", "INTEGER"),
    ("RACE_MDAMAGE_UNDEAD", "INTEGER"),
    ("RACE_MDAMAGE_ANIMAL", "INTEGER"),
    ("RACE_MDAMAGE_PLANT", "INTEGER"),
    ("RACE_MDAMAGE_INSECT", "INTEGER"),
    ("RACE_MDAMAGE_FISHS", "INTEGER"),
    ("RACE_MDAMAGE_DEVIL", "INTEGER"),
    ("RACE_MDAMAGE_HUMAN", "INTEGER"),
    ("RACE_MDAMAGE_ANGEL", "INTEGER"),
    ("RACE_MDAMAGE_DRAGON", "INTEGER"),
    ("RACE_CRI_PERCENT_NOTHING", "PERCENT"),
    ("RACE_CRI_PERCENT_UNDEAD", "PERCENT"),
    ("RACE_CRI_PERCENT_ANIMAL", "PERCENT"),
    ("RACE_CRI_PERCENT_PLANT", "PERCENT"),
    ("RACE_CRI_PERCENT_INSECT", "PERCENT"),
    ("RACE_CRI_PERCENT_FISHS", "PERCENT"),
    ("RACE_CRI_PERCENT_DEVIL", "PERCENT"),
    ("RACE_CRI_PERCENT_HUMAN", "PERCENT"),
    ("RACE_CRI_PERCENT_ANGEL", "PERCENT"),
    ("RACE_CRI_PERCENT_DRAGON", "PERCENT"),
    ("RACE_IGNORE_DEF_PERCENT_NOTHING", "PERCENT"),
    ("RACE_IGNORE_DEF_PERCENT_UNDEAD", "PERCENT"),
    ("RACE_IGNORE_DEF_PERCENT_ANIMAL", "PERCENT"),
    ("RACE_IGNORE_DEF_PERCENT_PLANT", "PERCENT"),
    ("RACE_IGNORE_DEF_PERCENT_INSECT", "PERCENT"),
    ("RACE_IGNORE_DEF_PERCENT_FISHS", "PERCENT"),
    ("RACE_IGNORE_DEF_PERCENT_DEVIL", "PERCENT"),
    ("RACE_IGNORE_DEF_PERCENT_HUMAN", "PERCENT"),
    ("RACE_IGNORE_DEF_PERCENT_ANGEL", "PERCENT"),
    ("RACE_IGNORE_DEF_PERCENT_DRAGON", "PERCENT"),
    ("RACE_IGNORE_MDEF_PERCENT_NOTHING", "PERCENT"),
    ("RACE_IGNORE_MDEF_PERCENT_UNDEAD", "PERCENT"),
    ("RACE_IGNORE_MDEF_PERCENT_ANIMAL", "PERCENT"),
    ("RACE_IGNORE_MDEF_PERCENT_PLANT", "PERCENT"),
    ("RACE_IGNORE_MDEF_PERCENT_INSECT", "PERCENT"),
    ("RACE_IGNORE_MDEF_PERCENT_FISHS", "PERCENT"),
    ("RACE_IGNORE_MDEF_PERCENT_DEVIL", "PERCENT"),
    ("RACE_IGNORE_MDEF_PERCENT_HUMAN", "PERCENT"),
    ("RACE_IGNORE_MDEF_PERCENT_ANGEL", "PERCENT"),
    ("RACE_IGNORE_MDEF_PERCENT_DRAGON", "PERCENT"),
    ("CLASS_DAMAGE_NORMAL_TARGET", "INTEGER"),
    ("CLASS_DAMAGE_BOSS_TARGET", "INTEGER"),
    ("CLASS_DAMAGE_NORMAL_USER", "INTEGER"),
    ("CLASS_DAMAGE_BOSS_USER", "INTEGER"),
    ("CLASS_MDAMAGE_NORMAL", "INTEGER"),
    ("CLASS_MDAMAGE_BOSS", "INTEGER"),
    ("CLASS_IGNORE_DEF_PERCENT_NORMAL", "PERCENT"),
    ("CLASS_IGNORE_DEF_PERCENT_BOSS", "PERCENT"),
    ("CLASS_IGNORE_MDEF_PERCENT_NORMAL", "PERCENT"),
    ("CLASS_IGNORE_MDEF_PERCENT_BOSS", "PERCENT"),
    ("DAMAGE_SIZE_SMALL_TARGET", "INTEGER"),
    ("DAMAGE_SIZE_MIDIUM_TARGET", "INTEGER"),
    ("DAMAGE_SIZE_LARGE_TARGET", "INTEGER"),
    ("DAMAGE_SIZE_SMALL_USER", "INTEGER"),
    ("DAMAGE_SIZE_MIDIUM_USER", "INTEGER"),
    ("DAMAGE_SIZE_LARGE_USER", "INTEGER"),
    # STATIC: bonus bNoSizeFix,1
    ("DAMAGE_SIZE_PERFECT", "STATIC"),
    ("DAMAGE_CRI_TARGET", "PERCENT"),
    ("DAMAGE_CRI_USER", "PERCENT"),
    ("RANGE_ATTACK_DAMAGE_TARGET", "PERCENT"),
    ("RANGE_ATTACK_DAMAGE_USER", "PERCENT"),
    ("HEAL_VALUE", "INTEGER"),
    ("HEAL_MODIFY_PERCENT", "PERCENT"),
    ("DEC_SPELL_CAST_TIME", "PERCENT"),
    ("DEC_SPELL_DELAY_TIME", "PERCENT"),
    ("DEC_SP_CONSUMPTION", "PERCENT"),
    # STATIC: เปลี่ยน element อาวุธ (bonus bAtkEle) ไม่ใช้ค่าจาก ROA_VALUE
    ("WEAPON_ATTR_NOTHING", "STATIC"),
    ("WEAPON_ATTR_WATER", "STATIC"),
    ("WEAPON_ATTR_GROUND", "STATIC"),
    ("WEAPON_ATTR_FIRE", "STATIC"),
    ("WEAPON_ATTR_WIND", "STATIC"),
    ("WEAPON_ATTR_POISON", "STATIC"),
    ("WEAPON_ATTR_SAINT", "STATIC"),
    ("WEAPON_ATTR_DARKNESS", "STATIC"),
    ("WEAPON_ATTR_TELEKINESIS", "STATIC"),
    ("WEAPON_ATTR_UNDEAD", "STATIC"),
    # STATIC: ทำให้ไม่แตก
    ("WEAPON_INDESTRUCTIBLE", "STATIC"),
    ("BODY_INDESTRUCTIBLE", "STATIC"),
    ("MDAMAGE_SIZE_SMALL_TARGET", "INTEGER"),
    ("MDAMAGE_SIZE_MIDIUM_TARGET", "INTEGER"),
    ("MDAMAGE_SIZE_LARGE_TARGET", "INTEGER"),
    ("MDAMAGE_SIZE_SMALL_USER", "INTEGER"),
    ("MDAMAGE_SIZE_MIDIUM_USER", "INTEGER"),
    ("MDAMAGE_SIZE_LARGE_USER", "INTEGER"),
    ("RACE_WEAPON_TOLERACE_NOTHING", "INTEGER"),
    ("RACE_WEAPON_TOLERACE_UNDEAD", "INTEGER"),
    ("RACE_WEAPON_TOLERACE_ANIMAL", "INTEGER"),
    ("RACE_WEAPON_TOLERACE_PLANT", "INTEGER"),
    ("RACE_WEAPON_TOLERACE_INSECT", "INTEGER"),
    ("RACE_WEAPON_TOLERACE_FISHS", "INTEGER"),
    ("RACE_WEAPON_TOLERACE_DEVIL", "INTEGER"),
    ("RACE_WEAPON_TOLERACE_HUMAN", "INTEGER"),
    ("RACE_WEAPON_TOLERACE_ANGEL", "INTEGER"),
    ("RACE_WEAPON_TOLERACE_DRAGON", "INTEGER"),
    ("RACE_TOLERACE_PLAYER_HUMAN", "INTEGER"),
    ("RACE_TOLERACE_PLAYER_DORAM", "INTEGER"),
    ("RACE_DAMAGE_PLAYER_HUMAN", "INTEGER"),
    ("RACE_DAMAGE_PLAYER_DORAM", "INTEGER"),
    ("RACE_MDAMAGE_PLAYER_HUMAN", "INTEGER"),
    ("RACE_MDAMAGE_PLAYER_DORAM", "INTEGER"),
    ("RACE_CRI_PERCENT_PLAYER_HUMAN", "INTEGER"),
    ("RACE_CRI_PERCENT_PLAYER_DORAM", "INTEGER"),
    ("RACE_IGNORE_DEF_PERCENT_PLAYER_HUMAN", "PERCENT"),
    ("RACE_IGNORE_DEF_PERCENT_PLAYER_DORAM", "PERCENT"),
    ("RACE_IGNORE_MDEF_PERCENT_PLAYER_HUMAN", "PERCENT"),
    ("RACE_IGNORE_MDEF_PERCENT_PLAYER_DORAM", "PERCENT"),
    ("REFLECT_DAMAGE_PERCENT", "PERCENT"),
    ("MELEE_ATTACK_DAMAGE_TARGET", "PERCENT"),
    ("MELEE_ATTACK_DAMAGE_USER", "PERCENT"),
    ("ADDSKILLMDAMAGE_NOTHING", "INTEGER"),
    ("ADDSKILLMDAMAGE_WATER", "INTEGER"),
    ("ADDSKILLMDAMAGE_GROUND", "INTEGER"),
    ("ADDSKILLMDAMAGE_FIRE", "INTEGER"),
    ("ADDSKILLMDAMAGE_WIND", "INTEGER"),
    ("ADDSKILLMDAMAGE_POISON", "INTEGER"),
    ("ADDSKILLMDAMAGE_SAINT", "INTEGER"),
    ("ADDSKILLMDAMAGE_DARKNESS", "INTEGER"),
    ("ADDSKILLMDAMAGE_TELEKINESIS", "INTEGER"),
    ("ADDSKILLMDAMAGE_UNDEAD", "INTEGER"),
    ("ADDSKILLMDAMAGE_ALL", "INTEGER"),
    ("ADDEXPPERCENT_KILLRACE_NOTHING", "PERCENT"),
    ("ADDEXPPERCENT_KILLRACE_UNDEAD", "PERCENT"),
    ("ADDEXPPERCENT_KILLRACE_ANIMAL", "PERCENT"),
    ("ADDEXPPERCENT_KILLRACE_PLANT", "PERCENT"),
    ("ADDEXPPERCENT_KILLRACE_INSECT", "PERCENT"),
    ("ADDEXPPERCENT_KILLRACE_FISHS", "PERCENT"),
    ("ADDEXPPERCENT_KILLRACE_DEVIL", "PERCENT"),
    ("ADDEXPPERCENT_KILLRACE_HUMAN", "PERCENT"),
    ("ADDEXPPERCENT_KILLRACE_ANGEL", "PERCENT"),
    ("ADDEXPPERCENT_KILLRACE_DRAGON", "PERCENT"),
    ("ADDEXPPERCENT_KILLRACE_ALL", "PERCENT"),
    ("VAR_POWAMOUNT", "INTEGER"),
    ("VAR_SPLAMOUNT", "INTEGER"),
    ("VAR_STAAMOUNT", "INTEGER"),
    ("VAR_WISAMOUNT", "INTEGER"),
    ("VAR_CONAMOUNT", "INTEGER"),
    ("VAR_CRTAMOUNT", "INTEGER"),
    ("VAR_PATKAMOUNT", "INTEGER"),
    ("VAR_SMATKAMOUNT", "INTEGER"),
    ("VAR_RESAMOUNT", "INTEGER"),
    ("VAR_MRESAMOUNT", "INTEGER"),
    ("VAR_HEAL_PLUS", "INTEGER"),
    ("VAR_CRITICAL_RATE", "INTEGER"),
]

MAX_VALUE = {
    "HP_SP":   100,
    "PERCENT":  10,
    "INTEGER":  10,
    "STATIC":    1,  # script ไม่อ่านค่านี้ ตั้ง 1 ให้ min <= max
}

HEADER = """\
# This file is a part of rAthena.
#   Copyright(C) 2021 rAthena Development Team
#   https://rathena.org - https://github.com/rathena
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
###########################################################################
# Item Random Option Group Database
###########################################################################
#
# Item Random Option Group Settings
#
###########################################################################
# - Id                  Item Random Option Group ID.
#   Group               Item Random Option Group constant.
#   Slots:              Slot in which an Item Random Option is guaranteed to be applied.
#     - Slot            Slot number.
#       Options:        List of possible Item Random Options for slot.
#         - Option      Item Random Option constant.
#           MinValue    Minimum value. (Default: 0)
#           MaxValue    Maximum value. (Default: 0)
#           Chance      Chance applied specifically to this option (1=0.01%, 10000=100%).
###########################################################################

Header:
  Type: RANDOM_OPTION_GROUP
  Version: 1

Body:"""


def build_slot_options(indent="          "):
    lines = []
    for name, otype in OPTIONS:
        lines.append(f"{indent}- Option: {name}")
        lines.append(f"{indent}  MinValue: 1")
        lines.append(f"{indent}  MaxValue: {MAX_VALUE[otype]}")
        lines.append(f"{indent}  Chance: 5000")
    return "\n".join(lines)


def generate(output_path: str):
    slot_block = build_slot_options()
    parts = [HEADER]

    for group_id in range(1, 6):
        parts.append(f"  - Id: {group_id}")
        parts.append(f"    Group: Group_RandomOpt{group_id}")
        parts.append(f"    Slots:")
        for slot_num in range(1, group_id + 1):
            parts.append(f"      - Slot: {slot_num}")
            parts.append(f"        Options:")
            parts.append(slot_block)

    content = "\n".join(parts) + "\n"

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(content)

    print(f"Generated: {output_path}")
    print(f"  Options per slot : {len(OPTIONS)}")
    print(f"  Total slots      : {sum(range(1, 6))}")
    print(f"  Total lines      : {content.count(chr(10)):,}")


if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_root = os.path.dirname(script_dir)
    output = os.path.join(repo_root, "db", "import", "item_randomopt_group.yml")
    generate(output)
