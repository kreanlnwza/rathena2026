// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ENCHANTGRADE_RANDOPT_HPP
#define ENCHANTGRADE_RANDOPT_HPP

#include <cstddef>

struct item;
struct s_random_opt_group;

/**
 * Roll one random option from the given group and place it into the next
 * empty option slot of the item. Existing options are preserved.
 *
 * Used by the enchantgrade success path so that each successful grade
 * upgrade awards an additional random option to the graded item.
 *
 * Returns true if an option was written, false otherwise (no candidates
 * in the group, or no empty slot available).
 */
bool enchantgrade_apply_random_option( struct item& target, s_random_opt_group& group );

/**
 * Roll one random option from the given group and write it into the
 * specified slot, overwriting anything already there.
 *
 * Used by the reroll script command so NPCs can offer a "reroll this
 * specific option" service without having to touch the grade system.
 *
 * Returns true if an option was written, false if slot is out of range
 * or the group has no candidates.
 */
bool enchantgrade_reroll_option_slot( struct item& target, s_random_opt_group& group, size_t slot );

#endif // ENCHANTGRADE_RANDOPT_HPP
