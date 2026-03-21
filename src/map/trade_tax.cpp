// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "trade_tax.hpp"

#include "battle.hpp"        // For battle_config
#include "chrif.hpp"         // For LOG_TYPE_TRADE
#include "log.hpp"           // For LOG_TYPE_TRADE
#include "clif.hpp"          // For clif_displaymessage
#include "itemdb.hpp"        // For item_db
#include "map.hpp"           // For map_id2sd
#include "pc.hpp"            // For pc_payzeny, pc_getzeny
#include "trade.hpp"
#include "unit.hpp"

#include <cstdio>
#include <cstring>

/**
 * Format a uint64 number with commas (e.g., 1234567 -> "1,234,567")
 * @param num: Number to format
 * @param buf: Output buffer
 * @param size: Buffer size
 * @return: Pointer to buf
 */
char* format_commas_u64(uint64 num, char* buf, size_t size) {
    if (size == 0) return buf;

    char temp[64];
    snprintf(temp, sizeof(temp), "%llu", (unsigned long long)num);

    size_t len = strlen(temp);
    size_t commas = (len - 1) / 3;
    size_t total = len + commas;

    if (total >= size) {
        strncpy(buf, temp, size - 1);
        buf[size - 1] = '\0';
        return buf;
    }

    size_t src = len;
    size_t dst = total;
    buf[dst] = '\0';
    int32 count = 0;

    while (src > 0) {
        src--;
        dst--;
        buf[dst] = temp[src];
        count++;
        if (count % 3 == 0 && src > 0) {
            dst--;
            buf[dst] = ',';
        }
    }

    return buf;
}

/**
 * Calculate zeny tax based on amount and tax rate
 * @param zeny_amount: Amount of zeny being traded
 * @param tax_rate: Tax rate as percentage
 * @return: Tax amount in zeny
 */
int32 calculate_zeny_tax(int32 zeny_amount, int32 tax_rate) {
    if (zeny_amount <= 0 || tax_rate <= 0) {
        return 0;
    }

    // Calculate percentage (tax_rate is percentage, so divide by 100)
    int64 tax_amount = ((int64)zeny_amount * tax_rate) / 100;

    // Prevent overflow
    if (tax_amount > INT32_MAX) {
        return INT32_MAX;
    }

    return (int32)tax_amount;
}

/**
 * Calculate item tax based on item count and fee per item
 * @param item_count: Number of items being traded
 * @param fee_per_item: Fee charged per item
 * @return: Total item tax
 */
int32 calculate_item_tax(int16 item_count, int32 fee_per_item) {
    if (item_count <= 0 || fee_per_item <= 0) {
        return 0;
    }

    int64 total_fee = (int64)item_count * fee_per_item;

    // Prevent overflow
    if (total_fee > INT32_MAX) {
        return INT32_MAX;
    }

    return (int32)total_fee;
}

/**
 * Calculate refine tax for items based on refine level
 * @param refine_level: Current refine level
 * @param fee_per_refine: Fee charged per refine level
 * @param amount: Number of items with this refine level
 * @return: Total refine tax
 */
int32 calculate_refine_tax(int8 refine_level, int32 fee_per_refine, int16 amount) {
    if (refine_level <= 0 || fee_per_refine <= 0 || amount <= 0) {
        return 0;
    }

    int64 total_refine_tax = (int64)refine_level * fee_per_refine * amount;

    // Prevent overflow
    if (total_refine_tax > INT32_MAX) {
        return INT32_MAX;
    }

    return (int32)total_refine_tax;
}

/**
 * Calculate enchant grade tax for items
 * @param enchant_grade: Enchant grade value
 * @param fee_per_enchant: Fee charged per enchant grade
 * @param amount: Number of items with this enchant grade
 * @return: Total enchant grade tax
 */
int32 calculate_enchant_tax(int8 enchant_grade, int32 fee_per_enchant, int16 amount) {
    if (enchant_grade <= 0 || fee_per_enchant <= 0 || amount <= 0) {
        return 0;
    }

    int64 total_enchant_tax = (int64)enchant_grade * fee_per_enchant * amount;

    // Prevent overflow
    if (total_enchant_tax > INT32_MAX) {
        return INT32_MAX;
    }

    return (int32)total_enchant_tax;
}

/**
 * Check if player can afford to pay the tax
 * @param sd: Player session data
 * @param tax_info: Tax information structure
 * @return: true if player can afford tax, false otherwise
 */
bool can_afford_tax(map_session_data* sd, const TradeTaxInfo& tax_info) {
    if (!sd || tax_info.total_tax <= 0) {
        return true;
    }

    return sd->status.zeny >= tax_info.total_tax;
}

/**
 * Display trade tax information to player
 * @param sd: Player session data
 * @param tax_info: Tax information structure
 */
void display_trade_tax_info(map_session_data* sd, const TradeTaxInfo& tax_info) {
    if (!sd) {
        return;
    }

    char message[1000];

    bool show_mixed_trade_message = false;

    // Show zeny trade details if zeny tax applies
    if (battle_config.trade_zeny_fee > 0 && tax_info.zeny_fee > 0) {
        char original_zeny[64], zeny_fee[64], total_tax[64], remaining_zeny[64], tax_percent[16];

        format_commas_u64((uint64)sd->deal.zeny, original_zeny, sizeof(original_zeny));
        format_commas_u64((uint64)tax_info.zeny_fee, zeny_fee, sizeof(zeny_fee));
        format_commas_u64((uint64)tax_info.total_tax, total_tax, sizeof(total_tax));
        format_commas_u64((uint64)(sd->status.zeny - tax_info.total_tax), remaining_zeny, sizeof(remaining_zeny));
        sprintf(tax_percent, "%d", battle_config.trade_zeny_fee);

        snprintf(message, sizeof(message),
                 msg_txt(sd, MSG_TAX_ZENY_TRADE),
                 original_zeny,
                 tax_percent,
                 zeny_fee,
                 total_tax,
                 remaining_zeny);
        clif_displaymessage(sd->fd, message);
    }

    // Show item tax details if any item taxes apply
    if (tax_info.item_fee > 0 || tax_info.refine_tax > 0 || tax_info.enchant_tax > 0) {
        char item_fee_str[64], refine_fee_str[64], enchant_fee_str[64];

        format_commas_u64((uint64)tax_info.item_fee, item_fee_str, sizeof(item_fee_str));
        format_commas_u64((uint64)tax_info.refine_tax, refine_fee_str, sizeof(refine_fee_str));
        format_commas_u64((uint64)tax_info.enchant_tax, enchant_fee_str, sizeof(enchant_fee_str));

        snprintf(message, sizeof(message),
                 msg_txt(sd, MSG_TAX_ITEM_FEE),
                 item_fee_str,
                 refine_fee_str,
                 enchant_fee_str);
        clif_displaymessage(sd->fd, message);

        // Check if we have both zeny and item taxes for mixed trade
        if (battle_config.trade_zeny_fee > 0 && tax_info.zeny_fee > 0) {
            show_mixed_trade_message = true;
        }

        // Show remaining balance after item tax deduction
        {
            char zeny_received[64], remaining_balance[64];
            format_commas_u64((uint64)sd->deal.zeny, zeny_received, sizeof(zeny_received));
            format_commas_u64((uint64)(sd->status.zeny - tax_info.total_tax), remaining_balance, sizeof(remaining_balance));

            snprintf(message, sizeof(message),
                     msg_txt(sd, MSG_TAX_ZENY_RECEIVED),
                     zeny_received,
                     remaining_balance);
            clif_displaymessage(sd->fd, message);
        }
    }

    // Show simple zeny received message if only zeny is involved
    if (sd->deal.zeny > 0 && !(battle_config.trade_zeny_fee > 0 && tax_info.zeny_fee > 0)) {
        char zeny_amount[64], current_zeny[64];

        format_commas_u64((uint64)sd->deal.zeny, zeny_amount, sizeof(zeny_amount));
        format_commas_u64((uint64)(sd->status.zeny + sd->deal.zeny), current_zeny, sizeof(current_zeny));

        snprintf(message, sizeof(message),
                 msg_txt(sd, MSG_TAX_ZENY_RECEIVED),
                 zeny_amount,
                 current_zeny);
        clif_displaymessage(sd->fd, message);
    }

    // Show mixed trade total tax message if both zeny and items are involved
    if (show_mixed_trade_message && tax_info.total_tax > 0) {
        char total_fee[64];
        format_commas_u64((uint64)tax_info.total_tax, total_fee, sizeof(total_fee));

        snprintf(message, sizeof(message),
                 msg_txt(sd, MSG_TAX_MIXED_TOTAL),
                 total_fee);
        clif_displaymessage(sd->fd, message);
    }
}

/**
 * Calculate trade tax for a player
 * @param sd: Player session data
 * @return: TradeTaxInfo with calculated taxes
 */
TradeTaxInfo calculate_trade_tax(map_session_data* sd) {
    TradeTaxInfo tax_info = {0, 0, 0, 0, 0};

    if (!sd || !sd->state.trading) {
        return tax_info;
    }

    // Calculate zeny tax
    tax_info.zeny_fee = calculate_zeny_tax(sd->deal.zeny, battle_config.trade_zeny_fee);

    // Count number of items being traded
    int16 item_count = 0;
    for (int i = 0; i < 10; i++) {
        if (sd->deal.item[i].amount) {
            item_count++;

            // Calculate special taxes for each item
            int n = sd->deal.item[i].index;
            struct item *item = &sd->inventory.u.items_inventory[n];

            // Calculate taxes (no individual item messages to avoid redundancy)
            tax_info.refine_tax += calculate_refine_tax(item->refine, battle_config.trade_refine_fee, sd->deal.item[i].amount);
            tax_info.enchant_tax += calculate_enchant_tax(item->enchantgrade, battle_config.trade_enchantgrade_fee, sd->deal.item[i].amount);
        }
    }

    // Calculate item tax
    tax_info.item_fee = calculate_item_tax(item_count, battle_config.tradeitem_fee);

    // Calculate total tax
    tax_info.total_tax = tax_info.zeny_fee + tax_info.item_fee + tax_info.refine_tax + tax_info.enchant_tax;

    return tax_info;
}

/**
 * Calculate trade tax for target player (silent mode - no messages)
 * @param sd: Target player session data
 * @return: TradeTaxInfo with calculated taxes
 */
TradeTaxInfo calculate_target_trade_tax(map_session_data* sd) {
    TradeTaxInfo tax_info = {0, 0, 0, 0, 0};

    if (!sd || !sd->state.trading) {
        return tax_info;
    }

    // Calculate zeny tax
    tax_info.zeny_fee = calculate_zeny_tax(sd->deal.zeny, battle_config.trade_zeny_fee);

    // Count number of items being traded and calculate special taxes
    int16 item_count = 0;
    for (int i = 0; i < 10; i++) {
        if (sd->deal.item[i].amount) {
            item_count++;

            // Calculate special taxes for each item (silent)
            int n = sd->deal.item[i].index;
            struct item *item = &sd->inventory.u.items_inventory[n];

            tax_info.refine_tax += calculate_refine_tax(item->refine, battle_config.trade_refine_fee, sd->deal.item[i].amount);
            tax_info.enchant_tax += calculate_enchant_tax(item->enchantgrade, battle_config.trade_enchantgrade_fee, sd->deal.item[i].amount);
        }
    }

    // Calculate item tax
    tax_info.item_fee = calculate_item_tax(item_count, battle_config.tradeitem_fee);

    // Calculate total tax
    tax_info.total_tax = tax_info.zeny_fee + tax_info.item_fee + tax_info.refine_tax + tax_info.enchant_tax;

    return tax_info;
}

/**
 * Deduct trade tax from player
 * @param sd: Player session data
 * @param tax_info: Tax information structure
 * @param target_char_id: Character ID of the trade partner (for logging)
 */
void deduct_trade_tax(map_session_data* sd, const TradeTaxInfo& tax_info, int32 target_char_id) {
    if (!sd || tax_info.total_tax <= 0) {
        return;
    }

    pc_payzeny(sd, tax_info.total_tax, LOG_TYPE_TRADE, target_char_id);
}
