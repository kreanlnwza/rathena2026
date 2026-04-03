// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <memory>
#include <unordered_map>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

struct s_storage;
struct item;
class map_session_data;

extern std::unordered_map<uint16, std::shared_ptr<struct s_storage_table>> storage_db;

enum e_storage_add {
	STORAGE_ADD_OK,
	STORAGE_ADD_NOROOM,
	STORAGE_ADD_NOACCESS,
	STORAGE_ADD_INVALID,
};


const char *storage_getName(uint8 id);
bool storage_exists(uint8 id);

int32 storage_delitem(map_session_data* sd, struct s_storage *stor, int32 index, int32 amount);
int32 storage_storageopen(map_session_data *sd);
void storage_storageadd(map_session_data *sd, struct s_storage *stor, int32 index, int32 amount);
void storage_storageget(map_session_data *sd, struct s_storage *stor, int32 index, int32 amount, bool favorite=false);
void storage_storageaddfromcart(map_session_data *sd, struct s_storage *stor, int32 index, int32 amount);
void storage_storagegettocart(map_session_data *sd, struct s_storage *stor, int32 index, int32 amount);
void storage_storagesave(map_session_data *sd);
void storage_storageclose(map_session_data *sd);
void storage_sortitem(struct item* items, uint32 size);
void do_init_storage(void);
void do_final_storage(void);
void do_reconnect_storage(void);
void storage_storage_quit(map_session_data *sd, int32 flag);


// Premium Storage [Cydh]
void storage_premiumStorage_open(map_session_data *sd);
bool storage_premiumStorage_load(map_session_data *sd, uint8 num, uint8 mode);
void storage_premiumStorage_save(map_session_data *sd);
void storage_premiumStorage_close(map_session_data *sd);
void storage_premiumStorage_quit(map_session_data *sd);

int32 compare_item(struct item *a, struct item *b);

#endif /* STORAGE_HPP */
