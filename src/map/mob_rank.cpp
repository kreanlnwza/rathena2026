// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Monster Rank System - Implementation
 * Multiplier table, rank assignment (weighted random), and prefix helpers.
 * Author: V!be Coding [kreanlnwza] AI Assistant (claude-sonnet-4-6)
 *
 * NOTE: mob_assign_rank() is called in mob_spawn() before status_calc_mob().
 *       EXP/Drop modifiers are applied in mob_dead() and mob_getdroprate().
 **/

#include "mob_rank.hpp"

#include "battle.hpp"
#include "clif.hpp"
#include "map.hpp"
#include "mob.hpp"
#include "status.hpp"

#include "common/random.hpp"
#include "common/showmsg.hpp"
#include "common/timer.hpp"

/// Multiplier table for each rank
/// Values are percentages (100 = 1x, 200 = 2x, etc.)
//                                     HP    ATK  ATK2  DEF  MDEF  RES  MRES Stat  Cri   EXP  Drop  Defl DmgTk DmgDl Coin
const s_mob_rank_multiplier mob_rank_multipliers[MOBRANK_MAX] = {
	/* F    */ {  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   0, 100, 100,    0 },
	/* E    */ {  100,  150,  150,  150,  150,  150,  150,  150,  150,  105,  110,   0,  98, 105,  100 },
	/* D    */ {  100,  175,  175,  175,  175,  175,  175,  175,  175,  110,  125,   0,  95, 110,  150 },
	/* C    */ {  100,  200,  200,  200,  200,  200,  200,  200,  200,  120,  140,   0,  90, 120,  200 },
	/* B    */ {  200,  250,  250,  250,  250,  250,  250,  250,  250,  130,  160,   2,  85, 130,  250 },
	/* B+   */ {  250,  300,  300,  300,  300,  300,  300,  300,  300,  140,  185,   4,  80, 145,  300 },
	/* A    */ {  300,  375,  375,  375,  375,  375,  375,  375,  375,  155,  215,   6,  75, 160,  500 },
	/* A+   */ {  350,  450,  450,  450,  450,  450,  450,  450,  450,  170,  250,   8,  70, 180,  800 },
	/* S    */ {  500,  550,  550,  550,  550,  550,  550,  550,  550,  190,  300,  10,  60, 200,  1000 },
	/* S+   */ {  700,  675,  675,  675,  675,  675,  675,  675,  675,  215,  360,  13,  50, 230,  1500 },
	/* SS   */ {  800,  800,  800,  800,  800,  800,  800,  800,  800,  240,  430,  16,  40, 260,  2000 },
	/* SS+  */ { 1000,  950,  950,  950,  950,  950,  950,  950,  950,  270,  510,  20,  30, 300,  3000 },
	/* SSS  */ { 1500, 1100, 1100, 1100, 1100, 1100, 1100, 1100, 1100,  300,  600,  25,  20, 350,  4000 },
	/* EX   */ { 5000, 1300, 1300, 1300, 1300, 1300, 1300, 1300, 1300,  340,  700,  35,  10, 420,  5000 },
	/* EX+  */ {10000, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500,  380,  800,  50,   5, 500,  8000 },
};

/**
 * Get display prefix string for a rank
 * @param rank Monster rank
 * @return Prefix string (e.g. "[S+]"), empty string for rank F
 */
const char* mob_rank_prefix(e_mob_rank rank) {
	switch (rank) {
		case MOBRANK_F:      return "";
		case MOBRANK_E:      return "E";
		case MOBRANK_D:      return "D";
		case MOBRANK_C:      return "C";
		case MOBRANK_B:      return "B";
		case MOBRANK_BPLUS:  return "B+";
		case MOBRANK_A:      return "A";
		case MOBRANK_APLUS:  return "A+";
		case MOBRANK_S:      return "S";
		case MOBRANK_SPLUS:  return "S+";
		case MOBRANK_SS:     return "SS";
		case MOBRANK_SSPLUS: return "SS+";
		case MOBRANK_SSS:    return "SSS";
		case MOBRANK_EX:     return "EX";
		case MOBRANK_EXPLUS: return "EX+";
		default:             return "";
	}
}

/**
 * Assign a random rank to a monster based on weighted probability
 * @param md Monster data
 */
void mob_assign_rank(struct mob_data* md) {
	if (md == nullptr)
		return;

	// Default to rank F
	md->rank = MOBRANK_F;

	// System disabled
	if (!battle_config.mob_rank_system)
		return;

	// Check boss/MVP exclusions
	// NOTE: Must use db->get_bosstype() instead of md->get_bosstype()
	//       because mob_assign_rank() is called BEFORE status_calc_mob(),
	//       so md->status does not have the correct mode flags yet.
	e_mob_bosstype bosstype = md->db->get_bosstype();
	bool is_mvp = (bosstype == BOSSTYPE_MVP);
	bool is_boss = (bosstype != BOSSTYPE_NONE);

	if (is_mvp && !battle_config.mob_rank_mvp)
		return;

	if (is_boss && !is_mvp && !battle_config.mob_rank_boss)
		return;

	// Build weight array from battle config
	int32 weights[MOBRANK_MAX] = {
		battle_config.mob_rank_weight_f,
		battle_config.mob_rank_weight_e,
		battle_config.mob_rank_weight_d,
		battle_config.mob_rank_weight_c,
		battle_config.mob_rank_weight_b,
		battle_config.mob_rank_weight_bplus,
		battle_config.mob_rank_weight_a,
		battle_config.mob_rank_weight_aplus,
		battle_config.mob_rank_weight_s,
		battle_config.mob_rank_weight_splus,
		battle_config.mob_rank_weight_ss,
		battle_config.mob_rank_weight_ssplus,
		battle_config.mob_rank_weight_sss,
		battle_config.mob_rank_weight_ex,
		battle_config.mob_rank_weight_explus,
	};

	// MVP: only ranks B ~ EX+.
	// Bosses that participate in the rank system should always display a rank,
	// so they start at E instead of F.
	int32 start_rank = is_mvp ? MOBRANK_B : (is_boss ? MOBRANK_E : MOBRANK_F);

	// Calculate total weight
	int32 total = 0;
	for (int32 i = start_rank; i < MOBRANK_MAX; i++)
		total += weights[i];

	if (total <= 0)
		return;

	// Weighted random selection
	int32 roll = rnd() % total;
	int32 cumulative = 0;

	for (int32 i = start_rank; i < MOBRANK_MAX; i++) {
		cumulative += weights[i];
		if (roll < cumulative) {
			md->rank = (e_mob_rank)i;
			return;
		}
	}

	// Fallback (should not reach here)
	md->rank = (e_mob_rank)start_rank;
}

/**
 * Sub-function: re-roll rank for a single monster
 */
static int32 mob_rank_reshuffle_sub(mob_data* md, va_list ap) {
	if (md == nullptr)
		return 0;

	// Skip dead monsters
	if (md->status.hp <= 0)
		return 0;

	// Re-assign rank
	e_mob_rank old_rank = md->rank;
	mob_assign_rank(md);

	// Recalculate stats if rank changed
	if (md->rank != old_rank) {
		status_calc_mob(md, SCO_NONE);
		// Update name display for nearby players
		clif_name_area(md);
	}

	return 1;
}

/**
 * Timer callback: re-roll ranks for all alive monsters
 */
TIMER_FUNC(mob_rank_reshuffle_timer) {
	if (!battle_config.mob_rank_system)
		return 0;

	map_foreachmob(mob_rank_reshuffle_sub);
	ShowInfo("Monster Rank System: All monster ranks have been reshuffled.\n");

	// Broadcast announcement to all players
	char msg[256];
	snprintf(msg, sizeof(msg), "[Monster Rank] All monster ranks have been reshuffled!");
	clif_broadcast(nullptr, msg, strlen(msg) + 1, BC_DEFAULT, ALL_CLIENT);
	return 0;
}
