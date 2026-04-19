import discord
from discord import app_commands
from discord.ext import commands
from utils.db import get_pool
from utils.constants import format_zeny
from utils import gamedata


class ServerCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot

    @app_commands.command(name='serverstatus', description='ดูสถานะเซิร์ฟเวอร์')
    async def serverstatus(self, interaction: discord.Interaction):
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute('SELECT COUNT(*) FROM `char` WHERE online = 1')
                online = (await cur.fetchone())[0]

                await cur.execute("SELECT COUNT(*) FROM `login` WHERE group_id < 99 AND state = 0")
                total_acc = (await cur.fetchone())[0]

                await cur.execute('SELECT COUNT(*) FROM `char`')
                total_char = (await cur.fetchone())[0]

                await cur.execute('SELECT MAX(lastlogin) FROM `login`')
                last_login = (await cur.fetchone())[0]

        embed = discord.Embed(
            title='🖥️ Server Status',
            color=discord.Color.green() if online > 0 else discord.Color.greyple(),
        )
        embed.add_field(name='🟢 Online',        value=f'{online:,} คน',     inline=True)
        embed.add_field(name='👥 บัญชีทั้งหมด',  value=f'{total_acc:,}',     inline=True)
        embed.add_field(name='🧑 ตัวละครทั้งหมด', value=f'{total_char:,}',    inline=True)
        embed.add_field(name='🕐 Login ล่าสุด',  value=str(last_login) if last_login else '-', inline=True)
        embed.add_field(name='📦 Items loaded',  value=f'{gamedata.item_count():,}', inline=True)
        await interaction.response.send_message(embed=embed)

    @app_commands.command(name='totalzeny', description='ยอด Zeny รวมทั้งหมดในเซิร์ฟเวอร์')
    async def totalzeny(self, interaction: discord.Interaction):
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                # zeny on characters
                await cur.execute('SELECT SUM(zeny) FROM `char`')
                char_zeny = (await cur.fetchone())[0] or 0

                # top 10 richest characters
                await cur.execute(
                    'SELECT name, class, zeny FROM `char` ORDER BY zeny DESC LIMIT 10'
                )
                top = await cur.fetchall()

                # zeny held per job class bucket
                await cur.execute(
                    'SELECT COUNT(*) as cnt, SUM(zeny) as total '
                    'FROM `char` WHERE zeny > 0'
                )
                stats = await cur.fetchone()

        from utils.constants import job_name
        embed = discord.Embed(
            title='💰 Zeny รวมทั้งหมดในเซิร์ฟเวอร์',
            color=discord.Color.gold(),
        )
        embed.add_field(name='💵 รวมทั้งสิ้น', value=format_zeny(char_zeny), inline=False)
        if stats:
            embed.add_field(name='👤 ตัวละครที่มี Zeny', value=f'{stats[0]:,} คน', inline=True)
            avg = int(stats[1] / stats[0]) if stats[0] else 0
            embed.add_field(name='📊 เฉลี่ย/ตัวละคร', value=format_zeny(avg), inline=True)

        if top:
            lines = [
                f'`{i+1}.` **{r[0]}** ({job_name(r[1])}) — {format_zeny(r[2])}'
                for i, r in enumerate(top)
            ]
            embed.add_field(name='🏆 Top 10 ร่ำรวยที่สุด', value='\n'.join(lines), inline=False)

        await interaction.response.send_message(embed=embed)


async def setup(bot: commands.Bot):
    await bot.add_cog(ServerCog(bot))
