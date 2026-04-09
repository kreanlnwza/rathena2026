// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * item_override.cpp
 *
 * Implements server-side overrides for cross-job equipment and universal item selling.
 * See item_override.hpp for full documentation.
 */

#include "item_override.hpp"
#include "../common/nullpo.hpp"
#include "itemdb.hpp"
#include "pc.hpp"

/**
 * Cross-job equip override.
 * Bypasses all job/class restrictions — any job can equip any item.
 */
bool item_override_alljob_equip(const map_session_data* sd, const item_data* item) {
	nullpo_retr(false, sd);
	nullpo_retr(false, item);
	// Always allow — job/class restrictions are intentionally ignored.
	return true;
}

/**
 * Universal sell override.
 * Bypasses the NoSell trade restriction — all items can be sold to NPC shops.
 */
bool item_override_cansell(const item_data* item) {
	// Always allow selling if item pointer is valid.
	return item != nullptr;
}

/**
 * Universal buying store override.
 * Bypasses flag.buyingstore — all items can be listed in a buying store.
 */
bool item_override_canbuyingstore(const item_data* item) {
	// Always allow if item pointer is valid.
	return item != nullptr;
}
