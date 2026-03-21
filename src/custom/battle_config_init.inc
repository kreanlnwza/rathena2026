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

// Trade tax configuration
{ "trade_zeny_fee", &battle_config.trade_zeny_fee, 20, 0, INT_MAX },
{ "tradeitem_fee", &battle_config.tradeitem_fee, 2500, 0, INT_MAX },
{ "trade_refine_fee", &battle_config.trade_refine_fee, 2500, 0, INT_MAX },
{ "trade_enchantgrade_fee", &battle_config.trade_enchantgrade_fee, 2500, 0, INT_MAX },

// Sell tax configuration
{ "sell_zeny_fee", &battle_config.sell_zeny_fee, 30, 0, 100 },

