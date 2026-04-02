--
-- Premium Storage Tables
-- Includes VIP storage and type-restricted premium storages
--

-- VIP Storage
CREATE TABLE IF NOT EXISTS `storage_vip` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `account_id` int(11) unsigned NOT NULL default '0',
  `nameid` int(10) unsigned NOT NULL default '0',
  `amount` smallint(11) unsigned NOT NULL default '0',
  `equip` int(11) unsigned NOT NULL default '0',
  `identify` smallint(6) unsigned NOT NULL default '0',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `attribute` tinyint(4) unsigned NOT NULL default '0',
  `card0` int(10) unsigned NOT NULL default '0',
  `card1` int(10) unsigned NOT NULL default '0',
  `card2` int(10) unsigned NOT NULL default '0',
  `card3` int(10) unsigned NOT NULL default '0',
  `option_id0` smallint(5) unsigned NOT NULL default '0',
  `option_val0` smallint(5) unsigned NOT NULL default '0',
  `option_parm0` tinyint(3) unsigned NOT NULL default '0',
  `option_id1` smallint(5) unsigned NOT NULL default '0',
  `option_val1` smallint(5) unsigned NOT NULL default '0',
  `option_parm1` tinyint(3) unsigned NOT NULL default '0',
  `option_id2` smallint(5) unsigned NOT NULL default '0',
  `option_val2` smallint(5) unsigned NOT NULL default '0',
  `option_parm2` tinyint(3) unsigned NOT NULL default '0',
  `option_id3` smallint(5) unsigned NOT NULL default '0',
  `option_val3` smallint(5) unsigned NOT NULL default '0',
  `option_parm3` tinyint(3) unsigned NOT NULL default '0',
  `option_id4` smallint(5) unsigned NOT NULL default '0',
  `option_val4` smallint(5) unsigned NOT NULL default '0',
  `option_parm4` tinyint(3) unsigned NOT NULL default '0',
  `expire_time` int(11) unsigned NOT NULL default '0',
  `bound` tinyint(3) unsigned NOT NULL default '0',
  `unique_id` bigint(20) unsigned NOT NULL default '0',
  `enchantgrade` tinyint unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

-- Costume Storage (ID: 9)
CREATE TABLE IF NOT EXISTS `storage_costume` LIKE `storage_vip`;

-- Healing Storage (ID: 10)
CREATE TABLE IF NOT EXISTS `storage_healing` LIKE `storage_vip`;

-- Usable Storage (ID: 11)
CREATE TABLE IF NOT EXISTS `storage_usable` LIKE `storage_vip`;

-- Etc Storage (ID: 12)
CREATE TABLE IF NOT EXISTS `storage_etc` LIKE `storage_vip`;

-- Armor Storage (ID: 13)
CREATE TABLE IF NOT EXISTS `storage_armor` LIKE `storage_vip`;

-- Weapon Storage (ID: 14)
CREATE TABLE IF NOT EXISTS `storage_weapon` LIKE `storage_vip`;

-- Card Storage (ID: 15)
CREATE TABLE IF NOT EXISTS `storage_card` LIKE `storage_vip`;

-- Pet Egg Storage (ID: 16)
CREATE TABLE IF NOT EXISTS `storage_petegg` LIKE `storage_vip`;

-- Pet Armor Storage (ID: 17)
CREATE TABLE IF NOT EXISTS `storage_petarmor` LIKE `storage_vip`;

-- Ammo Storage (ID: 18)
CREATE TABLE IF NOT EXISTS `storage_ammo` LIKE `storage_vip`;

-- Shadow Gear Storage (ID: 19)
CREATE TABLE IF NOT EXISTS `storage_shadowgear` LIKE `storage_vip`;
