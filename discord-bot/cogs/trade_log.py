import json
import logging
import os
from collections import defaultdict
from datetime import datetime
from typing import Any

import discord
from discord import app_commands
from discord.ext import commands, tasks

import config
from utils import gamedata
from utils.checks import admin_check
from utils.constants import format_zeny
from utils.db import get_log_pool, get_pool

log = logging.getLogger(__name__)

TRADE_TYPE = 'T'
STATE_FILE = os.path.join(os.path.dirname(__file__), '..', 'data', 'trade_log_state.json')

tradelog_group = app_commands.Group(name='tradelog', description='ดู trade logs ระหว่างผู้เล่น')


def _load_state() -> dict:
    try:
        with open(STATE_FILE, 'r', encoding='utf-8') as f:
            data = json.load(f)
            return data if isinstance(data, dict) else {}
    except (FileNotFoundError, json.JSONDecodeError):
        return {}


def _save_state(data: dict) -> None:
    os.makedirs(os.path.dirname(STATE_FILE), exist_ok=True)
    with open(STATE_FILE, 'w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)


def _time_key(ts: datetime) -> str:
    return ts.strftime('%Y-%m-%d %H:%M:%S')


def _item_name(nameid: int, refine: int) -> str:
    item = gamedata.get_item(nameid)
    label = item['name'] if item else f'Item#{nameid}'
    return f'+{refine} {label}' if refine > 0 else label


class TradeLogCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot
        self.state = _load_state()
        bot.tree.add_command(tradelog_group)

    def cog_load(self) -> None:
        if config.TRADE_LOG_CHANNEL_ID:
            self.trade_feed.change_interval(seconds=config.TRADE_LOG_POLL_INTERVAL)
            self.trade_feed.start()

    def cog_unload(self) -> None:
        self.trade_feed.cancel()

    def _get_last_ids(self) -> tuple[int, int]:
        return int(self.state.get('last_pick_id', 0)), int(self.state.get('last_zeny_id', 0))

    def _set_last_ids(self, pick_id: int, zeny_id: int) -> None:
        self.state['last_pick_id'] = int(pick_id)
        self.state['last_zeny_id'] = int(zeny_id)
        _save_state(self.state)

    def _set_bootstrap_sent(self) -> None:
        self.state['bootstrap_sent'] = True
        _save_state(self.state)

    async def _initialise_last_ids(self) -> None:
        log_pool = get_log_pool()
        if not log_pool:
            return

        async with log_pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute('SELECT COALESCE(MAX(id), 0) FROM picklog WHERE type = %s', (TRADE_TYPE,))
                pick_id = (await cur.fetchone())[0] or 0
                await cur.execute('SELECT COALESCE(MAX(id), 0) FROM zenylog WHERE type = %s', (TRADE_TYPE,))
                zeny_id = (await cur.fetchone())[0] or 0
        self._set_last_ids(pick_id, zeny_id)

    async def _resolve_char_names(self, char_ids: set[int]) -> dict[int, str]:
        if not char_ids:
            return {}

        ids = sorted(char_ids)
        placeholders = ','.join(['%s'] * len(ids))
        pool = get_pool()

        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    f'SELECT char_id, name FROM `char` WHERE char_id IN ({placeholders})',
                    ids,
                )
                rows = await cur.fetchall()
        return {row[0]: row[1] for row in rows}

    async def _fetch_recent_rows(self, raw_limit: int) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
        log_pool = get_log_pool()
        if not log_pool:
            raise RuntimeError('Log DB pool not initialised')

        async with log_pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    'SELECT id, `time`, char_id, nameid, amount, refine, map '
                    'FROM picklog WHERE type = %s ORDER BY id DESC LIMIT %s',
                    (TRADE_TYPE, raw_limit),
                )
                pick_rows = [
                    {
                        'id': row[0],
                        'time': row[1],
                        'char_id': row[2],
                        'nameid': row[3],
                        'amount': row[4],
                        'refine': row[5],
                        'map': row[6],
                    }
                    for row in await cur.fetchall()
                ]

                await cur.execute(
                    'SELECT id, `time`, char_id, src_id, amount, map '
                    'FROM zenylog WHERE type = %s ORDER BY id DESC LIMIT %s',
                    (TRADE_TYPE, raw_limit),
                )
                zeny_rows = [
                    {
                        'id': row[0],
                        'time': row[1],
                        'char_id': row[2],
                        'src_id': row[3],
                        'amount': row[4],
                        'map': row[5],
                    }
                    for row in await cur.fetchall()
                ]

        pick_rows.reverse()
        zeny_rows.reverse()
        return pick_rows, zeny_rows

    async def _fetch_new_rows(self, last_pick_id: int, last_zeny_id: int) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
        log_pool = get_log_pool()
        if not log_pool:
            raise RuntimeError('Log DB pool not initialised')

        async with log_pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    'SELECT id, `time`, char_id, nameid, amount, refine, map '
                    'FROM picklog WHERE id > %s AND type = %s ORDER BY id',
                    (last_pick_id, TRADE_TYPE),
                )
                pick_rows = [
                    {
                        'id': row[0],
                        'time': row[1],
                        'char_id': row[2],
                        'nameid': row[3],
                        'amount': row[4],
                        'refine': row[5],
                        'map': row[6],
                    }
                    for row in await cur.fetchall()
                ]

                await cur.execute(
                    'SELECT id, `time`, char_id, src_id, amount, map '
                    'FROM zenylog WHERE id > %s AND type = %s ORDER BY id',
                    (last_zeny_id, TRADE_TYPE),
                )
                zeny_rows = [
                    {
                        'id': row[0],
                        'time': row[1],
                        'char_id': row[2],
                        'src_id': row[3],
                        'amount': row[4],
                        'map': row[5],
                    }
                    for row in await cur.fetchall()
                ]

        return pick_rows, zeny_rows

    async def _build_events(
        self,
        pick_rows: list[dict[str, Any]],
        zeny_rows: list[dict[str, Any]],
    ) -> list[dict[str, Any]]:
        char_ids = {row['char_id'] for row in pick_rows}
        char_ids.update(row['char_id'] for row in zeny_rows)
        char_ids.update(row['src_id'] for row in zeny_rows if row['src_id'] > 0)
        names = await self._resolve_char_names(char_ids)

        positive_items: dict[tuple[str, int, int, int, str], list[dict[str, Any]]] = defaultdict(list)
        for row in pick_rows:
            if row['amount'] > 0:
                key = (
                    _time_key(row['time']),
                    row['nameid'],
                    abs(row['amount']),
                    row['refine'],
                    row['map'],
                )
                positive_items[key].append(row)

        events: dict[tuple[str, int, int, str], dict[str, Any]] = {}

        def ensure_event(ts: datetime, sender_id: int, receiver_id: int, map_name: str) -> dict[str, Any]:
            low, high = sorted((sender_id, receiver_id))
            key = (_time_key(ts), low, high, map_name)
            event = events.get(key)
            if event is None:
                event = {
                    'time': ts,
                    'map': map_name,
                    'participants': {sender_id, receiver_id},
                    'names': {},
                    'flows': {},
                }
                events[key] = event
            return event

        def ensure_flow(event: dict[str, Any], sender_id: int, receiver_id: int) -> dict[str, Any]:
            key = (sender_id, receiver_id)
            flow = event['flows'].get(key)
            if flow is None:
                flow = {'zeny': 0, 'items': defaultdict(int)}
                event['flows'][key] = flow
            return flow

        for row in zeny_rows:
            if row['amount'] >= 0 or row['src_id'] <= 0:
                continue
            event = ensure_event(row['time'], row['char_id'], row['src_id'], row['map'])
            flow = ensure_flow(event, row['char_id'], row['src_id'])
            flow['zeny'] += abs(row['amount'])

        for row in pick_rows:
            if row['amount'] >= 0:
                continue

            key = (
                _time_key(row['time']),
                row['nameid'],
                abs(row['amount']),
                row['refine'],
                row['map'],
            )
            candidates = positive_items.get(key, [])
            idx = next((i for i, candidate in enumerate(candidates) if candidate['char_id'] != row['char_id']), None)
            if idx is None:
                continue

            receiver = candidates.pop(idx)
            if not candidates:
                positive_items.pop(key, None)

            event = ensure_event(row['time'], row['char_id'], receiver['char_id'], row['map'])
            flow = ensure_flow(event, row['char_id'], receiver['char_id'])
            flow['items'][_item_name(row['nameid'], row['refine'])] += abs(row['amount'])

        built = []
        for event in events.values():
            participant_ids = sorted(
                event['participants'],
                key=lambda char_id: names.get(char_id, f'char#{char_id}').lower(),
            )
            event['participants'] = participant_ids
            event['names'] = {char_id: names.get(char_id, f'char#{char_id}') for char_id in participant_ids}
            built.append(event)

        built.sort(key=lambda event: event['time'], reverse=True)
        return built

    async def _fetch_recent_event_bundle(self, limit: int) -> tuple[list[dict[str, Any]], list[dict[str, Any]], list[dict[str, Any]]]:
        pick_rows, zeny_rows = await self._fetch_recent_rows(max(limit * 40, 200))
        events = await self._build_events(pick_rows, zeny_rows)
        return pick_rows, zeny_rows, events[:limit]

    def _build_raw_embeds(
        self,
        pick_rows: list[dict[str, Any]],
        zeny_rows: list[dict[str, Any]],
        names: dict[int, str],
        limit: int,
    ) -> list[discord.Embed]:
        entries: list[dict[str, Any]] = []

        for row in pick_rows:
            if row['amount'] >= 0:
                continue
            entries.append(
                {
                    'time': row['time'],
                    'kind': 'item',
                    'sender_id': row['char_id'],
                    'receiver_id': None,
                    'map': row['map'],
                    'detail': f'{_item_name(row["nameid"], row["refine"])} ×{abs(row["amount"])}',
                }
            )

        for row in zeny_rows:
            if row['amount'] >= 0:
                continue
            entries.append(
                {
                    'time': row['time'],
                    'kind': 'zeny',
                    'sender_id': row['char_id'],
                    'receiver_id': row['src_id'] if row['src_id'] > 0 else None,
                    'map': row['map'],
                    'detail': format_zeny(abs(row['amount'])),
                }
            )

        entries.sort(key=lambda entry: entry['time'], reverse=True)

        embeds = []
        for entry in entries[:limit]:
            sender = names.get(entry['sender_id'], f'char#{entry["sender_id"]}')
            receiver = (
                names.get(entry['receiver_id'], f'char#{entry["receiver_id"]}')
                if entry['receiver_id']
                else 'ไม่ทราบปลายทาง'
            )
            title = '📦 Raw Trade Item Log' if entry['kind'] == 'item' else '💰 Raw Trade Zeny Log'
            embed = discord.Embed(
                title=title,
                color=discord.Color.orange(),
                timestamp=entry['time'],
            )
            embed.add_field(name='ผู้ให้', value=sender, inline=True)
            embed.add_field(name='ผู้รับ', value=receiver, inline=True)
            embed.add_field(name='Map', value=entry['map'] or '-', inline=True)
            embed.add_field(name='รายละเอียด', value=entry['detail'], inline=False)
            embed.set_footer(text='fallback from raw trade logs')
            embeds.append(embed)

        return embeds

    def _build_embed(self, event: dict[str, Any]) -> discord.Embed:
        names = event['names']
        title = ' ↔ '.join(names[char_id] for char_id in event['participants'])
        embed = discord.Embed(
            title=f'🤝 Trade Log: {title}',
            color=discord.Color.teal(),
            timestamp=event['time'],
        )
        embed.add_field(name='📍 Map', value=event['map'] or '-', inline=True)
        embed.add_field(
            name='👥 ผู้เล่น',
            value=' / '.join(names[char_id] for char_id in event['participants']),
            inline=False,
        )

        flow_lines = []
        for (sender_id, receiver_id), flow in sorted(
            event['flows'].items(),
            key=lambda entry: (names.get(entry[0][0], ''), names.get(entry[0][1], '')),
        ):
            parts = [f'**{names.get(sender_id, f"char#{sender_id}")}** → **{names.get(receiver_id, f"char#{receiver_id}")}**']
            if flow['zeny']:
                parts.append(f'Zeny: {format_zeny(flow["zeny"])}')
            if flow['items']:
                items = ', '.join(f'{item_name} ×{amount}' for item_name, amount in sorted(flow['items'].items()))
                parts.append(f'Items: {items}')
            flow_lines.append('\n'.join(parts))

        embed.add_field(name='รายละเอียด', value='\n\n'.join(flow_lines)[:1024] or '-', inline=False)
        embed.set_footer(text='rAthena trade logs')
        return embed

    async def _send_backfill(self, channel: discord.abc.Messageable, limit: int) -> int:
        pick_rows, zeny_rows, events = await self._fetch_recent_event_bundle(limit)
        if events:
            for event in reversed(events):
                await channel.send(embed=self._build_embed(event))
            return len(events)

        char_ids = {row['char_id'] for row in pick_rows}
        char_ids.update(row['char_id'] for row in zeny_rows)
        char_ids.update(row['src_id'] for row in zeny_rows if row['src_id'] > 0)
        names = await self._resolve_char_names(char_ids)
        raw_embeds = self._build_raw_embeds(pick_rows, zeny_rows, names, limit)
        for embed in reversed(raw_embeds):
            await channel.send(embed=embed)
        return len(raw_embeds)

    async def _send_bootstrap_message(self, channel: discord.abc.Messageable) -> None:
        embed = discord.Embed(
            title='📒 Trade Log Feed พร้อมใช้งาน',
            description=(
                'บอทเชื่อมต่อระบบ trade log แล้ว\n'
                'จากนี้เมื่อมีการแลกเปลี่ยนใหม่ ระบบจะประกาศในห้องนี้อัตโนมัติ'
            ),
            color=discord.Color.green(),
            timestamp=discord.utils.utcnow(),
        )
        embed.add_field(name='Poll Interval', value=f'{config.TRADE_LOG_POLL_INTERVAL} วินาที', inline=True)
        embed.add_field(name='Log DB', value=config.DB_LOG_NAME, inline=True)
        embed.set_footer(text='trade log bootstrap')
        await channel.send(embed=embed)

    @tasks.loop(seconds=30)
    async def trade_feed(self) -> None:
        channel = self.bot.get_channel(config.TRADE_LOG_CHANNEL_ID)
        if channel is None:
            log.warning('TRADE_LOG_CHANNEL_ID %s not found', config.TRADE_LOG_CHANNEL_ID)
            return

        if not self.state.get('bootstrap_sent'):
            try:
                sent = await self._send_backfill(channel, 5)
                if sent == 0:
                    await self._send_bootstrap_message(channel)
                self._set_bootstrap_sent()
            except Exception:
                log.exception('Failed to send trade log bootstrap message')
            last_pick_id, last_zeny_id = self._get_last_ids()
            if last_pick_id == 0 and last_zeny_id == 0:
                await self._initialise_last_ids()
            return

        last_pick_id, last_zeny_id = self._get_last_ids()
        if last_pick_id == 0 and last_zeny_id == 0:
            await self._initialise_last_ids()
            return

        try:
            pick_rows, zeny_rows = await self._fetch_new_rows(last_pick_id, last_zeny_id)
        except Exception:
            log.exception('Failed to query trade logs')
            return

        if not pick_rows and not zeny_rows:
            return

        events = await self._build_events(pick_rows, zeny_rows)
        for event in reversed(events):
            await channel.send(embed=self._build_embed(event))

        self._set_last_ids(
            pick_rows[-1]['id'] if pick_rows else last_pick_id,
            zeny_rows[-1]['id'] if zeny_rows else last_zeny_id,
        )

    @trade_feed.before_loop
    async def _before_trade_feed(self) -> None:
        await self.bot.wait_until_ready()


@tradelog_group.command(name='recent', description='[ADMIN] ดู trade logs ล่าสุด')
@app_commands.describe(limit='จำนวนดีลที่ต้องการดู (สูงสุด 10)')
@admin_check()
async def tradelog_recent(interaction: discord.Interaction, limit: app_commands.Range[int, 1, 10] = 5):
    await interaction.response.defer(ephemeral=True)

    cog: TradeLogCog = interaction.client.cogs.get('TradeLogCog')
    if cog is None:
        return await interaction.followup.send('❌ ไม่พบ TradeLogCog', ephemeral=True)
    if not get_log_pool():
        return await interaction.followup.send('❌ Log DB ไม่พร้อมใช้งาน', ephemeral=True)

    pick_rows, zeny_rows = await cog._fetch_recent_rows(max(limit * 20, 100))
    events = await cog._build_events(pick_rows, zeny_rows)
    if not events:
        char_ids = {row['char_id'] for row in pick_rows}
        char_ids.update(row['char_id'] for row in zeny_rows)
        char_ids.update(row['src_id'] for row in zeny_rows if row['src_id'] > 0)
        names = await cog._resolve_char_names(char_ids)
        raw_embeds = cog._build_raw_embeds(pick_rows, zeny_rows, names, limit)
        if raw_embeds:
            return await interaction.followup.send(
                content='⚠️ พบ raw trade rows แต่ยังสรุปเป็นดีลอัตโนมัติไม่ได้ จึงแสดงรายการดิบแทน',
                embeds=raw_embeds,
                ephemeral=True,
            )
        return await interaction.followup.send('📭 ยังไม่พบ trade logs ในระบบ', ephemeral=True)

    await interaction.followup.send(
        embeds=[cog._build_embed(event) for event in events[:limit]],
        ephemeral=True,
    )


@tradelog_group.command(name='player', description='[ADMIN] ดู trade logs ของตัวละครที่ระบุ')
@app_commands.describe(character_name='ชื่อตัวละคร', limit='จำนวนดีลที่ต้องการดู (สูงสุด 10)')
@admin_check()
async def tradelog_player(
    interaction: discord.Interaction,
    character_name: str,
    limit: app_commands.Range[int, 1, 10] = 5,
):
    await interaction.response.defer(ephemeral=True)

    cog: TradeLogCog = interaction.client.cogs.get('TradeLogCog')
    if cog is None:
        return await interaction.followup.send('❌ ไม่พบ TradeLogCog', ephemeral=True)
    if not get_log_pool():
        return await interaction.followup.send('❌ Log DB ไม่พร้อมใช้งาน', ephemeral=True)

    pick_rows, zeny_rows = await cog._fetch_recent_rows(max(limit * 40, 200))
    events = await cog._build_events(pick_rows, zeny_rows)
    filtered = [
        event for event in events
        if any(character_name.lower() in event['names'][char_id].lower() for char_id in event['participants'])
    ]
    if not filtered:
        char_ids = {row['char_id'] for row in pick_rows}
        char_ids.update(row['char_id'] for row in zeny_rows)
        char_ids.update(row['src_id'] for row in zeny_rows if row['src_id'] > 0)
        names = await cog._resolve_char_names(char_ids)
        raw_embeds = [
            embed for embed in cog._build_raw_embeds(pick_rows, zeny_rows, names, max(limit * 3, 20))
            if character_name.lower() in (embed.fields[0].value.lower() + ' ' + embed.fields[1].value.lower())
        ]
        if raw_embeds:
            return await interaction.followup.send(
                content='⚠️ พบ raw trade rows ของตัวละครนี้ แต่ยังสรุปเป็นดีลอัตโนมัติไม่ได้ จึงแสดงรายการดิบแทน',
                embeds=raw_embeds[:limit],
                ephemeral=True,
            )
        return await interaction.followup.send(
            f'📭 ไม่พบ trade logs ของตัวละครที่มีคำว่า **{character_name}**',
            ephemeral=True,
        )

    await interaction.followup.send(
        embeds=[cog._build_embed(event) for event in filtered[:limit]],
        ephemeral=True,
    )


@tradelog_group.command(name='status', description='[ADMIN] ตรวจสถานะระบบ trade logs')
@admin_check()
async def tradelog_status(interaction: discord.Interaction):
    cog: TradeLogCog = interaction.client.cogs.get('TradeLogCog')
    last_pick_id, last_zeny_id = cog._get_last_ids() if cog else (0, 0)

    embed = discord.Embed(title='📒 Trade Log Status', color=discord.Color.blurple())
    embed.add_field(name='Log DB', value='พร้อมใช้งาน' if get_log_pool() else 'ไม่พร้อมใช้งาน', inline=True)
    embed.add_field(
        name='Feed Channel',
        value=f'<#{config.TRADE_LOG_CHANNEL_ID}>' if config.TRADE_LOG_CHANNEL_ID else 'ยังไม่ได้ตั้ง',
        inline=True,
    )
    embed.add_field(name='Poll Interval', value=f'{config.TRADE_LOG_POLL_INTERVAL} วินาที', inline=True)
    embed.add_field(name='last picklog id', value=str(last_pick_id), inline=True)
    embed.add_field(name='last zenylog id', value=str(last_zeny_id), inline=True)
    await interaction.response.send_message(embed=embed, ephemeral=True)


@tradelog_group.command(name='backfill', description='[ADMIN] ส่ง trade logs ย้อนหลังไปยังห้อง feed')
@app_commands.describe(limit='จำนวนดีลย้อนหลังที่ต้องการส่ง (สูงสุด 10)')
@admin_check()
async def tradelog_backfill(interaction: discord.Interaction, limit: app_commands.Range[int, 1, 10] = 5):
    await interaction.response.defer(ephemeral=True)

    cog: TradeLogCog = interaction.client.cogs.get('TradeLogCog')
    if cog is None:
        return await interaction.followup.send('❌ ไม่พบ TradeLogCog', ephemeral=True)
    if not get_log_pool():
        return await interaction.followup.send('❌ Log DB ไม่พร้อมใช้งาน', ephemeral=True)
    if not config.TRADE_LOG_CHANNEL_ID:
        return await interaction.followup.send('❌ ยังไม่ได้ตั้ง TRADE_LOG_CHANNEL_ID', ephemeral=True)

    channel = interaction.client.get_channel(config.TRADE_LOG_CHANNEL_ID)
    if channel is None:
        return await interaction.followup.send('❌ ไม่พบห้อง feed trade logs ที่ตั้งไว้', ephemeral=True)

    sent = await cog._send_backfill(channel, limit)
    if sent == 0:
        return await interaction.followup.send('📭 ยังไม่พบ trade logs สำหรับ backfill', ephemeral=True)

    await interaction.followup.send(
        f'✅ ส่ง trade logs ย้อนหลัง {sent} รายการ ไปยัง {channel.mention} แล้ว',
        ephemeral=True,
    )


async def setup(bot: commands.Bot):
    await bot.add_cog(TradeLogCog(bot))
