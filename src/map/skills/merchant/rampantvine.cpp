// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: V!be Coding [kreanlnwza] AI Assistant (GPT-6)

#include "rampantvine.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillRampantVine::SkillRampantVine() : WeaponSkillImpl(BO_RAMPANT_VINE) {
}

void SkillRampantVine::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change* sc = status_get_sc(&src);

	dmg.div_ = sc != nullptr && sc->hasSCE(SC_BIONIC_CREEPER) ? 5 : 2;
}

void SkillRampantVine::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 1000 + 700 * skill_lv;
	skillratio += 40 * pc_checkskill(sd, BO_BIONICS_M);
	// TODO: The client description does not publish the POW scaling coefficient.
	RE_LVL_DMOD(100);
}
