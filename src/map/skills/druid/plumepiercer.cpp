// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: V!be Coding [kreanlnwza] AI Assistant (GPT-6)

#include "plumepiercer.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillPlumePiercer::SkillPlumePiercer() : SkillImplRecursiveDamageSplash(AT_PLUME_PIERCER) {
}

void SkillPlumePiercer::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 900 * skill_lv;
	// Primary data only states that Base Level and CON increase damage. Use
	// rAthena's existing fourth-job scaling convention for the coefficients.
	skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
}
