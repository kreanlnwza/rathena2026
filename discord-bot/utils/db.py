import asyncio
import os
import aiomysql
import logging
import config

logger = logging.getLogger(__name__)

_pool: aiomysql.Pool | None = None
_log_pool: aiomysql.Pool | None = None

_MIGRATION_MARKER = os.path.join(os.path.dirname(__file__), '..', 'data', '.db_migrated')

_CREATE_LINKS_TABLE = """
CREATE TABLE IF NOT EXISTS `discord_links` (
    `discord_id` BIGINT UNSIGNED NOT NULL,
    `account_id` INT UNSIGNED NOT NULL,
    `linked_at` DATETIME NOT NULL DEFAULT NOW(),
    PRIMARY KEY (`discord_id`, `account_id`),
    UNIQUE KEY `account_id` (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
"""

# Migration: เปลี่ยน PRIMARY KEY เดิม (discord_id) เป็น compound key (discord_id, account_id)
_MIGRATE_LINKS_TABLE = """
ALTER TABLE `discord_links`
    DROP PRIMARY KEY,
    ADD PRIMARY KEY (`discord_id`, `account_id`);
"""


async def _create_main_pool(common: dict) -> None:
    global _pool
    try:
        _pool = await aiomysql.create_pool(db=config.DB_NAME, **common)
        # ทำ migration เพียงครั้งเดียว (ถ้ามีไฟล์ marker อยู่แล้วข้าม)
        if not os.path.exists(_MIGRATION_MARKER):
            async with _pool.acquire() as conn:
                async with conn.cursor() as cur:
                    await cur.execute(_CREATE_LINKS_TABLE)
                    try:
                        await cur.execute(_MIGRATE_LINKS_TABLE)
                    except Exception:
                        pass
            try:
                os.makedirs(os.path.dirname(_MIGRATION_MARKER), exist_ok=True)
                open(_MIGRATION_MARKER, 'w').close()
            except Exception:
                pass
        logger.info('Main DB pool ready')
    except Exception as e:
        logger.warning('Main DB unavailable (%s) — คำสั่งที่ต้องใช้ฐานข้อมูลจะไม่ทำงาน', e)
        _pool = None


async def _create_log_pool(common: dict) -> None:
    global _log_pool
    try:
        _log_pool = await aiomysql.create_pool(db=config.DB_LOG_NAME, **common)
        logger.info('Log DB pool ready')
    except Exception:
        logger.warning('Log DB unavailable — drop feed will be disabled')
        _log_pool = None


async def create_pools() -> None:
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
    # สร้าง main pool + log pool พร้อมกัน ลดเวลาเชื่อมต่อครึ่งหนึ่ง
    await asyncio.gather(_create_main_pool(common), _create_log_pool(common))


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
