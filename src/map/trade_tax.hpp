// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef TRADE_TAX_HPP
#define TRADE_TAX_HPP

#include "../common/cbasetypes.hpp"
#include "../common/mmo.hpp" // For map_session_data

/**
 * Trade Tax System
 * Handles tax calculations for zeny, items, refine levels, and enchant grades
 */

// Tax calculation structure
struct TradeTaxInfo {
    int32 zeny_fee;          // Tax on zeny transfer
    int32 item_fee;          // Tax on regular items
    int32 refine_tax;        // Additional tax for refine levels
    int32 enchant_tax;       // Additional tax for enchant grades
    int32 total_tax;         // Total tax for this player
};

// Tax message IDs (defined in conf/msg_conf/import/custom_msg.conf)
#define MSG_TAX_ITEM_FEE      3000
#define MSG_TAX_ZENY_TRADE    3001
#define MSG_TAX_ZENY_RECEIVED 3002
#define MSG_TAX_MIXED_TOTAL   3003
#define MSG_TAX_SELL_FEE      3004
#define MSG_TAX_NOT_ENOUGH    3005

// Utility function
char* format_commas_u64(uint64 num, char* buf, size_t size);

// Function declarations
int32 calculate_zeny_tax(int32 zeny_amount, int32 tax_rate);
int32 calculate_item_tax(int16 item_count, int32 fee_per_item);
int32 calculate_refine_tax(int8 refine_level, int32 fee_per_refine, int16 amount);
int32 calculate_enchant_tax(int8 enchant_grade, int32 fee_per_enchant, int16 amount);
bool can_afford_tax(map_session_data* sd, const TradeTaxInfo& tax_info);
void display_trade_tax_info(map_session_data* sd, const TradeTaxInfo& tax_info);
TradeTaxInfo calculate_trade_tax(map_session_data* sd);
TradeTaxInfo calculate_target_trade_tax(map_session_data* sd);
void deduct_trade_tax(map_session_data* sd, const TradeTaxInfo& tax_info, int32 target_char_id);

#endif /* TRADE_TAX_HPP */