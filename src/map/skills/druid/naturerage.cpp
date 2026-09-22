// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "naturerage.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

#include "groundbloom.hpp"
#include "skill_factory_druid.hpp"

SkillNatureRage::SkillNatureRage() : SkillImplRecursiveDamageSplash(AT_NATURE_RAGE) {
}

void SkillNatureRage::calculateSkillRatio(const Damage*, const block_list*, const block_list*, uint16 skill_lv, int32& skillratio, int32) const {
	skillratio += -100 + 700 * skill_lv;
}

void SkillNatureRage::modifyElement(const Damage&, const block_list& src, const block_list&, uint16, int32& element, int32) const {
	const status_change* sc = status_get_sc(&src);

	if (sc == nullptr)
		return;

	if (sc->hasSCE(SC_TRUTH_OF_ICE))
		element = ELE_WATER;
	else if (sc->hasSCE(SC_TRUTH_OF_WIND))
		element = ELE_WIND;
	else if (sc->hasSCE(SC_TRUTH_OF_EARTH))
		element = ELE_EARTH;
}

void SkillNatureRage::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	const status_change* sc = status_get_sc(src);

	if (sc != nullptr && sc->hasSCE(SC_TRUTH_OF_WIND)) {
		// Gain charge, but intentionally do not fire an enhanced spell even if
		// the caster is already overcharged.
		SkillFactoryDruid::addThunderingCharge(src, getSkillId(), skill_lv, 1);
	} else if (sc != nullptr && sc->hasSCE(SC_TRUTH_OF_EARTH)) {
		// Nature Rage grants two growth stacks and must not auto-cast Ground
		// Bloom when the completed-growth threshold is reached.
		SkillGroundBloom::addGrowth(src, tick, 2, false, false);
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
