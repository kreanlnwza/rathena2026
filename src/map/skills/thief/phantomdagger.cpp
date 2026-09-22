// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: V!be Coding [kreanlnwza] AI Assistant (GPT-6)

#include "phantomdagger.hpp"

#include <config/core.hpp>

#include "common/random.hpp"

SkillPhantomDagger::SkillPhantomDagger() : WeaponSkillImpl(ABC_PHANTOM_DAGGER) {
}

void SkillPhantomDagger::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	dmg.div_ = rnd_chance(15 * skill_lv, 100) ? 4 : 3;
}

void SkillPhantomDagger::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 7000 + 1150 * skill_lv;
	// TODO: The client description does not publish the POW scaling coefficient.
	RE_LVL_DMOD(100);
}
