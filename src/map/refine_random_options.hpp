// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef REFINE_RANDOM_OPTIONS_HPP
#define REFINE_RANDOM_OPTIONS_HPP

#include <memory>

struct item;
struct s_random_opt_group;

// Roll one random option from the given group into the next empty option slot
// of the target item. Existing options are left untouched. No-op when the group
// is null, the item has no free slot, or the group has no candidate entries.
void refine_apply_random_option( const std::shared_ptr<s_random_opt_group>& group, struct item& target );

#endif /* REFINE_RANDOM_OPTIONS_HPP */
