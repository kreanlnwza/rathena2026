// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "char_ip_limit.hpp"

#include <unordered_map>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/utilities.hpp>

#include "char.hpp"

static std::unordered_map<uint32, int32> ip_connection_count;
static std::vector<struct s_ip_whitelist> ip_whitelist;

/**
 * Parse an IP/mask string into s_ip_whitelist.
 * Supports: ip, ip/mask, ip/cidr
 * @param str: IP string to parse
 * @param entry: output whitelist entry
 * @return true on success
 */
static bool char_parse_ipmask(const char* str, struct s_ip_whitelist* entry) {
	uint32 a[4];
	uint32 m[4];
	int32 n;

	if (((n = sscanf(str, "%3u.%3u.%3u.%3u/%3u.%3u.%3u.%3u", a, a+1, a+2, a+3, m, m+1, m+2, m+3)) != 8 &&
		(n = sscanf(str, "%3u.%3u.%3u.%3u/%3u", a, a+1, a+2, a+3, m)) != 5 &&
		(n = sscanf(str, "%3u.%3u.%3u.%3u", a, a+1, a+2, a+3)) != 4) ||
		a[0] > 255 || a[1] > 255 || a[2] > 255 || a[3] > 255 ||
		(n == 8 && (m[0] > 255 || m[1] > 255 || m[2] > 255 || m[3] > 255)) ||
		(n == 5 && m[0] > 32)) {
		return false;
	}

	entry->ip = MAKEIP(a[0], a[1], a[2], a[3]);

	if (n == 8) {
		// standard mask
		entry->mask = MAKEIP(m[0], m[1], m[2], m[3]);
	} else if (n == 5) {
		// CIDR bit mask
		entry->mask = 0;
		uint32 bits = m[0];
		while (bits) {
			entry->mask = (entry->mask >> 1) | 0x80000000;
			--bits;
		}
	} else {
		// single IP
		entry->mask = 0xFFFFFFFF;
	}

	return true;
}

/**
 * Check if an IP is in the whitelist.
 * @param ip: IP address to check
 * @return true if whitelisted
 */
static bool char_ip_is_whitelisted(uint32 ip) {
	for (const auto& entry : ip_whitelist) {
		if ((ip & entry.mask) == (entry.ip & entry.mask))
			return true;
	}
	return false;
}

/**
 * Check if a new connection from this IP would exceed the per-IP limit.
 * @param ip: IP address
 * @param group_id: account group ID (for GM bypass)
 * @return true if connection is allowed, false if limit exceeded
 */
bool char_check_ip_connection_limit(uint32 ip, int32 group_id) {
	int32 limit = charserv_config.max_connect_user_per_ip;

	// Disabled
	if (limit < 0)
		return true;

	// GM bypass
	if (group_id >= charserv_config.max_connect_user_per_ip_gm_allow_group)
		return true;

	// Whitelisted IP
	if (char_ip_is_whitelisted(ip))
		return true;

	// Check current count
	auto it = ip_connection_count.find(ip);
	int32 count = (it != ip_connection_count.end()) ? it->second : 0;

	if (count >= limit) {
		char ip_str[16];
		ip2str(ip, ip_str);
		ShowNotice("Connection refused: IP '%s' has reached the per-IP limit of %d connections.\n", ip_str, limit);
		return false;
	}

	return true;
}

/**
 * Increment the connection counter for an IP.
 * @param ip: IP address
 */
void char_ip_connection_increment(uint32 ip) {
	if (ip == 0)
		return;
	ip_connection_count[ip]++;
}

/**
 * Decrement the connection counter for an IP.
 * @param ip: IP address
 */
void char_ip_connection_decrement(uint32 ip) {
	if (ip == 0)
		return;
	auto it = ip_connection_count.find(ip);
	if (it != ip_connection_count.end()) {
		it->second--;
		if (it->second <= 0)
			ip_connection_count.erase(it);
	}
}

/**
 * Clear all IP connection counters.
 */
void char_ip_connection_clear() {
	ip_connection_count.clear();
}

/**
 * Check prevent change IP policy when an account tries to authenticate.
 * @param account_id: account trying to login
 * @param new_ip: IP of the new connection
 * @param group_id: account group ID (for GM bypass)
 * @return 0 = allow (no conflict or disabled),
 *         1 = block new connection (mode 1),
 *         2 = disconnect old session (mode 2)
 */
int32 char_check_prevent_change_ip(uint32 account_id, uint32 new_ip, int32 group_id) {
	int32 mode = charserv_config.prevent_change_ip;

	// Disabled
	if (mode == 0)
		return 0;

	// GM bypass
	if (group_id >= charserv_config.max_connect_user_per_ip_gm_allow_group)
		return 0;

	// Check if account is already online with a different IP
	std::shared_ptr<struct online_char_data> character = util::umap_find(char_get_onlinedb(), account_id);

	if (character == nullptr)
		return 0;

	// Only check if character is actually on a map server (playing)
	if (character->server < 0)
		return 0;

	// No IP recorded (e.g. GM or autotrade)
	if (character->ip == 0)
		return 0;

	// Same IP — no conflict
	if (character->ip == new_ip)
		return 0;

	char old_ip_str[16], new_ip_str[16];
	ip2str(character->ip, old_ip_str);
	ip2str(new_ip, new_ip_str);

	if (mode == 1) {
		ShowNotice("Prevent Change IP: Account %u blocked login from new IP '%s' (current IP: '%s', mode: block)\n",
			account_id, new_ip_str, old_ip_str);
		return 1;
	} else if (mode == 2) {
		ShowNotice("Prevent Change IP: Account %u login from new IP '%s', disconnecting old IP '%s' (mode: kick old)\n",
			account_id, new_ip_str, old_ip_str);
		return 2;
	}

	return 0;
}

/**
 * Read IP limit configuration.
 * @param w1: config key
 * @param w2: config value
 * @return true if the key was handled
 */
bool char_ip_limit_config_read(const char* w1, const char* w2) {
	if (strcmpi(w1, "max_connect_user_per_ip") == 0) {
		charserv_config.max_connect_user_per_ip = atoi(w2);
		if (charserv_config.max_connect_user_per_ip < -1)
			charserv_config.max_connect_user_per_ip = -1;
		ShowInfo("Per-IP connection limit set to: %d\n", charserv_config.max_connect_user_per_ip);
		return true;
	} else if (strcmpi(w1, "max_connect_user_per_ip_gm_allow_group") == 0) {
		charserv_config.max_connect_user_per_ip_gm_allow_group = atoi(w2);
		ShowInfo("Per-IP GM bypass group_id: %d\n", charserv_config.max_connect_user_per_ip_gm_allow_group);
		return true;
	} else if (strcmpi(w1, "prevent_change_ip") == 0) {
		charserv_config.prevent_change_ip = atoi(w2);
		if (charserv_config.prevent_change_ip < 0 || charserv_config.prevent_change_ip > 2)
			charserv_config.prevent_change_ip = 0;
		ShowInfo("Prevent Change IP mode: %d\n", charserv_config.prevent_change_ip);
		return true;
	} else if (strcmpi(w1, "ip_connection_whitelist") == 0) {
		struct s_ip_whitelist entry;
		if (char_parse_ipmask(w2, &entry)) {
			ip_whitelist.push_back(entry);
			ShowStatus("IP connection whitelist: %s\n", w2);
		} else {
			ShowError("char_ip_limit_config_read: Invalid IP/mask '%s'!\n", w2);
		}
		return true;
	}
	return false;
}

/**
 * Initialize IP limit system.
 */
void char_ip_limit_init() {
	ip_connection_count.clear();
	ip_whitelist.clear();
}

/**
 * Finalize IP limit system.
 */
void char_ip_limit_final() {
	ip_connection_count.clear();
	ip_whitelist.clear();
}
