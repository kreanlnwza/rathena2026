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

void enchantgrade_apply_random_option( struct item& target, s_random_opt_group& group ){
	// Find the first empty option slot, existing options are preserved.
	size_t slot = MAX_ITEM_RDM_OPT;
	for( size_t i = 0; i < MAX_ITEM_RDM_OPT; i++ ){
		if( target.option[i].id == 0 ){
			slot = i;
			break;
		}
	}

	// No empty slot available - nothing to do
	if( slot >= MAX_ITEM_RDM_OPT ){
		return;
	}

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
		return;
	}

	std::shared_ptr<s_random_opt_group_entry> picked = util::vector_random( candidates );

	target.option[slot].id = picked->id;
	target.option[slot].value = rnd_value( picked->min_value, picked->max_value );
	target.option[slot].param = picked->param;
}
