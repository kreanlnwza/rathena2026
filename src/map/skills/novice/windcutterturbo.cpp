// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: V!be Coding [kreanlnwza] AI Assistant (GPT-6)

#include "windcutterturbo.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillWindCutterTurbo::SkillWindCutterTurbo() : SkillImplRecursiveDamageSplash(HN_WIND_CUTTER_TURBO) {
}

void SkillWindCutterTurbo::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 550 * skill_lv;
	skillratio += 15 * pc_checkskill(sd, HN_SELFSTUDY_TATICS);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}
