// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the body of custom script commands in this file.
 * Format:
 *	BUILDIN_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//BUILDIN_FUNC(example)
//{
//	ShowInfo("buildin_example: It works!\n");
//	script_pushint(st,1);
//	return 0;
//}

// ==========================================================
// Gachapong System - Script Commands
// Author: V!be Coding [kreanlnwza] AI Assistant (Claude)
// ==========================================================

#include "../map/gachapong.hpp"
#include "../map/itemdb.hpp"

/**
 * gachapong_rand(<type>{,<amount>})
 * สุ่มไอเทมจาก item_db ตาม item_types แล้วให้ผู้เล่น
 * @param type - item_types (IT_ETC=3, IT_ARMOR=4, IT_WEAPON=5, IT_CARD=6, IT_PETARMOR=8, IT_SHADOWGEAR=12)
 * @param amount - จำนวน (default: 1)
 * @return item_id ที่สุ่มได้ (0 = ไม่พบ)
 */
BUILDIN_FUNC(gachapong_rand)
{
	map_session_data* sd;

	if (!script_rid2sd(sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	int32 type = script_getnum(st, 2);
	int32 amount = 1;

	if (script_hasdata(st, 3))
		amount = script_getnum(st, 3);

	if (amount <= 0)
		amount = 1;

	// ตรวจสอบว่า gachapong_db ถูก init แล้ว
	if (!gachapong_db.is_initialized()) {
		ShowError("buildin_gachapong_rand: Gachapong database not initialized!\n");
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	// สุ่มไอเทม
	t_itemid nameid = gachapong_db.get_random_item(type);

	if (nameid == 0) {
		ShowWarning("buildin_gachapong_rand: No items found for type %d.\n", type);
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	// ตรวจสอบไอเทมว่ามีอยู่จริง
	std::shared_ptr<item_data> id = item_db.find(nameid);

	if (id == nullptr) {
		ShowError("buildin_gachapong_rand: Item %u not found in item_db.\n", nameid);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	// สร้าง item และให้ผู้เล่น
	struct item it = {};
	it.nameid = nameid;
	it.identify = 1;

	int32 get_count;
	if (!itemdb_isstackable2(id.get())) {
		it.amount = 1;
		get_count = amount;
	} else {
		it.amount = amount;
		get_count = 1;
	}

	for (int32 i = 0; i < get_count; i++) {
		if (!pet_create_egg(sd, nameid)) {
			e_additem_result flag = pc_additem(sd, &it, it.amount, LOG_TYPE_SCRIPT);

			if (flag != ADDITEM_SUCCESS) {
				clif_additem(sd, 0, 0, flag);
				ShowError("buildin_gachapong_rand: Failed to add item %u to player.\n", nameid);
				script_pushint(st, 0);
				return SCRIPT_CMD_FAILURE;
			}
		}
	}

	// Return item_id ที่สุ่มได้
	script_pushint(st, nameid);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * gachapong_count(<type>)
 * นับจำนวนไอเทมใน item_db ตาม item_types
 * @param type - item_types
 * @return จำนวนไอเทม
 */
BUILDIN_FUNC(gachapong_count)
{
	int32 type = script_getnum(st, 2);

	if (!gachapong_db.is_initialized()) {
		ShowError("buildin_gachapong_count: Gachapong database not initialized!\n");
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, gachapong_db.get_type_count(type));
	return SCRIPT_CMD_SUCCESS;
}

/**
 * gachapong_build()
 * สร้าง/rebuild cache ของ gachapong database
 * ใช้เมื่อ reload item_db แล้วต้องการ update cache
 */
BUILDIN_FUNC(gachapong_build)
{
	gachapong_db.build_cache();
	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * gachapong_rand_equip(<equip_pos>{,<amount>})
 * สุ่มไอเทมจาก item_db ตาม equip_pos แล้วให้ผู้เล่น
 * @param equip_pos - EQP_HEAD_TOP=0x100, EQP_ARMOR=0x10, EQP_HAND_R=0x2, etc.
 * @param amount - จำนวน (default: 1)
 * @return item_id ที่สุ่มได้ (0 = ไม่พบ)
 */
BUILDIN_FUNC(gachapong_rand_equip)
{
	map_session_data* sd;

	if (!script_rid2sd(sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	int32 pos = script_getnum(st, 2);
	int32 amount = 1;

	if (script_hasdata(st, 3))
		amount = script_getnum(st, 3);

	if (amount <= 0)
		amount = 1;

	if (!gachapong_db.is_initialized()) {
		ShowError("buildin_gachapong_rand_equip: Gachapong database not initialized!\n");
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	t_itemid nameid = gachapong_db.get_random_equip_item(static_cast<uint32>(pos));

	if (nameid == 0) {
		ShowWarning("buildin_gachapong_rand_equip: No items found for equip_pos 0x%X.\n", pos);
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	std::shared_ptr<item_data> id = item_db.find(nameid);

	if (id == nullptr) {
		ShowError("buildin_gachapong_rand_equip: Item %u not found in item_db.\n", nameid);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	struct item it = {};
	it.nameid = nameid;
	it.identify = 1;

	int32 get_count;
	if (!itemdb_isstackable2(id.get())) {
		it.amount = 1;
		get_count = amount;
	} else {
		it.amount = amount;
		get_count = 1;
	}

	for (int32 i = 0; i < get_count; i++) {
		if (!pet_create_egg(sd, nameid)) {
			e_additem_result flag = pc_additem(sd, &it, it.amount, LOG_TYPE_SCRIPT);

			if (flag != ADDITEM_SUCCESS) {
				clif_additem(sd, 0, 0, flag);
				ShowError("buildin_gachapong_rand_equip: Failed to add item %u to player.\n", nameid);
				script_pushint(st, 0);
				return SCRIPT_CMD_FAILURE;
			}
		}
	}

	script_pushint(st, nameid);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * gachapong_count_equip(<equip_pos>)
 * นับจำนวนไอเทมใน item_db ตาม equip_pos
 * @param equip_pos - EQP_HEAD_TOP, EQP_ARMOR, etc.
 * @return จำนวนไอเทม
 */
BUILDIN_FUNC(gachapong_count_equip)
{
	int32 pos = script_getnum(st, 2);

	if (!gachapong_db.is_initialized()) {
		ShowError("buildin_gachapong_count_equip: Gachapong database not initialized!\n");
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, gachapong_db.get_equip_count(static_cast<uint32>(pos)));
	return SCRIPT_CMD_SUCCESS;
}
