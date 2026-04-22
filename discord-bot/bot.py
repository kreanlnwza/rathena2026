import asyncio
import logging
import os
import discord
from discord import app_commands
from discord.ext import commands
from dotenv import load_dotenv

load_dotenv()
import config
from utils.db import create_pools, close_pools
from utils.gamedata import load_game_data

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s  %(levelname)-8s  %(name)s — %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S',
)

COGS = [
    'cogs.account',
    'cogs.character',
    'cogs.server',
    'cogs.trade_log',
    'cogs.admin',
    'cogs.manage',
    'cogs.automod',
    'cogs.welcome',
    'cogs.reaction_roles',
    'cogs.message_log',
    'cogs.sticky_message',
    'cogs.register_panel',
    'cogs.tickets',
]

_GLOBAL_CLEARED_MARKER = os.path.join(os.path.dirname(__file__), 'data', '.global_commands_cleared')


class RathenaBot(commands.Bot):
    def __init__(self):
        intents = discord.Intents.default()
        intents.members = True          # ต้องการสำหรับ fetch_member และ add_roles
        intents.message_content = True  # ต้องการสำหรับ reaction events
        super().__init__(command_prefix='!', intents=intents)

    async def setup_hook(self):
        import time
        t0 = time.perf_counter()

        # 1) Pool + game data + cog loading ทำขนานทั้งหมด
        #    Cogs ใช้ get_pool()/gamedata เฉพาะใน runtime callbacks — ปลอดภัยที่จะโหลดขนาน
        cog_tasks = [self.load_extension(cog) for cog in COGS]
        await asyncio.gather(
            create_pools(),
            load_game_data(),
            *cog_tasks,
        )

        log = logging.getLogger(__name__)
        log.info('Startup tasks done in %.2fs', time.perf_counter() - t0)

        # 2) Sync slash commands
        if config.GUILD_ID:
            guild = discord.Object(id=config.GUILD_ID)
            self.tree.copy_global_to(guild=guild)
            await self.tree.sync(guild=guild)
            # ล้าง global commands ครั้งเดียวเท่านั้น (หลังจากนั้นข้ามไป ไม่ต้องเรียก API ซ้ำ)
            if not os.path.exists(_GLOBAL_CLEARED_MARKER):
                self.tree.clear_commands(guild=None)
                await self.tree.sync()
                try:
                    os.makedirs(os.path.dirname(_GLOBAL_CLEARED_MARKER), exist_ok=True)
                    open(_GLOBAL_CLEARED_MARKER, 'w').close()
                except Exception:
                    pass
                log.info('Global commands cleared (one-time)')
            log.info('Slash commands synced to guild %s', config.GUILD_ID)
        else:
            await self.tree.sync()
            log.info('Slash commands synced globally')

    async def on_ready(self):
        logging.getLogger(__name__).info('Logged in as %s (ID: %s)', self.user, self.user.id)
        await self.change_presence(activity=discord.Game(name='✎﹏ Ǥᗩ∫ᕼᗩᑭᗝᑎ ूाीू'))

    async def on_tree_error(self, interaction: discord.Interaction, error: app_commands.AppCommandError):
        if isinstance(error, app_commands.CheckFailure):
            # Message already sent by the check predicate
            return

        cause = getattr(error, '__cause__', error)
        if isinstance(cause, RuntimeError) and 'pool' in str(cause).lower():
            msg = '❌ ฐานข้อมูลไม่พร้อมใช้งาน กรุณาตรวจสอบการเชื่อมต่อ MySQL'
        else:
            msg = f'❌ เกิดข้อผิดพลาด: {cause}'
        if interaction.response.is_done():
            await interaction.followup.send(msg, ephemeral=True)
        else:
            await interaction.response.send_message(msg, ephemeral=True)

    async def close(self):
        await close_pools()
        await super().close()


bot = RathenaBot()

if __name__ == '__main__':
    if not config.TOKEN:
        raise SystemExit('DISCORD_TOKEN is not set in .env')
    bot.run(config.TOKEN)
