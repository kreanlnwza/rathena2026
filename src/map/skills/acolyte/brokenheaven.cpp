// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: V!be Coding [kreanlnwza] AI Assistant (GPT-6)

#include "brokenheaven.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillBrokenHeaven::SkillBrokenHeaven() : SkillImplRecursiveDamageSplash(IQ_BROKENHEAVEN) {
}

void SkillBrokenHeaven::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change* sc = status_get_sc(&src);

	if (sc == nullptr)
		return;

	if (sc->hasSCE(SC_FIRST_FAITH_POWER))
		dmg.div_ = 3;
	else if (sc->hasSCE(SC_SECOND_JUDGE))
		dmg.div_ = 4;
	else if (sc->hasSCE(SC_THIRD_EXOR_FLAME))
		dmg.div_ = 2;
}

void SkillBrokenHeaven::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 2450 + 1100 * skill_lv;
	// TODO: The client description does not publish the POW scaling coefficient.
	RE_LVL_DMOD(100);
}

void SkillBrokenHeaven::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_SECOND_BRAND, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillBrokenHeaven::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
