// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fragmentbolt.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillFragmentBolt::SkillFragmentBolt() : SkillImplRecursiveDamageSplash(WH_FRAGMENT_BOLT) {
}

void SkillFragmentBolt::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 3000 + 800 * skill_lv;
	skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
}
