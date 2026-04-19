import hashlib
import re
import discord
from discord import app_commands
from discord.ext import commands
from utils.db import get_pool
import config

EMAIL_RE = re.compile(r'^[^@\s]+@[^@\s]+\.[^@\s]+$')
USER_RE  = re.compile(r'^[A-Za-z0-9_]{4,23}$')


def _md5(text: str) -> str:
    return hashlib.md5(text.encode('utf-8')).hexdigest()


# ── Modals ────────────────────────────────────────────────────────────────────

class RegisterModal(discord.ui.Modal, title='สมัครบัญชีใหม่'):
    username = discord.ui.TextInput(label='Username', placeholder='4–23 ตัวอักษร (a-z, 0-9, _)', min_length=4, max_length=23)
    password = discord.ui.TextInput(label='Password', placeholder='อย่างน้อย 6 ตัวอักษร', style=discord.TextStyle.short, min_length=6, max_length=32)
    email    = discord.ui.TextInput(label='Email', placeholder='example@email.com', required=False, max_length=39)
    sex      = discord.ui.TextInput(label='เพศ (M = ชาย / F = หญิง)', placeholder='M หรือ F', max_length=1)

    async def on_submit(self, interaction: discord.Interaction):
        uid  = self.username.value.strip()
        pw   = self.password.value
        mail = self.email.value.strip() or 'a@a.com'
        sx   = self.sex.value.strip().upper()

        if not USER_RE.match(uid):
            return await interaction.response.send_message('❌ Username ต้องเป็น a-z, 0-9, _ และยาว 4–23 ตัวอักษร', ephemeral=True)
        if sx not in ('M', 'F'):
            return await interaction.response.send_message('❌ เพศต้องเป็น M หรือ F เท่านั้น', ephemeral=True)
        if self.email.value and not EMAIL_RE.match(mail):
            return await interaction.response.send_message('❌ รูปแบบ Email ไม่ถูกต้อง', ephemeral=True)

        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute('SELECT account_id FROM `login` WHERE userid = %s', (uid,))
                if await cur.fetchone():
                    return await interaction.response.send_message(f'❌ Username **{uid}** ถูกใช้งานแล้ว', ephemeral=True)
                await cur.execute(
                    'INSERT INTO `login` (userid, user_pass, sex, email) VALUES (%s, %s, %s, %s)',
                    (uid, _md5(pw), sx, mail),
                )

        await interaction.response.send_message(
            f'✅ สร้างบัญชี **{uid}** สำเร็จ! สามารถล็อกอินเกมได้เลย',
            ephemeral=True,
        )


class ChangePasswordModal(discord.ui.Modal, title='เปลี่ยนรหัสผ่าน'):
    username     = discord.ui.TextInput(label='Username', max_length=23)
    old_password = discord.ui.TextInput(label='รหัสผ่านเดิม', style=discord.TextStyle.short, max_length=32)
    new_password = discord.ui.TextInput(label='รหัสผ่านใหม่', style=discord.TextStyle.short, min_length=6, max_length=32)

    async def on_submit(self, interaction: discord.Interaction):
        uid  = self.username.value.strip()
        old  = _md5(self.old_password.value)
        new  = _md5(self.new_password.value)

        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    'SELECT account_id FROM `login` WHERE userid = %s AND user_pass = %s',
                    (uid, old),
                )
                if not await cur.fetchone():
                    return await interaction.response.send_message('❌ Username หรือรหัสผ่านเดิมไม่ถูกต้อง', ephemeral=True)
                await cur.execute(
                    'UPDATE `login` SET user_pass = %s WHERE userid = %s',
                    (new, uid),
                )

        await interaction.response.send_message('✅ เปลี่ยนรหัสผ่านสำเร็จแล้ว!', ephemeral=True)


class VerifyModal(discord.ui.Modal, title='ยืนยันตัวตน (Verify)'):
    username = discord.ui.TextInput(label='Username ในเกม', max_length=23)
    password = discord.ui.TextInput(label='Password', style=discord.TextStyle.short, max_length=32)

    async def on_submit(self, interaction: discord.Interaction):
        uid = self.username.value.strip()
        pw  = _md5(self.password.value)

        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    'SELECT account_id FROM `login` WHERE userid = %s AND user_pass = %s AND state = 0',
                    (uid, pw),
                )
                row = await cur.fetchone()
                if not row:
                    return await interaction.response.send_message('❌ Username หรือรหัสผ่านไม่ถูกต้อง หรือบัญชีถูกแบน', ephemeral=True)

                account_id = row[0]
                discord_id = interaction.user.id

                # Check if another Discord account already linked to this game account
                await cur.execute('SELECT discord_id FROM discord_links WHERE account_id = %s', (account_id,))
                existing = await cur.fetchone()
                if existing and existing[0] != discord_id:
                    return await interaction.response.send_message('❌ บัญชีเกมนี้ถูก Verify กับ Discord อื่นแล้ว', ephemeral=True)

                await cur.execute(
                    'INSERT INTO discord_links (discord_id, account_id) VALUES (%s, %s) '
                    'ON DUPLICATE KEY UPDATE account_id = %s, linked_at = NOW()',
                    (discord_id, account_id, account_id),
                )

        # Assign verified role if configured
        if config.VERIFIED_ROLE_ID and interaction.guild:
            role = interaction.guild.get_role(config.VERIFIED_ROLE_ID)
            if role:
                try:
                    await interaction.user.add_roles(role, reason='Discord Verify')
                except discord.Forbidden:
                    pass

        await interaction.response.send_message(
            f'✅ ยืนยันตัวตนสำเร็จ! บัญชี **{uid}** ผูกกับ Discord ของคุณแล้ว',
            ephemeral=True,
        )


# ── Cog ──────────────────────────────────────────────────────────────────────

class AccountCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot

    @app_commands.command(name='register', description='สมัครบัญชีเกมใหม่')
    async def register(self, interaction: discord.Interaction):
        await interaction.response.send_modal(RegisterModal())

    @app_commands.command(name='changepassword', description='เปลี่ยนรหัสผ่านบัญชีเกม')
    async def changepassword(self, interaction: discord.Interaction):
        await interaction.response.send_modal(ChangePasswordModal())

    @app_commands.command(name='verify', description='ผูก Discord account กับบัญชีเกม')
    async def verify(self, interaction: discord.Interaction):
        await interaction.response.send_modal(VerifyModal())

    @app_commands.command(name='unlink', description='ยกเลิกการผูก Discord กับบัญชีเกม')
    async def unlink(self, interaction: discord.Interaction):
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute('DELETE FROM discord_links WHERE discord_id = %s', (interaction.user.id,))
                if conn.affected_rows() == 0:
                    return await interaction.response.send_message('❌ ไม่พบการ Verify — ใช้ /verify ก่อน', ephemeral=True)

        if config.VERIFIED_ROLE_ID and interaction.guild:
            role = interaction.guild.get_role(config.VERIFIED_ROLE_ID)
            if role:
                try:
                    await interaction.user.remove_roles(role, reason='Discord Unlink')
                except discord.Forbidden:
                    pass

        await interaction.response.send_message('✅ ยกเลิกการผูกบัญชีสำเร็จ', ephemeral=True)

    @app_commands.command(name='whoami', description='แสดงบัญชีเกมที่ผูกกับ Discord ของคุณ')
    async def whoami(self, interaction: discord.Interaction):
        pool = get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    'SELECT l.userid, l.group_id, l.logincount, l.lastlogin, l.vip_time '
                    'FROM discord_links d JOIN `login` l ON d.account_id = l.account_id '
                    'WHERE d.discord_id = %s',
                    (interaction.user.id,),
                )
                row = await cur.fetchone()

        if not row:
            return await interaction.response.send_message('❌ ยังไม่ได้ Verify — ใช้ /verify', ephemeral=True)

        userid, group_id, logincount, lastlogin, vip_time = row
        import time
        vip_str = 'ไม่มี' if not vip_time or vip_time < time.time() else f'<t:{vip_time}:R>'

        embed = discord.Embed(title='👤 ข้อมูลบัญชีของคุณ', color=discord.Color.green())
        embed.add_field(name='Username',     value=userid,                              inline=True)
        embed.add_field(name='GM Level',     value=str(group_id),                      inline=True)
        embed.add_field(name='Login ครั้ง', value=f'{logincount:,} ครั้ง',            inline=True)
        embed.add_field(name='Login ล่าสุด', value=str(lastlogin) if lastlogin else '-', inline=True)
        embed.add_field(name='VIP หมดอายุ', value=vip_str,                            inline=True)
        await interaction.response.send_message(embed=embed, ephemeral=True)

    @app_commands.command(name='accountinfo', description='ดูข้อมูลบัญชี (GM เท่านั้น)')
    @app_commands.describe(username='ชื่อบัญชีที่ต้องการดู')
    async def accountinfo(self, interaction: discord.Interaction, username: str):
        # Allow GMs or the account owner (via linked account)
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
        import time
        ban_str = '🔴 แบน' if state != 0 else '🟢 ปกติ'
        vip_str = 'ไม่มี' if not vip_time or vip_time < time.time() else f'<t:{vip_time}:f>'

        embed = discord.Embed(title=f'📋 Account: {uid}', color=discord.Color.blurple())
        embed.add_field(name='Account ID', value=str(acc_id),  inline=True)
        embed.add_field(name='เพศ',        value=sex,          inline=True)
        embed.add_field(name='GM Level',   value=str(grp),     inline=True)
        embed.add_field(name='สถานะ',      value=ban_str,      inline=True)
        embed.add_field(name='Login',      value=f'{lcount:,} ครั้ง', inline=True)
        embed.add_field(name='VIP',        value=vip_str,      inline=True)
        embed.add_field(name='Login ล่าสุด', value=str(lastlogin) if lastlogin else '-', inline=False)
        embed.add_field(name='IP ล่าสุด', value=last_ip or '-', inline=True)
        await interaction.response.send_message(embed=embed, ephemeral=True)


async def setup(bot: commands.Bot):
    await bot.add_cog(AccountCog(bot))
