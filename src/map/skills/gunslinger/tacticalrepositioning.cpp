// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: kreanlnwza Ai assistant Code [Astra].

#include "tacticalrepositioning.hpp"

#include <algorithm>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillTacticalRepositioning::SkillTacticalRepositioning() : SkillImpl(NW_TACTICAL_REPOSITIONING) {
}

void SkillTacticalRepositioning::castendPos2(block_list* src, int32 x, int32 y, uint16, t_tick, int32&) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	status_change* sc = status_get_sc(src);
	status_change_entry* count = sc != nullptr ? sc->getSCE(SC_INTENSIVE_AIM_COUNT) : nullptr;
	status_change_entry* aim = sc != nullptr ? sc->getSCE(SC_INTENSIVE_AIM) : nullptr;

	if (count == nullptr || aim == nullptr || count->val1 <= 0) {
		if (sd != nullptr)
			clif_skill_fail(*sd, getSkillId());
		return;
	}

	if (!unit_movepos(src, x, y, 1, true)) {
		if (sd != nullptr)
			clif_skill_fail(*sd, getSkillId());
		return;
	}

	clif_snap(src, src->x, src->y);

	const int32 remaining = std::max(0, count->val1 - 1);
	aim->val4 = remaining;

	if (remaining == 0)
		status_change_end(src, SC_INTENSIVE_AIM_COUNT);
	else
		sc_start(src, src, SC_INTENSIVE_AIM_COUNT, 100, remaining, INFINITE_TICK);
}
