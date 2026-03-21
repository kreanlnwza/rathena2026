/**
 * Craft Barter System
 * Custom barter shop with crafting success/failure rates and announcements.
 * Author: V!be Coding [kreanlnwza] AI Assistant (Antigravity)
 **/
#include "craft.hpp"
#include "map.hpp"
#include "clif.hpp"
#include "pc.hpp"
#include "itemdb.hpp"
#include "intif.hpp"
#include "log.hpp"
#include "npc.hpp"
#include "unit.hpp"

/**
 * Parse CraftBarter and WaitingRoom fields from barter YAML node
 * @param node YAML node
 * @param barter barter data
 * @return true on success
 */
bool craft_parse_barter_node(const ryml::NodeRef& node, std::shared_ptr<s_npc_barter> barter) {
	barter->waitingroom.clear();
	if (node.has_child("CraftBarter")) {
		node["CraftBarter"] >> barter->is_craft;
	}

	if (node.has_child("WaitingRoom")) {
		std::string chat;
		node["WaitingRoom"] >> chat;
		barter->waitingroom = chat;
	}
	return true;
}

/**
 * Parse Rate, AnnounceSuccess, AnnounceFail from barter item YAML node
 * @param itemNode YAML item node
 * @param item barter item data
 * @return true on success
 */
bool craft_parse_barter_item_node(const ryml::NodeRef& itemNode, std::shared_ptr<s_npc_barter_item> item) {
	if (itemNode.has_child("Rate")) {
		itemNode["Rate"] >> item->craft_rate;
	}

	if (itemNode.has_child("AnnounceSuccess")) {
		itemNode["AnnounceSuccess"] >> item->announcesuccess;
	}

	if (itemNode.has_child("AnnounceFail")) {
		itemNode["AnnounceFail"] >> item->announcefail;
	}
	return true;
}

/**
 * Show craft rate information when player opens a craft barter shop
 * @param sd player session
 * @param barter barter data
 */
void craft_barter_open(map_session_data* sd, std::shared_ptr<s_npc_barter> barter) {
	if (!barter->is_craft)
		return;

	char output[CHAT_SIZE_MAX];

	safesnprintf(output, sizeof(output), "%s", msg_txt(sd, 3010));
	clif_broadcast(sd, output, (int32)strlen(output) + 1, BC_BLUE, SELF);
	clif_messagecolor(sd, color_table[COLOR_DEFAULT], msg_txt(sd, 3011), false, SELF);
	clif_messagecolor(sd, color_table[COLOR_DEFAULT], msg_txt(sd, 3012), false, SELF);

	char craft_msg[CHAT_SIZE_MAX];
	char zeny_str[32];

	for (const auto& itemPair : barter->items) {
		std::shared_ptr<item_data> id = item_db.find(itemPair.second->nameid);
		format_number_comma(itemPair.second->price, zeny_str, sizeof(zeny_str));

		if (!itemPair.second->inherit_str.length())
			safesnprintf(craft_msg, sizeof(craft_msg), msg_txt(sd, 3013), item_db.create_item_link(id).c_str(), zeny_str, (float)itemPair.second->craft_rate / 100);
		else
			safesnprintf(craft_msg, sizeof(craft_msg), msg_txt(sd, 3014), item_db.create_item_link(id).c_str(), zeny_str, (float)itemPair.second->craft_rate / 100, itemPair.second->inherit_str.c_str());

		clif_messagecolor(sd, color_table[COLOR_DEFAULT], craft_msg, false, SELF);
	}
	clif_messagecolor(sd, color_table[COLOR_DEFAULT], msg_txt(sd, 3015), false, SELF);
}

/**
 * Check if player is trying to buy more than one type in craft barter
 * @param sd player session
 * @param barter barter data
 * @param entries number of different items being purchased
 * @return true if purchase is allowed
 */
bool craft_barter_check_buy(map_session_data* sd, std::shared_ptr<s_npc_barter> barter, int32 entries) {
	if (barter->is_craft && entries > 1) {
		clif_messagecolor(sd, color_table[COLOR_LIGHT_GREEN], msg_txt(sd, 3016), false, SELF);
		return false;
	}
	return true;
}

/**
 * Process craft barter purchase with success/failure rate
 * @param sd player session
 * @param barter barter data
 * @param purchases list of purchases
 * @param requiredZeny total zeny cost
 * @param requiredItems array of required item amounts per inventory index
 * @return purchase result
 */
e_purchase_result craft_barter_purchase(map_session_data* sd, std::shared_ptr<s_npc_barter> barter, std::vector<s_barter_purchase>& purchases, int64 requiredZeny, uint32* requiredItems) {
	for (int32 i = 0; i < MAX_INVENTORY; i++) {
		if (requiredItems[i] > 0) {
			if (pc_delitem(sd, i, requiredItems[i], 0, 0, LOG_TYPE_BARTER) != 0) {
				return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
			}
		}
	}

	for (s_barter_purchase& purchase : purchases) {
		if (purchase.data->type == IT_PETEGG) {
			return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
		}
		if (purchase.item->stockLimited) {
			purchase.item->stock -= purchase.amount;

			if (Sql_Query(mmysql_handle, "REPLACE INTO `%s` (`name`,`index`,`amount`) VALUES ( '%s', '%hu', '%hu' )", barter_table, barter->name.c_str(), purchase.item->index, purchase.item->stock) != SQL_SUCCESS) {
				Sql_ShowDebug(mmysql_handle);
				return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
			}
		}

		int32 total_amount = purchase.amount;
		int32 success_count = 0;
		int32 fail_count = 0;
		struct item it = {};
		it.nameid = purchase.item->nameid;
		it.identify = true;

		for (int32 i = 0; i < purchase.amount; i++) {
			int32 chance = rnd() % 10000;
			int32 zenyeach = (int32)(requiredZeny / purchase.amount);
			bool isSuccess = (chance < purchase.item->craft_rate);

			if (isSuccess) {
				success_count++;
				clif_specialeffect(sd, 154, AREA);
				pc_payzeny(sd, zenyeach, LOG_TYPE_BARTER, 0);
				pc_additem(sd, &it, 1, LOG_TYPE_BARTER);
			} else {
				fail_count++;
				clif_specialeffect(sd, 155, AREA);
				pc_payzeny(sd, zenyeach, LOG_TYPE_BARTER, 0);
			}
		}

		if (purchase.item->announcesuccess || purchase.item->announcefail) {
			char ann_msg[CHAT_SIZE_MAX];
			std::shared_ptr<item_data> ann_id = item_db.find(purchase.item->nameid);
			safesnprintf(ann_msg, sizeof(ann_msg), msg_txt(sd, 3017), sd->status.name,
				ann_id ? item_db.create_item_link(ann_id).c_str() : "Unknown",
				total_amount, success_count, fail_count);
			intif_broadcast2(ann_msg, (int32)strlen(ann_msg) + 1, 0x0000ff, FW_NORMAL, 12, 0, 0);
		}
	}
	return e_purchase_result::PURCHASE_SUCCEED;
}
