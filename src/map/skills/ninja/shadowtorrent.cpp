// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shadowtorrent.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillShadowTorrent::SkillShadowTorrent() : SkillImplRecursiveDamageSplash(SS_KAGEGEKIRYU) {
}

void SkillShadowTorrent::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const int32 level_ratio = sc != nullptr && sc->hasSCE(SC_NOBORU) ? 850 : 650;

	skillratio += -100 + level_ratio * skill_lv;
	skillratio += 120 * pc_checkskill(sd, SS_KAGENOMAI);
}
