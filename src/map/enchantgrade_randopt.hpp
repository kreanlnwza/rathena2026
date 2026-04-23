// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ENCHANTGRADE_RANDOPT_HPP
#define ENCHANTGRADE_RANDOPT_HPP

struct item;
struct s_random_opt_group;

/**
 * Roll one random option from the given group and place it into the next
 * empty option slot of the item. Existing options are preserved.
 *
 * Used by the enchantgrade success path so that each successful grade
 * upgrade awards an additional random option to the graded item.
 */
void enchantgrade_apply_random_option( struct item& target, s_random_opt_group& group );

#endif // ENCHANTGRADE_RANDOPT_HPP
