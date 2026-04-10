// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef BARTER_HPP
#define BARTER_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>

#include "clif.hpp"   // e_purchase_result
#include "itemdb.hpp" // item_data

class map_session_data;

struct s_npc_barter_requirement{
	uint16 index;
	t_itemid nameid;
	uint16 amount;
	int8 refine;
};

struct s_npc_barter_item{
	uint16 index;
	t_itemid nameid;
	bool stockLimited;
	uint32 stock;
	uint32 price;
	int8 refine;
	uint16 successRate; // craft success rate in basis points (0-10000, 10000 = 100.00%). Used when barter->craft is true.
	std::map<uint16, std::shared_ptr<s_npc_barter_requirement>> requirements;
};

struct s_npc_barter{
	std::string name;
	int16 m;
	uint16 x;
	uint16 y;
	uint8 dir;
	int16 sprite;
	bool craft; // if true, use crafting mechanics: items/zeny are always consumed but item is only given on successful rate roll.
	std::map<uint16, std::shared_ptr<s_npc_barter_item>> items;
	int32 npcid;

	~s_npc_barter();
};

class BarterDatabase : public TypesafeYamlDatabase<std::string, s_npc_barter>{
public:
	BarterDatabase() : TypesafeYamlDatabase( "BARTER_DB", 2, 1 ){

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode( const ryml::NodeRef& node );
	void loadingFinished();
};

extern BarterDatabase barter_db;

struct s_barter_purchase{
	std::shared_ptr<s_npc_barter_item> item;
	uint32 amount;
	item_data* data;
};

e_purchase_result npc_barter_purchase( map_session_data& sd, std::shared_ptr<s_npc_barter> barter, std::vector<s_barter_purchase>& purchases );

#endif /* BARTER_HPP */
