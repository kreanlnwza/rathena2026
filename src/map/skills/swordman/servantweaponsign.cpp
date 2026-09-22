// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "servantweaponsign.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillServantWeaponSign::SkillServantWeaponSign() : WeaponSkillImpl(DK_SERVANT_W_SIGN) {
}

void SkillServantWeaponSign::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	mob_data* md = BL_CAST(BL_MOB, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	status_change* tsc = status_get_sc(target);
	const sc_type type = skill_get_sc(getSkillId());

	// Max allowed targets to be marked.
	// Only players and monsters can be marked....I think??? [Rytech]
	// Lets only allow players and monsters to use this skill for safety reasons.
	if ((!dstsd && !dstmd) || !sd && !md) {
		if (sd)
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
		return;
	}

	// Check if the target is already marked by another source.
	if (tsc && tsc->getSCE(type) && tsc->getSCE(type)->val1 != src->id) {
		if (sd)
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	if (sd) {
		int8 i;
		const int8 count = MAX_SERVANT_SIGN;

		ARR_FIND(0, count, i, sd->servant_sign[i] == target->id);
		if (i == count) {
			ARR_FIND(0, count, i, sd->servant_sign[i] == 0);
			if (i == count) { // Max number of targets marked. Fail the skill.
				clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
				flag |= SKILL_NOCONSUME_REQ;
				return;
			}
		}
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillServantWeaponSign::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	// The client description does not publish the POW coefficient.
	skillratio += -100 + 1300 + 2200 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillServantWeaponSign::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	mob_data* md = BL_CAST(BL_MOB, src);
	const sc_type type = skill_get_sc(getSkillId());
	status_change* tsc = status_get_sc(target);

	// The target may have been marked by another source after the cast check.
	if (tsc != nullptr && tsc->getSCE(type) != nullptr && tsc->getSCE(type)->val1 != src->id)
		return;

	if (sd != nullptr) {
		int8 i;
		const int8 count = MAX_SERVANT_SIGN;

		ARR_FIND(0, count, i, sd->servant_sign[i] == target->id);
		if (i == count)
			ARR_FIND(0, count, i, sd->servant_sign[i] == 0);
		if (i == count)
			return;

		if (sc_start4(src, target, type, 100, src->id, i, skill_lv, 0, skill_get_time(getSkillId(), skill_lv)))
			sd->servant_sign[i] = target->id;
	} else if (md != nullptr) {
		// Monsters cannot track marked targets; preserve the existing status values.
		sc_start4(src, target, type, 100, 0, 0, skill_lv, 0, skill_get_time(getSkillId(), skill_lv));
	}
}
