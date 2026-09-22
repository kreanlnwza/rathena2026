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
		// The official demonstration clears the completed orbit as the enhanced
		// cast starts. Consuming it here also prevents a miss from retaining it.
		status_change_end(src, SC_SEVENTH_KICK_MAX);
		status_change_end(src, SC_SEVENTH_KICK_SKILLORB);
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

void SkillSeventhKick::applyCounterAdditionalEffects(block_list* src, block_list*, uint16 skill_lv, t_tick, int32&) const {
	if (status_isdead(*src))
		return;

	const status_change* sc = status_get_sc(src);
	const status_change_entry* sce = sc != nullptr ? sc->getSCE(SC_SEVENTH_KICK_SKILLORB) : nullptr;
	const int32 satellites = min(7, (sce != nullptr ? sce->val1 : 0) + 1);
	const t_tick duration = skill_get_time(getSkillId(), skill_lv);

	// The official demonstration shows each successful normal hit lighting one
	// satellite and promoting the seventh to the completed-orbit state.
	sc_start(src, src, SC_SEVENTH_KICK_SKILLORB, 100, satellites, duration);
	if (satellites == 7)
		sc_start(src, src, SC_SEVENTH_KICK_MAX, 100, 1, duration);
}

SkillSeventhKickS::SkillSeventhKickS() : WeaponSkillImpl(SKE_SEVENTH_KICK_S) {
}

void SkillSeventhKickS::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 7770 * (skill_lv + 1);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}
