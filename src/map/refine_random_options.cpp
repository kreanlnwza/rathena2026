// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "refine_random_options.hpp"

#include <common/mmo.hpp>
#include <common/random.hpp>

#include "itemdb.hpp"

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

	// Sum the chance of every candidate entry (Must slots + Random pool). The
	// chance field is used as a relative weight so the selection respects the
	// database's configured rarity instead of being uniform.
	uint32 total_weight = 0;
	for( const auto& pair : group->slots ){
		for( const auto& entry : pair.second ){
			total_weight += entry->chance;
		}
	}
	for( const auto& entry : group->random_options ){
		total_weight += entry->chance;
	}

	if( total_weight == 0 ){
		return;
	}

	uint32 roll = rnd() % total_weight;
	uint32 accumulated = 0;
	std::shared_ptr<s_random_opt_group_entry> selected;

	for( const auto& pair : group->slots ){
		for( const auto& entry : pair.second ){
			accumulated += entry->chance;
			if( roll < accumulated ){
				selected = entry;
				break;
			}
		}
		if( selected != nullptr ){
			break;
		}
	}

	if( selected == nullptr ){
		for( const auto& entry : group->random_options ){
			accumulated += entry->chance;
			if( roll < accumulated ){
				selected = entry;
				break;
			}
		}
	}

	if( selected == nullptr ){
		return;
	}

	target.option[slot].id = selected->id;
	target.option[slot].value = rnd_value( selected->min_value, selected->max_value );
	target.option[slot].param = selected->param;
}
