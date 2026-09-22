// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "elementalintegration.hpp"

#include <config/core.hpp>

#include "map/elemental.hpp"
#include "map/pc.hpp"

SkillElementalIntegration::SkillElementalIntegration() : StatusSkillImpl(EM_ELEMENTAL_INTEGRATION) {
}

SkillElementalIntegrationAttack::SkillElementalIntegrationAttack(e_skill skill_id, int32 elemental_id) : SkillImplRecursiveDamageSplash(skill_id), elemental_id_(elemental_id) {
}

void SkillElementalIntegrationAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	bool matching_elemental = sd != nullptr && sd->ed != nullptr && sd->ed->elemental.class_ == elemental_id_;

	skillratio += -100 + (matching_elemental ? 5100 : 4400) + 400 * skill_lv;
	// TODO: The client description does not publish the SPL scaling coefficient.
	RE_LVL_DMOD(100);
}
