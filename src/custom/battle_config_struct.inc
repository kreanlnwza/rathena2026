// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom battle config structure
 * Place the definition of custom battle configuration structure members in this file.
 *
 * Make sure that the name you want is not already taken by looking into battle.hpp
 * Do not forget to add initilization logic to battle_config_init.inc
 *
 * Format:
 *	 <datatype> name;
 **/

	int32 override_equip_all_jobs;		///< 1 = bypass job/class restrictions for non-weapon/ammo slots
	int32 override_item_sell_all;		///< 1 = bypass NoSell trade restriction for all items
