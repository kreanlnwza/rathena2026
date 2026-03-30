/**
 * MVP Announce & PK on Spawn System
 * Handles MVP spawn/death announcements, PK mode toggle per map,
 * and MVP respawn persistence across server restarts via mapreg.
 * Author: V!be Coding [kreanlnwza] AI Assistant (Claude Opus 4.6)
 *
 * NOTE: Uses MF_PK mapflag to enable per-map PK mode when MVP is alive.
 *       Uses mapreg to persist MVP death state across server restarts.
 **/

#ifndef MVP_PK_HPP
#define MVP_PK_HPP

struct mob_data;
class map_session_data;
struct spawn_data;

/**
 * mvp_pk_on_delayspawn - Enable MF_PK and broadcast spawn message when MVP respawns via timer
 * @param md mob_data of the MVP
 * @author V!be Coding [kreanlnwza] AI Assistant (Claude Opus 4.6)
 */
void mvp_pk_on_delayspawn(struct mob_data *md);

/**
 * mvp_pk_on_spawn - Enable MF_PK and broadcast alert message when MVP first spawns
 * @param md mob_data of the MVP
 * @author V!be Coding [kreanlnwza] AI Assistant (Claude Opus 4.6)
 */
void mvp_pk_on_spawn(struct mob_data *md);

/**
 * mvp_pk_save_spawn_state - Save mapreg state=2 (alive) for MVP
 * @param md mob_data of the MVP
 * @author V!be Coding [kreanlnwza] AI Assistant (Claude Opus 4.6)
 */
void mvp_pk_save_spawn_state(struct mob_data *md);

/**
 * mvp_pk_on_death - Disable MF_PK, save mapreg death data, and broadcast death message
 * @param md mob_data of the MVP
 * @param mvp_sd player who got MVP reward (can be nullptr)
 * @param sd player who killed (can be nullptr)
 * @author V!be Coding [kreanlnwza] AI Assistant (Claude Opus 4.6)
 */
void mvp_pk_on_death(struct mob_data *md, map_session_data *mvp_sd, map_session_data *sd);

/**
 * mvp_pk_check_respawn - Check mapreg for MVP respawn persistence after server restart
 * @param md mob_data of the MVP
 * @param mob spawn_data for the MVP
 * @return true if spawn was handled (caller should skip normal spawn with continue)
 * @author V!be Coding [kreanlnwza] AI Assistant (Claude Opus 4.6)
 */
bool mvp_pk_check_respawn(struct mob_data *md, struct spawn_data *mob);

#endif /* MVP_PK_HPP */
