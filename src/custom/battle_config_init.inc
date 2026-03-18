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
