// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: V!be Coding [kreanlnwza] AI Assistant (GPT-6)

#include "overdriveprotocol.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillOverdriveProtocol::SkillOverdriveProtocol() : SkillImplRecursiveDamageSplash(MT_OVERDRIVE_PROTOCAL) {
}

void SkillOverdriveProtocol::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change* sc = status_get_sc(&src);

	dmg.div_ = sc != nullptr && sc->hasSCE(SC_ABR_DUAL_CANNON) ? 4 : 2;
}

void SkillOverdriveProtocol::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 2600 + 1050 * skill_lv;
	skillratio += 50 * pc_checkskill(sd, MT_ABR_M);
	// TODO: The client description does not publish the POW scaling coefficient.
	RE_LVL_DMOD(100);
}

void SkillOverdriveProtocol::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
