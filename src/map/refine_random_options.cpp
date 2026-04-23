// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "refine_random_options.hpp"

#include <vector>

#include <common/mmo.hpp>
#include <common/random.hpp>
#include <common/utilities.hpp>

#include "itemdb.hpp"

using namespace rathena;

void refine_apply_random_option( const std::shared_ptr<s_random_opt_group>& group, struct item& target ){
	if( group == nullptr ){
		return;
	}

	// Find the first empty option slot
	size_t slot = MAX_ITEM_RDM_OPT;
	for( size_t i = 0; i < MAX_ITEM_RDM_OPT; i++ ){
		if( target.option[i].id == 0 ){
			slot = i;
			break;
		}
	}

	// No empty slot available
	if( slot >= MAX_ITEM_RDM_OPT ){
		return;
	}

	// Collect all candidate entries from Must slots and Random pool
	std::vector<std::shared_ptr<s_random_opt_group_entry>> candidates;
	for( const auto& pair : group->slots ){
		for( const auto& entry : pair.second ){
			candidates.push_back( entry );
		}
	}
	for( const auto& entry : group->random_options ){
		candidates.push_back( entry );
	}

	if( candidates.empty() ){
		return;
	}

	std::shared_ptr<s_random_opt_group_entry> option = util::vector_random( candidates );

	target.option[slot].id = option->id;
	target.option[slot].value = rnd_value( option->min_value, option->max_value );
	target.option[slot].param = option->param;
}
