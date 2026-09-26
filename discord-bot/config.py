import os
from dotenv import load_dotenv

load_dotenv()

TOKEN = os.getenv('DISCORD_TOKEN', '')
ADMIN_ROLE_ID = int(os.getenv('ADMIN_ROLE_ID') or 0)
VERIFIED_ROLE_ID = int(os.getenv('VERIFIED_ROLE_ID') or 0)
PLAYER_ROLE_ID = int(os.getenv('PLAYER_ROLE_ID') or 0)
REGISTER_CHANNEL_ID = int(os.getenv('REGISTER_CHANNEL_ID') or 0)

DB_HOST = os.getenv('DB_HOST', '127.0.0.1')
DB_PORT = int(os.getenv('DB_PORT', 3306))
DB_USER = os.getenv('DB_USER', 'ragnarok')
DB_PASS = os.getenv('DB_PASS', 'ragnarok')
DB_NAME = os.getenv('DB_NAME', 'ragnarok')
DB_LOG_NAME = os.getenv('DB_LOG_NAME', 'ragnarok_log')

RATHENA_PATH = os.getenv('RATHENA_PATH', '/home/user/rathena2026')
GUILD_ID = int(os.getenv('GUILD_ID') or 0)

EMAIL_SENDER   = os.getenv('EMAIL_SENDER', '')
EMAIL_PASSWORD = os.getenv('EMAIL_PASSWORD', '')

PANEL_IMAGE_URL = os.getenv('PANEL_IMAGE_URL', '')
PANEL_AUTHOR_ICON_URL = os.getenv('PANEL_AUTHOR_ICON_URL', '')

STATUS_CHANNEL_ID = int(os.getenv('STATUS_CHANNEL_ID') or 0)
STATUS_POLL_INTERVAL = int(os.getenv('STATUS_POLL_INTERVAL') or 30)  # วินาที
ZENY_CHANNEL_ID = int(os.getenv('ZENY_CHANNEL_ID') or 0)
TRADE_LOG_CHANNEL_ID = int(os.getenv('TRADE_LOG_CHANNEL_ID') or 0)
TRADE_LOG_POLL_INTERVAL = int(os.getenv('TRADE_LOG_POLL_INTERVAL') or 30)

WELCOME_CHANNEL_ID = int(os.getenv('WELCOME_CHANNEL_ID') or 0)
DOWNLOAD_CHANNEL_ID = int(os.getenv('DOWNLOAD_CHANNEL_ID') or 0)
RULES_CHANNEL_ID = int(os.getenv('RULES_CHANNEL_ID') or 0)
TICKET_FORUM_ID = int(os.getenv('TICKET_FORUM_ID') or 0)

RATHENA_HOST = os.getenv('RATHENA_HOST', '127.0.0.1')
LOGIN_PORT = int(os.getenv('LOGIN_PORT') or 6900)
CHAR_PORT  = int(os.getenv('CHAR_PORT')  or 6121)
MAP_PORT   = int(os.getenv('MAP_PORT')   or 5121)
WEB_PORT   = int(os.getenv('WEB_PORT')   or 8888)
