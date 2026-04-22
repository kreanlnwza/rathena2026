import asyncio
import json
import logging
import os

import discord
from discord import app_commands
from discord.ext import commands

from cogs.manage import admin_check

log = logging.getLogger(__name__)

DATA_FILE = os.path.join(os.path.dirname(__file__), '..', 'data', 'sticky_messages.json')


def _load_data() -> dict:
    try:
        with open(DATA_FILE, 'r', encoding='utf-8') as f:
            return json.load(f)
    except (FileNotFoundError, json.JSONDecodeError):
        return {}


def _save_data(data: dict) -> None:
    os.makedirs(os.path.dirname(DATA_FILE), exist_ok=True)
    with open(DATA_FILE, 'w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)


sticky_group = app_commands.Group(name='sticky', description='จัดการข้อความติดหนึบ (Sticky Message)')


@sticky_group.command(name='set', description='[ADMIN] ตั้งค่าข้อความติดหนึบในช่องทาง')
@app_commands.describe(
    channel='ช่องทางที่ต้องการติดข้อความ',
    message='ข้อความที่ต้องการให้ติดอยู่ล่างสุดเสมอ',
)
@admin_check()
async def sticky_set(
    interaction: discord.Interaction,
    channel: discord.TextChannel,
    message: str,
):
    cog: StickyMessageCog = interaction.client.cogs.get('StickyMessageCog')
    if cog is None:
        return await interaction.response.send_message('❌ ไม่พบ StickyMessageCog', ephemeral=True)

    await interaction.response.defer(ephemeral=True)

    # ลบข้อความ sticky เก่าถ้ามี
    old = cog.sticky.get(str(channel.id))
    if old:
        try:
            old_msg = await channel.fetch_message(old['message_id'])
            await old_msg.delete()
        except (discord.NotFound, discord.HTTPException):
            pass

    sent = await channel.send(embed=_build_embed(message))

    cog.sticky[str(channel.id)] = {'text': message, 'message_id': sent.id}
    _save_data(cog.sticky)

    await interaction.followup.send(
        f'📌 ตั้งค่า Sticky Message ใน {channel.mention} สำเร็จแล้ว', ephemeral=True
    )


@sticky_group.command(name='remove', description='[ADMIN] ลบข้อความติดหนึบออกจากช่องทาง')
@app_commands.describe(channel='ช่องทางที่ต้องการลบข้อความติดหนึบ')
@admin_check()
async def sticky_remove(interaction: discord.Interaction, channel: discord.TextChannel):
    cog: StickyMessageCog = interaction.client.cogs.get('StickyMessageCog')
    if cog is None:
        return await interaction.response.send_message('❌ ไม่พบ StickyMessageCog', ephemeral=True)

    entry = cog.sticky.pop(str(channel.id), None)
    if not entry:
        return await interaction.response.send_message(
            f'⚠️ ไม่มี Sticky Message ใน {channel.mention}', ephemeral=True
        )

    try:
        old_msg = await channel.fetch_message(entry['message_id'])
        await old_msg.delete()
    except (discord.NotFound, discord.HTTPException):
        pass

    _save_data(cog.sticky)
    await interaction.response.send_message(
        f'🗑️ ลบ Sticky Message ใน {channel.mention} สำเร็จแล้ว', ephemeral=True
    )


@sticky_group.command(name='list', description='แสดงรายการช่องทางที่มีข้อความติดหนึบ')
async def sticky_list(interaction: discord.Interaction):
    cog: StickyMessageCog = interaction.client.cogs.get('StickyMessageCog')
    if cog is None:
        return await interaction.response.send_message('❌ ไม่พบ StickyMessageCog', ephemeral=True)

    if not cog.sticky:
        return await interaction.response.send_message('ℹ️ ไม่มีช่องทางที่ตั้งค่า Sticky Message', ephemeral=True)

    lines = []
    for ch_id, entry in cog.sticky.items():
        ch = interaction.guild.get_channel(int(ch_id))
        ch_name = ch.mention if ch else f'`{ch_id}`'
        preview = entry['text'][:60] + ('…' if len(entry['text']) > 60 else '')
        lines.append(f'{ch_name} — {preview}')

    embed = discord.Embed(
        title='📌 Sticky Messages ทั้งหมด',
        description='\n'.join(lines),
        color=discord.Color.gold(),
    )
    await interaction.response.send_message(embed=embed, ephemeral=True)


def _build_embed(text: str) -> discord.Embed:
    return discord.Embed(description=f'📌 **ƒɾìҽɾҽղ ༺ Ai ༻**\n\n{text}', color=discord.Color.gold())


class StickyMessageCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot
        # {channel_id_str: {text, message_id}}
        self.sticky: dict = _load_data()
        # debounce: ป้องกันส่งข้อความซ้ำถี่เกินไป
        self._pending: set[int] = set()
        bot.tree.add_command(sticky_group)

    @commands.Cog.listener()
    async def on_message(self, message: discord.Message):
        # ไม่ตอบสนองต่อข้อความของบอทเอง
        if message.author.bot:
            return
        if not message.guild:
            return

        ch_id = str(message.channel.id)
        entry = self.sticky.get(ch_id)
        if not entry:
            return

        # debounce: ถ้ากำลังรอส่งอยู่แล้ว ไม่ต้องซ้ำ
        if message.channel.id in self._pending:
            return

        self._pending.add(message.channel.id)
        try:
            # รอ 0.8 วินาทีเพื่อรวมหลายข้อความที่ส่งติดกัน
            await asyncio.sleep(0.8)

            # ลบข้อความ sticky เก่า
            try:
                old_msg = await message.channel.fetch_message(entry['message_id'])
                await old_msg.delete()
            except (discord.NotFound, discord.HTTPException):
                pass

            # ส่งข้อความ sticky ใหม่ล่างสุด
            sent = await message.channel.send(embed=_build_embed(entry['text']))
            self.sticky[ch_id]['message_id'] = sent.id
            _save_data(self.sticky)

        except discord.HTTPException as e:
            log.warning('sticky_message: ส่งข้อความล้มเหลวใน channel %s: %s', ch_id, e)
        finally:
            self._pending.discard(message.channel.id)


async def setup(bot: commands.Bot):
    await bot.add_cog(StickyMessageCog(bot))
