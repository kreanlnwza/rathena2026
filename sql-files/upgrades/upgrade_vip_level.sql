-- VIP Multi-Level System
-- Adds vip_level column to login table for multi-level VIP support (levels 1-10)
ALTER TABLE `login` ADD COLUMN `vip_level` tinyint(3) unsigned NOT NULL DEFAULT '0' AFTER `vip_time`;
