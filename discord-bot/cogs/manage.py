import discord
from discord import app_commands
from discord.ext import commands
import config
from utils.checks import admin_check, player_check


# ── Role Group ────────────────────────────────────────────────────────────────


# ── Role Group ────────────────────────────────────────────────────────────────

role_group = app_commands.Group(name='role', description='จัดการบทบาทสมาชิก')


@role_group.command(name='create', description='[ADMIN] สร้างบทบาทใหม่')
@app_commands.describe(
    name='ชื่อบทบาท',
    color='สีบทบาท (hex เช่น ff0000)',
    hoist='แสดงแยกในรายชื่อสมาชิก',
    mentionable='อนุญาตให้ mention ได้',
)
@admin_check()
async def role_create(
    interaction: discord.Interaction,
    name: str,
    color: str = 'ffffff',
    hoist: bool = False,
    mentionable: bool = False,
):
    try:
        hex_color = int(color.lstrip('#'), 16)
    except ValueError:
        return await interaction.response.send_message('❌ รูปแบบสีไม่ถูกต้อง เช่น ff0000', ephemeral=True)

    role = await interaction.guild.create_role(
        name=name,
        color=discord.Color(hex_color),
        hoist=hoist,
        mentionable=mentionable,
        reason=f'สร้างโดย {interaction.user}',
    )
    embed = discord.Embed(title='✅ สร้างบทบาทสำเร็จ', color=role.color)
    embed.add_field(name='ชื่อ', value=role.mention, inline=True)
    embed.add_field(name='ID', value=str(role.id), inline=True)
    embed.set_footer(text=f'โดย {interaction.user}')
    await interaction.response.send_message(embed=embed)


@role_group.command(name='delete', description='[ADMIN] ลบบทบาท')
@app_commands.describe(role='บทบาทที่ต้องการลบ')
@admin_check()
async def role_delete(interaction: discord.Interaction, role: discord.Role):
    name = role.name
    await role.delete(reason=f'ลบโดย {interaction.user}')
    await interaction.response.send_message(f'🗑️ ลบบทบาท **{name}** สำเร็จแล้ว')


@role_group.command(name='assign', description='[ADMIN] มอบบทบาทให้สมาชิก')
@app_commands.describe(member='สมาชิก', role='บทบาท')
@admin_check()
async def role_assign(interaction: discord.Interaction, member: discord.Member, role: discord.Role):
    if role in member.roles:
        return await interaction.response.send_message(
            f'⚠️ {member.mention} มีบทบาท {role.mention} อยู่แล้ว', ephemeral=True
        )
    await member.add_roles(role, reason=f'มอบโดย {interaction.user}')
    embed = discord.Embed(title='✅ มอบบทบาทสำเร็จ', color=role.color)
    embed.add_field(name='สมาชิก', value=member.mention, inline=True)
    embed.add_field(name='บทบาท', value=role.mention, inline=True)
    embed.set_footer(text=f'โดย {interaction.user}')
    await interaction.response.send_message(embed=embed)


@role_group.command(name='remove', description='[ADMIN] ถอดบทบาทจากสมาชิก')
@app_commands.describe(member='สมาชิก', role='บทบาท')
@admin_check()
async def role_remove(interaction: discord.Interaction, member: discord.Member, role: discord.Role):
    if role not in member.roles:
        return await interaction.response.send_message(
            f'⚠️ {member.mention} ไม่มีบทบาท {role.mention}', ephemeral=True
        )
    await member.remove_roles(role, reason=f'ถอดโดย {interaction.user}')
    embed = discord.Embed(title='✅ ถอดบทบาทสำเร็จ', color=discord.Color.red())
    embed.add_field(name='สมาชิก', value=member.mention, inline=True)
    embed.add_field(name='บทบาท', value=role.mention, inline=True)
    embed.set_footer(text=f'โดย {interaction.user}')
    await interaction.response.send_message(embed=embed)


@role_group.command(name='list', description='แสดงรายการบทบาทในเซิร์ฟเวอร์')
@player_check()
async def role_list(interaction: discord.Interaction):
    roles = [r for r in interaction.guild.roles if r.name != '@everyone']
    roles.sort(key=lambda r: r.position, reverse=True)
    lines = [f'{r.mention} — ID: `{r.id}`' for r in roles[:25]]
    embed = discord.Embed(
        title=f'📋 บทบาททั้งหมด ({len(roles)} บทบาท)',
        description='\n'.join(lines) or '_ไม่มีบทบาท_',
        color=discord.Color.blurple(),
    )
    await interaction.response.send_message(embed=embed, ephemeral=True)


# ── Channel Group ─────────────────────────────────────────────────────────────

channel_group = app_commands.Group(name='channel', description='จัดการช่องทางในเซิร์ฟเวอร์')

ChannelTypeChoice = [
    app_commands.Choice(name='ข้อความ (Text)',   value='text'),
    app_commands.Choice(name='เสียง (Voice)',     value='voice'),
    app_commands.Choice(name='ประกาศ (Announce)', value='news'),
    app_commands.Choice(name='Stage',             value='stage'),
]


@channel_group.command(name='create', description='[ADMIN] สร้างช่องทางใหม่')
@app_commands.describe(
    name='ชื่อช่องทาง',
    channel_type='ประเภทช่องทาง',
    category='หมวดหมู่ (ไม่บังคับ)',
    topic='หัวข้อช่องทาง (สำหรับ text/news)',
)
@app_commands.choices(channel_type=ChannelTypeChoice)
@admin_check()
async def channel_create(
    interaction: discord.Interaction,
    name: str,
    channel_type: app_commands.Choice[str],
    category: discord.CategoryChannel = None,
    topic: str = None,
):
    guild = interaction.guild
    reason = f'สร้างโดย {interaction.user}'

    if channel_type.value == 'text':
        ch = await guild.create_text_channel(name, category=category, topic=topic, reason=reason)
        icon = '💬'
    elif channel_type.value == 'voice':
        ch = await guild.create_voice_channel(name, category=category, reason=reason)
        icon = '🔊'
    elif channel_type.value == 'news':
        ch = await guild.create_text_channel(name, category=category, topic=topic, news=True, reason=reason)
        icon = '📢'
    else:  # stage
        ch = await guild.create_stage_channel(name, category=category, reason=reason)
        icon = '🎤'

    embed = discord.Embed(title=f'{icon} สร้างช่องทางสำเร็จ', color=discord.Color.green())
    embed.add_field(name='ชื่อ',     value=ch.mention,         inline=True)
    embed.add_field(name='ประเภท',  value=channel_type.name,  inline=True)
    embed.add_field(name='ID',       value=str(ch.id),         inline=True)
    if category:
        embed.add_field(name='หมวดหมู่', value=category.name, inline=True)
    embed.set_footer(text=f'โดย {interaction.user}')
    await interaction.response.send_message(embed=embed)


@channel_group.command(name='delete', description='[ADMIN] ลบช่องทาง')
@app_commands.describe(channel='ช่องทางที่ต้องการลบ')
@admin_check()
async def channel_delete(interaction: discord.Interaction, channel: discord.abc.GuildChannel):
    name = channel.name
    await channel.delete(reason=f'ลบโดย {interaction.user}')
    await interaction.response.send_message(f'🗑️ ลบช่องทาง **#{name}** สำเร็จแล้ว')


@channel_group.command(name='create_category', description='[ADMIN] สร้างหมวดหมู่ใหม่')
@app_commands.describe(name='ชื่อหมวดหมู่')
@admin_check()
async def channel_create_category(interaction: discord.Interaction, name: str):
    cat = await interaction.guild.create_category(name, reason=f'สร้างโดย {interaction.user}')
    embed = discord.Embed(title='📁 สร้างหมวดหมู่สำเร็จ', color=discord.Color.green())
    embed.add_field(name='ชื่อ', value=cat.name, inline=True)
    embed.add_field(name='ID',   value=str(cat.id), inline=True)
    embed.set_footer(text=f'โดย {interaction.user}')
    await interaction.response.send_message(embed=embed)


# ── Forum Group ───────────────────────────────────────────────────────────────

forum_group = app_commands.Group(name='forum', description='จัดการฟอรั่มในเซิร์ฟเวอร์')


@forum_group.command(name='create', description='[ADMIN] สร้างช่องฟอรั่มใหม่')
@app_commands.describe(
    name='ชื่อฟอรั่ม',
    category='หมวดหมู่ (ไม่บังคับ)',
    topic='คำอธิบายฟอรั่ม',
)
@admin_check()
async def forum_create(
    interaction: discord.Interaction,
    name: str,
    category: discord.CategoryChannel = None,
    topic: str = None,
):
    forum = await interaction.guild.create_forum(
        name,
        category=category,
        topic=topic,
        reason=f'สร้างโดย {interaction.user}',
    )
    embed = discord.Embed(title='📰 สร้างฟอรั่มสำเร็จ', color=discord.Color.green())
    embed.add_field(name='ชื่อ',  value=forum.mention,  inline=True)
    embed.add_field(name='ID',    value=str(forum.id),  inline=True)
    if category:
        embed.add_field(name='หมวดหมู่', value=category.name, inline=True)
    if topic:
        embed.add_field(name='คำอธิบาย', value=topic, inline=False)
    embed.set_footer(text=f'โดย {interaction.user}')
    await interaction.response.send_message(embed=embed)


@forum_group.command(name='add_tag', description='[ADMIN] เพิ่ม tag ให้ฟอรั่ม')
@app_commands.describe(
    forum='ช่องฟอรั่ม',
    tag_name='ชื่อ tag',
    emoji='emoji (ไม่บังคับ เช่น 🔥)',
    moderated='เฉพาะ moderator เท่านั้น',
)
@admin_check()
async def forum_add_tag(
    interaction: discord.Interaction,
    forum: discord.ForumChannel,
    tag_name: str,
    emoji: str = None,
    moderated: bool = False,
):
    if len(forum.available_tags) >= 20:
        return await interaction.response.send_message('❌ ฟอรั่มมี tag เต็ม 20 รายการแล้ว', ephemeral=True)

    new_tag = discord.ForumTag(name=tag_name, moderated=moderated, emoji=emoji)
    await forum.edit(available_tags=[*forum.available_tags, new_tag])
    embed = discord.Embed(title='🏷️ เพิ่ม tag สำเร็จ', color=discord.Color.green())
    embed.add_field(name='ฟอรั่ม', value=forum.mention,  inline=True)
    embed.add_field(name='Tag',    value=tag_name,        inline=True)
    embed.set_footer(text=f'โดย {interaction.user}')
    await interaction.response.send_message(embed=embed)


@forum_group.command(name='delete', description='[ADMIN] ลบช่องฟอรั่ม')
@app_commands.describe(forum='ช่องฟอรั่มที่ต้องการลบ')
@admin_check()
async def forum_delete(interaction: discord.Interaction, forum: discord.ForumChannel):
    name = forum.name
    await forum.delete(reason=f'ลบโดย {interaction.user}')
    await interaction.response.send_message(f'🗑️ ลบฟอรั่ม **#{name}** สำเร็จแล้ว')


# ── Cog ───────────────────────────────────────────────────────────────────────

class ManageCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot
        bot.tree.add_command(role_group)
        bot.tree.add_command(channel_group)
        bot.tree.add_command(forum_group)


async def setup(bot: commands.Bot):
    await bot.add_cog(ManageCog(bot))
