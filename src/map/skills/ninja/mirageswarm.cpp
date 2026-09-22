// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mirageswarm.hpp"

#include <array>
#include <vector>

#include "map/clif.hpp"
#include "map/skill.hpp"
#include "map/unit.hpp"

SkillMirageSwarm::SkillMirageSwarm() : SkillImpl(SS_SHINKIROU_GUNSHU) {
}

void SkillMirageSwarm::castendNoDamageId(block_list* src, block_list*, uint16 skill_lv, t_tick, int32&) const {
	unit_data* ud = unit_bl2ud(src);

	if (ud == nullptr)
		return;

	std::vector<std::shared_ptr<s_skill_unit_group>> old_mirages;
	for (const std::shared_ptr<s_skill_unit_group>& group : ud->skillunits) {
		if (group != nullptr && (group->skill_id == SS_SHINKIROU || group->skill_id == SS_SHINKIROU_GUNSHU))
			old_mirages.emplace_back(group);
	}

	for (const std::shared_ptr<s_skill_unit_group>& group : old_mirages)
		skill_delunitgroup(group);

	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);

	// The client data does not expose an official three-mirage layout. Place
	// them deterministically around the caster, beginning behind and at both
	// sides, and let skill_unitsetting reject blocked cells.
	static constexpr std::array<uint8, DIR_MAX> direction_offsets = { 4, 2, 6, 3, 5, 1, 7, 0 };
	const uint8 facing = unit_getdir(src);
	uint8 spawned = 0;

	for (const uint8 offset : direction_offsets) {
		const uint8 direction = (facing + offset) % DIR_MAX;
		const int16 x = src->x + dirx[direction];
		const int16 y = src->y + diry[direction];

		if (skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0) != nullptr && ++spawned == 3)
			break;
	}
}
