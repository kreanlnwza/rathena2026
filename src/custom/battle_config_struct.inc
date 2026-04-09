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
int32 mob_rank_reshuffle_interval; ///< Re-roll rank interval in minutes (0=off, default: 180 = 3h)

// Trade tax configuration
int32 trade_zeny_fee;
int32 tradeitem_fee;
int32 trade_refine_fee;
int32 trade_enchantgrade_fee;

// Sell tax configuration
int32 sell_zeny_fee;

// Refine UI Protection
int32 refine_delay_time; // Delay in milliseconds between refine attempts (default: 1800ms) [@krit.k #3614]
int32 refine_ui_lock_time; // Lock time in milliseconds for refine UI after each refine attempt (default: 3000ms) [@krit.k #3614]

// Item Override System
int32 override_equip_all_jobs;       ///< 1 = bypass job/class restrictions for non-weapon/ammo slots
int32 override_item_sell_all;        ///< 1 = bypass NoSell trade restriction for all items
int32 override_item_buyingstore_all; ///< 1 = bypass flag.buyingstore, all items listable in buying store
