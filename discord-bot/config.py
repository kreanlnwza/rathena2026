import os
from dotenv import load_dotenv

load_dotenv()

TOKEN = os.getenv('DISCORD_TOKEN', '')
GUILD_ID = int(os.getenv('GUILD_ID') or 0)
DROP_CHANNEL_ID = int(os.getenv('DROP_CHANNEL_ID') or 0)
ADMIN_ROLE_ID = int(os.getenv('ADMIN_ROLE_ID') or 0)
VERIFIED_ROLE_ID = int(os.getenv('VERIFIED_ROLE_ID') or 0)

DB_HOST = os.getenv('DB_HOST', '127.0.0.1')
DB_PORT = int(os.getenv('DB_PORT', 3306))
DB_USER = os.getenv('DB_USER', 'ragnarok')
DB_PASS = os.getenv('DB_PASS', 'ragnarok')
DB_NAME = os.getenv('DB_NAME', 'ragnarok')
DB_LOG_NAME = os.getenv('DB_LOG_NAME', 'ragnarok_log')

RATHENA_PATH = os.getenv('RATHENA_PATH', '/home/user/rathena2026')
DROP_POLL_INTERVAL = int(os.getenv('DROP_POLL_INTERVAL', 30))
