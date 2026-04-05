// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef COSTUME_STORAGE_HPP
#define COSTUME_STORAGE_HPP

/**
 * Premium Storage Item-Type Filter
 * Validates items stored in type-specific premium storages.
 * Supports costume-only (ID 9) and item-type based storages (ID 10-19).
 * Author: V!be Coding [kreanlnwza] AI Assistant (Claude)
 **/

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

struct s_storage;
struct item;
struct item_data;

/// Costume storage ID
#define COSTUME_STORAGE_ID 9

/// Item-type storage ID range
#define ITEMTYPE_STORAGE_ID_FIRST 10
#define ITEMTYPE_STORAGE_ID_LAST  19

/// Error message format shown when item type is not allowed in the storage
#define STORAGE_TYPE_DENY_MSG_FMT "This storage only accepts %s items."

bool costume_storage_canstore(uint16 stor_id, struct item_data *data);
const char* storage_type_item_get_type_name(uint16 stor_id);

#endif /* COSTUME_STORAGE_HPP */
