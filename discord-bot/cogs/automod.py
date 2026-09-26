import json
import logging
import os
import re
from collections import defaultdict, deque
from datetime import datetime, timedelta
from typing import Optional
from urllib.parse import urlparse

import discord
from discord import app_commands
from discord.ext import commands

import config
from utils.checks import admin_check

log = logging.getLogger(__name__)

DATA_FILE = os.path.join(os.path.dirname(__file__), '..', 'data', 'automod.json')
URL_RE = re.compile(r'((?:https?://|www\.)[^\s<>()]+|discord\.gg/[^\s<>()]+)', re.IGNORECASE)
IMAGE_EXTENSIONS = {'.png', '.jpg', '.jpeg', '.gif', '.webp', '.bmp', '.tiff'}

FEATURE_CHOICES = [
    app_commands.Choice(name='กันสแปมข้อความ', value='message_spam_enabled'),
    app_commands.Choice(name='กันข้อความซ้ำ', value='duplicate_spam_enabled'),
    app_commands.Choice(name='กันลิงก์นอก allowlist', value='link_filter_enabled'),
    app_commands.Choice(name='กันรูปภาพสแปม', value='image_spam_enabled'),
]

LIMIT_CHOICES = [
    app_commands.Choice(name='ข้อความสูงสุดต่อช่วงเวลา', value='spam_max_messages'),
    app_commands.Choice(name='ช่วงเวลาเช็กสแปมข้อความ (วินาที)', value='spam_interval_seconds'),
    app_commands.Choice(name='จำนวนข้อความซ้ำสูงสุด', value='duplicate_max_messages'),
    app_commands.Choice(name='ช่วงเวลาเช็กข้อความซ้ำ (วินาที)', value='duplicate_interval_seconds'),
    app_commands.Choice(name='จำนวนรูปสูงสุดต่อ 1 ข้อความ', value='image_max_attachments_per_message'),
    app_commands.Choice(name='จำนวนข้อความรูปสูงสุดต่อช่วงเวลา', value='image_spam_max_messages'),
    app_commands.Choice(name='ช่วงเวลาเช็กรูปสแปม (วินาที)', value='image_spam_interval_seconds'),
    app_commands.Choice(name='Timeout เมื่อโดนซ้ำ (นาที)', value='timeout_minutes'),
]

DEFAULT_SETTINGS = {
    'message_spam_enabled': True,
    'duplicate_spam_enabled': True,
    'link_filter_enabled': True,
    'image_spam_enabled': True,
    'spam_max_messages': 5,
    'spam_interval_seconds': 8,
    'duplicate_max_messages': 3,
    'duplicate_interval_seconds': 20,
    'image_max_attachments_per_message': 4,
    'image_spam_max_messages': 3,
    'image_spam_interval_seconds': 15,
    'timeout_minutes': 30,
    'log_channel_id': 0,
    'allowed_domains': [
        'discord.com',
        'discord.gg',
        'discordapp.com',
        'youtube.com',
        'youtu.be',
        'facebook.com',
        'fb.watch',
    ],
    'exempt_channel_ids': [],
    'exempt_role_ids': [],
}

automod_group = app_commands.Group(name='automod', description='ตั้งค่าระบบป้องกันสแปมและลิงก์แปลก')


def _load_data() -> dict:
    try:
        with open(DATA_FILE, 'r', encoding='utf-8') as f:
            data = json.load(f)
            return data if isinstance(data, dict) else {}
    except (FileNotFoundError, json.JSONDecodeError):
        return {}


def _save_data(data: dict) -> None:
    os.makedirs(os.path.dirname(DATA_FILE), exist_ok=True)
    with open(DATA_FILE, 'w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)


def _normalize_domain(domain_or_url: str) -> Optional[str]:
    raw = domain_or_url.strip().lower()
    if not raw:
        return None
    if raw.startswith('discord.gg/'):
        raw = f'https://{raw}'
    elif '://' not in raw:
        raw = f'https://{raw}'

    parsed = urlparse(raw)
    domain = parsed.netloc.lower() or parsed.path.lower().split('/')[0]
    if domain.startswith('www.'):
        domain = domain[4:]
    return domain or None


def _normalize_content(content: str) -> str:
    normalized = re.sub(r'\s+', ' ', content.lower()).strip()
    return normalized[:300]


def _is_image_attachment(attachment: discord.Attachment) -> bool:
    if attachment.content_type and attachment.content_type.startswith('image/'):
        return True
    _, ext = os.path.splitext(attachment.filename.lower())
    return ext in IMAGE_EXTENSIONS


def _domain_allowed(domain: str, allowed_domains: list[str]) -> bool:
    for allowed in allowed_domains:
        if domain == allowed or domain.endswith(f'.{allowed}'):
            return True
    return False


class AutoModCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot
        self.data = _load_data()
        self.message_history = defaultdict(deque)
        self.duplicate_history = defaultdict(deque)
        self.image_history = defaultdict(deque)
        bot.tree.add_command(automod_group)

    def _get_settings(self, guild_id: int) -> dict:
        key = str(guild_id)
        settings = dict(DEFAULT_SETTINGS)
        settings.update(self.data.get(key, {}))
        settings['allowed_domains'] = sorted(
            {d for d in settings.get('allowed_domains', []) if d},
            key=str.casefold,
        )
        settings['exempt_channel_ids'] = [int(v) for v in settings.get('exempt_channel_ids', [])]
        settings['exempt_role_ids'] = [int(v) for v in settings.get('exempt_role_ids', [])]
        return settings

    def _save_settings(self, guild_id: int, settings: dict) -> None:
        key = str(guild_id)
        serializable = dict(settings)
        serializable['allowed_domains'] = sorted(serializable.get('allowed_domains', []), key=str.casefold)
        serializable['exempt_channel_ids'] = sorted(set(serializable.get('exempt_channel_ids', [])))
        serializable['exempt_role_ids'] = sorted(set(serializable.get('exempt_role_ids', [])))
        self.data[key] = serializable
        _save_data(self.data)

    def _is_exempt(self, message: discord.Message, settings: dict) -> bool:
        if not isinstance(message.author, discord.Member):
            return True
        if is_admin_from_message(message):
            return True
        if message.channel.id in settings['exempt_channel_ids']:
            return True
        member_role_ids = {role.id for role in message.author.roles}
        return any(role_id in member_role_ids for role_id in settings['exempt_role_ids'])

    def _extract_blocked_domains(self, message: discord.Message, settings: dict) -> list[str]:
        blocked = []
        for match in URL_RE.findall(message.content or ''):
            domain = _normalize_domain(match)
            if not domain:
                continue
            if not _domain_allowed(domain, settings['allowed_domains']):
                blocked.append(domain)
        return sorted(set(blocked))

    @commands.Cog.listener()
    async def on_message(self, message: discord.Message):
        if message.author.bot or not message.guild:
            return

        settings = self._get_settings(message.guild.id)
        if self._is_exempt(message, settings):
            return

        now = message.created_at.timestamp()
        reasons = []

        if settings['link_filter_enabled']:
            blocked_domains = self._extract_blocked_domains(message, settings)
            if blocked_domains:
                reasons.append(f'ลิงก์ไม่อยู่ใน allowlist: {", ".join(blocked_domains)}')

        if settings['message_spam_enabled']:
            msg_key = (message.guild.id, message.author.id)
            msg_times = self.message_history[msg_key]
            self._prune_deque(msg_times, now, settings['spam_interval_seconds'])
            msg_times.append(now)
            if len(msg_times) > settings['spam_max_messages']:
                reasons.append(
                    f'ส่งข้อความถี่เกินไป ({len(msg_times)} ข้อความใน {settings["spam_interval_seconds"]} วินาที)'
                )

        if settings['duplicate_spam_enabled'] and message.content.strip():
            content_key = (message.guild.id, message.author.id, _normalize_content(message.content))
            duplicate_times = self.duplicate_history[content_key]
            self._prune_deque(duplicate_times, now, settings['duplicate_interval_seconds'])
            duplicate_times.append(now)
            if len(duplicate_times) >= settings['duplicate_max_messages']:
                reasons.append(
                    f'ส่งข้อความซ้ำ {len(duplicate_times)} ครั้งใน {settings["duplicate_interval_seconds"]} วินาที'
                )

        image_count = sum(1 for attachment in message.attachments if _is_image_attachment(attachment))
        if settings['image_spam_enabled'] and image_count:
            if image_count > settings['image_max_attachments_per_message']:
                reasons.append(f'แนบรูปมากเกินไปในข้อความเดียว ({image_count} รูป)')

            image_key = (message.guild.id, message.author.id)
            image_times = self.image_history[image_key]
            self._prune_deque(image_times, now, settings['image_spam_interval_seconds'])
            image_times.append(now)
            if len(image_times) > settings['image_spam_max_messages']:
                reasons.append(
                    f'ส่งรูปถี่เกินไป ({len(image_times)} ข้อความรูปใน {settings["image_spam_interval_seconds"]} วินาที)'
                )

        if not reasons:
            return

        await self._handle_violation(message, settings, reasons)

    def _prune_deque(self, items: deque, now_ts: float, window_seconds: int) -> None:
        while items and now_ts - items[0] > window_seconds:
            items.popleft()

    async def _handle_violation(self, message: discord.Message, settings: dict, reasons: list[str]) -> None:
        deleted = False
        try:
            await message.delete()
            deleted = True
        except discord.HTTPException:
            log.warning('automod: failed to delete message %s', message.id, exc_info=True)

        timeout_applied = False
        timeout_until = None
        if isinstance(message.author, discord.Member):
            timeout_minutes = settings['timeout_minutes']
            if timeout_minutes > 0:
                timeout_until = discord.utils.utcnow() + timedelta(minutes=timeout_minutes)
                try:
                    await message.author.timeout(
                        timeout_until,
                        reason='AutoMod: ' + '; '.join(reasons)[:400],
                    )
                    timeout_applied = True
                except discord.HTTPException:
                    log.warning('automod: failed to timeout user %s', message.author.id, exc_info=True)

        try:
            await message.author.send(
                'ข้อความของคุณถูกลบโดยระบบ AutoMod\n'
                f'เหตุผล: {"; ".join(reasons)}'
                + (f'\nคุณถูก timeout {settings["timeout_minutes"]} นาทีชั่วคราว' if timeout_applied else '')
            )
        except discord.HTTPException:
            pass

        await self._send_log(message, settings, reasons, deleted, timeout_applied, timeout_until)

    async def _send_log(
        self,
        message: discord.Message,
        settings: dict,
        reasons: list[str],
        deleted: bool,
        timeout_applied: bool,
        timeout_until: Optional[datetime],
    ) -> None:
        channel_id = settings.get('log_channel_id') or 0
        if not channel_id:
            return

        log_channel = message.guild.get_channel(channel_id)
        if not isinstance(log_channel, discord.TextChannel):
            return

        embed = discord.Embed(
            title='🛡️ AutoMod จัดการข้อความ',
            color=discord.Color.orange(),
            timestamp=discord.utils.utcnow(),
        )
        embed.add_field(name='ผู้ใช้', value=f'{message.author.mention} (`{message.author.id}`)', inline=False)
        embed.add_field(name='ช่อง', value=message.channel.mention, inline=True)
        embed.add_field(name='ลบข้อความ', value='ใช่' if deleted else 'ไม่สำเร็จ', inline=True)
        embed.add_field(name='Timeout', value='ใช่' if timeout_applied else 'ไม่', inline=True)
        embed.add_field(name='เหตุผล', value='\n'.join(f'• {reason}' for reason in reasons)[:1024], inline=False)

        preview = message.content.strip() or '[ไม่มีข้อความ]'
        if len(preview) > 1000:
            preview = preview[:997] + '...'
        embed.add_field(name='ข้อความ', value=preview, inline=False)

        if timeout_applied and timeout_until:
            embed.set_footer(text=f'timeout until {timeout_until:%Y-%m-%d %H:%M:%S UTC}')

        try:
            await log_channel.send(embed=embed)
        except discord.HTTPException:
            log.warning('automod: failed to send log to channel %s', channel_id, exc_info=True)


def is_admin_from_message(message: discord.Message) -> bool:
    if not isinstance(message.author, discord.Member):
        return False
    if message.author.guild_permissions.administrator or message.author.id == message.guild.owner_id:
        return True
    if not config.ADMIN_ROLE_ID:
        return False
    return any(role.id == config.ADMIN_ROLE_ID for role in message.author.roles)


@automod_group.command(name='status', description='ดูสถานะ AutoMod ปัจจุบัน')
@admin_check()
async def automod_status(interaction: discord.Interaction):
    cog: AutoModCog = interaction.client.cogs.get('AutoModCog')
    settings = cog._get_settings(interaction.guild.id)

    lines = [
        f'กันสแปมข้อความ: {"เปิด" if settings["message_spam_enabled"] else "ปิด"}',
        f'กันข้อความซ้ำ: {"เปิด" if settings["duplicate_spam_enabled"] else "ปิด"}',
        f'กันลิงก์แปลก: {"เปิด" if settings["link_filter_enabled"] else "ปิด"}',
        f'กันรูปภาพสแปม: {"เปิด" if settings["image_spam_enabled"] else "ปิด"}',
        f'log channel: <#{settings["log_channel_id"]}>' if settings['log_channel_id'] else 'log channel: ยังไม่ได้ตั้ง',
        f'allowlist domains: {", ".join(settings["allowed_domains"][:12]) or "-"}',
    ]
    embed = discord.Embed(
        title='🛡️ AutoMod Status',
        description='\n'.join(lines),
        color=discord.Color.blurple(),
    )
    await interaction.response.send_message(embed=embed, ephemeral=True)


@automod_group.command(name='toggle', description='[ADMIN] เปิดหรือปิดฟีเจอร์ AutoMod')
@app_commands.describe(feature='ฟีเจอร์ที่ต้องการปรับ', enabled='เปิดหรือปิด')
@app_commands.choices(feature=FEATURE_CHOICES)
@admin_check()
async def automod_toggle(
    interaction: discord.Interaction,
    feature: app_commands.Choice[str],
    enabled: bool,
):
    cog: AutoModCog = interaction.client.cogs.get('AutoModCog')
    settings = cog._get_settings(interaction.guild.id)
    settings[feature.value] = enabled
    cog._save_settings(interaction.guild.id, settings)
    await interaction.response.send_message(
        f'✅ ตั้งค่า **{feature.name}** เป็น **{"เปิด" if enabled else "ปิด"}** แล้ว',
        ephemeral=True,
    )


@automod_group.command(name='set_limit', description='[ADMIN] ปรับ threshold ของ AutoMod')
@app_commands.describe(setting='ค่าที่ต้องการปรับ', value='ค่าตัวเลขใหม่')
@app_commands.choices(setting=LIMIT_CHOICES)
@admin_check()
async def automod_set_limit(
    interaction: discord.Interaction,
    setting: app_commands.Choice[str],
    value: app_commands.Range[int, 1, 300],
):
    cog: AutoModCog = interaction.client.cogs.get('AutoModCog')
    settings = cog._get_settings(interaction.guild.id)
    settings[setting.value] = value
    cog._save_settings(interaction.guild.id, settings)
    await interaction.response.send_message(
        f'✅ ปรับ **{setting.name}** เป็น `{value}` แล้ว',
        ephemeral=True,
    )


@automod_group.command(name='set_log_channel', description='[ADMIN] ตั้งค่าช่องแจ้งเตือนของ AutoMod')
@app_commands.describe(channel='ช่องที่ให้ AutoMod ส่ง log')
@admin_check()
async def automod_set_log_channel(interaction: discord.Interaction, channel: discord.TextChannel):
    cog: AutoModCog = interaction.client.cogs.get('AutoModCog')
    settings = cog._get_settings(interaction.guild.id)
    settings['log_channel_id'] = channel.id
    cog._save_settings(interaction.guild.id, settings)
    await interaction.response.send_message(
        f'✅ ตั้งค่า log channel เป็น {channel.mention} แล้ว',
        ephemeral=True,
    )


@automod_group.command(name='allow_domain_add', description='[ADMIN] เพิ่มโดเมนเข้า allowlist')
@app_commands.describe(domain='โดเมน เช่น yoursite.com')
@admin_check()
async def automod_allow_domain_add(interaction: discord.Interaction, domain: str):
    normalized = _normalize_domain(domain)
    if not normalized:
        return await interaction.response.send_message('❌ รูปแบบโดเมนไม่ถูกต้อง', ephemeral=True)

    cog: AutoModCog = interaction.client.cogs.get('AutoModCog')
    settings = cog._get_settings(interaction.guild.id)
    allowed = set(settings['allowed_domains'])
    allowed.add(normalized)
    settings['allowed_domains'] = sorted(allowed, key=str.casefold)
    cog._save_settings(interaction.guild.id, settings)
    await interaction.response.send_message(
        f'✅ เพิ่มโดเมน `{normalized}` เข้า allowlist แล้ว',
        ephemeral=True,
    )


@automod_group.command(name='allow_domain_remove', description='[ADMIN] ลบโดเมนออกจาก allowlist')
@app_commands.describe(domain='โดเมนที่ต้องการลบ')
@admin_check()
async def automod_allow_domain_remove(interaction: discord.Interaction, domain: str):
    normalized = _normalize_domain(domain)
    if not normalized:
        return await interaction.response.send_message('❌ รูปแบบโดเมนไม่ถูกต้อง', ephemeral=True)

    cog: AutoModCog = interaction.client.cogs.get('AutoModCog')
    settings = cog._get_settings(interaction.guild.id)
    allowed = set(settings['allowed_domains'])
    if normalized not in allowed:
        return await interaction.response.send_message(
            f'⚠️ ไม่พบ `{normalized}` ใน allowlist',
            ephemeral=True,
        )
    allowed.remove(normalized)
    settings['allowed_domains'] = sorted(allowed, key=str.casefold)
    cog._save_settings(interaction.guild.id, settings)
    await interaction.response.send_message(
        f'✅ ลบโดเมน `{normalized}` ออกจาก allowlist แล้ว',
        ephemeral=True,
    )


@automod_group.command(name='allow_domain_list', description='ดูรายการโดเมนที่อนุญาต')
@admin_check()
async def automod_allow_domain_list(interaction: discord.Interaction):
    cog: AutoModCog = interaction.client.cogs.get('AutoModCog')
    settings = cog._get_settings(interaction.guild.id)
    domains = settings['allowed_domains']
    embed = discord.Embed(
        title='🌐 AutoMod Allowlist',
        description='\n'.join(f'• {domain}' for domain in domains[:50]) or '_ไม่มีรายการ_',
        color=discord.Color.green(),
    )
    await interaction.response.send_message(embed=embed, ephemeral=True)


@automod_group.command(name='exempt_channel', description='[ADMIN] ยกเว้นหรือยกเลิกยกเว้นช่องทาง')
@app_commands.describe(channel='ช่องที่ต้องการตั้งค่า', enabled='true = ยกเว้น, false = เอาออกจากรายการยกเว้น')
@admin_check()
async def automod_exempt_channel(
    interaction: discord.Interaction,
    channel: discord.TextChannel,
    enabled: bool,
):
    cog: AutoModCog = interaction.client.cogs.get('AutoModCog')
    settings = cog._get_settings(interaction.guild.id)
    exempt_channels = set(settings['exempt_channel_ids'])
    if enabled:
        exempt_channels.add(channel.id)
    else:
        exempt_channels.discard(channel.id)
    settings['exempt_channel_ids'] = sorted(exempt_channels)
    cog._save_settings(interaction.guild.id, settings)
    await interaction.response.send_message(
        f'✅ {"ยกเว้น" if enabled else "ยกเลิกยกเว้น"} {channel.mention} แล้ว',
        ephemeral=True,
    )


@automod_group.command(name='exempt_role', description='[ADMIN] ยกเว้นหรือยกเลิกยกเว้น role')
@app_commands.describe(role='role ที่ต้องการตั้งค่า', enabled='true = ยกเว้น, false = เอาออกจากรายการยกเว้น')
@admin_check()
async def automod_exempt_role(
    interaction: discord.Interaction,
    role: discord.Role,
    enabled: bool,
):
    cog: AutoModCog = interaction.client.cogs.get('AutoModCog')
    settings = cog._get_settings(interaction.guild.id)
    exempt_roles = set(settings['exempt_role_ids'])
    if enabled:
        exempt_roles.add(role.id)
    else:
        exempt_roles.discard(role.id)
    settings['exempt_role_ids'] = sorted(exempt_roles)
    cog._save_settings(interaction.guild.id, settings)
    await interaction.response.send_message(
        f'✅ {"ยกเว้น" if enabled else "ยกเลิกยกเว้น"} role {role.mention} แล้ว',
        ephemeral=True,
    )


async def setup(bot: commands.Bot):
    await bot.add_cog(AutoModCog(bot))
