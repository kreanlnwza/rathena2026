// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fieldofkirin.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillFieldOfKirin::SkillFieldOfKirin() : SkillImplRecursiveDamageSplash(SOA_FIELD_OF_KIRIN) {
}

void SkillFieldOfKirin::castendNoDamageId(block_list* src, block_list*, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
	map_foreachinrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
}

void SkillFieldOfKirin::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);
	const int32 level_ratio = sc != nullptr && sc->hasSCE(SC_T_FIFTH_GOD) ? 1950 : 1550;

	skillratio += -100 + 850 + level_ratio * skill_lv;
	skillratio += 100 * pc_checkskill(sd, SOA_TALISMAN_MASTERY);
	skillratio += 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}
