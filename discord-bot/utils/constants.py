import discord

# ── Item type display ─────────────────────────────────────────────────────────
ITEM_TYPE_LABEL = {
    'Weapon':  '⚔️ อาวุธ',
    'Armor':   '🛡️ ชุดเกราะ',
    'Card':    '🃏 การ์ด',
}

ITEM_TYPE_COLOR = {
    'Weapon': discord.Color.from_rgb(231, 76, 60),   # red
    'Armor':  discord.Color.from_rgb(52, 152, 219),  # blue
    'Card':   discord.Color.from_rgb(241, 196, 15),  # gold
}

# Rarity brackets (base rate per 10000)
def rarity_label(base_rate: int) -> str:
    if base_rate <= 1:
        return '✨ Legendary'
    if base_rate <= 10:
        return '💎 Very Rare'
    if base_rate <= 100:
        return '⭐ Rare'
    if base_rate <= 500:
        return '🔹 Uncommon'
    return '⬜ Common'


# ── Job class names ───────────────────────────────────────────────────────────
JOB_NAMES: dict[int, str] = {
    0: 'Novice', 1: 'Swordman', 2: 'Mage', 3: 'Archer',
    4: 'Acolyte', 5: 'Merchant', 6: 'Thief',
    7: 'Knight', 8: 'Priest', 9: 'Wizard',
    10: 'Blacksmith', 11: 'Hunter', 12: 'Assassin',
    13: 'Knight (Peco)', 14: 'Crusader', 15: 'Monk',
    16: 'Sage', 17: 'Rogue', 18: 'Alchemist',
    19: 'Bard', 20: 'Dancer', 21: 'Crusader (Peco)',
    22: 'Wedding', 23: 'Super Novice', 24: 'Gunslinger',
    25: 'Ninja',
    # Transcendent
    4001: 'High Novice', 4002: 'High Swordman', 4003: 'High Mage',
    4004: 'High Archer', 4005: 'High Acolyte', 4006: 'High Merchant',
    4007: 'High Thief', 4008: 'Lord Knight', 4009: 'High Priest',
    4010: 'High Wizard', 4011: 'Whitesmith', 4012: 'Sniper',
    4013: 'Assassin Cross', 4014: 'Lord Knight (Peco)', 4015: 'Paladin',
    4016: 'Champion', 4017: 'Professor', 4018: 'Stalker',
    4019: 'Creator', 4020: 'Clown', 4021: 'Gypsy',
    4022: 'Paladin (Peco)',
    # 3rd jobs
    4054: 'Rune Knight', 4055: 'Warlock', 4056: 'Ranger',
    4057: 'Arch Bishop', 4058: 'Mechanic', 4059: 'Guillotine Cross',
    4060: 'Rune Knight (Dragon)', 4061: 'Royal Guard', 4062: 'Sorcerer',
    4063: 'Minstrel', 4064: 'Wanderer', 4065: 'Sura',
    4066: 'Genetic', 4067: 'Shadow Chaser', 4068: 'Royal Guard (Peco)',
    4069: 'Ranger (Wolf)', 4070: 'Mechanic (Mado)',
    # 4th jobs
    4211: 'Dragon Knight', 4212: 'Meister', 4213: 'Shadow Cross',
    4214: 'Arch Mage', 4215: 'Cardinal', 4216: 'Wind Hawk',
    4217: 'Soul Reaper', 4218: 'Night Watch', 4219: 'Elemental Master',
    4220: 'Troubadour', 4221: 'Trouvere', 4222: 'Biolo',
    4223: 'Abyss Chaser',
}


def job_name(class_id: int) -> str:
    return JOB_NAMES.get(class_id, f'Job#{class_id}')


def format_zeny(amount: int) -> str:
    return f'{amount:,} z'
