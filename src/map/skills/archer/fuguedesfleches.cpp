// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: kreanlnwza Ai assistant Code [Astra].

#include "fuguedesfleches.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillFugueDesFleches::SkillFugueDesFleches() : SkillImplRecursiveDamageSplash(TR_FUGUE_DES_FLECHES) {
}

void SkillFugueDesFleches::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

void SkillFugueDesFleches::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	if (sc != nullptr && sc->hasSCE(SC_MYSTIC_SYMPHONY))
		skillratio += -100 + 200 + 350 * skill_lv;
	else
		skillratio += -100 + 300 + 250 * skill_lv;

	if (pc_checkskill(sd, TR_STAGE_MANNER) > 0)
		skillratio += 5 * sstatus->con;

	RE_LVL_DMOD(100);
}

void SkillFugueDesFleches::modifyElement(const Damage&, const block_list& src, const block_list&, uint16, int32& element, int32) const {
	const map_session_data* sd = BL_CAST(BL_PC, &src);

	if (sd != nullptr)
		element = sd->bonus.arrow_ele;
}
