// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: kreanlnwza Ai assistant Code [Astra].

#include "servantweaponcleave.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillServantWeaponCleave::SkillServantWeaponCleave() : WeaponSkillImpl(DK_SERVANT_W_CLEAVE) {
}

void SkillServantWeaponCleave::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillServantWeaponCleave::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change* sc = status_get_sc(&src);

	dmg.div_ = sc != nullptr && sc->hasSCE(SC_VIGOR) ? 4 : 3;
}

void SkillServantWeaponCleave::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);

	if (sc != nullptr && sc->hasSCE(SC_VIGOR))
		skillratio += -100 + 150 + 1050 * skill_lv;
	else
		skillratio += -100 + 300 + 750 * skill_lv;

	// TODO: The client description does not publish the POW scaling coefficient.
	RE_LVL_DMOD(100);
}
