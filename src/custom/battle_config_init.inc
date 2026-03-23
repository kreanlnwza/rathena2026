// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom battle config initializations
 * Place the definition of custom battle configuration initializations in this file.
 *
 * Do not forget to add a comma at the end of the line, if you want to add another config
 *
 * Format:
 *	 { "name", &battle_config.<variable name>, <default value>, <minimum value>, <maximum value> },
 **/

	{ "refine_delay_time",                  &battle_config.refine_delay_time,                  1800,   0,      30000,        },
	{ "refine_ui_lock_time",                &battle_config.refine_ui_lock_time,                3000,   0,      10000,        },

