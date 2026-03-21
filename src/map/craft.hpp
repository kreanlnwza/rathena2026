/**
 * Craft Barter System
 * Custom barter shop with crafting success/failure rates and announcements.
 * Author: V!be Coding [kreanlnwza] AI Assistant (Antigravity)
 **/
#ifndef MAP_CRAFT_HPP
#define MAP_CRAFT_HPP

#include "npc.hpp"
#include <ryml_std.hpp>

// Craft System message IDs (conf/msg_conf/import/Custom_msg.conf)
#define MSG_CRAFT_ENTER_SHOP    3010
#define MSG_CRAFT_MULTI_INFO    3011
#define MSG_CRAFT_HEADER        3012
#define MSG_CRAFT_ITEM_RATE     3013
#define MSG_CRAFT_ITEM_RATE_EX  3014
#define MSG_CRAFT_FOOTER        3015
#define MSG_CRAFT_ONE_TYPE      3016
#define MSG_CRAFT_ANNOUNCE      3017

// Parsing functions
bool craft_parse_barter_node(const ryml::NodeRef& node, std::shared_ptr<s_npc_barter> barter);
bool craft_parse_barter_item_node(const ryml::NodeRef& itemNode, std::shared_ptr<s_npc_barter_item> item);

// Logic functions
void craft_barter_open(map_session_data* sd, std::shared_ptr<s_npc_barter> barter);
bool craft_barter_check_buy(map_session_data* sd, std::shared_ptr<s_npc_barter> barter, int32 entries);
e_purchase_result craft_barter_purchase(map_session_data* sd, std::shared_ptr<s_npc_barter> barter, std::vector<s_barter_purchase>& purchases, int64 requiredZeny, uint32* requiredItems);

#endif // MAP_CRAFT_HPP
