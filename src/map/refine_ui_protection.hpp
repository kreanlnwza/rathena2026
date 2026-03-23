// Copyright (c) rAthena Project (All rights reserved)
// Original Author: krit.k #3614
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#ifndef REFINE_UI_PROTECTION_HPP
#define REFINE_UI_PROTECTION_HPP

#include "../common/cbasetypes.hpp"
#include "../common/timer.hpp"

// Forward declaration
class map_session_data;

/**
 * Refine UI Protection System
 *
 * This system prevents speed hacking in the refine UI by implementing
 * delay mechanisms and UI locking after refine attempts.
 *
 * Features:
 * - Delay between refine attempts (configurable)
 * - UI locking after each refine attempt (configurable)
 * - Warning messages for players
 */

// Message IDs for refine UI protection
static const uint16 MSG_REFINE_DELAY = 3020;	// "Refine UI is locked. Please wait before attempting to refine again."
static const uint16 MSG_UI_LOCK_TIME = 3021;	// "Please wait %d seconds before refining %s again."

class RefineUIProtection {
public:
	/**
	 * Check if player can attempt refine based on delay time
	 * @param sd Player session data
	 * @param current_tick Current tick
	 * @return true if player can refine, false otherwise
	 */
	static bool can_refine(map_session_data* sd, t_tick current_tick);

	/**
	 * Check if refine UI is locked and display warning message
	 * @param sd Player session data
	 * @return true if UI is not locked, false otherwise
	 */
	static bool check_ui_lock(map_session_data* sd);

	/**
	 * Set refine attempt time and lock UI
	 * @param sd Player session data
	 */
	static void set_refine_attempt(map_session_data* sd);

	/**
	 * Lock the refine UI for specified duration
	 * @param sd Player session data
	 */
	static void lock_ui(map_session_data* sd);

	/**
	 * Get remaining lock time in milliseconds
	 * @param sd Player session data
	 * @return remaining lock time, 0 if not locked
	 */
	static t_tick get_remaining_lock_time(map_session_data* sd);
};

#endif /* REFINE_UI_PROTECTION_HPP */