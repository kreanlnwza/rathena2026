import time
import discord
from discord import app_commands
from discord.ext import commands
from utils.db import get_pool, get_log_pool
from utils.checks import admin_check
import config


class AdminCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot

    # ── Ban ───────────────────────────────────────────────────────────────────

    @app_commands.command(name='ban', description='[ADMIN] แบนบัญชีถาวร')
    @app_commands.describe(username='ชื่อบัญชี', reason='เหตุผล')
    @app_commands.default_permissions(administrator=True)
    @admin_check()
    async def ban(self, interaction: discord.Interaction, username: str, reason: str = 'ไม่ระบุ'):
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute('SELECT account_id, state FROM `login` WHERE userid = %s', (username,))
                row = await cur.fetchone()
                if not row:
                    return await interaction.response.send_message(f'❌ ไม่พบบัญชี **{username}**', ephemeral=True)
                if row[1] == 5:
                    return await interaction.response.send_message(f'⚠️ บัญชี **{username}** ถูกแบนอยู่แล้ว', ephemeral=True)
                await cur.execute("UPDATE `login` SET state = 5 WHERE userid = %s", (username,))

        embed = discord.Embed(title='🔨 แบนบัญชีสำเร็จ', color=discord.Color.red())
        embed.add_field(name='Username', value=username, inline=True)
        embed.add_field(name='เหตุผล',  value=reason,   inline=True)
        embed.set_footer(text=f'โดย {interaction.user}')
        await interaction.response.send_message(embed=embed)

    @app_commands.command(name='tempban', description='[ADMIN] แบนบัญชีชั่วคราว')
    @app_commands.describe(username='ชื่อบัญชี', hours='จำนวนชั่วโมง', reason='เหตุผล')
    @app_commands.default_permissions(administrator=True)
    @admin_check()
    async def tempban(self, interaction: discord.Interaction, username: str, hours: int, reason: str = 'ไม่ระบุ'):
        if hours <= 0:
            return await interaction.response.send_message('❌ จำนวนชั่วโมงต้องมากกว่า 0', ephemeral=True)

        unban_ts = int(time.time()) + hours * 3600
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute('SELECT account_id FROM `login` WHERE userid = %s', (username,))
                if not await cur.fetchone():
                    return await interaction.response.send_message(f'❌ ไม่พบบัญชี **{username}**', ephemeral=True)
                await cur.execute(
                    'UPDATE `login` SET unban_time = %s WHERE userid = %s',
                    (unban_ts, username),
                )

        embed = discord.Embed(title='⏱️ แบนชั่วคราวสำเร็จ', color=discord.Color.orange())
        embed.add_field(name='Username',  value=username,                   inline=True)
        embed.add_field(name='ระยะเวลา', value=f'{hours} ชั่วโมง',        inline=True)
        embed.add_field(name='หมดอายุ',  value=f'<t:{unban_ts}:f>',        inline=True)
        embed.add_field(name='เหตุผล',   value=reason,                      inline=False)
        embed.set_footer(text=f'โดย {interaction.user}')
        await interaction.response.send_message(embed=embed)

    @app_commands.command(name='unban', description='[ADMIN] ปลดแบนบัญชี')
    @app_commands.describe(username='ชื่อบัญชี')
    @app_commands.default_permissions(administrator=True)
    @admin_check()
    async def unban(self, interaction: discord.Interaction, username: str):
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute('SELECT account_id, state, unban_time FROM `login` WHERE userid = %s', (username,))
                row = await cur.fetchone()
                if not row:
                    return await interaction.response.send_message(f'❌ ไม่พบบัญชี **{username}**', ephemeral=True)
                if row[1] == 0 and (not row[2] or row[2] < time.time()):
                    return await interaction.response.send_message(f'⚠️ บัญชี **{username}** ไม่ได้ถูกแบน', ephemeral=True)
                await cur.execute(
                    'UPDATE `login` SET state = 0, unban_time = 0 WHERE userid = %s',
                    (username,),
                )

        await interaction.response.send_message(f'✅ ปลดแบน **{username}** สำเร็จแล้ว')

    # ── VIP ───────────────────────────────────────────────────────────────────

    @app_commands.command(name='addvip', description='[ADMIN] เพิ่มเวลา VIP')
    @app_commands.describe(username='ชื่อบัญชี', days='จำนวนวัน')
    @app_commands.default_permissions(administrator=True)
    @admin_check()
    async def addvip(self, interaction: discord.Interaction, username: str, days: int):
        if days <= 0:
            return await interaction.response.send_message('❌ จำนวนวันต้องมากกว่า 0', ephemeral=True)

        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute('SELECT account_id, vip_time FROM `login` WHERE userid = %s', (username,))
                row = await cur.fetchone()
                if not row:
                    return await interaction.response.send_message(f'❌ ไม่พบบัญชี **{username}**', ephemeral=True)

                now = int(time.time())
                current_vip = row[1] if row[1] and row[1] > now else now
                new_vip = current_vip + days * 86400

                await cur.execute('UPDATE `login` SET vip_time = %s WHERE userid = %s', (new_vip, username))

        embed = discord.Embed(title='⭐ เพิ่ม VIP สำเร็จ', color=discord.Color.gold())
        embed.add_field(name='Username',  value=username,          inline=True)
        embed.add_field(name='เพิ่ม',    value=f'{days} วัน',     inline=True)
        embed.add_field(name='VIP ถึง',  value=f'<t:{new_vip}:f>', inline=True)
        embed.set_footer(text=f'โดย {interaction.user}')
        await interaction.response.send_message(embed=embed)

    # ── Logs ─────────────────────────────────────────────────────────────────

    _LOG_TYPES = app_commands.Choice
    LogTypeChoice = [
        app_commands.Choice(name='GM Commands (atcommandlog)', value='gm'),
        app_commands.Choice(name='Cash/Points (cashlog)',      value='cash'),
        app_commands.Choice(name='Login history (loginlog)',   value='login'),
    ]

    @app_commands.command(name='logs', description='[ADMIN] ดู Logs')
    @app_commands.describe(log_type='ประเภท log', limit='จำนวนรายการ (สูงสุด 25)')
    @app_commands.choices(log_type=LogTypeChoice)
    @app_commands.default_permissions(administrator=True)
    @admin_check()
    async def logs(
        self,
        interaction: discord.Interaction,
        log_type: app_commands.Choice[str],
        limit: int = 10,
    ):
        limit = max(1, min(limit, 25))
        log_pool = get_log_pool()

        if log_type.value == 'gm':
            if not log_pool:
                return await interaction.response.send_message('❌ Log DB ไม่พร้อมใช้งาน', ephemeral=True)
            async with log_pool.acquire() as conn:
                async with conn.cursor() as cur:
                    await cur.execute(
                        'SELECT `time`, `account_id`, `char_id`, `map`, `command` '
                        'FROM atcommandlog ORDER BY `time` DESC LIMIT %s',
                        (limit,),
                    )
                    rows = await cur.fetchall()
            title = '📋 GM Command Log'
            lines = [f'`{r[0]}` acc:{r[1]} — `{r[4]}` @ {r[3]}' for r in rows]

        elif log_type.value == 'cash':
            if not log_pool:
                return await interaction.response.send_message('❌ Log DB ไม่พร้อมใช้งาน', ephemeral=True)
            async with log_pool.acquire() as conn:
                async with conn.cursor() as cur:
                    await cur.execute(
                        'SELECT `time`, `char_id`, `type`, `amount`, `nameid` '
                        'FROM cashlog ORDER BY `time` DESC LIMIT %s',
                        (limit,),
                    )
                    rows = await cur.fetchall()
            title = '💸 Cash Log'
            lines = [f'`{r[0]}` char:{r[1]} {r[2]} {r[3]:+,} (item:{r[4]})' for r in rows]

        else:  # login
            pool = get_pool()
            async with pool.acquire() as conn:
                async with conn.cursor() as cur:
                    await cur.execute(
                        'SELECT `time`, `userid`, `ip`, `rcode`, `log` '
                        'FROM `loginlog` ORDER BY `time` DESC LIMIT %s',
                        (limit,),
                    )
                    rows = await cur.fetchall()
            title = '🔑 Login Log'
            lines = [f'`{r[0]}` **{r[1]}** from `{r[2]}` — {r[4]}' for r in rows]

        embed = discord.Embed(
            title=title,
            description='\n'.join(lines) if lines else '_ไม่มีข้อมูล_',
            color=discord.Color.blurple(),
        )
        await interaction.response.send_message(embed=embed, ephemeral=True)


async def setup(bot: commands.Bot):
    await bot.add_cog(AdminCog(bot))
