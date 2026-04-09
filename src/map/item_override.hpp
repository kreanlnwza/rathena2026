// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ITEM_OVERRIDE_HPP
#define ITEM_OVERRIDE_HPP

/**
 * item_override.hpp / item_override.cpp
 *
 * Server-side override system providing two permanent features:
 *
 * 1. CROSS-JOB EQUIPMENT: Items equipping outside weapon/ammo slots can be worn by any job.
 *    Slots using EQP_ARMS (EQP_HAND_R|EQP_HAND_L) and EQP_AMMO still enforce the
 *    original job/class restrictions from item_db.yml.
 *    All other equip slots (armor, headgear, accessory, garment, shoes, shadowgear, etc.)
 *    bypass job/class restrictions via item_override_alljob_equip().
 *
 * 2. UNIVERSAL SELL: All items can be sold to NPC shops regardless of the
 *    NoSell trade restriction in item_db.yml.
 *    This is enforced in itemdb_cansell_sub() via item_override_cansell().
 *
 * To add to Visual Studio build: see map-server.vcxproj / map-server.vcxproj.filters
 * CMake: automatically picked up by file(GLOB_RECURSE) in src/map/CMakeLists.txt
 */

struct item_data;
class map_session_data;

/**
 * Cross-job equip override.
 * Called in place of pc_job_can_use_item() and pc_isItemClass() in pc_isequip().
 *
 * @param sd   Player session data (unused but kept for signature compatibility)
 * @param item Item data to check
 * @return true always (any job can equip any item), false only if item is null
 */
bool item_override_alljob_equip(const map_session_data* sd, const item_data* item);

/**
 * Universal sell override.
 * Called in place of the NoSell restriction logic in itemdb_cansell_sub().
 *
 * @param item Item data to check
 * @return true always (any item can be sold), false only if item is null
 */
bool item_override_cansell(const item_data* item);

/**
 * Universal buying store override.
 * Called in place of the flag.buyingstore check in buyingstore.cpp.
 *
 * @param item Item data to check
 * @return true always (any item can be listed in buying store), false only if item is null
 */
bool item_override_canbuyingstore(const item_data* item);

#endif // ITEM_OVERRIDE_HPP
