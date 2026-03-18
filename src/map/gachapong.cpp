/**
 * Gachapong System - Random Item by Type from Item Database
 * ระบบสุ่มไอเทมจาก item_db ตาม item_types
 * Author: V!be Coding [kreanlnwza] AI Assistant (Claude)
 **/

#include "gachapong.hpp"

#include <algorithm>

#include "../common/showmsg.hpp"
#include "../common/random.hpp"
#include "../common/utilities.hpp"

#include "itemdb.hpp"

// Global instance
GachapongDatabase gachapong_db;

/**
 * Build cache จาก item_db โดยกรองเฉพาะ types ที่รองรับ
 */
void GachapongDatabase::build_cache() {
	// Blacklist: ไอเทมที่ห้ามสุ่มออก (GM items, godly items)
	this->blacklist = {
		1599,   // Angra Manyu
		2199,   // Ahura Mazdah
		2904,   // Naqsi
		15065,  // Amesha Spenta
		19429,  // Zarathustra
	};

	// Clear caches (but keep blacklist)
	this->type_cache.clear();
	this->equip_cache.clear();
	this->initialized = false;

	// Types ที่รองรับในระบบ Gachapong
	const int32 supported_types[] = {
		IT_ETC,
		IT_ARMOR,
		IT_WEAPON,
		IT_CARD,
		IT_PETARMOR,
		IT_SHADOWGEAR,
		IT_HEALING,
		IT_USABLE,
		IT_PETEGG,
		IT_AMMO,
		IT_DELAYCONSUME
	};

	// Equip positions ที่รองรับ
	const uint32 supported_equips[] = {
		EQP_HEAD_LOW,
		EQP_HEAD_MID,
		EQP_HEAD_TOP,
		EQP_HAND_R,
		EQP_HAND_L,
		EQP_ARMOR,
		EQP_SHOES,
		EQP_GARMENT,
		EQP_ACC_R,
		EQP_ACC_L,
		EQP_COSTUME_HEAD_TOP,
		EQP_COSTUME_HEAD_MID,
		EQP_COSTUME_HEAD_LOW,
		EQP_COSTUME_GARMENT,
		EQP_AMMO,
		EQP_SHADOW_ARMOR,
		EQP_SHADOW_WEAPON,
		EQP_SHADOW_SHIELD,
		EQP_SHADOW_SHOES,
		EQP_SHADOW_ACC_R,
		EQP_SHADOW_ACC_L
	};

	// สร้าง empty vectors สำหรับทุก type ที่รองรับ
	for (auto type : supported_types) {
		this->type_cache[type] = std::vector<t_itemid>();
	}

	// สร้าง empty vectors สำหรับทุก equip_pos ที่รองรับ
	for (auto pos : supported_equips) {
		this->equip_cache[pos] = std::vector<t_itemid>();
	}

	// Scan item_db และจัดกลุ่มตาม type + equip_pos
	for (const auto& pair : item_db) {
		std::shared_ptr<item_data> id = pair.second;

		if (id == nullptr)
			continue;

		// Skip blacklist items
		if (this->blacklist.find(id->nameid) != this->blacklist.end())
			continue;

		int32 type = static_cast<int32>(id->type);

		// เพิ่มเฉพาะ types ที่รองรับ
		if (this->type_cache.find(type) != this->type_cache.end()) {
			this->type_cache[type].push_back(id->nameid);
		}

		// จัดกลุ่มตาม equip_pos (bitmask check)
		if (id->equip != 0) {
			for (auto pos : supported_equips) {
				if (id->equip & pos) {
					this->equip_cache[pos].push_back(id->nameid);
				}
			}
		}
	}

	this->initialized = true;

	// แสดงสถิติ
	int32 total_type = 0;
	for (const auto& pair : this->type_cache) {
		total_type += static_cast<int32>(pair.second.size());
	}

	int32 total_equip = 0;
	int32 equip_types = 0;
	for (const auto& pair : this->equip_cache) {
		int32 count = static_cast<int32>(pair.second.size());
		if (count > 0) {
			total_equip += count;
			equip_types++;
		}
	}

	ShowStatus("[Gachapong] Cache built: %d items across %d types, %d equip entries across %d positions.\n",
		total_type, static_cast<int32>(this->type_cache.size()), total_equip, equip_types);
}

/**
 * สุ่มไอเทม 1 ชิ้นจาก type ที่ระบุ
 */
t_itemid GachapongDatabase::get_random_item(int32 type) {
	if (!this->initialized) {
		ShowError("[Gachapong] Database not initialized!\n");
		return 0;
	}

	auto it = this->type_cache.find(type);
	if (it == this->type_cache.end() || it->second.empty()) {
		return 0;
	}

	size_t idx = rnd_value<size_t>(0, it->second.size() - 1);
	return it->second[idx];
}

/**
 * นับจำนวนไอเทมใน type ที่ระบุ
 */
int32 GachapongDatabase::get_type_count(int32 type) {
	auto it = this->type_cache.find(type);
	if (it == this->type_cache.end()) {
		return 0;
	}
	return static_cast<int32>(it->second.size());
}

/**
 * ดึง item_id ตาม index จาก type ที่ระบุ
 */
t_itemid GachapongDatabase::get_item_at(int32 type, int32 index) {
	auto it = this->type_cache.find(type);
	if (it == this->type_cache.end() || index < 0 || index >= static_cast<int32>(it->second.size())) {
		return 0;
	}
	return it->second[index];
}

/**
 * ตรวจสอบว่า type นี้มีในระบบหรือไม่
 */
bool GachapongDatabase::type_exists(int32 type) {
	auto it = this->type_cache.find(type);
	return (it != this->type_cache.end() && !it->second.empty());
}

/**
 * สุ่มไอเทม 1 ชิ้นจาก equip_pos ที่ระบุ
 */
t_itemid GachapongDatabase::get_random_equip_item(uint32 pos) {
	if (!this->initialized) {
		ShowError("[Gachapong] Database not initialized!\n");
		return 0;
	}

	auto it = this->equip_cache.find(pos);
	if (it == this->equip_cache.end() || it->second.empty()) {
		return 0;
	}

	size_t idx = rnd_value<size_t>(0, it->second.size() - 1);
	return it->second[idx];
}

/**
 * นับจำนวนไอเทมใน equip_pos ที่ระบุ
 */
int32 GachapongDatabase::get_equip_count(uint32 pos) {
	auto it = this->equip_cache.find(pos);
	if (it == this->equip_cache.end()) {
		return 0;
	}
	return static_cast<int32>(it->second.size());
}

/**
 * ตรวจสอบว่า equip_pos นี้มีในระบบหรือไม่
 */
bool GachapongDatabase::equip_exists(uint32 pos) {
	auto it = this->equip_cache.find(pos);
	return (it != this->equip_cache.end() && !it->second.empty());
}

/**
 * ล้าง cache เท่านั้น (blacklist ยังคงอยู่)
 */
void GachapongDatabase::clear() {
	this->type_cache.clear();
	this->equip_cache.clear();
	// Note: blacklist ไม่ถูกลบ - ถูกสร้างใหม่ทุกครั้ง build_cache() ถูกเรียก
	this->initialized = false;
}

/**
 * Initialize gachapong system
 */
void do_init_gachapong() {
	gachapong_db.build_cache();
}
