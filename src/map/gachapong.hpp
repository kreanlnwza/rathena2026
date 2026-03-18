/**
 * Gachapong System - Random Item by Type from Item Database
 * ระบบสุ่มไอเทมจาก item_db ตาม item_types
 * รองรับ: IT_ETC, IT_ARMOR, IT_WEAPON, IT_CARD, IT_PETARMOR, IT_SHADOWGEAR
 * Author: V!be Coding [kreanlnwza] AI Assistant (Claude)
 *
 * NOTE: ใช้ร่วมกับ NPC script ผ่าน buildin commands
 **/

#ifndef GACHAPONG_HPP
#define GACHAPONG_HPP

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

#include "../common/cbasetypes.hpp"
#include "../common/mmo.hpp"

// Forward declaration
struct item_data;

/**
 * Gachapong item cache - เก็บ item_id ที่จัดกลุ่มตาม item_types
 * สร้าง cache ตอน server start เพื่อไม่ต้อง scan item_db ทุกครั้ง
 */
class GachapongDatabase {
private:
	// Cache: item_types -> vector of item_id
	std::unordered_map<int32, std::vector<t_itemid>> type_cache;
	// Cache: equip_pos -> vector of item_id
	std::unordered_map<uint32, std::vector<t_itemid>> equip_cache;
	// Blacklist: item_id ที่ห้ามสุ่มออก
	std::unordered_set<t_itemid> blacklist;
	bool initialized;

public:
	GachapongDatabase() : initialized(false) {}

	/**
	 * Build cache จาก item_db โดยกรองเฉพาะ types ที่กำหนด
	 * เรียกตอน server start หรือ reload item_db
	 */
	void build_cache();

	/**
	 * สุ่มไอเทม 1 ชิ้นจาก type ที่ระบุ
	 * @param type item_types (IT_ETC, IT_ARMOR, etc.)
	 * @return item_id ที่สุ่มได้ หรือ 0 ถ้าไม่พบ
	 */
	t_itemid get_random_item(int32 type);

	/**
	 * นับจำนวนไอเทมใน type ที่ระบุ
	 * @param type item_types
	 * @return จำนวนไอเทมใน cache
	 */
	int32 get_type_count(int32 type);

	/**
	 * ดึง item_id ตาม index จาก type ที่ระบุ
	 * @param type item_types
	 * @param index ลำดับใน cache
	 * @return item_id หรือ 0 ถ้า index ไม่ถูกต้อง
	 */
	t_itemid get_item_at(int32 type, int32 index);

	/**
	 * ตรวจสอบว่า type นี้มีในระบบหรือไม่
	 */
	bool type_exists(int32 type);

	/**
	 * สุ่มไอเทม 1 ชิ้นจาก equip_pos ที่ระบุ
	 * @param pos equip_pos bitmask (EQP_HEAD_TOP, EQP_ARMOR, etc.)
	 * @return item_id ที่สุ่มได้ หรือ 0 ถ้าไม่พบ
	 */
	t_itemid get_random_equip_item(uint32 pos);

	/**
	 * นับจำนวนไอเทมใน equip_pos ที่ระบุ
	 * @param pos equip_pos bitmask
	 * @return จำนวนไอเทมใน cache
	 */
	int32 get_equip_count(uint32 pos);

	/**
	 * ตรวจสอบว่า equip_pos นี้มีในระบบหรือไม่
	 */
	bool equip_exists(uint32 pos);

	/**
	 * ตรวจสอบว่า cache ถูกสร้างแล้ว
	 */
	bool is_initialized() const { return initialized; }

	/**
	 * ล้าง cache ทั้งหมด
	 */
	void clear();
};

extern GachapongDatabase gachapong_db;

// Initialize function (เรียกจาก do_init_itemdb)
void do_init_gachapong();

#endif /* GACHAPONG_HPP */
