import random
import re
import time

import discord
from discord import app_commands
from discord.ext import commands

import config
from utils.db import get_pool

USER_RE = re.compile(r'^[A-Za-z0-9]{6,32}$')


def _validate_password(pw: str) -> bool:
    return (
        6 <= len(pw) <= 32
        and ' ' not in pw
        and any(c.islower() for c in pw)
        and any(c.isupper() for c in pw)
        and any(c.isdigit() for c in pw)
    )


_pending: dict[int, dict] = {}


# ── Modal ─────────────────────────────────────────────────────────────────────

class RegisterModal(discord.ui.Modal, title='สมัครสมาชิก'):
    username = discord.ui.TextInput(
        label='Username',
        placeholder='ตัวอักษร a-z A-Z 0-9 ความยาว 6-32 ตัว ห้ามมีช่องว่าง',
        min_length=6, max_length=32,
    )
    password = discord.ui.TextInput(
        label='Password',
        placeholder='6-32 ตัว พิมพ์เล็ก+ใหญ่+ตัวเลข ห้ามมีช่องว่าง',
        style=discord.TextStyle.short, min_length=6, max_length=32,
    )
    confirm_password = discord.ui.TextInput(
        label='Confirm Password',
        placeholder='กรอก Password อีกครั้ง',
        style=discord.TextStyle.short, min_length=6, max_length=32,
    )
    sex = discord.ui.TextInput(
        label='เพศ (M = ชาย / F = หญิง)',
        placeholder='M หรือ F',
        min_length=1, max_length=1,
    )

    async def on_submit(self, interaction: discord.Interaction):
        uid  = self.username.value.strip()
        pw   = self.password.value
        pw2  = self.confirm_password.value
        sx   = self.sex.value.strip().upper()

        if not USER_RE.match(uid):
            return await interaction.response.send_message(
                '❌ **Username** ใช้ตัวอักษร a-z A-Z 0-9 เท่านั้น ความยาว 6–32 ตัว ห้ามมีช่องว่าง',
                ephemeral=True,
            )
        if not _validate_password(pw):
            return await interaction.response.send_message(
                '❌ **Password** ต้องมีความยาว 6–32 ตัว ประกอบด้วย **พิมพ์เล็ก + พิมพ์ใหญ่ + ตัวเลข** และห้ามมีช่องว่าง',
                ephemeral=True,
            )
        if pw != pw2:
            return await interaction.response.send_message(
                '❌ **Confirm Password** ไม่ตรงกับ Password กรุณากรอกใหม่',
                ephemeral=True,
            )
        if sx not in ('M', 'F'):
            return await interaction.response.send_message(
                '❌ **เพศ** ต้องเป็น `M` (ชาย) หรือ `F` (หญิง) เท่านั้น',
                ephemeral=True,
            )

        discord_id = interaction.user.id
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute('SELECT COUNT(*) FROM discord_links WHERE discord_id = %s', (discord_id,))
                (count,) = await cur.fetchone()
                if count >= 3:
                    return await interaction.response.send_message(
                        '❌ Discord ของคุณสมัครบัญชีครบ **3 ID** แล้ว', ephemeral=True
                    )
                await cur.execute('SELECT 1 FROM `login` WHERE userid = %s', (uid,))
                if await cur.fetchone():
                    return await interaction.response.send_message(
                        f'❌ Username **{uid}** ถูกใช้งานแล้ว กรุณาเลือกชื่ออื่น', ephemeral=True
                    )

        _pending[discord_id] = {
            'username':  uid,
            'password':  pw,
            'sex':       sx,
            'acc_count': count,
        }

        code = [random.randint(0, 9) for _ in range(4)]
        view = RegCaptchaView(discord_id, code)
        await interaction.response.send_message(
            embed=view.build_embed(),
            view=view,
            ephemeral=True,
        )


# ── CAPTCHA → สร้างบัญชี ──────────────────────────────────────────────────────

class RegCaptchaView(discord.ui.View):
    _LAYOUT = [
        [1, 2, 3],
        [4, 5, 6],
        [7, 8, 9],
    ]

    def __init__(self, discord_id: int, code: list[int]):
        super().__init__(timeout=120)
        self.discord_id = discord_id
        self.code       = code
        self.entered: list[int] = []

        for row_idx, row in enumerate(self._LAYOUT):
            for d in row:
                btn = discord.ui.Button(label=str(d), style=discord.ButtonStyle.primary, row=row_idx)
                btn.callback = self._make_cb(d)
                self.add_item(btn)

        back = discord.ui.Button(label='⌫', style=discord.ButtonStyle.secondary, row=3)
        back.callback = self._back_cb
        self.add_item(back)

        zero = discord.ui.Button(label='0', style=discord.ButtonStyle.primary, row=3)
        zero.callback = self._make_cb(0)
        self.add_item(zero)

        ok = discord.ui.Button(label='✅', style=discord.ButtonStyle.success, row=3)
        ok.callback = self._ok_cb
        self.add_item(ok)

    def build_embed(self) -> discord.Embed:
        code_str    = '  '.join(str(d) for d in self.code)
        entered_str = '  '.join(str(self.entered[i]) if i < len(self.entered) else '＿'
                                for i in range(len(self.code)))
        embed = discord.Embed(
            title='🔒 ยืนยันตัวตน — CAPTCHA',
            description=(
                f'กรอกรหัสที่แสดงด้านล่างโดยใช้ numpad\n\n'
                f'```\n'
                f'รหัส  :  {code_str}\n'
                f'กรอก  :  {entered_str}\n'
                f'```'
            ),
            color=discord.Color.gold(),
        )
        return embed

    def _make_cb(self, digit: int):
        async def cb(interaction: discord.Interaction):
            if len(self.entered) < len(self.code):
                self.entered.append(digit)
            await interaction.response.edit_message(embed=self.build_embed(), view=self)
        return cb

    async def _back_cb(self, interaction: discord.Interaction):
        if self.entered:
            self.entered.pop()
        await interaction.response.edit_message(embed=self.build_embed(), view=self)

    async def _ok_cb(self, interaction: discord.Interaction):
        if len(self.entered) < len(self.code):
            embed = self.build_embed()
            embed.set_footer(text='⚠️ กรุณากรอกให้ครบก่อนกด ✅')
            return await interaction.response.edit_message(embed=embed, view=self)
        if self.entered != self.code:
            self.entered.clear()
            embed = self.build_embed()
            embed.color = discord.Color.red()
            embed.set_footer(text='❌ รหัสไม่ถูกต้อง กรุณากรอกใหม่')
            return await interaction.response.edit_message(embed=embed, view=self)

        for child in self.children:
            child.disabled = True
        self.stop()
        await self._create_account(interaction)

    async def _create_account(self, interaction: discord.Interaction):
        data = _pending.pop(self.discord_id, None)
        if not data:
            return await interaction.response.edit_message(
                content='❌ Session หมดอายุ กรุณาเริ่มใหม่', embed=None, view=None,
            )

        try:
            pool = get_pool()
            async with pool.acquire() as conn:
                async with conn.cursor() as cur:
                    await cur.execute(
                        'SELECT COUNT(*) FROM discord_links WHERE discord_id = %s', (self.discord_id,)
                    )
                    (count,) = await cur.fetchone()
                    if count >= 3:
                        return await interaction.response.edit_message(
                            content='❌ Discord ของคุณสมัครบัญชีครบ 3 ID แล้ว', embed=None, view=None,
                        )
                    await cur.execute('SELECT 1 FROM `login` WHERE userid = %s', (data['username'],))
                    if await cur.fetchone():
                        return await interaction.response.edit_message(
                            content=f'❌ Username **{data["username"]}** ถูกใช้งานแล้ว', embed=None, view=None,
                        )
                    await cur.execute(
                        'INSERT INTO `login` (userid, user_pass, sex) VALUES (%s, %s, %s)',
                        (data['username'], data['password'], data['sex']),
                    )
                    await cur.execute('SELECT LAST_INSERT_ID()')
                    (account_id,) = await cur.fetchone()
                    await cur.execute(
                        'INSERT INTO discord_links (discord_id, account_id) VALUES (%s, %s)',
                        (self.discord_id, account_id),
                    )
        except Exception as e:
            return await interaction.response.edit_message(
                content=f'❌ เกิดข้อผิดพลาด: {e}', embed=None, view=None,
            )

        if config.VERIFIED_ROLE_ID and interaction.guild:
            role = interaction.guild.get_role(config.VERIFIED_ROLE_ID)
            if role:
                try:
                    await interaction.user.add_roles(role, reason='Auto-verify after register')
                except discord.Forbidden:
                    pass

        sex_label = '🚹 ชาย' if data['sex'] == 'M' else '🚺 หญิง'
        embed = discord.Embed(title='✅ สมัครสมาชิกสำเร็จ!', color=discord.Color.green())
        embed.add_field(name='Username', value=f'`{data["username"]}`', inline=True)
        embed.add_field(name='เพศ', value=sex_label, inline=True)
        embed.add_field(name='บัญชี', value=f'{data["acc_count"] + 1}/3', inline=True)
        embed.set_footer(text='สามารถล็อกอินเกมได้ทันที')
        await interaction.response.edit_message(content=None, embed=embed, view=None)

    async def on_timeout(self):
        _pending.pop(self.discord_id, None)


# ── Change Password Modal ─────────────────────────────────────────────────────

class ChangePasswordModal(discord.ui.Modal, title='เปลี่ยนรหัสผ่าน'):
    username     = discord.ui.TextInput(label='Username', max_length=32)
    old_password = discord.ui.TextInput(label='รหัสผ่านเดิม', style=discord.TextStyle.short, max_length=32)
    new_password = discord.ui.TextInput(
        label='รหัสผ่านใหม่ (พิมพ์เล็ก+ใหญ่+ตัวเลข 6-32 ตัว)',
        style=discord.TextStyle.short, min_length=6, max_length=32,
    )

    async def on_submit(self, interaction: discord.Interaction):
        uid = self.username.value.strip()
        old = self.old_password.value
        new = self.new_password.value

        if not _validate_password(new):
            return await interaction.response.send_message(
                '❌ รหัสผ่านใหม่ต้องมีความยาว 6–32 ตัว ประกอบด้วย **พิมพ์เล็ก + พิมพ์ใหญ่ + ตัวเลข** และห้ามมีช่องว่าง',
                ephemeral=True,
            )

        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    'SELECT account_id FROM `login` WHERE userid = %s AND user_pass = %s',
                    (uid, old),
                )
                if not await cur.fetchone():
                    return await interaction.response.send_message(
                        '❌ Username หรือรหัสผ่านเดิมไม่ถูกต้อง', ephemeral=True
                    )
                await cur.execute(
                    'UPDATE `login` SET user_pass = %s WHERE userid = %s',
                    (new, uid),
                )

        await interaction.response.send_message('✅ เปลี่ยนรหัสผ่านสำเร็จแล้ว!', ephemeral=True)


# ── Cog ───────────────────────────────────────────────────────────────────────

class AccountCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot

    @app_commands.command(name='register', description='สมัครบัญชีเกมใหม่')
    async def register(self, interaction: discord.Interaction):
        await interaction.response.send_modal(RegisterModal())

    @app_commands.command(name='changepassword', description='เปลี่ยนรหัสผ่านบัญชีเกม')
    async def changepassword(self, interaction: discord.Interaction):
        await interaction.response.send_modal(ChangePasswordModal())

    @app_commands.command(name='unlink', description='ยกเลิกการผูก Discord กับบัญชีเกม')
    @app_commands.describe(username='ระบุ username ที่ต้องการยกเลิก (ถ้าไม่ระบุ จะยกเลิกทั้งหมด)')
    async def unlink(self, interaction: discord.Interaction, username: str = None):
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                if username:
                    await cur.execute(
                        'DELETE d FROM discord_links d '
                        'JOIN `login` l ON d.account_id = l.account_id '
                        'WHERE d.discord_id = %s AND l.userid = %s',
                        (interaction.user.id, username.strip()),
                    )
                    if conn.affected_rows() == 0:
                        return await interaction.response.send_message(
                            f'❌ ไม่พบบัญชี **{username}** ที่ผูกกับ Discord ของคุณ', ephemeral=True
                        )
                    await cur.execute(
                        'SELECT COUNT(*) FROM discord_links WHERE discord_id = %s', (interaction.user.id,)
                    )
                    (remaining,) = await cur.fetchone()
                    msg = f'✅ ยกเลิกการผูกบัญชี **{username}** สำเร็จ'
                else:
                    await cur.execute(
                        'DELETE FROM discord_links WHERE discord_id = %s', (interaction.user.id,)
                    )
                    if conn.affected_rows() == 0:
                        return await interaction.response.send_message(
                            '❌ ไม่พบการผูกบัญชีใดๆ', ephemeral=True
                        )
                    remaining = 0
                    msg = '✅ ยกเลิกการผูกบัญชีทั้งหมดสำเร็จ'

        if remaining == 0 and config.VERIFIED_ROLE_ID and interaction.guild:
            role = interaction.guild.get_role(config.VERIFIED_ROLE_ID)
            if role:
                try:
                    await interaction.user.remove_roles(role, reason='Discord Unlink')
                except discord.Forbidden:
                    pass

        await interaction.response.send_message(msg, ephemeral=True)

    @app_commands.command(name='whoami', description='แสดงบัญชีเกมที่ผูกกับ Discord ของคุณ')
    async def whoami(self, interaction: discord.Interaction):
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    'SELECT l.userid, l.group_id, l.logincount, l.lastlogin, l.vip_time '
                    'FROM discord_links d JOIN `login` l ON d.account_id = l.account_id '
                    'WHERE d.discord_id = %s ORDER BY d.linked_at ASC',
                    (interaction.user.id,),
                )
                rows = await cur.fetchall()

        if not rows:
            return await interaction.response.send_message(
                '❌ ยังไม่ได้ผูกบัญชีใดๆ — ใช้ /register เพื่อสมัครสมาชิก', ephemeral=True
            )

        embed = discord.Embed(
            title=f'👤 บัญชีเกมของคุณ ({len(rows)}/3)',
            color=discord.Color.from_rgb(70, 130, 180),
        )
        for i, (userid, group_id, logincount, lastlogin, vip_time) in enumerate(rows, 1):
            vip_str = 'ไม่มี' if not vip_time or vip_time < time.time() else f'<t:{vip_time}:R>'
            embed.add_field(
                name=f'#{i}  {userid}',
                value=(
                    f'GM Level: `{group_id}`\n'
                    f'เข้าเกม: `{logincount:,}` ครั้ง\n'
                    f'ล่าสุด: {lastlogin or "-"}\n'
                    f'VIP: {vip_str}'
                ),
                inline=True,
            )
        await interaction.response.send_message(embed=embed, ephemeral=True)

    @app_commands.command(name='accountinfo', description='ดูข้อมูลบัญชี (GM เท่านั้น)')
    @app_commands.describe(username='ชื่อบัญชีที่ต้องการดู')
    async def accountinfo(self, interaction: discord.Interaction, username: str):
        is_admin = False
        if interaction.guild:
            if config.ADMIN_ROLE_ID:
                is_admin = any(r.id == config.ADMIN_ROLE_ID for r in interaction.user.roles)
            else:
                is_admin = interaction.user.guild_permissions.administrator
        if not is_admin:
            return await interaction.response.send_message('❌ คุณไม่มีสิทธิ์ใช้คำสั่งนี้', ephemeral=True)

        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    'SELECT account_id, userid, sex, group_id, state, logincount, lastlogin, '
                    'last_ip, vip_time, expiration_time '
                    'FROM `login` WHERE userid = %s',
                    (username.strip(),),
                )
                row = await cur.fetchone()

        if not row:
            return await interaction.response.send_message('❌ ไม่พบบัญชีนี้', ephemeral=True)

        acc_id, uid, sex, grp, state, lcount, lastlogin, last_ip, vip_time, exp_time = row
        ban_str = '🔴 แบน' if state != 0 else '🟢 ปกติ'
        vip_str = 'ไม่มี' if not vip_time or vip_time < time.time() else f'<t:{vip_time}:f>'

        embed = discord.Embed(title=f'📋 Account: {uid}', color=discord.Color.blurple())
        embed.add_field(name='Account ID', value=str(acc_id),        inline=True)
        embed.add_field(name='เพศ',        value=sex,                inline=True)
        embed.add_field(name='GM Level',   value=str(grp),           inline=True)
        embed.add_field(name='สถานะ',      value=ban_str,            inline=True)
        embed.add_field(name='Login',      value=f'{lcount:,} ครั้ง', inline=True)
        embed.add_field(name='VIP',        value=vip_str,            inline=True)
        embed.add_field(name='Login ล่าสุด', value=str(lastlogin) if lastlogin else '-', inline=False)
        embed.add_field(name='IP ล่าสุด', value=last_ip or '-',     inline=True)
        await interaction.response.send_message(embed=embed, ephemeral=True)


async def setup(bot: commands.Bot):
    await bot.add_cog(AccountCog(bot))
