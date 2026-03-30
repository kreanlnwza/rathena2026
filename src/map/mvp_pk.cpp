/**
 * MVP Announce & PK on Spawn System
 * Handles MVP spawn/death announcements, PK mode toggle per map,
 * and MVP respawn persistence across server restarts via mapreg.
 * Author: V!be Coding [kreanlnwza] AI Assistant (Claude Opus 4.6)
 *
 * NOTE: Uses MF_PK mapflag to enable per-map PK mode when MVP is alive.
 *       Uses mapreg to persist MVP death state across server restarts.
 **/

#include <sstream>
#include <string>
#include <ctime>
#include <cmath>
#include <memory>

#include <common/cbasetypes.hpp>
#include <common/timer.hpp>

#include "battle.hpp"
#include "clif.hpp"
#include "map.hpp"
#include "mapreg.hpp"
#include "mob.hpp"
#include "mvp_pk.hpp"
#include "pc.hpp"
#include "npc.hpp"
#include "script.hpp"
#include "status.hpp"

/**
 * mvp_pk_on_delayspawn - Enable MF_PK and broadcast spawn message when MVP respawns via timer
 * @param md mob_data of the MVP
 * @author V!be Coding [kreanlnwza] AI Assistant (Claude Opus 4.6)
 */
void mvp_pk_on_delayspawn(struct mob_data *md)
{
	if (!md || !md->spawn || !md->state.boss)
		return;

	map_setmapflag(md->m, MF_PK, true);

	char message[128];
	sprintf(message, "[MVP Spawn]: %s has been spawned on %s map.", md->name, map_mapid2mapname(md->spawn->m));
	clif_broadcast(md, message, strlen(message) + 1, BC_DEFAULT, ALL_CLIENT);
}

/**
 * mvp_pk_on_spawn - Enable MF_PK and broadcast alert message when MVP first spawns
 * @param md mob_data of the MVP
 * @author V!be Coding [kreanlnwza] AI Assistant (Claude Opus 4.6)
 */
void mvp_pk_on_spawn(struct mob_data *md)
{
	if (!md || !md->spawn || !md->spawn->state.boss)
		return;

	map_setmapflag(md->m, MF_PK, true);

	std::ostringstream oss;
	oss << "[MVP Alert] " << md->name << " Lv. " << md->db->lv << " ";
	oss << "has appeared on the " << map_mapid2mapname(md->spawn->m) << " map!";

	std::string message = oss.str();
	clif_broadcast(md, message.c_str(), message.length() + 1, BC_BLUE, ALL_CLIENT);
}

/**
 * mvp_pk_save_spawn_state - Save mapreg state=2 (alive) for MVP
 * @param md mob_data of the MVP
 * @author V!be Coding [kreanlnwza] AI Assistant (Claude Opus 4.6)
 */
void mvp_pk_save_spawn_state(struct mob_data *md)
{
	if (!md || !md->spawn || !md->spawn->state.boss)
		return;

	std::string mapregname = "$" + std::to_string(md->db->id) + "_" + std::to_string(md->m);
	mapreg_setreg(reference_uid(add_str(mapregname.c_str()), 0), 2);
}

/**
 * mvp_pk_on_death - Disable MF_PK, save mapreg death data, and broadcast death message
 * @param md mob_data of the MVP
 * @param mvp_sd player who got MVP reward (can be nullptr)
 * @param sd player who killed (can be nullptr)
 * @author V!be Coding [kreanlnwza] AI Assistant (Claude Opus 4.6)
 */
void mvp_pk_on_death(struct mob_data *md, map_session_data *mvp_sd, map_session_data *sd)
{
	if (!md || !md->spawn || !md->spawn->state.boss)
		return;

	map_setmapflag(md->m, MF_PK, false);

	std::string mapregname = "$" + std::to_string(md->db->id) + "_" + std::to_string(md->m);
	std::string mapregnamestr = mapregname + "$";
	mapreg_setreg(reference_uid(add_str(mapregname.c_str()), 0), 1);
	mapreg_setreg(reference_uid(add_str(mapregname.c_str()), 1), md->x);
	mapreg_setreg(reference_uid(add_str(mapregname.c_str()), 2), md->y);
	mapreg_setreg(reference_uid(add_str(mapregname.c_str()), 3), time(NULL));
	mapreg_setregstr(reference_uid(add_str(mapregnamestr.c_str()), 4), mvp_sd ? mvp_sd->status.name : NULL);

	std::ostringstream oss;
	oss << "[MVP Alert] " << md->name << " Lv. " << md->db->lv << " ";
	if (sd != nullptr) {
		oss << " has been slain by " << sd->status.name << "!";
	} else {
		oss << " has been slain!";
	}
	std::string message = oss.str();

	clif_broadcast(md, message.c_str(), message.length() + 1, BC_DEFAULT, ALL_CLIENT);
}

/**
 * mvp_pk_check_respawn - Check mapreg for MVP respawn persistence after server restart
 * @param md mob_data of the MVP
 * @param mob spawn_data for the MVP
 * @return true if spawn was handled (caller should skip normal spawn with continue)
 * @author V!be Coding [kreanlnwza] AI Assistant (Claude Opus 4.6)
 */
bool mvp_pk_check_respawn(struct mob_data *md, struct spawn_data *mob)
{
	if (!md || !md->state.boss)
		return false;

	std::string mapregname = "$" + std::to_string(md->db->id) + "_" + std::to_string(md->m);

	if (1 == mapreg_readreg(reference_uid(add_str(mapregname.c_str()), 0))) { //1 = dead - 2 = alive
		long int timer = static_cast<long int>(mapreg_readreg(reference_uid(add_str(mapregname.c_str()), 3)));
		long int now = static_cast<long int>(time(NULL));

		long int difftime = mob->delay1; //Base respawn time
		if (mob->delay2) //random variance
			difftime += rnd() % mob->delay2;

		difftime = now - timer - (difftime / 1000);

		if (difftime < 0) { //mvp is still dead
			if (battle_config.mvp_tomb_enabled && map_getmapflag(md->m, MF_NOTOMB) != 1) { //is tomb enabled ?
				std::string mapregnamestr = mapregname + "$";
				int32 old_pos_x = md->x;
				int32 old_pos_y = md->y;
				md->x = static_cast<int16>(mapreg_readreg(reference_uid(add_str(mapregname.c_str()), 1)));
				md->y = static_cast<int16>(mapreg_readreg(reference_uid(add_str(mapregname.c_str()), 2)));
				char* killerid = mapreg_readregstr(reference_uid(add_str(mapregnamestr.c_str()), 4));
				mvptomb_create(md, killerid, timer);
				md->x = old_pos_x;
				md->y = old_pos_y;
			}

			difftime = abs(difftime) * 1000;

			//Apply the spawn delay fix
			std::shared_ptr<s_mob_db> db = mob_db.find(md->db->id);

			if (status_has_mode(&db->status, MD_STATUSIMMUNE)) { // Status Immune
				if (battle_config.boss_spawn_delay != 100) {
					difftime = difftime / 100 * battle_config.boss_spawn_delay;
				}
			} else if (status_has_mode(&db->status, MD_IGNOREMELEE | MD_IGNOREMAGIC | MD_IGNORERANGED | MD_IGNOREMISC)) { // Plant type
				if (battle_config.plant_spawn_delay != 100) {
					difftime = difftime / 100 * battle_config.plant_spawn_delay;
				}
			} else if (battle_config.mob_spawn_delay != 100) { //Normal mobs
				difftime = difftime / 100 * battle_config.mob_spawn_delay;
			}

			if (difftime < 5000) //Monsters should never respawn faster than within 5 seconds
				difftime = 5000;

			if (md->spawn_timer != INVALID_TIMER)
				delete_timer(md->spawn_timer, mob_delayspawn);

			md->spawn = mob;
			md->spawn->active++;
			md->spawn_timer = add_timer(gettick() + difftime, mob_delayspawn, md->id, 0);
			return true; // skip normal spawn
		}
	}

	return false; // proceed with normal spawn
}
