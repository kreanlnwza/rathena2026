import discord
from discord import app_commands
from discord.ext import commands
from utils.db import get_pool
from utils.constants import job_name, format_zeny
from utils.checks import player_check


class CharacterCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot

    @app_commands.command(name='charinfo', description='ดูข้อมูลตัวละคร')
    @app_commands.describe(name='ชื่อตัวละคร')
    @app_commands.default_permissions(send_messages=True)
    @player_check()
    async def charinfo(self, interaction: discord.Interaction, name: str):
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    '''SELECT c.char_id, c.name, c.class, c.base_level, c.job_level,
                              c.zeny, c.str, c.agi, c.vit, c.`int`, c.dex, c.luk,
                              c.hp, c.max_hp, c.sp, c.max_sp, c.online,
                              c.last_map, c.fame,
                              g.name AS guild_name
                       FROM `char` c
                       LEFT JOIN guild g ON c.guild_id = g.guild_id
                       WHERE c.name = %s''',
                    (name.strip(),),
                )
                row = await cur.fetchone()

        if not row:
            return await interaction.response.send_message(f'❌ ไม่พบตัวละคร **{name}**', ephemeral=True)

        (char_id, cname, cls, blv, jlv, zeny,
         str_, agi, vit, int_, dex, luk,
         hp, max_hp, sp, max_sp, online,
         last_map, fame, guild_name) = row

        status = '🟢 Online' if online else '⚫ Offline'
        embed = discord.Embed(
            title=f'🧑‍⚔️ {cname}',
            description=f'{status}  •  {job_name(cls)}',
            color=discord.Color.green() if online else discord.Color.greyple(),
        )
        embed.add_field(name='Base Lv', value=str(blv),  inline=True)
        embed.add_field(name='Job Lv',  value=str(jlv),  inline=True)
        embed.add_field(name='Zeny',    value=format_zeny(zeny), inline=True)
        embed.add_field(name='HP', value=f'{hp:,}/{max_hp:,}', inline=True)
        embed.add_field(name='SP', value=f'{sp:,}/{max_sp:,}', inline=True)
        embed.add_field(name='Map',     value=last_map or '-', inline=True)
        embed.add_field(
            name='Stats',
            value=f'STR {str_}  AGI {agi}  VIT {vit}\nINT {int_}  DEX {dex}  LUK {luk}',
            inline=False,
        )
        if guild_name:
            embed.add_field(name='Guild', value=guild_name, inline=True)
        if fame:
            embed.add_field(name='Fame', value=f'{fame:,}', inline=True)

        embed.set_footer(text=f'Char ID: {char_id}')
        await interaction.response.send_message(embed=embed)

    @app_commands.command(name='online', description='ดูผู้เล่นที่กำลัง Online อยู่')
    @app_commands.default_permissions(send_messages=True)
    @player_check()
    async def online(self, interaction: discord.Interaction):
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    'SELECT name, class, base_level, last_map FROM `char` WHERE online = 1 ORDER BY base_level DESC LIMIT 25'
                )
                rows = await cur.fetchall()
                await cur.execute('SELECT COUNT(*) FROM `char` WHERE online = 1')
                total = (await cur.fetchone())[0]

        if not rows:
            return await interaction.response.send_message('📭 ไม่มีผู้เล่น Online ขณะนี้', ephemeral=True)

        lines = [f'`{r[0]:<20}` {job_name(r[1]):<22} Lv {r[2]:>3}  📍 {r[3]}' for r in rows]
        shown = len(rows)
        desc = '\n'.join(lines)
        if total > shown:
            desc += f'\n_...และอีก {total - shown} คน_'

        embed = discord.Embed(
            title=f'🌐 ผู้เล่น Online: {total} คน',
            description=desc,
            color=discord.Color.green(),
        )
        await interaction.response.send_message(embed=embed)


async def setup(bot: commands.Bot):
    await bot.add_cog(CharacterCog(bot))
