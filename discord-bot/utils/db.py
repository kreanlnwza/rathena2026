import aiomysql
import logging
import config

logger = logging.getLogger(__name__)

_pool: aiomysql.Pool | None = None
_log_pool: aiomysql.Pool | None = None

_CREATE_LINKS_TABLE = """
CREATE TABLE IF NOT EXISTS `discord_links` (
    `discord_id` BIGINT UNSIGNED NOT NULL,
    `account_id` INT UNSIGNED NOT NULL,
    `linked_at` DATETIME NOT NULL DEFAULT NOW(),
    PRIMARY KEY (`discord_id`),
    UNIQUE KEY `account_id` (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
"""


async def create_pools() -> None:
    global _pool, _log_pool
    common = dict(
        host=config.DB_HOST,
        port=config.DB_PORT,
        user=config.DB_USER,
        password=config.DB_PASS,
        autocommit=True,
        charset='utf8mb4',
        minsize=1,
        maxsize=5,
    )
    _pool = await aiomysql.create_pool(db=config.DB_NAME, **common)
    try:
        _log_pool = await aiomysql.create_pool(db=config.DB_LOG_NAME, **common)
    except Exception:
        logger.warning('Log DB unavailable — drop feed will be disabled')
        _log_pool = None

    # ensure discord_links table exists in main DB
    async with _pool.acquire() as conn:
        async with conn.cursor() as cur:
            await cur.execute(_CREATE_LINKS_TABLE)
    logger.info('Database pools ready')


async def close_pools() -> None:
    global _pool, _log_pool
    for p in (_pool, _log_pool):
        if p:
            p.close()
            await p.wait_closed()


def get_pool() -> aiomysql.Pool:
    if _pool is None:
        raise RuntimeError('DB pool not initialised')
    return _pool


def get_log_pool() -> aiomysql.Pool | None:
    return _log_pool
