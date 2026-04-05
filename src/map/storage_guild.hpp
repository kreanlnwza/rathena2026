// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef STORAGE_GUILD_HPP
#define STORAGE_GUILD_HPP

#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

struct s_storage;
struct item;
class map_session_data;

///Databases of guild_storage : int32 guild_id -> (uint8 stor_id -> struct guild_storage)
extern std::map<int32, std::map<uint8, struct s_storage>> guild_storage_db;

///Guild storage table database: uint8 stor_id -> shared_ptr<s_guild_storage_table>
extern std::unordered_map<uint8, std::shared_ptr<struct s_guild_storage_table>> guild_storage_table_db;

/// Guild storage flags
enum e_guild_storage_flags : uint8 {
	GSTORAGE_OPEN = 0,
	GSTORAGE_STORAGE_ALREADY_OPEN,
	GSTORAGE_ALREADY_OPEN,
	GSTORAGE_NO_GUILD,
	GSTORAGE_NO_STORAGE,
	GSTORAGE_NO_PERMISSION
};

enum e_guild_storage_log : uint16 {
	GUILDSTORAGE_LOG_SUCCESS,
	GUILDSTORAGE_LOG_FINAL_SUCCESS,
	GUILDSTORAGE_LOG_EMPTY,
	GUILDSTORAGE_LOG_FAILED,
};

struct guild_log_entry{
	uint32 id;
	char name[NAME_LENGTH];
	char time[NAME_LENGTH];
	struct item item;
	int16 amount;
};

struct s_storage* guild2storage(int32 guild_id, uint8 stor_id = 0);
struct s_storage* guild2storage2(int32 guild_id, uint8 stor_id = 0);
void storage_guild_delete(int32 guild_id);
char storage_guild_storageopen(map_session_data *sd, uint8 stor_id = 0);
enum e_guild_storage_log storage_guild_log_read( map_session_data* sd );
bool storage_guild_additem(map_session_data *sd,struct s_storage *stor,struct item *item_data,int32 amount);
bool storage_guild_additem2(struct s_storage* stor, struct item* item, int32 amount);
bool storage_guild_delitem(map_session_data *sd,struct s_storage *stor,int32 n,int32 amount);
void storage_guild_storageadd(map_session_data *sd,int32 index,int32 amount);
void storage_guild_storageget(map_session_data *sd,int32 index,int32 amount, bool favorite=false);
void storage_guild_storageaddfromcart(map_session_data *sd,int32 index,int32 amount);
void storage_guild_storagegettocart(map_session_data *sd,int32 index,int32 amount);
void storage_guild_storageclose(map_session_data *sd);
void storage_guild_storage_quit(map_session_data *sd,int32 flag);
bool storage_guild_storagesave(uint32 account_id, int32 guild_id, int32 flag, uint8 stor_id = 0);
void storage_guild_storagesaved(int32 guild_id); //Ack from char server that guild store was saved.

void do_init_guild_storage(void);
void do_final_guild_storage(void);
void do_reconnect_guild_storage(void);

#endif /* STORAGE_GUILD_HPP */
