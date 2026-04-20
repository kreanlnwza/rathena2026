import asyncio
import logging
import discord
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
    'cogs.admin',
    'cogs.drops',
]


class RathenaBot(commands.Bot):
    def __init__(self):
        intents = discord.Intents.default()
        super().__init__(command_prefix='!', intents=intents)

    async def setup_hook(self):
        await create_pools()
        await load_game_data()

        for cog in COGS:
            await self.load_extension(cog)

        await self.tree.sync()
        logging.getLogger(__name__).info('Slash commands synced globally')

    async def on_ready(self):
        logging.getLogger(__name__).info('Logged in as %s (ID: %s)', self.user, self.user.id)
        await self.change_presence(activity=discord.Game(name='Ragnarok Online'))

    async def close(self):
        await close_pools()
        await super().close()


bot = RathenaBot()

if __name__ == '__main__':
    if not config.TOKEN:
        raise SystemExit('DISCORD_TOKEN is not set in .env')
    bot.run(config.TOKEN)
