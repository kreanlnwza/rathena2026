import json
import logging
import os

import discord
from discord import app_commands
from discord.ext import commands

import config
from cogs.account import RegisterModal, ChangePasswordModal
from utils.checks import admin_check

log = logging.getLogger(__name__)

DATA_FILE = os.path.join(os.path.dirname(__file__), '..', 'data', 'register_panels.json')

_panels: list[dict] = []


def _load():
    global _panels
    try:
        with open(DATA_FILE, 'r', encoding='utf-8') as f:
            _panels = json.load(f)
    except (FileNotFoundError, json.JSONDecodeError):
        _panels = []


def _save():
    os.makedirs(os.path.dirname(DATA_FILE), exist_ok=True)
    with open(DATA_FILE, 'w', encoding='utf-8') as f:
        json.dump(_panels, f, ensure_ascii=False, indent=2)


# ── Persistent Buttons ────────────────────────────────────────────────────────

class RegisterButton(discord.ui.Button):
    def __init__(self):
        super().__init__(
            label='สมัครสมาชิก',
            style=discord.ButtonStyle.success,
            custom_id='reg_panel:register',
            emoji='⚔️',
        )

    async def callback(self, interaction: discord.Interaction):
        await interaction.response.send_modal(RegisterModal())


class ChangePasswordButton(discord.ui.Button):
    def __init__(self):
        super().__init__(
            label='เปลี่ยนรหัสผ่าน',
            style=discord.ButtonStyle.secondary,
            custom_id='reg_panel:changepassword',
            emoji='🔑',
        )

    async def callback(self, interaction: discord.Interaction):
        await interaction.response.send_modal(ChangePasswordModal())


class RegisterPanelView(discord.ui.View):
    def __init__(self):
        super().__init__(timeout=None)
        self.add_item(RegisterButton())
        self.add_item(ChangePasswordButton())


# ── Panel embed ───────────────────────────────────────────────────────────────

def _build_embed(description: str) -> discord.Embed:
    embed = discord.Embed(
        color=discord.Color.from_rgb(26, 30, 58),
    )

    embed.set_author(
        name='ƒɾìҽɾҽղ ༺ Ai ༻',
        icon_url=config.PANEL_AUTHOR_ICON_URL or None,
    )

    embed.description = (
        '**Register System**\n\n'
        f'{description}\n​'
    )

    embed.add_field(
        name='🔤  Username',
        value='ใช้ตัวอักษร a-z A-Z 0-9 เท่านั้น ความยาว 6-32 ตัว ห้ามมีช่องว่าง',
        inline=False,
    )
    embed.add_field(
        name='🔒  Password',
        value='ความยาว 6-32 ตัว ต้องมีพิมพ์เล็ก + พิมพ์ใหญ่ + ตัวเลข ห้ามมีช่องว่าง',
        inline=False,
    )
    embed.add_field(
        name='⚧  เพศ',
        value='`M` = ชาย ♂️ | `F` = หญิง ♀️',
        inline=False,
    )
    embed.add_field(
        name='🛡️  Security',
        value='ทีมงานจะไม่ถามรหัสผ่านของคุณผ่านแชทหรือ DM',
        inline=False,
    )

    if config.PANEL_IMAGE_URL:
        embed.set_image(url=config.PANEL_IMAGE_URL)

    embed.set_footer(text='กดปุ่มด้านล่างเพื่อเปิดหน้าต่างสมัครสมาชิก')
    return embed


# ── Slash commands ────────────────────────────────────────────────────────────

regpanel_group = app_commands.Group(name='regpanel', description='ระบบ Register Panel')


@regpanel_group.command(name='create', description='[ADMIN] สร้างแผงปุ่มสมัครบัญชีในช่องทาง')
@app_commands.describe(
    channel='ช่องทางที่จะส่งแผงปุ่ม',
    description='คำอธิบายเหนือแผงปุ่ม',
)
@admin_check()
async def regpanel_create(
    interaction: discord.Interaction,
    channel: discord.TextChannel,
    description: str = 'พร้อมเริ่มต้นการผจญภัยแล้วหรือยัง ?\nกดปุ่มด้านล่างเพื่อเปิดหน้าต่างสมัครสมาชิก ระบบจะสร้างบัญชีเกมให้ทันทีหลังจากกรอกข้อมูลถูกต้อง',
):
    await interaction.response.defer(ephemeral=True)

    view = RegisterPanelView()
    msg = await channel.send(embed=_build_embed(description), view=view)

    _panels.append({'channel_id': channel.id, 'message_id': msg.id})
    _save()

    embed = discord.Embed(title='✅ สร้าง Register Panel สำเร็จ', color=discord.Color.green())
    embed.add_field(name='ช่องทาง', value=channel.mention, inline=True)
    embed.add_field(name='ข้อความ', value=f'[ไปที่แผงปุ่ม]({msg.jump_url})', inline=True)
    embed.set_footer(text=f'โดย {interaction.user}')
    await interaction.followup.send(embed=embed)


@regpanel_group.command(name='remove', description='[ADMIN] ลบแผงปุ่มสมัครบัญชี')
@app_commands.describe(message_id='ID ของข้อความที่มีแผงปุ่ม')
@admin_check()
async def regpanel_remove(interaction: discord.Interaction, message_id: str):
    await interaction.response.defer(ephemeral=True)

    entry = next((p for p in _panels if str(p['message_id']) == message_id), None)
    if not entry:
        return await interaction.followup.send('⚠️ ไม่พบแผงปุ่มนี้ในระบบ')

    _panels.remove(entry)
    _save()

    channel = interaction.guild.get_channel(entry['channel_id'])
    if channel:
        try:
            msg = await channel.fetch_message(entry['message_id'])
            await msg.delete()
        except (discord.NotFound, discord.HTTPException):
            pass

    await interaction.followup.send('🗑️ ลบ Register Panel สำเร็จแล้ว')


@regpanel_group.command(name='list', description='[ADMIN] แสดงแผงปุ่มทั้งหมด')
@admin_check()
async def regpanel_list(interaction: discord.Interaction):
    if not _panels:
        return await interaction.response.send_message('_ยังไม่มี Register Panel_', ephemeral=True)

    lines = []
    for p in _panels:
        ch = interaction.guild.get_channel(p['channel_id'])
        ch_str = ch.mention if ch else f'`ID:{p["channel_id"]}`'
        lines.append(f'`msg:{p["message_id"]}` → {ch_str}')

    embed = discord.Embed(
        title=f'📋 Register Panel ({len(lines)} รายการ)',
        description='\n'.join(lines),
        color=discord.Color.blurple(),
    )
    await interaction.response.send_message(embed=embed, ephemeral=True)


# ── Cog ───────────────────────────────────────────────────────────────────────

class RegisterPanelCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot
        _load()
        bot.tree.add_command(regpanel_group)
        bot.add_view(RegisterPanelView())


async def setup(bot: commands.Bot):
    await bot.add_cog(RegisterPanelCog(bot))
