// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Author: kreanlnwza Ai assistant Code [Astra].

#include "lexexpiatrix.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillLexExpiatrix::SkillLexExpiatrix() : SkillImpl(CD_LEX_EXPIATRIX) {
}

void SkillLexExpiatrix::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillLexExpiatrix::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);

	if (sc != nullptr && sc->hasSCE(SC_COMPETENTIA))
		skillratio += -100 + 1500 + 1350 * skill_lv;
	else
		skillratio += -100 + 1500 + 1000 * skill_lv;

	skillratio += 50 * pc_checkskill(sd, CD_FIDUS_ANIMUS);
	// TODO: The client description does not publish the SPL scaling coefficient.
	RE_LVL_DMOD(100);
}

void SkillLexExpiatrix::modifyElement(const Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv, int32& element, int32 flag) const {
	const status_change* sc = status_get_sc(&src);

	if (sc != nullptr && sc->hasSCE(SC_ANCILLA))
		element = ELE_NEUTRAL;
}
