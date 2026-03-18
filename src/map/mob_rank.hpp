// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Monster Rank System - Header
 * Defines rank enum (F~EX+), multiplier struct, and function declarations.
 * Rank is randomly assigned on monster spawn and modifies stats, EXP, and drop rates.
 * Author: V!be Coding [kreanlnwza] AI Assistant (claude-sonnet-4-6)
 *
 * NOTE: Controlled by battle_config.mob_rank_system (default: off)
 **/

#ifndef MOB_RANK_HPP
#define MOB_RANK_HPP

#include "common/cbasetypes.hpp"

/// Monster Rank System
/// Randomly assigns a rank (F~EX+) to monsters on spawn.
/// Rank modifies stats, EXP, and drop rates.

enum e_mob_rank : uint8 {
	MOBRANK_F = 0,   // Baseline (no modification)
	MOBRANK_E,
	MOBRANK_D,
	MOBRANK_C,
	MOBRANK_B,
	MOBRANK_BPLUS,   // B+
	MOBRANK_A,
	MOBRANK_APLUS,   // A+
	MOBRANK_S,
	MOBRANK_SPLUS,   // S+
	MOBRANK_SS,
	MOBRANK_SSPLUS,  // SS+
	MOBRANK_SSS,
	MOBRANK_EX,
	MOBRANK_EXPLUS,  // EX+
	MOBRANK_MAX      // 15
};

/// Multiplier values for each rank (percent, 100 = 1x)
struct s_mob_rank_multiplier {
	uint16 hp;        ///< max_hp, max_sp
	uint16 atk;       ///< rhw.atk
	uint16 atk2;      ///< rhw.atk2, rhw.matk
	uint16 def;       ///< def
	uint16 mdef;      ///< mdef
	uint16 res;       ///< res
	uint16 mres;      ///< mres
	uint16 stat;      ///< STR/AGI/VIT/INT/DEX/LUK
	uint16 cri;       ///< cri
	uint16 exp;       ///< base_exp, job_exp
	uint16 drop;      ///< drop rate
	uint16 deflect;   ///< % chance to block all damage (0-100)
	uint16 dmg_taken; ///< % of damage received (100 = normal, lower = tankier)
	uint16 dmg_dealt; ///< % of damage dealt (100 = normal, higher = stronger)
	uint16 coin_rate; ///< coin drop rate (0-10000, per 10000 = 100%)
};

// Forward declaration
struct mob_data;

extern const s_mob_rank_multiplier mob_rank_multipliers[MOBRANK_MAX];

const char* mob_rank_prefix(e_mob_rank rank);
void mob_assign_rank(struct mob_data* md);

#endif /* MOB_RANK_HPP */
