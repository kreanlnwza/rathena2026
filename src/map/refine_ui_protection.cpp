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

#include "refine_ui_protection.hpp"

#include "battle.hpp"
#include "clif.hpp"
#include "pc.hpp"

bool RefineUIProtection::can_refine(map_session_data* sd, t_tick current_tick) {
	if (sd->state.last_refine_tick != 0 &&
		(current_tick - sd->state.last_refine_tick) < battle_config.refine_delay_time) {
		return false;
	}
	return true;
}

bool RefineUIProtection::check_ui_lock(map_session_data* sd) {
	if (sd->state.refineui_locked) {
		t_tick remaining = battle_config.refine_ui_lock_time - DIFF_TICK(gettick(), sd->state.refineui_lock_tick);

		if (remaining > 0) {
			int32 current_seconds = (int32)((remaining + 999) / 1000); // Round up to nearest second

			char message[128];
			snprintf(message, sizeof(message), msg_txt(sd, ::MSG_UI_LOCK_TIME),
				current_seconds, (current_seconds > 1) ? "s" : "");
			clif_messagecolor(sd, color_table[COLOR_RED], message, false, SELF);

			return false;
		}
	}
	return true;
}

void RefineUIProtection::set_refine_attempt(map_session_data* sd) {
	t_tick current_tick = gettick();
	sd->state.last_refine_tick = current_tick;

	// Also lock the UI after each attempt
	lock_ui(sd);
}

void RefineUIProtection::lock_ui(map_session_data* sd) {
	sd->state.refineui_locked = true;
	sd->state.refineui_lock_tick = gettick();
}

t_tick RefineUIProtection::get_remaining_lock_time(map_session_data* sd) {
	if (!sd->state.refineui_locked) {
		return 0;
	}

	t_tick remaining = battle_config.refine_ui_lock_time - DIFF_TICK(gettick(), sd->state.refineui_lock_tick);
	return (remaining > 0) ? remaining : 0;
}
