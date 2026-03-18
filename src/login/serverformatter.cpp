// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "serverformatter.hpp"

#include <cstdio>

#include <common/strlib.hpp>

/**
 * Format char server name with online user count for display in client server list.
 * @param server_name: original char server name
 * @param users: current number of online users
 * @param buffer: output buffer to write formatted name
 * @param buffer_size: size of the output buffer
 */
void login_format_server_name( const char* server_name, int32 users, char* buffer, size_t buffer_size ) {
	snprintf( buffer, buffer_size, "%s [%d]", server_name, U. );
}
