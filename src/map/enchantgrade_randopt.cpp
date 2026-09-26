// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "enchantgrade_randopt.hpp"

#include <memory>
#include <vector>

#include <common/mmo.hpp>
#include <common/random.hpp>
#include <common/utilities.hpp>

#include "itemdb.hpp"

using namespace rathena;

static std::shared_ptr<s_random_opt_group_entry> enchantgrade_pick_entry( s_random_opt_group& group ){
	// Collect all candidate entries from Must slots and from the Random pool.
	std::vector<std::shared_ptr<s_random_opt_group_entry>> candidates;
	for( const auto& it_slot : group.slots ){
		for( const auto& entry : it_slot.second ){
			candidates.push_back( entry );
		}
	}
	for( const auto& entry : group.random_options ){
		candidates.push_back( entry );
	}

	if( candidates.empty() ){
		return nullptr;
	}

	return util::vector_random( candidates );
}

static void enchantgrade_write_entry( struct item& target, size_t slot, const s_random_opt_group_entry& entry ){
	target.option[slot].id = entry.id;
	target.option[slot].value = rnd_value( entry.min_value, entry.max_value );
	target.option[slot].param = entry.param;
}

bool enchantgrade_apply_random_option( struct item& target, s_random_opt_group& group ){
	// Find the first empty option slot, existing options are preserved.
	size_t slot = MAX_ITEM_RDM_OPT;
	for( size_t i = 0; i < MAX_ITEM_RDM_OPT; i++ ){
		if( target.option[i].id == 0 ){
			slot = i;
			break;
		}
	}

	if( slot >= MAX_ITEM_RDM_OPT ){
		return false;
	}

	std::shared_ptr<s_random_opt_group_entry> picked = enchantgrade_pick_entry( group );

	if( picked == nullptr ){
		return false;
	}

	enchantgrade_write_entry( target, slot, *picked );
	return true;
}

bool enchantgrade_reroll_option_slot( struct item& target, s_random_opt_group& group, size_t slot ){
	if( slot >= MAX_ITEM_RDM_OPT ){
		return false;
	}

	std::shared_ptr<s_random_opt_group_entry> picked = enchantgrade_pick_entry( group );

	if( picked == nullptr ){
		return false;
	}

	enchantgrade_write_entry( target, slot, *picked );
	return true;
}
