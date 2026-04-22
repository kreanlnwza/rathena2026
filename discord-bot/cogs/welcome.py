import json
import logging
import os

import discord
from discord import app_commands
from discord.ext import commands

import config
from cogs.manage import admin_check

log = logging.getLogger(__name__)

_DATA_FILE = os.path.join(os.path.dirname(__file__), '..', 'data', 'welcome.json')

_DEFAULT_MSG = (
    'สู่เซิร์ฟเวอร์ **{guild}**!\n\n'
    'เพื่อเริ่มต้นการผจญภัย กรุณาอ่านกฎที่ {rules_channel} '
    'ก่อนเริ่มใช้งาน ⚔️'
)

_cfg: dict = {}


def _load() -> None:
    global _cfg
    try:
        with open(_DATA_FILE, encoding='utf-8') as f:
            _cfg = json.load(f)
    except (FileNotFoundError, json.JSONDecodeError):
        _cfg = {}


def _save() -> None:
    os.makedirs(os.path.dirname(_DATA_FILE), exist_ok=True)
    with open(_DATA_FILE, 'w', encoding='utf-8') as f:
        json.dump(_cfg, f, ensure_ascii=False, indent=2)


def _welcome_message() -> str:
    return _cfg.get('message') or _DEFAULT_MSG


def _build_welcome_embed(member: discord.Member) -> discord.Embed:
    guild = member.guild

    register_channel = guild.get_channel(config.REGISTER_CHANNEL_ID) if config.REGISTER_CHANNEL_ID else None
    reg_mention = register_channel.mention if register_channel else '`#register`'

    download_channel = guild.get_channel(config.DOWNLOAD_CHANNEL_ID) if config.DOWNLOAD_CHANNEL_ID else None
    dl_mention = download_channel.mention if download_channel else '`#download`'

    rules_channel = guild.get_channel(config.RULES_CHANNEL_ID) if config.RULES_CHANNEL_ID else None
    rules_mention = rules_channel.mention if rules_channel else '`#rules`'

    msg = _welcome_message().format(
        guild=guild.name,
        member=member.mention,
        register_channel=reg_mention,
        download_channel=dl_mention,
        rules_channel=rules_mention,
    )

    embed = discord.Embed(
        description=msg,
        color=discord.Color.from_rgb(26, 30, 58),
    )
    embed.set_author(
        name=f'ยินดีต้อนรับ {member.display_name}!',
        icon_url=member.display_avatar.url,
    )
    embed.set_thumbnail(url=member.display_avatar.url)
    embed.add_field(
        name='📋 ขั้นตอนถัดไป',
        value=(
            f'1️⃣ ไปที่ {reg_mention}\n'
            '2️⃣ กดปุ่ม **สมัครสมาชิก** เพื่อสร้างบัญชีเกม\n'
            f'3️⃣ ดาวน์โหลดไคลเอนต์ได้ที่ {dl_mention} แล้วเข้าเล่นได้เลย!\n'
            f'4️⃣ หากต้องการเปลี่ยน Password สามารถทำได้ที่ {reg_mention}\n'
            f'5️⃣ หากพบปัญหาในการเข้าสู่ระบบ สามารถติดต่อ @Game Master ได้เลย'
        ),
        inline=False,
    )
    embed.set_footer(text=f'สมาชิกคนที่ {guild.member_count:,}')
    return embed


async def _send_welcome_dm(member: discord.Member) -> bool:
    """ส่ง DM ต้อนรับ คืน True ถ้าสำเร็จ"""
    try:
        embed = _build_welcome_embed(member)
        await member.send(embed=embed)
        return True
    except discord.Forbidden:
        log.info('Cannot DM welcome to %s (DMs disabled)', member)
        return False
    except Exception:
        log.exception('Failed to send welcome DM to %s', member)
        return False


# ── Cog ───────────────────────────────────────────────────────────────────────

welcome_group = app_commands.Group(name='welcome', description='ตั้งค่าข้อความต้อนรับ')


@welcome_group.command(name='set', description='[ADMIN] ตั้งค่าข้อความต้อนรับ (ส่งเป็น DM)')
@app_commands.describe(
    message='ข้อความ custom (ใช้ {member} {guild} {rules_channel} {register_channel} {download_channel})',
)
@admin_check()
async def welcome_set(interaction: discord.Interaction, message: str = ''):
    if message:
        _cfg['message'] = message
    elif 'message' in _cfg:
        del _cfg['message']
    _save()

    embed = discord.Embed(
        title='✅ ตั้งค่าข้อความต้อนรับสำเร็จ',
        color=discord.Color.green(),
    )
    embed.add_field(
        name='ข้อความ',
        value='ค่าเริ่มต้น' if not message else message[:200],
        inline=False,
    )
    embed.set_footer(text=f'โดย {interaction.user} • ส่งเป็น DM')
    await interaction.response.send_message(embed=embed, ephemeral=True)


@welcome_group.command(name='test', description='[ADMIN] ทดสอบส่ง DM ต้อนรับมาหาตัวเอง')
@admin_check()
async def welcome_test(interaction: discord.Interaction):
    await interaction.response.defer(ephemeral=True)
    ok = await _send_welcome_dm(interaction.user)
    if ok:
        await interaction.followup.send('✅ ส่ง DM ต้อนรับมาหาคุณแล้ว ตรวจสอบ DM ได้เลย', ephemeral=True)
    else:
        await interaction.followup.send(
            '❌ ส่ง DM ไม่ได้ กรุณาเปิดการรับ DM จากสมาชิกในเซิร์ฟเวอร์นี้ก่อน', ephemeral=True
        )


@welcome_group.command(name='info', description='ดูการตั้งค่าปัจจุบัน')
@admin_check()
async def welcome_info(interaction: discord.Interaction):
    embed = discord.Embed(title='⚙️ การตั้งค่าข้อความต้อนรับ', color=discord.Color.blurple())
    embed.add_field(name='รูปแบบ', value='📩 ส่งเป็น DM ส่วนตัว', inline=True)
    embed.add_field(name='ข้อความ', value=_welcome_message()[:300], inline=False)
    await interaction.response.send_message(embed=embed, ephemeral=True)


class WelcomeCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot
        _load()
        bot.tree.add_command(welcome_group)

    @commands.Cog.listener()
    async def on_member_join(self, member: discord.Member):
        await _send_welcome_dm(member)


async def setup(bot: commands.Bot):
    await bot.add_cog(WelcomeCog(bot))
