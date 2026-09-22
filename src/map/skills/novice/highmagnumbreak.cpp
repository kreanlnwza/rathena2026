// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: V!be Coding [kreanlnwza] AI Assistant (GPT-6)

#include "highmagnumbreak.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHighMagnumBreak::SkillHighMagnumBreak() : SkillImplRecursiveDamageSplash(HN_HIGH_MAGNUM_BREAK) {
}

void SkillHighMagnumBreak::castendNoDamageId(block_list* src, block_list*, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
	map_foreachinrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
}

void SkillHighMagnumBreak::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1600 * skill_lv;
	skillratio += 15 * pc_checkskill(sd, HN_SELFSTUDY_TATICS);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}
