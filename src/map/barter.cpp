// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "barter.hpp"

#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

#include "battle.hpp"
#include "clif.hpp"
#include "custom_msg.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "pc.hpp"
#include "pet.hpp"
#include "script.hpp"
#include "status.hpp"
#include "unit.hpp"

using namespace rathena;

const std::string BarterDatabase::getDefaultLocation(){
	return "npc/barters.yml";
}

uint64 BarterDatabase::parseBodyNode( const ryml::NodeRef& node ){
	std::string npcname;

	if( !this->asString( node, "Name", npcname ) ){
		return 0;
	}

	std::shared_ptr<s_npc_barter> barter = this->find( npcname );
	bool exists = barter != nullptr;

	if( !exists ){
		barter = std::make_shared<s_npc_barter>();
		barter->name = npcname;
		barter->npcid = 0;
	}

	if( this->nodeExists( node, "Map" ) ){
		std::string map;

		if( !this->asString( node, "Map", map ) ){
			return 0;
		}

		uint16 index = mapindex_name2idx( map.c_str(), nullptr );

		if( index == 0 ){
			this->invalidWarning( node["Map"], "barter_parseBodyNode: Unknown mapname %s, skipping.\n", map.c_str());
			return 0;
		}

		barter->m = map_mapindex2mapid( index );

		// Skip silently if the map is not on this map-server
		if( barter->m < 0 ){
			return 1;
		}
	}else{
		if( !exists ){
			barter->m = -1;
		}
	}

	struct map_data* mapdata = nullptr;

	if( barter->m >= 0 ){
		mapdata = map_getmapdata( barter->m );
	}

	if( this->nodeExists( node, "X" ) ){
		uint16 x;

		if( !this->asUInt16( node, "X", x ) ){
			return 0;
		}

		if( mapdata == nullptr ){
			this->invalidWarning( node["X"], "barter_parseBodyNode: Barter NPC is not on a map. Ignoring X coordinate...\n" );
			x = 0;
		}else if( x >= mapdata->xs ){
			this->invalidWarning( node["X"], "barter_parseBodyNode: X coordinate %hu is out of bound %hu...\n", x, mapdata->xs );
			return 0;
		}

		barter->x = x;
	}else{
		if( !exists ){
			barter->x = 0;
		}
	}

	if( this->nodeExists( node, "Y" ) ){
		uint16 y;

		if( !this->asUInt16( node, "Y", y ) ){
			return 0;
		}

		if( mapdata == nullptr ){
			this->invalidWarning( node["Y"], "barter_parseBodyNode: Barter NPC is not on a map. Ignoring Y coordinate...\n" );
			y = 0;
		}else if( y >= mapdata->ys ){
			this->invalidWarning( node["Y"], "barter_parseBodyNode: Y coordinate %hu is out of bound %hu...\n", y, mapdata->ys );
			return 0;
		}

		barter->y = y;
	}else{
		if( !exists ){
			barter->y = 0;
		}
	}

	if( this->nodeExists( node, "Direction" ) ){
		std::string direction_name;

		if( !this->asString( node, "Direction", direction_name ) ){
			return 0;
		}

		int64 constant;

		if( !script_get_constant( ( "DIR_" + direction_name ).c_str(), &constant ) ){
			this->invalidWarning( node["Direction"], "barter_parseBodyNode: Unknown direction %s, skipping.\n", direction_name.c_str() );
			return 0;
		}

		if( constant < DIR_NORTH || constant >= DIR_MAX ){
			this->invalidWarning( node["Direction"], "barter_parseBodyNode: Invalid direction %s, defaulting to North.\n", direction_name.c_str() );
			constant = DIR_NORTH;
		}

		barter->dir = (uint8)constant;
	}else{
		if( !exists ){
			barter->dir = (uint8)DIR_NORTH;
		}
	}

	if( this->nodeExists( node, "Sprite" ) ){
		std::string sprite_name;

		if( !this->asString( node, "Sprite", sprite_name ) ){
			return 0;
		}

		int64 constant;

		if( !script_get_constant( sprite_name.c_str(), &constant ) ){
			this->invalidWarning( node["Sprite"], "barter_parseBodyNode: Unknown sprite name %s, skipping.\n", sprite_name.c_str());
			return 0;
		}

		if( constant != JT_FAKENPC && !npcdb_checkid( constant ) ){
			this->invalidWarning( node["Sprite"], "barter_parseBodyNode: Invalid sprite name %s, skipping.\n", sprite_name.c_str());
			return 0;
		}

		barter->sprite = (int16)constant;
	}else{
		if( !exists ){
			barter->sprite = JT_FAKENPC;
		}
	}

	if( this->nodeExists( node, "Craft" ) ){
		bool craft;

		if( !this->asBool( node, "Craft", craft ) ){
			return 0;
		}

		barter->craft = craft;
	}else{
		if( !exists ){
			barter->craft = false;
		}
	}

	if( this->nodeExists( node, "Items" ) ){
		for( const ryml::NodeRef& itemNode : node["Items"] ){
			uint16 index;

			if( !this->asUInt16( itemNode, "Index", index ) ){
				return 0;
			}

			std::shared_ptr<s_npc_barter_item> item = util::map_find( barter->items, index );
			bool item_exists = item != nullptr;

			if( !item_exists ){
				if( !this->nodesExist( itemNode, { "Item" } ) ){
					return 0;
				}

				item = std::make_shared<s_npc_barter_item>();
				item->index = index;
			}

			if( this->nodeExists( itemNode, "Item" ) ){
				std::string aegis_name;

				if( !this->asString( itemNode, "Item", aegis_name ) ){
					return 0;
				}

				std::shared_ptr<item_data> id = item_db.search_aegisname( aegis_name.c_str() );

				if( id == nullptr ){
					this->invalidWarning( itemNode["Item"], "barter_parseBodyNode: Unknown item %s.\n", aegis_name.c_str() );
					return 0;
				}

				item->nameid = id->nameid;
			}

			if( this->nodeExists( itemNode, "Stock" ) ){
				uint32 stock;

				if( !this->asUInt32( itemNode, "Stock", stock ) ){
					return 0;
				}

				item->stock = stock;
				item->stockLimited = ( stock > 0 );
			}else{
				if( !item_exists ){
					item->stock = 0;
					item->stockLimited = false;
				}
			}

			if( this->nodeExists( itemNode, "Zeny" ) ){
				uint32 zeny;

				if( !this->asUInt32( itemNode, "Zeny", zeny ) ){
					return 0;
				}

				if( zeny > MAX_ZENY ){
					this->invalidWarning( itemNode["Zeny"], "barter_parseBodyNode: Zeny price %u is above MAX_ZENY (%u), capping...\n", zeny, MAX_ZENY );
					zeny = MAX_ZENY;
				}

				item->price = zeny;
			}else{
				if( !item_exists ){
					item->price = 0;
				}
			}

			if( this->nodeExists( itemNode, "Refine" ) ){
				std::shared_ptr<item_data> data = item_db.find( item->nameid );

				if( data->flag.no_refine ){
					this->invalidWarning( itemNode["Refine"], "barter_parseBodyNode: Item %s is not refineable.\n", data->name.c_str() );
					return 0;
				}

				int16 refine;

				if( !this->asInt16( itemNode, "Refine", refine ) ){
					return 0;
				}

				if( refine > MAX_REFINE ){
					this->invalidWarning( itemNode["Refine"], "barter_parseBodyNode: Refine %hd is too high, capping to %d.\n", refine, MAX_REFINE );
					refine = MAX_REFINE;
				}

				item->refine = (int8)refine;
			}else{
				if( !item_exists ){
					item->refine = 0;
				}
			}

			if( this->nodeExists( itemNode, "SuccessRate" ) ){
				uint16 successRate;

				if( !this->asUInt16( itemNode, "SuccessRate", successRate ) ){
					return 0;
				}

				if( successRate > 10000 ){
					this->invalidWarning( itemNode["SuccessRate"], "barter_parseBodyNode: SuccessRate %hu is above 10000 (100%%), capping...\n", successRate );
					successRate = 10000;
				}

				item->successRate = successRate;
			}else{
				if( !item_exists ){
					item->successRate = 10000;
				}
			}

			if( this->nodeExists( itemNode, "RequiredItems" ) ){
				for( const ryml::NodeRef& requiredItemNode : itemNode["RequiredItems"] ){
					uint16 requirement_index;

					if( !this->asUInt16( requiredItemNode, "Index", requirement_index ) ){
						return 0;
					}

					if( item->requirements.size() >= MAX_BARTER_REQUIREMENTS ){
						this->invalidWarning( requiredItemNode["Index"], "barter_parseBodyNode: Failed at Index %hu. Too many requirements, Barters support up to %d.\n", requirement_index, MAX_BARTER_REQUIREMENTS );
						return 0;
					}

					std::shared_ptr<s_npc_barter_requirement> requirement = util::map_find( item->requirements, requirement_index );
					bool requirement_exists = requirement != nullptr;

					if( !requirement_exists ){
						if( !this->nodesExist( requiredItemNode, { "Item" } ) ){
							return 0;
						}

						requirement = std::make_shared<s_npc_barter_requirement>();
						requirement->index = requirement_index;
					}

					if( this->nodeExists( requiredItemNode, "Item" ) ){
						std::string aegis_name;

						if( !this->asString( requiredItemNode, "Item", aegis_name ) ){
							return 0;
						}

						std::shared_ptr<item_data> data = item_db.search_aegisname( aegis_name.c_str() );

						if( data == nullptr ){
							this->invalidWarning( requiredItemNode["Item"], "barter_parseBodyNode: Unknown required item %s.\n", aegis_name.c_str() );
							return 0;
						}

						requirement->nameid = data->nameid;
					}

					if( this->nodeExists( requiredItemNode, "Amount" ) ){
						uint16 amount;

						if( !this->asUInt16( requiredItemNode, "Amount", amount ) ){
							return 0;
						}

						if( amount > MAX_AMOUNT ){
							this->invalidWarning( requiredItemNode["Amount"], "barter_parseBodyNode: Amount %hu is too high, capping to %hu...\n", amount, MAX_AMOUNT );
							amount = MAX_AMOUNT;
						}

						requirement->amount = amount;
					}else{
						if( !requirement_exists ){
							requirement->amount = 1;
						}
					}

					if( this->nodeExists( requiredItemNode, "Refine" ) ){
						std::shared_ptr<item_data> data = item_db.find( requirement->nameid );

						if( data->flag.no_refine ){
							this->invalidWarning( requiredItemNode["Refine"], "barter_parseBodyNode: Item %s is not refineable.\n", data->name.c_str() );
							return 0;
						}

						int16 refine;

						if( !this->asInt16( requiredItemNode, "Refine", refine ) ){
							return 0;
						}

						if( refine > MAX_REFINE ){
							this->invalidWarning( requiredItemNode["Refine"], "barter_parseBodyNode: Refine %hd is too high, capping to %d.\n", refine, MAX_REFINE );
							refine = MAX_REFINE;
						}

						requirement->refine = (int8)refine;
					}else{
						if( !requirement_exists ){
							requirement->refine = -1;
						}
					}

					if( !requirement_exists ){
						item->requirements[requirement->index] = requirement;
					}
				}
			}

			if( !item_exists ){
				barter->items[index] = item;
			}
		}
	}

	if( !exists ){
		this->put( npcname, barter );
	}

	return 1;
}

BarterDatabase barter_db;

s_npc_barter::~s_npc_barter(){
	if( this->npcid != 0 ){
		npc_data* nd = map_id2nd( this->npcid );

		// Check if the NPC still exists or has been removed already
		if( nd != nullptr ){
			// Delete the NPC
			npc_unload( nd, true );
			// Update NPC event database
			npc_read_event_script();
		}
	}
}

e_purchase_result npc_barter_purchase( map_session_data& sd, std::shared_ptr<s_npc_barter> barter, std::vector<s_barter_purchase>& purchases ){
	uint64 requiredZeny = 0;
	uint32 requiredWeight = 0;
	uint32 reducedWeight = 0;
	uint16 requiredSlots = 0;
	uint32 requiredItems[MAX_INVENTORY] = { 0 };

	for( s_barter_purchase& purchase : purchases ){
		purchase.data = item_db.find( purchase.item->nameid ).get();

		if( purchase.data == nullptr ){
			return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
		}

		uint32 amount = purchase.amount;

		if( purchase.item->stockLimited && purchase.item->stock < amount ){
			return e_purchase_result::PURCHASE_FAIL_STOCK_EMPTY;
		}

		char result = pc_checkadditem( &sd, purchase.item->nameid, amount );

		if( result == CHKADDITEM_OVERAMOUNT ){
			return e_purchase_result::PURCHASE_FAIL_COUNT;
		}else if( result == CHKADDITEM_NEW ){
			requiredSlots += purchase.data->inventorySlotNeeded( amount );
		}

		requiredZeny += ( purchase.item->price * amount );
		requiredWeight += ( purchase.data->weight * amount );

		for( const auto& requirementPair : purchase.item->requirements ){
			std::shared_ptr<s_npc_barter_requirement> requirement = requirementPair.second;
			std::shared_ptr<item_data> id = item_db.find(requirement->nameid);

			if( id == nullptr ){
				return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
			}

			if( itemdb_isstackable2( id.get() ) ){
				int32 j;

				for( j = 0; j < MAX_INVENTORY; j++ ){
					if( sd.inventory.u.items_inventory[j].nameid == requirement->nameid ){
						// Equipped items are not taken into account
						if( sd.inventory.u.items_inventory[j].equip != 0 ){
							continue;
						}

						// Items in equip switch are not taken into account
						if( sd.inventory.u.items_inventory[j].equipSwitch != 0 ){
							continue;
						}

						// Server is configured to hide favorite items on selling
						if( battle_config.hide_fav_sell && sd.inventory.u.items_inventory[j].favorite != 0 ){
							continue;
						}

						// Actually stackable items should never be refinable, but who knows...
						if( requirement->refine >= 0 && sd.inventory.u.items_inventory[j].refine != requirement->refine ){
							// Refine does not match, continue with next item
							continue;
						}

						// Found a match, accumulate required amount
						requiredItems[j] += requirement->amount * amount;

						// Check if there are still enough items available
						if( requiredItems[j] > sd.inventory.u.items_inventory[j].amount ){
							return e_purchase_result::PURCHASE_FAIL_GOODS;
						}

						// Cancel the loop
						break;
					}
				}

				// Required item not found
				if( j == MAX_INVENTORY ){
					return e_purchase_result::PURCHASE_FAIL_GOODS;
				}
			}else{
				for( int32 i = 0; i < (requirement->amount * amount); i++ ){
					int32 j;

					for( j = 0; j < MAX_INVENTORY; j++ ){
						if( sd.inventory.u.items_inventory[j].nameid == requirement->nameid ){
							// Equipped items are not taken into account
							if( sd.inventory.u.items_inventory[j].equip != 0 ){
								continue;
							}

							// Items in equip switch are not taken into account
							if( sd.inventory.u.items_inventory[j].equipSwitch != 0 ){
								continue;
							}

							// Server is configured to hide favorite items on selling
							if( battle_config.hide_fav_sell && sd.inventory.u.items_inventory[j].favorite != 0 ){
								continue;
							}

							// If necessary, check if the refine rate matches
							if( requirement->refine >= 0 && sd.inventory.u.items_inventory[j].refine != requirement->refine ){
								// Refine does not match, continue with next item
								continue;
							}

							// Found a match, since it is not stackable, check if it was already taken
							if( requiredItems[j] > 0 ){
								// Item was already taken, try to find another match
								continue;
							}

							// Mark it as taken
							requiredItems[j] = 1;

							// Cancel the loop
							break;
						}
					}

					// Required item not found
					if( j == MAX_INVENTORY ){
						// Maybe the refine level did not match
						if( requirement->refine >= 0 ){
							int32 refine;

							// Try to find a higher refine level, going from the next lowest to the highest possible
							for( refine = requirement->refine + 1; refine <= MAX_REFINE; refine++ ){
								for( j = 0; j < MAX_INVENTORY; j++ ){
									if( sd.inventory.u.items_inventory[j].nameid == requirement->nameid ){
										// Equipped items are not taken into account
										if( sd.inventory.u.items_inventory[j].equip != 0 ){
											continue;
										}

										// Items in equip switch are not taken into account
										if(	sd.inventory.u.items_inventory[j].equipSwitch != 0 ){
											continue;
										}

										// Server is configured to hide favorite items on selling
										if( battle_config.hide_fav_sell && sd.inventory.u.items_inventory[j].favorite != 0 ){
											continue;
										}

										// If necessary, check if the refine rate matches
										if( requirement->refine >= 0 && sd.inventory.u.items_inventory[j].refine != refine ){
											// Refine does not match, continue with next item
											continue;
										}

										// Found a match, since it is not stackable, check if it was already taken
										if( requiredItems[j] > 0 ){
											// Item was already taken, try to find another match
											continue;
										}

										// Mark it as taken
										requiredItems[j] = 1;

										// Cancel the loop
										break;
									}
								}

								// If a match was found, make sure to cancel the loop
								if( j < MAX_INVENTORY ){
									// Cancel the loop
									break;
								}
							}

							// No matching entry found
							if( refine > MAX_REFINE ){
								return e_purchase_result::PURCHASE_FAIL_GOODS;
							}
						}else{
							return e_purchase_result::PURCHASE_FAIL_GOODS;
						}
					}
				}
			}

			reducedWeight += ( purchase.amount * requirement->amount * id->weight );
		}
	}

	// Check if there is enough Zeny
	if( sd.status.zeny < requiredZeny ){
		return e_purchase_result::PURCHASE_FAIL_MONEY;
	}

	// Check if there is enough Weight Limit
	if( ( sd.weight + requiredWeight - reducedWeight ) > sd.max_weight ){
		return e_purchase_result::PURCHASE_FAIL_WEIGHT;
	}

	if( pc_inventoryblank( &sd ) < requiredSlots ){
		return e_purchase_result::PURCHASE_FAIL_COUNT;
	}

	for( int32 i = 0; i < MAX_INVENTORY; i++ ){
		if( requiredItems[i] > 0 ){
			if( pc_delitem( &sd, i, requiredItems[i], 0, 0, LOG_TYPE_BARTER ) != 0 ){
				return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
			}
		}
	}

	if( pc_payzeny( &sd, (int32)requiredZeny, LOG_TYPE_BARTER ) != 0 ){
		return e_purchase_result::PURCHASE_FAIL_MONEY;
	}

	for( s_barter_purchase& purchase : purchases ){
		if( purchase.item->stockLimited ){
			purchase.item->stock -= purchase.amount;

			if( Sql_Query( mmysql_handle, "REPLACE INTO `%s` (`name`,`index`,`amount`) VALUES ( '%s', '%hu', '%hu' )", barter_table, barter->name.c_str(), purchase.item->index, purchase.item->stock ) != SQL_SUCCESS ){
				Sql_ShowDebug( mmysql_handle );
				return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
			}
		}

		// Craft mode: roll each attempt independently.
		// Materials and zeny are always consumed regardless of how many succeed.
		uint32 succeededAmount = purchase.amount;
		std::vector<bool> craftResults;

		if( barter->craft ){
			succeededAmount = 0;
			craftResults.reserve( purchase.amount );

			for( uint32 i = 0; i < purchase.amount; i++ ){
				bool success = ( rnd() % 10000 < purchase.item->successRate );
				craftResults.push_back( success );
				if( success ){
					succeededAmount++;
				}
			}
		}

		// Unified item-giving logic for both craft and normal mode
		if( succeededAmount > 0 ){
			if( itemdb_isstackable2( purchase.data ) ){
				struct item it = {};

				it.nameid = purchase.item->nameid;
				it.identify = true;

				if( pc_additem( &sd, &it, succeededAmount, LOG_TYPE_BARTER ) != ADDITEM_SUCCESS ){
					return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
				}
			}else{
				for( uint32 i = 0; i < succeededAmount; i++ ){
					if( purchase.data->type == IT_PETEGG ){
						if( !pet_create_egg( &sd, purchase.item->nameid ) ){
							return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
						}
					}else{
						struct item it = {};

						it.nameid = purchase.item->nameid;
						it.identify = true;
						it.refine = purchase.item->refine;

						if( pc_additem( &sd, &it, 1, LOG_TYPE_BARTER ) != ADDITEM_SUCCESS ){
							return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
						}
					}
				}
			}
		}

		// Craft effects and summary (craft mode only)
		if( barter->craft ){
			int16 craftSlot = -1;
			if( succeededAmount > 0 ){
				craftSlot = pc_search_inventory( &sd, purchase.item->nameid );
			}

			for( bool success : craftResults ){
				if( success && craftSlot >= 0 ){
					clif_refine( sd, (uint16)craftSlot, ITEMREFINING_SUCCESS );
				}else{
					clif_refine( sd, 0, ITEMREFINING_FAILURE );
				}
			}

			// Craft summary
			{
				uint32 failCount = purchase.amount - succeededAmount;
				double successPct = purchase.amount > 0 ? ( succeededAmount * 100.0 / purchase.amount ) : 0.0;
				char msg[512];

				std::shared_ptr<item_data> craftItem = item_db.find( purchase.item->nameid );
				const char* craftName = craftItem ? craftItem->ename.c_str() : "Unknown";

				safesnprintf( msg, sizeof(msg), msg_txt( &sd, MSG_CRAFT_SUMMARY ),
					craftName, purchase.amount, succeededAmount, failCount, successPct );
				clif_messagecolor( &sd, color_table[COLOR_YELLOW], msg, false, SELF );

				std::string costStr;
				for( const auto& [reqIdx, req] : purchase.item->requirements ){
					std::shared_ptr<item_data> id = item_db.find( req->nameid );
					if( id ){
						if( !costStr.empty() ) costStr += " | ";
						char tmp[128];
						safesnprintf( tmp, sizeof(tmp), "%s x%u", id->ename.c_str(), req->amount * purchase.amount );
						costStr += tmp;
					}
				}
				if( purchase.item->price > 0 ){
					if( !costStr.empty() ) costStr += " | ";
					char tmp[64];
					safesnprintf( tmp, sizeof(tmp), "%u Zeny", (uint32)( purchase.item->price * (uint64)purchase.amount ) );
					costStr += tmp;
				}
				if( !costStr.empty() ){
					safesnprintf( msg, sizeof(msg), msg_txt( &sd, MSG_CRAFT_COST ), costStr.c_str() );
					clif_messagecolor( &sd, color_table[COLOR_WHITE], msg, false, SELF );
				}
			}
		}
	}

	return e_purchase_result::PURCHASE_SUCCEED;
}
