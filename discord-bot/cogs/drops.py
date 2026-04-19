import discord
from discord import app_commands
from discord.ext import commands, tasks
from utils.db import get_pool, get_log_pool
from utils import gamedata
from utils.constants import ITEM_TYPE_LABEL, ITEM_TYPE_COLOR, rarity_label
import config

# item types we want to track
TRACKED_TYPES = {'Weapon', 'Armor', 'Card'}

# picklog type 'L' = loot (player picks up item dropped by monster)
LOOT_TYPE = 'L'


class DropsCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot
        self._last_id: int = 0
        self._drop_feed.start()

    def cog_unload(self):
        self._drop_feed.cancel()

    # ── Background feed ───────────────────────────────────────────────────────

    @tasks.loop(seconds=config.DROP_POLL_INTERVAL)
    async def _drop_feed(self):
        if not config.DROP_CHANNEL_ID:
            return
        channel = self.bot.get_channel(config.DROP_CHANNEL_ID)
        if not channel:
            return

        log_pool = get_log_pool()
        if not log_pool:
            return

        async with log_pool.acquire() as log_conn:
            async with log_conn.cursor() as cur:
                # Initialise last_id on first run
                if self._last_id == 0:
                    await cur.execute('SELECT MAX(id) FROM picklog')
                    row = await cur.fetchone()
                    self._last_id = row[0] or 0
                    return

                await cur.execute(
                    'SELECT id, time, char_id, nameid, amount, refine, map '
                    'FROM picklog WHERE id > %s AND type = %s ORDER BY id',
                    (self._last_id, LOOT_TYPE),
                )
                rows = await cur.fetchall()

        if not rows:
            return

        self._last_id = rows[-1][0]

        # Resolve char names in bulk
        char_ids = list({r[2] for r in rows})
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                fmt = ','.join(['%s'] * len(char_ids))
                await cur.execute(f'SELECT char_id, name FROM `char` WHERE char_id IN ({fmt})', char_ids)
                char_map = {r[0]: r[1] for r in await cur.fetchall()}

        for drop_id, ts, char_id, nameid, amount, refine, map_name in rows:
            item = gamedata.get_item(nameid)
            if not item or item['type'] not in TRACKED_TYPES:
                continue

            itype     = item['type']
            iname     = item['name']
            char_name = char_map.get(char_id, f'char#{char_id}')
            color     = ITEM_TYPE_COLOR.get(itype, discord.Color.default())

            # Best drop rate across all mobs
            drops_info = gamedata.get_item_drops(nameid)
            rate_str = '-'
            rarity   = ''
            if drops_info:
                best = max(drops_info, key=lambda d: d['base_rate'])
                rate_str = gamedata.format_rate(best['base_rate'], itype)
                rarity   = rarity_label(best['base_rate'])

            title = iname
            if refine > 0 and itype in ('Weapon', 'Armor'):
                title = f'+{refine} {iname}'

            embed = discord.Embed(
                title=f'{ITEM_TYPE_LABEL.get(itype, itype)}  {title}',
                color=color,
            )
            embed.add_field(name='👤 ผู้เล่น',  value=char_name,   inline=True)
            embed.add_field(name='📍 Map',       value=map_name,    inline=True)
            embed.add_field(name='📦 จำนวน',    value=str(amount),  inline=True)
            embed.add_field(name='📊 Drop Rate (สูงสุด)', value=rate_str, inline=True)
            if rarity:
                embed.add_field(name='✨ Rarity', value=rarity, inline=True)
            embed.set_footer(text=str(ts))

            await channel.send(embed=embed)

    @_drop_feed.before_loop
    async def _before_feed(self):
        await self.bot.wait_until_ready()

    # ── Slash commands ────────────────────────────────────────────────────────

    @app_commands.command(name='recentdrops', description='ดูการดรอปไอเทมล่าสุด (เกราะ/อาวุธ/การ์ด)')
    @app_commands.describe(limit='จำนวนรายการ (สูงสุด 20, ค่าเริ่มต้น 10)')
    async def recentdrops(self, interaction: discord.Interaction, limit: int = 10):
        limit = max(1, min(limit, 20))
        await interaction.response.defer()

        log_pool = get_log_pool()
        if not log_pool:
            return await interaction.followup.send('❌ Log DB ไม่พร้อมใช้งาน', ephemeral=True)

        # Fetch a larger batch then filter by type in Python (since type info is in YAML)
        async with log_pool.acquire() as log_conn:
            async with log_conn.cursor() as cur:
                await cur.execute(
                    'SELECT id, time, char_id, nameid, amount, refine, map '
                    'FROM picklog WHERE type = %s ORDER BY id DESC LIMIT 200',
                    (LOOT_TYPE,),
                )
                raw_rows = await cur.fetchall()

        # Filter by tracked item types
        filtered = []
        for row in raw_rows:
            item = gamedata.get_item(row[3])
            if item and item['type'] in TRACKED_TYPES:
                filtered.append((row, item))
            if len(filtered) >= limit:
                break

        if not filtered:
            return await interaction.followup.send('📭 ยังไม่มีการดรอปไอเทมที่ต้องการ')

        # Resolve char names
        char_ids = list({r[0][2] for r in filtered})
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                fmt = ','.join(['%s'] * len(char_ids))
                await cur.execute(f'SELECT char_id, name FROM `char` WHERE char_id IN ({fmt})', char_ids)
                char_map = {r[0]: r[1] for r in await cur.fetchall()}

        lines = []
        for (drop_id, ts, char_id, nameid, amount, refine, map_name), item in filtered:
            itype     = item['type']
            iname     = item['name']
            char_name = char_map.get(char_id, f'char#{char_id}')
            label     = ITEM_TYPE_LABEL.get(itype, itype)

            name_str = f'+{refine} {iname}' if refine > 0 and itype in ('Weapon', 'Armor') else iname

            drops_info = gamedata.get_item_drops(nameid)
            if drops_info:
                best = max(drops_info, key=lambda d: d['base_rate'])
                rate_str = gamedata.format_rate(best['base_rate'], itype)
            else:
                rate_str = '-'

            lines.append(
                f'{label} **{name_str}** × {amount}\n'
                f'  👤 {char_name}  📍 {map_name}  📊 {rate_str}\n'
                f'  🕐 `{ts}`'
            )

        embed = discord.Embed(
            title=f'🎲 Recent Drops ({len(filtered)} รายการ)',
            description='\n\n'.join(lines),
            color=discord.Color.purple(),
        )
        await interaction.followup.send(embed=embed)

    @app_commands.command(name='droprate', description='ดู Drop Rate ของไอเทม (ค้นจากชื่อ)')
    @app_commands.describe(item_name='ชื่อหรือบางส่วนของชื่อไอเทม')
    async def droprate(self, interaction: discord.Interaction, item_name: str):
        results = gamedata.search_items(item_name, limit=5)
        if not results:
            return await interaction.response.send_message(f'❌ ไม่พบไอเทมที่มีคำว่า **{item_name}**', ephemeral=True)

        # Use first match
        item = results[0]
        item_id   = item['id']
        iname     = item['name']
        itype     = item['type']
        drops_info = gamedata.get_item_drops(item_id)

        embed = discord.Embed(
            title=f'📊 Drop Rate: {iname}',
            description=f'ประเภท: {ITEM_TYPE_LABEL.get(itype, itype)}  •  ID: {item_id}',
            color=ITEM_TYPE_COLOR.get(itype, discord.Color.blurple()),
        )

        if not drops_info:
            embed.description += '\n\n_ไม่พบข้อมูล Drop จาก mob ใดๆ_'
        else:
            # Sort by effective rate descending
            sorted_drops = sorted(drops_info, key=lambda d: d['base_rate'], reverse=True)[:20]
            mult = gamedata.get_multipliers()
            card_mult  = mult['card']
            equip_mult = mult['equip']
            com_mult   = mult['common']

            lines = []
            for d in sorted_drops:
                rate_str = gamedata.format_rate(d['base_rate'], itype)
                rar = rarity_label(d['base_rate'])
                lines.append(f'{rar} **{d["mob"]}** — {rate_str}')

            embed.add_field(name=f'Mob ที่ดรอป ({len(sorted_drops)} รายการ)', value='\n'.join(lines), inline=False)

            # Show multiplier info
            if itype == 'Card' and card_mult != 1.0:
                embed.set_footer(text=f'Server Card Rate: ×{card_mult}')
            elif itype in ('Weapon', 'Armor') and equip_mult != 1.0:
                embed.set_footer(text=f'Server Equip Rate: ×{equip_mult}')

        await interaction.response.send_message(embed=embed)


async def setup(bot: commands.Bot):
    await bot.add_cog(DropsCog(bot))
