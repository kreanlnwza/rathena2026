"""
ระบบแจ้งปัญหา (Ticket System)
──────────────────────────────
ผู้เล่นกดปุ่ม "แจ้งปัญหา" → เลือกหมวดหมู่ (ปุ่มกด)
→ กรอกข้อมูล → ยืนยันด้วย CAPTCHA (ปุ่มกด) → สร้าง Forum Thread
"""
import json
import logging
import os
import random
from datetime import datetime, timezone, timedelta

import discord
from discord import app_commands
from discord.ext import commands

import config
from utils.checks import admin_check

log = logging.getLogger(__name__)

DATA_FILE = os.path.join(os.path.dirname(__file__), '..', 'data', 'ticket_panels.json')

# ── Ticket counter ────────────────────────────────────────────────────────────

_counter_file = os.path.join(os.path.dirname(__file__), '..', 'data', 'ticket_counter.json')


def _load_counter() -> int:
    try:
        with open(_counter_file, 'r') as f:
            return json.load(f).get('count', 0)
    except (FileNotFoundError, json.JSONDecodeError):
        return 0


def _save_counter(count: int):
    os.makedirs(os.path.dirname(_counter_file), exist_ok=True)
    with open(_counter_file, 'w') as f:
        json.dump({'count': count}, f)


def _next_ticket_id() -> int:
    c = _load_counter() + 1
    _save_counter(c)
    return c


# ── Panel persistence ─────────────────────────────────────────────────────────

_panels: list[dict] = []


def _load_panels():
    global _panels
    try:
        with open(DATA_FILE, 'r', encoding='utf-8') as f:
            _panels = json.load(f)
    except (FileNotFoundError, json.JSONDecodeError):
        _panels = []


def _save_panels():
    os.makedirs(os.path.dirname(DATA_FILE), exist_ok=True)
    with open(DATA_FILE, 'w', encoding='utf-8') as f:
        json.dump(_panels, f, ensure_ascii=False, indent=2)


# ── Report categories ────────────────────────────────────────────────────────

CATEGORIES = {
    'bug':        {'label': 'บัค / ระบบผิดปกติ',      'emoji': '🐛', 'color': discord.Color.red(),          'style': discord.ButtonStyle.danger},
    'item':       {'label': 'ไอเทม / อุปกรณ์',        'emoji': '🎒', 'color': discord.Color.orange(),       'style': discord.ButtonStyle.secondary},
    'character':  {'label': 'ตัวละคร / บัญชี',        'emoji': '⚔',  'color': discord.Color.blue(),         'style': discord.ButtonStyle.primary},
    'map':        {'label': 'แผนที่ / NPC',           'emoji': '🗺',  'color': discord.Color.dark_green(),   'style': discord.ButtonStyle.secondary},
    'server':     {'label': 'เซิร์ฟเวอร์ / เชื่อมต่อ',  'emoji': '🖥',  'color': discord.Color.dark_red(),     'style': discord.ButtonStyle.danger},
    'report':     {'label': 'แจ้งผู้เล่นทำผิดกฎ',      'emoji': '🚨', 'color': discord.Color.dark_orange(),  'style': discord.ButtonStyle.danger},
    'suggestion': {'label': 'ข้อเสนอแนะ',             'emoji': '💡', 'color': discord.Color.teal(),         'style': discord.ButtonStyle.success},
    'other':      {'label': 'อื่นๆ',                   'emoji': '📝', 'color': discord.Color.greyple(),      'style': discord.ButtonStyle.secondary},
}


# ── Forum thread creator ─────────────────────────────────────────────────────

async def _create_ticket(interaction: discord.Interaction, category: str,
                         title: str, char_name: str, desc: str, steps: str):
    """Actually create the forum thread. Called after CAPTCHA passes."""
    # Try cache first, then API call as fallback
    forum = interaction.client.get_channel(config.TICKET_FORUM_ID)
    if forum is None:
        try:
            forum = await interaction.client.fetch_channel(config.TICKET_FORUM_ID)
        except Exception:
            forum = None

    if not forum or not isinstance(forum, discord.ForumChannel):
        return await interaction.followup.send(
            f'❌ ไม่พบช่องฟอรั่ม (ID: {config.TICKET_FORUM_ID}) — ตรวจสอบว่าเป็นช่อง Forum และบอทมีสิทธิ์เข้าถึง',
            ephemeral=True,
        )

    ticket_id = _next_ticket_id()
    cat_info = CATEGORIES.get(category, CATEGORIES['other'])
    cat_label = f'{cat_info["emoji"]} {cat_info["label"]}'
    color = cat_info['color']
    now = datetime.now(timezone(timedelta(hours=7)))

    thread_title = f'#{ticket_id:04d} — {title}'

    embed = discord.Embed(
        title=f'📋 Ticket #{ticket_id:04d}',
        description=desc,
        color=color,
        timestamp=discord.utils.utcnow(),
    )
    embed.add_field(name='📂 หมวดหมู่', value=cat_label, inline=True)
    embed.add_field(name='📌 สถานะ', value='🟡 รอตรวจสอบ', inline=True)
    embed.add_field(name='👤 ผู้แจ้ง', value=interaction.user.mention, inline=True)

    if char_name:
        embed.add_field(name='⚔ ตัวละคร', value=char_name, inline=True)
    if steps:
        embed.add_field(name='🔄 ขั้นตอนที่ทำให้เกิด', value=steps, inline=False)

    embed.set_footer(
        text=f'แจ้งโดย {interaction.user.display_name} • {now.strftime("%d/%m/%Y %H:%M")}',
    )

    applied_tags = []
    for tag in forum.available_tags:
        tag_lower = tag.name.lower()
        if tag_lower == category or tag_lower in ('รอตรวจสอบ', 'pending'):
            applied_tags.append(tag)

    # Forum requires at least 1 tag — fallback to first available
    if not applied_tags and forum.available_tags:
        applied_tags.append(forum.available_tags[0])

    thread_with_msg = await forum.create_thread(
        name=thread_title,
        content=(
            f'**ผู้แจ้ง:** {interaction.user.mention}\n\n'
            f'สวัสดี! ทีมงานได้รับเรื่องแจ้งของคุณแล้ว\n'
            f'กรุณารอทีมงานตรวจสอบ หากต้องการเพิ่มเติมข้อมูล '
            f'สามารถพิมพ์ใน Thread นี้ได้เลย'
        ),
        embed=embed,
        applied_tags=applied_tags[:5],
    )

    confirm_embed = discord.Embed(
        title='✅ ส่งเรื่องแจ้งปัญหาสำเร็จ!',
        description=(
            f'**Ticket #{ticket_id:04d}** ถูกสร้างเรียบร้อยแล้ว\n\n'
            f'📋 หัวข้อ: **{title}**\n'
            f'📂 หมวดหมู่: {cat_label}\n\n'
            f'👉 ติดตามสถานะได้ที่: {thread_with_msg.thread.mention}'
        ),
        color=discord.Color.green(),
    )
    await interaction.followup.send(embed=confirm_embed, ephemeral=True)

    log.info('Ticket #%04d created by %s (%s) — %s', ticket_id, interaction.user, category, title)


# ── CAPTCHA: Numpad ───────────────────────────────────────────────────────────

def _generate_captcha() -> tuple[str, int]:
    """Returns (question, correct_answer)."""
    a = random.randint(10, 50)
    b = random.randint(1, 30)
    op = random.choice(['+', '-'])
    if op == '+':
        answer = a + b
    else:
        if a < b:
            a, b = b, a
        answer = a - b
    return f'{a} {op} {b} = ?', answer


class NumpadCaptchaView(discord.ui.View):
    """Ephemeral numpad view — user taps digits then ✅ to confirm."""

    _LAYOUT = [
        ['1', '2', '3'],
        ['4', '5', '6'],
        ['7', '8', '9'],
        ['⌫', '0', '✅'],
    ]

    def __init__(self, question: str, correct_answer: int, ticket_data: dict):
        super().__init__(timeout=120)
        self.question = question
        self.correct_answer = correct_answer
        self.ticket_data = ticket_data
        self.entered: list[str] = []

        for row_idx, row in enumerate(self._LAYOUT):
            for label in row:
                if label == '⌫':
                    btn = discord.ui.Button(label='⌫', style=discord.ButtonStyle.secondary, row=row_idx)
                    btn.callback = self._backspace_cb
                elif label == '✅':
                    btn = discord.ui.Button(label='✅', style=discord.ButtonStyle.success, row=row_idx)
                    btn.callback = self._ok_cb
                else:
                    btn = discord.ui.Button(label=label, style=discord.ButtonStyle.primary, row=row_idx)
                    btn.callback = self._make_digit_cb(label)
                self.add_item(btn)

    def build_embed(self) -> discord.Embed:
        display = ''.join(self.entered) if self.entered else '▌'
        return discord.Embed(
            title='🔒 ยืนยันตัวตน — CAPTCHA',
            description=(
                f'กรุณาตอบคำถามเพื่อยืนยันว่าคุณไม่ใช่บอท\n\n'
                f'**{self.question}**\n\n'
                f'```\n{display}\n```\n'
                f'กดตัวเลขด้านล่าง แล้วกด ✅ เพื่อยืนยัน'
            ),
            color=discord.Color.gold(),
        )

    def _make_digit_cb(self, digit: str):
        async def cb(interaction: discord.Interaction):
            if len(self.entered) < 6:
                self.entered.append(digit)
            await interaction.response.edit_message(embed=self.build_embed())
        return cb

    async def _backspace_cb(self, interaction: discord.Interaction):
        if self.entered:
            self.entered.pop()
        await interaction.response.edit_message(embed=self.build_embed())

    async def _ok_cb(self, interaction: discord.Interaction):
        if ''.join(self.entered) == str(self.correct_answer):
            await interaction.response.defer(ephemeral=True)
            await interaction.edit_original_response(
                embed=discord.Embed(
                    title='✅ ยืนยันสำเร็จ!',
                    description='กำลังสร้าง Ticket...',
                    color=discord.Color.green(),
                ),
                view=None,
            )
            await _create_ticket(
                interaction,
                self.ticket_data['category'],
                self.ticket_data['title'],
                self.ticket_data['char_name'],
                self.ticket_data['description'],
                self.ticket_data['steps'],
            )
        else:
            await interaction.response.edit_message(
                embed=discord.Embed(
                    title='❌ คำตอบไม่ถูกต้อง!',
                    description='กรุณากดปุ่ม **แจ้งปัญหา** แล้วลองใหม่อีกครั้ง',
                    color=discord.Color.red(),
                ),
                view=None,
            )

    async def on_timeout(self):
        pass


# ── Modal: report details (no CAPTCHA text) ──────────────────────────────────

class ReportModal(discord.ui.Modal, title='📝 แจ้งปัญหา'):

    report_title = discord.ui.TextInput(
        label='หัวข้อปัญหา',
        placeholder='อธิบายปัญหาสั้นๆ เช่น "ไอเทม X ใช้ไม่ได้"',
        max_length=100,
        style=discord.TextStyle.short,
    )
    char_name = discord.ui.TextInput(
        label='ชื่อตัวละคร (ถ้ามี)',
        placeholder='ชื่อตัวละครที่พบปัญหา',
        max_length=24,
        required=False,
        style=discord.TextStyle.short,
    )
    description = discord.ui.TextInput(
        label='รายละเอียด',
        placeholder='อธิบายปัญหาอย่างละเอียด เกิดอะไรขึ้น? ทำอะไรอยู่ตอนนั้น?',
        max_length=2000,
        style=discord.TextStyle.paragraph,
    )
    steps = discord.ui.TextInput(
        label='ขั้นตอนที่ทำให้เกิดปัญหา (ถ้าทราบ)',
        placeholder='1. เปิดเกม\n2. ไปแมป prontera\n3. คุยกับ NPC\n4. เกิด error',
        max_length=1000,
        required=False,
        style=discord.TextStyle.paragraph,
    )

    def __init__(self, category: str):
        super().__init__()
        self.category = category

    async def on_submit(self, interaction: discord.Interaction):
        if not config.TICKET_FORUM_ID:
            return await interaction.response.send_message(
                '❌ ยังไม่ได้ตั้งค่า TICKET_FORUM_ID ใน .env', ephemeral=True,
            )

        # Generate CAPTCHA numpad
        question, correct = _generate_captcha()

        ticket_data = {
            'category': self.category,
            'title': self.report_title.value,
            'char_name': self.char_name.value,
            'description': self.description.value,
            'steps': self.steps.value,
        }

        view = NumpadCaptchaView(question, correct, ticket_data)
        await interaction.response.send_message(embed=view.build_embed(), view=view, ephemeral=True)


# ── Category Buttons (persistent) ────────────────────────────────────────────

class _CategoryButton(discord.ui.Button):
    def __init__(self, key: str, info: dict):
        super().__init__(
            label=info['label'],
            emoji=info['emoji'],
            style=info['style'],
            custom_id=f'ticket:cat:{key}',
        )
        self.category_key = key

    async def callback(self, interaction: discord.Interaction):
        await interaction.response.send_modal(ReportModal(self.category_key))


class CategoryButtonsView(discord.ui.View):
    def __init__(self):
        super().__init__(timeout=None)
        for key, info in CATEGORIES.items():
            self.add_item(_CategoryButton(key, info))


# ── Main Report Button (persistent) ──────────────────────────────────────────

class ReportButtonView(discord.ui.View):
    def __init__(self):
        super().__init__(timeout=None)

    @discord.ui.button(
        label='แจ้งปัญหา / Report',
        style=discord.ButtonStyle.danger,
        custom_id='ticket:open_report',
        emoji='📝',
    )
    async def open_report(self, interaction: discord.Interaction, button: discord.ui.Button):
        embed = discord.Embed(
            title='📂 เลือกประเภทปัญหา',
            description='กดปุ่มด้านล่างตามประเภทปัญหาที่ต้องการแจ้ง',
            color=discord.Color.from_rgb(255, 85, 85),
        )
        view = CategoryButtonsView()
        await interaction.response.send_message(embed=embed, view=view, ephemeral=True)


# ── Panel embed ───────────────────────────────────────────────────────────────

def _build_panel_embed() -> discord.Embed:
    embed = discord.Embed(
        title='🛡️ ศูนย์แจ้งปัญหา — Report Center',
        description=(
            'พบปัญหาในเกม? ต้องการแจ้งบัค?\n'
            'กดปุ่ม **📝 แจ้งปัญหา** ด้านล่างเพื่อเริ่มต้น\n\n'
            '**ขั้นตอน:**\n'
            '> 1️⃣ กดปุ่ม **แจ้งปัญหา**\n'
            '> 2️⃣ เลือก **ประเภทปัญหา** จากปุ่ม\n'
            '> 3️⃣ กรอก **รายละเอียด**\n'
            '> 4️⃣ กดปุ่ม **ยืนยัน CAPTCHA**\n'
            '> 5️⃣ ระบบสร้าง Thread ให้ทันที\n'
        ),
        color=discord.Color.from_rgb(255, 85, 85),
    )

    cat_lines = '\n'.join(
        f'> {info["emoji"]} **{info["label"]}**'
        for info in CATEGORIES.values()
    )
    embed.add_field(name='📂 หมวดหมู่ที่รองรับ', value=cat_lines, inline=False)

    embed.add_field(
        name='⚠️ หมายเหตุ',
        value='กรุณาอธิบายปัญหาให้ละเอียดที่สุด\nการแจ้งปัญหาเท็จอาจส่งผลต่อบัญชีของคุณ',
        inline=False,
    )

    embed.set_footer(text='ทีมงานจะตอบกลับโดยเร็วที่สุด ขอบคุณที่ช่วยแจ้งปัญหา ❤️')
    return embed


# ── Ticket admin commands ─────────────────────────────────────────────────────

ticket_group = app_commands.Group(name='ticket', description='ระบบแจ้งปัญหา')


@ticket_group.command(name='panel', description='[ADMIN] สร้างแผงปุ่มแจ้งปัญหา')
@app_commands.describe(channel='ช่องทางที่จะส่งแผงปุ่ม')
@app_commands.default_permissions(administrator=True)
@admin_check()
async def ticket_panel(interaction: discord.Interaction, channel: discord.TextChannel):
    await interaction.response.defer(ephemeral=True)

    view = ReportButtonView()
    msg = await channel.send(embed=_build_panel_embed(), view=view)

    _panels.append({'channel_id': channel.id, 'message_id': msg.id})
    _save_panels()

    embed = discord.Embed(title='✅ สร้างแผงแจ้งปัญหาสำเร็จ', color=discord.Color.green())
    embed.add_field(name='ช่องทาง', value=channel.mention, inline=True)
    embed.add_field(name='ข้อความ', value=f'[ไปที่แผงปุ่ม]({msg.jump_url})', inline=True)
    embed.set_footer(text=f'โดย {interaction.user}')
    await interaction.followup.send(embed=embed)


@ticket_group.command(name='remove', description='[ADMIN] ลบแผงปุ่มแจ้งปัญหา')
@app_commands.describe(message_id='ID ของข้อความแผงปุ่ม')
@app_commands.default_permissions(administrator=True)
@admin_check()
async def ticket_remove(interaction: discord.Interaction, message_id: str):
    await interaction.response.defer(ephemeral=True)

    entry = next((p for p in _panels if str(p['message_id']) == message_id), None)
    if not entry:
        return await interaction.followup.send('⚠️ ไม่พบแผงปุ่มนี้ในระบบ')

    _panels.remove(entry)
    _save_panels()

    channel = interaction.guild.get_channel(entry['channel_id'])
    if channel:
        try:
            msg = await channel.fetch_message(entry['message_id'])
            await msg.delete()
        except (discord.NotFound, discord.HTTPException):
            pass

    await interaction.followup.send('🗑️ ลบแผงแจ้งปัญหาสำเร็จ')


@ticket_group.command(name='close', description='[ADMIN] ปิด Ticket (ล็อค Thread)')
@app_commands.describe(reason='เหตุผลที่ปิด')
@app_commands.default_permissions(administrator=True)
@admin_check()
async def ticket_close(interaction: discord.Interaction, reason: str = 'แก้ไขเรียบร้อยแล้ว'):
    thread = interaction.channel
    if not isinstance(thread, discord.Thread):
        return await interaction.response.send_message(
            '❌ คำสั่งนี้ใช้ได้ใน Thread เท่านั้น', ephemeral=True,
        )

    close_embed = discord.Embed(
        title='🔒 Ticket ถูกปิดแล้ว',
        description=f'**เหตุผล:** {reason}',
        color=discord.Color.dark_grey(),
        timestamp=discord.utils.utcnow(),
    )
    close_embed.set_footer(text=f'ปิดโดย {interaction.user.display_name}')

    await interaction.response.send_message(embed=close_embed)
    await thread.edit(locked=True, archived=True,
                      reason=f'Ticket closed by {interaction.user}: {reason}')


# ── Cog ───────────────────────────────────────────────────────────────────────

class TicketsCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot
        _load_panels()
        bot.tree.add_command(ticket_group)
        bot.add_view(ReportButtonView())
        bot.add_view(CategoryButtonsView())

    def cog_unload(self):
        self.bot.tree.remove_command('ticket')


async def setup(bot: commands.Bot):
    await bot.add_cog(TicketsCog(bot))
