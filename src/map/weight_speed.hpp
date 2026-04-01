// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef WEIGHT_SPEED_HPP
#define WEIGHT_SPEED_HPP

#include <common/cbasetypes.hpp>

class map_session_data;

/**
 * Weight-based speed reduction system
 * Reduces player movement speed based on carried weight percentage
 *
 * Penalty tiers:
 * - 90%+ weight = 90% speed reduction
 * - 70-89% weight = 60% speed reduction
 * - 50-69% weight = 30% speed reduction
 * - <50% weight = no penalty
 */

namespace weight_speed {

/**
 * Calculate speed penalty based on player's current weight
 * @param sd Player data
 * @return Speed penalty percentage (0-90)
 */
int32 get_speed_penalty(const map_session_data& sd);

/**
 * Apply weight-based speed penalty to base speed
 * @param sd Player data
 * @param current_speed Current calculated speed
 * @return Modified speed with weight penalty applied
 */
int32 apply_speed_penalty(const map_session_data& sd, int32 current_speed);

} // namespace weight_speed

#endif // WEIGHT_SPEED_HPP
