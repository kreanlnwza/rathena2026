// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "weight_speed.hpp"

#include "pc.hpp"

namespace weight_speed {

/**
 * Calculate speed penalty based on player's current weight percentage
 * Penalty tiers:
 * - 90%+ weight = 90% speed reduction
 * - 70-89% weight = 60% speed reduction
 * - 50-69% weight = 30% speed reduction
 * - <50% weight = no penalty
 */
int32 get_speed_penalty(const map_session_data& sd) {
	int32 weight_percent = pc_getpercentweight(sd);

	if (weight_percent >= 90)
		return 90;   // 90%+ weight = 90% slower
	else if (weight_percent >= 70)
		return 60;   // 70-89% weight = 60% slower
	else if (weight_percent >= 50)
		return 30;   // 50-69% weight = 30% slower
	else
		return 0;     // <50% weight = no penalty
}

/**
 * Apply weight-based speed penalty to current speed
 * Formula: speed + (speed * penalty / 100)
 */
int32 apply_speed_penalty(const map_session_data& sd, int32 current_speed) {
	int32 penalty = get_speed_penalty(sd);

	if (penalty <= 0)
		return current_speed;

	return current_speed + (current_speed * penalty / 100);
}

} // namespace weight_speed
