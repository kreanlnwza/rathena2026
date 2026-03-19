// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include <common/cbasetypes.hpp>

struct s_ip_whitelist {
	uint32 ip;
	uint32 mask;
};

void char_ip_limit_init();
void char_ip_limit_final();
bool char_check_ip_connection_limit(uint32 ip, int32 group_id);
void char_ip_connection_increment(uint32 ip);
void char_ip_connection_decrement(uint32 ip);
void char_ip_connection_clear();
int32 char_check_prevent_change_ip(uint32 account_id, uint32 new_ip, int32 group_id);
bool char_ip_limit_config_read(const char* w1, const char* w2);
