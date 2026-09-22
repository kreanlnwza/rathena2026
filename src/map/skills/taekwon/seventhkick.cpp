// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: V!be Coding [kreanlnwza] AI Assistant (GPT-6)

#include "seventhkick.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillSeventhKick::SkillSeventhKick() : WeaponSkillImpl(SKE_SEVENTH_KICK) {
}

void SkillSeventhKick::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	const status_change* sc = status_get_sc(src);

	if (sc != nullptr && sc->hasSCE(SC_SEVENTH_KICK_MAX)) {
		skill_attack(skill_get_type(SKE_SEVENTH_KICK_S), src, src, target, SKE_SEVENTH_KICK_S, skill_lv, tick, flag);
		return;
	}

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillSeventhKick::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 7000 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillSeventhKick::applyAdditionalEffects(block_list* src, block_list*, uint16 skill_lv, t_tick, int32, enum damage_lv dmg_lv) const {
	if (dmg_lv == ATK_FLEE)
		return;

	// Primary client data exposes the timed satellite state, but not the
	// server-side transition that promotes it to SC_SEVENTH_KICK_MAX.
	sc_start(src, src, SC_SEVENTH_KICK_SKILLORB, 100, 1, skill_get_time(getSkillId(), skill_lv));
}

SkillSeventhKickS::SkillSeventhKickS() : WeaponSkillImpl(SKE_SEVENTH_KICK_S) {
}

void SkillSeventhKickS::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 7770 * (skill_lv + 1);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}
