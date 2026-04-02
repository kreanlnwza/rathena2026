// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Premium Storage Item-Type Filter
 * Restricts premium storages to only accept specific item types.
 * - Storage ID 9: Costume items only (equip mask check)
 * - Storage ID 10-19: Specific item_types only
 * Author: V!be Coding [kreanlnwza] AI Assistant (Claude)
 **/

#include "storage_type_item.hpp"

#include "itemdb.hpp"
#include "pc.hpp"

/**
 * Get the allowed item_types for a given storage ID
 * @param stor_id Storage ID (10-19)
 * @return Corresponding item_types value, or IT_MAX if not mapped
 * @author V!be Coding [kreanlnwza] AI Assistant (Claude)
 */
static item_types storage_get_allowed_type(uint16 stor_id)
{
	switch (stor_id) {
		case 10: return IT_HEALING;
		case 11: return IT_USABLE;
		case 12: return IT_ETC;
		case 13: return IT_ARMOR;
		case 14: return IT_WEAPON;
		case 15: return IT_CARD;
		case 16: return IT_PETEGG;
		case 17: return IT_PETARMOR;
		case 18: return IT_AMMO;
		case 19: return IT_SHADOWGEAR;
		default: return IT_MAX;
	}
}

/**
 * Get the display name of the allowed item type for a given storage ID
 * @param stor_id Storage ID
 * @return Human-readable type name string
 */
const char* storage_type_item_get_type_name(uint16 stor_id)
{
	switch (stor_id) {
		case 9:  return "Costume";
		case 10: return "Healing";
		case 11: return "Usable";
		case 12: return "Etc";
		case 13: return "Armor";
		case 14: return "Weapon";
		case 15: return "Card";
		case 16: return "Pet Egg";
		case 17: return "Pet Armor";
		case 18: return "Ammo";
		case 19: return "Shadow Gear";
		default: return "required";
	}
}

/**
 * Check if an item can be stored in a type-restricted premium storage
 * @param stor_id Storage ID
 * @param data Item data
 * @return true if item is allowed, false otherwise
 * @author V!be Coding [kreanlnwza] AI Assistant (Claude)
 */
bool costume_storage_canstore(uint16 stor_id, struct item_data *data)
{
	if (data == nullptr)
		return false;

	// Costume-Only Storage: check equip mask
	if (stor_id == COSTUME_STORAGE_ID) {
		if (!(data->equip & EQP_COSTUME))
			return false;
		return true;
	}

	// Item-Type Storage: check item type
	if (stor_id >= ITEMTYPE_STORAGE_ID_FIRST && stor_id <= ITEMTYPE_STORAGE_ID_LAST) {
		item_types allowed = storage_get_allowed_type(stor_id);
		if (allowed == IT_MAX)
			return true; // unmapped ID, allow all
		if (data->type != allowed)
			return false;
		return true;
	}

	// Not a type-restricted storage, allow everything
	return true;
}
