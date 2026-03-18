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

// Monster Rank System
int32 mob_rank_system;        ///< Monster Rank System: 0=off, 1=on
int32 mob_rank_boss;          ///< Allow rank for boss monsters: 0=no, 1=yes
int32 mob_rank_mvp;           ///< Allow rank for MVP monsters: 0=no, 1=yes
int32 mob_rank_weight_f;
int32 mob_rank_weight_e;
int32 mob_rank_weight_d;
int32 mob_rank_weight_c;
int32 mob_rank_weight_b;
int32 mob_rank_weight_bplus;
int32 mob_rank_weight_a;
int32 mob_rank_weight_aplus;
int32 mob_rank_weight_s;
int32 mob_rank_weight_splus;
int32 mob_rank_weight_ss;
int32 mob_rank_weight_ssplus;
int32 mob_rank_weight_sss;
int32 mob_rank_weight_ex;
int32 mob_rank_weight_explus;
int32 mob_rank_coin_drop;        ///< Mob Rank Coin Drop: 0=off, 1=on
int32 mob_rank_coin_itemid;      ///< Item ID for rank coin (default: 7539)
