// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: V!be Coding [kreanlnwza] AI Assistant (GPT-6)

#include "mirageswarm.hpp"

#include <array>
#include <vector>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/unit.hpp"

SkillMirageSwarm::SkillMirageSwarm() : SkillImpl(SS_SHINKIROU_GUNSHU) {
}

void SkillMirageSwarm::castendNoDamageId(block_list* src, block_list*, uint16 skill_lv, t_tick, int32&) const {
	unit_data* ud = unit_bl2ud(src);

	if (ud == nullptr)
		return;

	std::array<int16, 3> positions_x;
	std::array<int16, 3> positions_y;

	for (uint8 index = 0; index < 3; ++index) {
		if (!skill_get_mirage_swarm_position(*src, index, positions_x[index], positions_y[index])) {
			if (map_session_data* sd = BL_CAST(BL_PC, src); sd != nullptr)
				clif_skill_fail(*sd, getSkillId());
			return;
		}
	}

	std::vector<std::shared_ptr<s_skill_unit_group>> old_mirages;
	for (const std::shared_ptr<s_skill_unit_group>& group : ud->skillunits) {
		if (group != nullptr && (group->skill_id == SS_SHINKIROU || group->skill_id == SS_SHINKIROU_GUNSHU))
			old_mirages.emplace_back(group);
	}

	for (const std::shared_ptr<s_skill_unit_group>& group : old_mirages)
		skill_delunitgroup(group);

	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);

	for (uint8 index = 0; index < 3; ++index)
		skill_unitsetting(src, getSkillId(), skill_lv, positions_x[index], positions_y[index], 0);
}
