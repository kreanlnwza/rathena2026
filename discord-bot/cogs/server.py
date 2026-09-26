import asyncio
import json
import logging
import os

import discord
from discord import app_commands
from discord.ext import commands, tasks

import config
from utils import gamedata
from utils.constants import format_zeny, job_name
from utils.db import get_pool
from utils.checks import player_check

log = logging.getLogger(__name__)

_STATUS_DATA = os.path.join(os.path.dirname(__file__), '..', 'data', 'status_message.json')
_ZENY_DATA = os.path.join(os.path.dirname(__file__), '..', 'data', 'zeny_message.json')
async def _check_port(host: str, port: int, timeout: float = 2.0) -> bool:
    try:
        _, writer = await asyncio.wait_for(
            asyncio.open_connection(host, port), timeout=timeout
        )
        writer.close()
        await writer.wait_closed()
        return True
    except Exception:
        return False


def _svc(ok: bool) -> str:
    return '🟢' if ok else '🔴'


class ServerCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot
        self._status_msg_id: int | None = self._load_json(_STATUS_DATA, 'message_id')
        # Zeny tracker
        self._zeny_msg_id: int | None = self._load_json(_ZENY_DATA, 'message_id')

    # ── persistence ───────────────────────────────────────────────────────────

    @staticmethod
    def _load_json(path: str, key: str):
        try:
            with open(path) as f:
                return json.load(f).get(key)
        except (FileNotFoundError, json.JSONDecodeError):
            return None

    @staticmethod
    def _save_json(path: str, key: str, value):
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, 'w') as f:
            json.dump({key: value}, f)

    # ── state query ───────────────────────────────────────────────────────────

    async def _query_state(self) -> tuple:
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

        h = config.RATHENA_HOST
        login_ok, char_ok, map_ok, web_ok = await asyncio.gather(
            _check_port(h, config.LOGIN_PORT),
            _check_port(h, config.CHAR_PORT),
            _check_port(h, config.MAP_PORT),
            _check_port(h, config.WEB_PORT),
        )

        return (online, total_acc, total_char, last_login, login_ok, char_ok, map_ok, web_ok)

    def _build_embed(self, state: tuple) -> discord.Embed:
        online, total_acc, total_char, last_login, login_ok, char_ok, map_ok, web_ok = state

        all_core_up = login_ok and char_ok and map_ok
        color = discord.Color.green() if (all_core_up and online >= 0) else discord.Color.red()

        embed = discord.Embed(
            title='🖥️ Server Status',
            color=color,
            timestamp=discord.utils.utcnow(),
        )

        # ── player stats ──
        embed.add_field(name='🟢 Online',         value=f'{online:,} คน',   inline=True)
        embed.add_field(name='👥 บัญชีทั้งหมด',   value=f'{total_acc:,}',   inline=True)
        embed.add_field(name='🧑 ตัวละครทั้งหมด', value=f'{total_char:,}',  inline=True)
        embed.add_field(name='🕐 Login ล่าสุด',   value=str(last_login) if last_login else '-', inline=True)
        embed.add_field(name='📦 Items loaded',   value=f'{gamedata.item_count():,}', inline=True)

        # ── process status ──
        proc_lines = (
            f'{_svc(login_ok)} **Login**  '
            f'{_svc(char_ok)} **Char**  '
            f'{_svc(map_ok)} **Map**  '
            f'{_svc(web_ok)} **Web**  '
            f'🟢 **Bot**'
        )
        embed.add_field(name='⚙️ โปรเซส', value=proc_lines, inline=False)

        embed.set_footer(text=f'อัปเดตทุก {config.STATUS_POLL_INTERVAL} วินาที')
        return embed

    # ── Zeny query & embed ────────────────────────────────────────────────────

    async def _query_zeny_state(self) -> tuple:
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    'SELECT COALESCE(SUM(c.zeny), 0) '
                    'FROM `char` c '
                    'JOIN `login` l ON l.account_id = c.account_id '
                    'WHERE l.group_id <> 99'
                )
                char_zeny = (await cur.fetchone())[0] or 0

                await cur.execute(
                    'SELECT c.name, c.class, c.zeny '
                    'FROM `char` c '
                    'JOIN `login` l ON l.account_id = c.account_id '
                    'WHERE l.group_id <> 99 '
                    'ORDER BY c.zeny DESC LIMIT 10'
                )
                top = await cur.fetchall()

                await cur.execute(
                    'SELECT COUNT(*) as cnt, COALESCE(SUM(c.zeny), 0) as total '
                    'FROM `char` c '
                    'JOIN `login` l ON l.account_id = c.account_id '
                    'WHERE c.zeny > 0 AND l.group_id <> 99'
                )
                stats = await cur.fetchone()

        return (char_zeny, top, stats)

    def _build_zeny_embed(self, state: tuple) -> discord.Embed:
        char_zeny, top, stats = state

        embed = discord.Embed(
            title='💰 Zeny รวมทั้งหมดในเซิร์ฟเวอร์',
            color=discord.Color.gold(),
            timestamp=discord.utils.utcnow(),
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

        embed.set_footer(text=f'อัปเดตทุก {config.STATUS_POLL_INTERVAL} วินาที')
        return embed

    # ── auto-status background task ───────────────────────────────────────────

    def cog_load(self) -> None:
        if config.STATUS_CHANNEL_ID:
            self.auto_status.change_interval(seconds=config.STATUS_POLL_INTERVAL)
            self.auto_status.start()
        if config.ZENY_CHANNEL_ID:
            self.auto_zeny.change_interval(seconds=config.STATUS_POLL_INTERVAL)
            self.auto_zeny.start()

    def cog_unload(self) -> None:
        self.auto_status.cancel()
        self.auto_zeny.cancel()

    @tasks.loop(seconds=30)
    async def auto_status(self) -> None:
        channel = self.bot.get_channel(config.STATUS_CHANNEL_ID)
        if channel is None:
            log.warning('STATUS_CHANNEL_ID %s not found', config.STATUS_CHANNEL_ID)
            return

        try:
            state = await self._query_state()
        except Exception:
            log.exception('Failed to query server state')
            return

        embed = self._build_embed(state)

        if self._status_msg_id:
            try:
                msg = await channel.fetch_message(self._status_msg_id)
                await msg.edit(embed=embed)
                return
            except discord.NotFound:
                self._status_msg_id = None

        msg = await channel.send(embed=embed)
        self._status_msg_id = msg.id
        self._save_json(_STATUS_DATA, 'message_id', msg.id)
        log.info('Status message created in channel %s (id=%s)', channel.id, msg.id)

    @auto_status.before_loop
    async def _before_auto_status(self) -> None:
        await self.bot.wait_until_ready()

    # ── auto-zeny background task ─────────────────────────────────────────────

    @tasks.loop(seconds=60)
    async def auto_zeny(self) -> None:
        channel = self.bot.get_channel(config.ZENY_CHANNEL_ID)
        if channel is None:
            log.warning('ZENY_CHANNEL_ID %s not found', config.ZENY_CHANNEL_ID)
            return

        try:
            state = await self._query_zeny_state()
        except Exception:
            log.exception('Failed to query zeny state')
            return

        embed = self._build_zeny_embed(state)

        if self._zeny_msg_id:
            try:
                msg = await channel.fetch_message(self._zeny_msg_id)
                await msg.edit(embed=embed)
                return
            except discord.NotFound:
                self._zeny_msg_id = None

        msg = await channel.send(embed=embed)
        self._zeny_msg_id = msg.id
        self._save_json(_ZENY_DATA, 'message_id', msg.id)
        log.info('Zeny message created in channel %s (id=%s)', channel.id, msg.id)

    @auto_zeny.before_loop
    async def _before_auto_zeny(self) -> None:
        await self.bot.wait_until_ready()

    # ── slash commands ────────────────────────────────────────────────────────

    @app_commands.command(name='serverstatus', description='ดูสถานะเซิร์ฟเวอร์')
    @app_commands.default_permissions(send_messages=True)
    @player_check()
    async def serverstatus(self, interaction: discord.Interaction):
        await interaction.response.defer()
        state = await self._query_state()
        embed = self._build_embed(state)
        await interaction.followup.send(embed=embed)

    @app_commands.command(name='totalzeny', description='ยอด Zeny รวมทั้งหมดในเซิร์ฟเวอร์')
    @app_commands.default_permissions(send_messages=True)
    @player_check()
    async def totalzeny(self, interaction: discord.Interaction):
        await interaction.response.defer()
        state = await self._query_zeny_state()
        embed = self._build_zeny_embed(state)
        await interaction.followup.send(embed=embed)


async def setup(bot: commands.Bot):
    await bot.add_cog(ServerCog(bot))
