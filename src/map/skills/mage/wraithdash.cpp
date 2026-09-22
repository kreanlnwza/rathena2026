// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: V!be Coding [kreanlnwza] AI Assistant (GPT-6)

#include "wraithdash.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/unit.hpp"

SkillWraithDash::SkillWraithDash() : SkillImplRecursiveDamageSplash(AG_WRAITH_DASH) {
}

void SkillWraithDash::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);

	skill_area_temp[0] = 0;
	skill_area_temp[1] = src->id;
	map_foreachinrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);

	skill_blown(src, src, 7, (unit_getdir(src) + 4) % 8, BLOWN_IGNORE_NO_KNOCKBACK);
}

void SkillWraithDash::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 3000 + 4000 * skill_lv;
	RE_LVL_DMOD(100);
}
