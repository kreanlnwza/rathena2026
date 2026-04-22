import os
import logging
from logging.handlers import TimedRotatingFileHandler
import discord
from discord.ext import commands

LOG_DIR = os.path.join(os.path.dirname(__file__), '..', 'logs', 'messages')

def _get_logger() -> logging.Logger:
    os.makedirs(LOG_DIR, exist_ok=True)
    log = logging.getLogger('msg_log')
    if log.handlers:
        return log
    log.setLevel(logging.INFO)
    handler = TimedRotatingFileHandler(
        filename=os.path.join(LOG_DIR, 'messages.log'),
        when='midnight',
        backupCount=30,
        encoding='utf-8',
    )
    handler.setFormatter(logging.Formatter('%(message)s'))
    log.addHandler(handler)
    log.propagate = False
    return log


class MessageLogCog(commands.Cog):
    def __init__(self, bot: commands.Bot):
        self.bot = bot
        self.log = _get_logger()

    def _fmt(self, msg: discord.Message) -> str:
        guild   = msg.guild.name if msg.guild else 'DM'
        channel = getattr(msg.channel, 'name', str(msg.channel.id))
        author  = f'{msg.author} ({msg.author.id})'
        content = msg.content or ''
        attaches = ' '.join(a.url for a in msg.attachments)
        line = f'[{msg.created_at.strftime("%Y-%m-%d %H:%M:%S")}] [{guild}] [#{channel}] {author}: {content}'
        if attaches:
            line += f'  [files: {attaches}]'
        return line

    @commands.Cog.listener()
    async def on_message(self, message: discord.Message):
        if message.author.bot:
            return
        self.log.info(self._fmt(message))

    @commands.Cog.listener()
    async def on_message_edit(self, before: discord.Message, after: discord.Message):
        if after.author.bot:
            return
        if before.content == after.content:
            return
        guild   = after.guild.name if after.guild else 'DM'
        channel = getattr(after.channel, 'name', str(after.channel.id))
        author  = f'{after.author} ({after.author.id})'
        self.log.info(
            '[%s] [%s] [#%s] %s [EDIT] %s → %s',
            after.edited_at.strftime('%Y-%m-%d %H:%M:%S') if after.edited_at else '?',
            guild, channel, author, before.content, after.content,
        )

    @commands.Cog.listener()
    async def on_message_delete(self, message: discord.Message):
        if message.author.bot:
            return
        guild   = message.guild.name if message.guild else 'DM'
        channel = getattr(message.channel, 'name', str(message.channel.id))
        author  = f'{message.author} ({message.author.id})'
        self.log.info(
            '[%s] [%s] [#%s] %s [DELETED] %s',
            message.created_at.strftime('%Y-%m-%d %H:%M:%S'),
            guild, channel, author, message.content,
        )


async def setup(bot: commands.Bot):
    await bot.add_cog(MessageLogCog(bot))
