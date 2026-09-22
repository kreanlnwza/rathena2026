// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: kreanlnwza Ai assistant Code [Astra].

#include "primedelementaltrap.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillPrimedElementalTrap::SkillPrimedElementalTrap(e_skill skill_id) : SkillImplRecursiveDamageSplash(skill_id) {
}

void SkillPrimedElementalTrap::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

void SkillPrimedElementalTrap::modifyDamageData(Damage& dmg, const block_list&, const block_list&, uint16) const {
	// These attacks can be cast from range, but the primary description
	// explicitly classifies their damage as melee physical.
	dmg.flag = (dmg.flag & ~BF_RANGEMASK) | BF_SHORT;
}

void SkillPrimedElementalTrap::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 3950 * skill_lv;
	skillratio += 50 * pc_checkskill(sd, WH_ADVANCED_TRAP);
	skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
}
