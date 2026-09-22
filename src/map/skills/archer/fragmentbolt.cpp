// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: kreanlnwza Ai assistant Code [Astra].

#include "fragmentbolt.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillFragmentBolt::SkillFragmentBolt() : SkillImplRecursiveDamageSplash(WH_FRAGMENT_BOLT) {
}

void SkillFragmentBolt::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

void SkillFragmentBolt::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 3000 + 800 * skill_lv;
	skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
}
