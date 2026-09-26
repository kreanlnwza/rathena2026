import json
import os
import random
import logging
import discord
from discord import app_commands
from discord.ext import commands
import config

DATA_PATH = os.path.join(os.path.dirname(__file__), '..', 'data', 'reaction_roles.json')
log = logging.getLogger(__name__)

# [{ "role_id": int, "label": str, "channel_id": int, "message_id": int }]
_buttons: list[dict] = []


def _load():
    global _buttons
    if os.path.exists(DATA_PATH):
        with open(DATA_PATH, 'r', encoding='utf-8') as f:
            data = json.load(f)
        if isinstance(data, list):
            _buttons = data
        else:
            # รูปแบบเก่า (reaction roles) — รีเซ็ต
            _buttons = []
            _save()


def _save():
    os.makedirs(os.path.dirname(DATA_PATH), exist_ok=True)
    with open(DATA_PATH, 'w', encoding='utf-8') as f:
        json.dump(_buttons, f, ensure_ascii=False, indent=2)


def _is_admin(interaction: discord.Interaction) -> bool:
    if not interaction.guild:
        return False
    if interaction.guild.owner_id == interaction.user.id:
        return True
    if not isinstance(interaction.user, discord.Member):
        return False
    if interaction.user.guild_permissions.administrator:
        return True
    if config.ADMIN_ROLE_ID:
        return any(r.id == config.ADMIN_ROLE_ID for r in interaction.user.roles)
    return False


def admin_check():
    async def predicate(interaction: discord.Interaction) -> bool:
        if _is_admin(interaction):
            return True
        await interaction.response.send_message('❌ คุณไม่มีสิทธิ์ใช้คำสั่งนี้', ephemeral=True)
        return False
    return app_commands.check(predicate)


# ── CAPTCHA numpad (ephemeral) ─────────────────────────────────────────────────

class CaptchaView(discord.ui.View):
    def __init__(self, code: list[int], role: discord.Role):
        super().__init__(timeout=120)
        self.code    = code
        self.entered: list[int] = []
        self.role    = role

        # สุ่มตำแหน่งตัวเลข 0–9 วางใน 4 แถว (3+3+3+1)
        digits = list(range(10))
        random.shuffle(digits)
        for i, d in enumerate(digits):
            btn = discord.ui.Button(
                label=str(d),
                style=discord.ButtonStyle.secondary,
                row=i // 3,
            )
            btn.callback = self._digit_cb(d)
            self.add_item(btn)

        # ปุ่ม ⌫ และ OK แถวสุดท้าย
        back_btn = discord.ui.Button(label='⌫', style=discord.ButtonStyle.danger, row=3)
        back_btn.callback = self._back_cb
        self.add_item(back_btn)

        ok_btn = discord.ui.Button(label='✅ OK', style=discord.ButtonStyle.success, row=3)
        ok_btn.callback = self._ok_cb
        self.add_item(ok_btn)

    def _display(self) -> str:
        cells = []
        for i in range(len(self.code)):
            if i < len(self.entered):
                cells.append(str(self.entered[i]))
            else:
                cells.append('_')
        return '  '.join(cells)

    def _content(self) -> str:
        return (
            '🤖 **กรุณายืนยันว่าคุณไม่ใช่บอท**\n'
            '```\n'
            f'รหัส  :  {" ".join(str(d) for d in self.code)}\n'
            f'กดแล้ว:  {self._display()}\n'
            '```'
        )

    def _digit_cb(self, digit: int):
        async def callback(interaction: discord.Interaction):
            if len(self.entered) < len(self.code):
                self.entered.append(digit)
            await interaction.response.edit_message(content=self._content(), view=self)
        return callback

    async def _back_cb(self, interaction: discord.Interaction):
        if self.entered:
            self.entered.pop()
        await interaction.response.edit_message(content=self._content(), view=self)

    async def _ok_cb(self, interaction: discord.Interaction):
        if len(self.entered) < len(self.code):
            return await interaction.response.edit_message(
                content=self._content() + '\n⚠️ กรุณากรอกให้ครบ 4 หลักก่อนกด OK',
                view=self,
            )

        if self.entered == self.code:
            # ถูกต้อง
            self._disable_all()
            self.stop()
            try:
                await interaction.user.add_roles(self.role, reason='Reaction Role (CAPTCHA)')
                await interaction.response.edit_message(
                    content=f'✅ ยืนยันสำเร็จ! คุณได้รับยศ **{self.role.name}** แล้ว',
                    view=self,
                )
            except discord.Forbidden:
                await interaction.response.edit_message(
                    content='❌ บอทไม่มีสิทธิ์มอบยศ กรุณาแจ้งผู้ดูแล',
                    view=self,
                )
        else:
            # ผิด — รีเซ็ตให้กรอกใหม่
            self.entered.clear()
            await interaction.response.edit_message(
                content=(
                    '❌ **รหัสไม่ถูกต้อง** กรุณากรอกใหม่อีกครั้ง\n'
                    '```\n'
                    f'รหัส  :  {" ".join(str(d) for d in self.code)}\n'
                    f'กดแล้ว:  {self._display()}\n'
                    '```'
                ),
                view=self,
            )

    def _disable_all(self):
        for child in self.children:
            child.disabled = True

    async def on_timeout(self):
        self._disable_all()
        try:
            await self.message.edit(
                content='⏰ CAPTCHA หมดเวลา กรุณากดปุ่มรับยศใหม่อีกครั้ง',
                view=self,
            )
        except Exception:
            pass


# ── Role Button (persistent, ทุกคนเห็น) ──────────────────────────────────────

class RoleClaimButton(discord.ui.Button):
    def __init__(self, role_id: int, label: str):
        super().__init__(
            label=label,
            style=discord.ButtonStyle.primary,
            custom_id=f'rr_role_{role_id}',
        )
        self.role_id = role_id

    async def callback(self, interaction: discord.Interaction):
        role = interaction.guild.get_role(self.role_id)
        if not role:
            return await interaction.response.send_message('❌ ไม่พบยศนี้ กรุณาแจ้งผู้ดูแล', ephemeral=True)

        if role in interaction.user.roles:
            return await interaction.response.send_message(
                f'⚠️ คุณมียศ **{role.name}** อยู่แล้ว', ephemeral=True
            )

        code = [random.randint(0, 9) for _ in range(4)]
        view = CaptchaView(code, role)
        await interaction.response.send_message(
            content=view._content(),
            view=view,
            ephemeral=True,
        )


class PersistentRoleView(discord.ui.View):
    def __init__(self, role_id: int, label: str):
        super().__init__(timeout=None)
        self.add_item(RoleClaimButton(role_id, label))


# ── Slash commands ─────────────────────────────────────────────────────────────

rr_group = app_commands.Group(name='rr', description='ระบบ Role Button')


@rr_group.command(name='add', description='[ADMIN] สร้างปุ่มรับยศในห้อง')
@app_commands.describe(
    channel='ห้องที่จะส่งปุ่ม',
    role='ยศที่จะได้รับ',
    label='ข้อความบนปุ่ม',
    description='คำอธิบายเหนือปุ่ม (ไม่บังคับ)',
)
@admin_check()
async def rr_add(
    interaction: discord.Interaction,
    channel: discord.TextChannel,
    role: discord.Role,
    label: str = '🎮 รับยศ',
    description: str = '# กดปุ่มด้านล่างเพื่อรับยศ',
):
    await interaction.response.defer(ephemeral=True)

    view = PersistentRoleView(role.id, label)
    msg  = await channel.send(content=f'**{description}**', view=view)

    _buttons.append({
        'role_id':    role.id,
        'label':      label,
        'channel_id': channel.id,
        'message_id': msg.id,
    })
    _save()

    embed = discord.Embed(title='✅ สร้างปุ่มรับยศสำเร็จ', color=discord.Color.green())
    embed.add_field(name='ห้อง',  value=channel.mention,                     inline=True)
    embed.add_field(name='ยศ',    value=role.mention,                        inline=True)
    embed.add_field(name='ปุ่ม',  value=label,                               inline=True)
    embed.add_field(name='ข้อความ', value=f'[ไปที่ข้อความ]({msg.jump_url})', inline=True)
    embed.set_footer(text=f'โดย {interaction.user}')
    await interaction.followup.send(embed=embed)


@rr_group.command(name='remove', description='[ADMIN] ลบปุ่มรับยศ')
@app_commands.describe(message_id='ID ของข้อความที่มีปุ่ม')
@admin_check()
async def rr_remove(interaction: discord.Interaction, message_id: str):
    await interaction.response.defer(ephemeral=True)

    entry = next((b for b in _buttons if str(b['message_id']) == message_id), None)
    if not entry:
        return await interaction.followup.send('⚠️ ไม่พบปุ่มนี้ในระบบ')

    _buttons.remove(entry)
    _save()

    channel = interaction.guild.get_channel(entry['channel_id'])
    if channel:
        try:
            msg = await channel.fetch_message(entry['message_id'])
            await msg.delete()
        except (discord.NotFound, discord.HTTPException):
            pass

    await interaction.followup.send('🗑️ ลบปุ่มรับยศสำเร็จแล้ว')


@rr_group.command(name='list', description='[ADMIN] แสดงปุ่มรับยศทั้งหมด')
@admin_check()
async def rr_list(interaction: discord.Interaction):
    if not _buttons:
        return await interaction.response.send_message('_ยังไม่มีปุ่มรับยศ_', ephemeral=True)

    lines = []
    for b in _buttons:
        role = interaction.guild.get_role(b['role_id'])
        role_str = role.mention if role else f'`ID:{b["role_id"]}`'
        lines.append(f'`msg:{b["message_id"]}` **{b["label"]}** → {role_str}')

    embed = discord.Embed(
        title=f'📋 ปุ่มรับยศ ({len(lines)} รายการ)',
        description='\n'.join(lines),
        color=discord.Color.blurple(),
    )
    await interaction.response.send_message(embed=embed, ephemeral=True)


# ── Cog ───────────────────────────────────────────────────────────────────────

class ReactionRolesCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot
        _load()
        bot.tree.add_command(rr_group)

        # ลงทะเบียน persistent views ทุกปุ่มที่มีอยู่
        for b in _buttons:
            bot.add_view(PersistentRoleView(b['role_id'], b['label']))


async def setup(bot: commands.Bot):
    await bot.add_cog(ReactionRolesCog(bot))
