// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: V!be Coding [kreanlnwza] AI Assistant (GPT-6)

#include "punitio.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/unit.hpp"

SkillPunitio::SkillPunitio() : WeaponSkillImpl(CD_PUNITIO) {
}

void SkillPunitio::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	uint8 dir = map_calc_dir(target, src->x, src->y);

	if (skill_check_unit_movepos(5, src, target->x + dirx[dir], target->y + diry[dir], 0, true))
		clif_blown(src);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillPunitio::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const map_session_data* sd = BL_CAST(BL_PC, &src);

	dmg.flag &= ~BF_RANGEMASK;
	dmg.flag |= sd != nullptr && (sd->status.weapon == W_MACE || sd->status.weapon == W_2HMACE) ? BF_LONG : BF_SHORT;
	dmg.div_ = 4;
}

void SkillPunitio::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 1000 + 550 * skill_lv;
	skillratio += 35 * pc_checkskill(sd, CD_MACE_BOOK_M);
	// TODO: The client description does not publish the POW scaling coefficient.
	RE_LVL_DMOD(100);
}
