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

// Monster Rank System
{ "mob_rank_system",        &battle_config.mob_rank_system,        0,    0, 1,     },
{ "mob_rank_boss",          &battle_config.mob_rank_boss,          0,    0, 1,     },
{ "mob_rank_mvp",           &battle_config.mob_rank_mvp,           0,    0, 1,     },
{ "mob_rank_weight_f",      &battle_config.mob_rank_weight_f,      3000, 0, 10000, },
{ "mob_rank_weight_e",      &battle_config.mob_rank_weight_e,      2000, 0, 10000, },
{ "mob_rank_weight_d",      &battle_config.mob_rank_weight_d,      1500, 0, 10000, },
{ "mob_rank_weight_c",      &battle_config.mob_rank_weight_c,      1000, 0, 10000, },
{ "mob_rank_weight_b",      &battle_config.mob_rank_weight_b,      700,  0, 10000, },
{ "mob_rank_weight_bplus",  &battle_config.mob_rank_weight_bplus,  500,  0, 10000, },
{ "mob_rank_weight_a",      &battle_config.mob_rank_weight_a,      350,  0, 10000, },
{ "mob_rank_weight_aplus",  &battle_config.mob_rank_weight_aplus,  250,  0, 10000, },
{ "mob_rank_weight_s",      &battle_config.mob_rank_weight_s,      200,  0, 10000, },
{ "mob_rank_weight_splus",  &battle_config.mob_rank_weight_splus,  150,  0, 10000, },
{ "mob_rank_weight_ss",     &battle_config.mob_rank_weight_ss,     120,  0, 10000, },
{ "mob_rank_weight_ssplus", &battle_config.mob_rank_weight_ssplus, 100,  0, 10000, },
{ "mob_rank_weight_sss",    &battle_config.mob_rank_weight_sss,    60,   0, 10000, },
{ "mob_rank_weight_ex",     &battle_config.mob_rank_weight_ex,     40,   0, 10000, },
{ "mob_rank_weight_explus", &battle_config.mob_rank_weight_explus, 30,   0, 10000, },
{ "mob_rank_coin_drop",     &battle_config.mob_rank_coin_drop,     1,    0, 1,          },
{ "mob_rank_coin_itemid",   &battle_config.mob_rank_coin_itemid,   7539, 0, INT32_MAX,  },
{ "mob_rank_reshuffle_interval", &battle_config.mob_rank_reshuffle_interval, 180, 0, 1440, },

// Trade tax configuration
{ "trade_zeny_fee", &battle_config.trade_zeny_fee, 20, 0, INT_MAX },
{ "tradeitem_fee", &battle_config.tradeitem_fee, 2500, 0, INT_MAX },
{ "trade_refine_fee", &battle_config.trade_refine_fee, 2500, 0, INT_MAX },
{ "trade_enchantgrade_fee", &battle_config.trade_enchantgrade_fee, 2500, 0, INT_MAX },

// Sell tax configuration
{ "sell_zeny_fee", &battle_config.sell_zeny_fee, 30, 0, 100 },

// Refine UI Protection
{ "refine_delay_time",                  &battle_config.refine_delay_time,                  1800,   0,      30000,        },
{ "refine_ui_lock_time",                &battle_config.refine_ui_lock_time,                3000,   0,      10000,        },
