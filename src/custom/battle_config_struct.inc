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

	int32 refine_delay_time; // Delay in milliseconds between refine attempts (default: 1800ms) [@krit.k #3614]
	int32 refine_ui_lock_time; // Lock time in milliseconds for refine UI after each refine attempt (default: 3000ms) [@krit.k #3614]

