// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Emotion UI Packet System (PACKETVER >= 20230925)
 * Handles the new emotion expansion UI packets introduced in client 20230925.
 * Author: V!be Coding [kreanlnwza] AI Assistant (Claude Sonnet 4.6)
 **/

#ifndef EMOTION_UI_PACKET_HPP
#define EMOTION_UI_PACKET_HPP

#ifdef _MSC_VER
#pragma warning( disable : 4200 )
#endif

#include <cstdint>
#include <string>
#include <vector>

#include <common/database.hpp>

// Forward declarations to avoid circular includes
enum emotion_type : int;  // defined in clif.hpp
struct block_list;
class map_session_data;

// ============================================================
// Packet header constants (PACKETVER >= 20230925)
// ============================================================

inline constexpr uint16_t HEADER_CZ_REQ_EMOTION2           = 0x0be9;
inline constexpr uint16_t HEADER_ZC_EMOTION2                = 0x0bea;
inline constexpr uint16_t HEADER_ZC_EMOTION2_FAIL           = 0x0beb;
inline constexpr uint16_t HEADER_CZ_REQ_EMOTION2_EXPANTION  = 0x0bec;
inline constexpr uint16_t HEADER_ZC_EMOTION2_EXPANTION      = 0x0bed;
inline constexpr uint16_t HEADER_ZC_EMOTION2_EXPANTION_FAIL = 0x0bee;
inline constexpr uint16_t HEADER_ZC_EMOTION2_EXPANTION_LIST = 0x0bf6;

// ============================================================
// Packet structs
// ============================================================

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000)
#pragma pack(push, 1)
#endif

/// Client -> Server: Use expanded emotion
struct PACKET_CZ_REQ_EMOTION2 {
	uint16_t PacketType;
	uint16_t ExpantionId;
	uint16_t EmotionId;
};

/// Server -> Client: Broadcast expanded emotion to area
struct PACKET_ZC_EMOTION2 {
	uint16_t PacketType;
	uint32_t GID;
	uint16_t ExpantionId;
	uint16_t EmotionId;
};

/// Server -> Client: Expanded emotion use failed
struct PACKET_ZC_EMOTION2_FAIL {
	uint16_t PacketType;
	uint16_t ExpantionId;
	uint16_t EmotionId;
	uint8_t  Status;
};

/// Client -> Server: Request to purchase an emotion expansion
struct PACKET_CZ_REQ_EMOTION2_EXPANTION {
	uint16_t PacketType;
	uint16_t ExpantionId;
	uint16_t ItemId;
	uint8_t  Amount;
};

/// Server -> Client: Emotion expansion purchase success
struct PACKET_ZC_EMOTION2_EXPANTION {
	uint16_t PacketType;
	uint16_t ExpantionId;
	uint8_t  bRented;
	uint32_t Timestamp;
};

/// Server -> Client: Emotion expansion purchase failed
struct PACKET_ZC_EMOTION2_EXPANTION_FAIL {
	uint16_t PacketType;
	uint16_t ExpantionId;
	uint8_t  Status;
};

/// Sub-entry in the expansion list packet
struct PACKET_ZC_EMOTION2_EXPANTION_LIST_SUB {
	uint16_t ExpantionId;
	uint8_t  Rented;
	uint32_t Timestamp;
};

/// Server -> Client: Send full expansion list to client (variable length)
struct PACKET_ZC_EMOTION2_EXPANTION_LIST {
	uint16_t PacketType;
	uint16_t PacketLength;
	uint32_t Timestamp;
	int16_t  Timezone;
	PACKET_ZC_EMOTION2_EXPANTION_LIST_SUB List[];
};

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000)
#pragma pack(pop)
#endif

// ============================================================
// Status enums
// ============================================================

/// Status codes for ZC_EMOTION2_FAIL
enum EEmotionStatus : uint8_t {
	EMSG_EMOTION_EXPANTION_USE_FAIL_DATE,
	EMSG_EMOTION_EXPANTION_USE_FAIL_UNPURCHASED,
	EMSG_EMOTION_USE_FAIL_SKILL_LEVEL,
	EMSG_EMOTION_EXPANTION_USE_FAIL_UNKNOWN,
};

/// Status codes for ZC_EMOTION2_EXPANTION_FAIL
enum EEmotionExpantionStatus : uint8_t {
	EMSG_EMOTION_EXPANTION_NOT_ENOUGH_NYANGVINE,
	EMSG_EMOTION_EXPANTION_FAIL_DATE,
	EMSG_EMOTION_EXPANTION_FAIL_ALREADY_BUY,
	EMSG_EMOTION_EXPANTION_FAIL_ANOTHER_SALE_BUY,
	EMSG_EMOTION_EXPANTION_NOT_ENOUGH_BASICSKILL_LEVEL,
	EMSG_NOT_YET_SALE_START_TIME,
	EMSG_EMOTION_EXPANTION_FAIL_UNKNOWN,
};

// ============================================================
// Database structures
// ============================================================

/// Single emotion expansion entry loaded from db/emotion_db.yml
struct s_emotion_db {
	uint16_t Id;
	uint16_t Price;
	uint16_t Type;
	uint32_t SaleStart;
	uint32_t SaleEnd;
	uint32_t SaleRentalPeriod;
	std::vector<emotion_type> Emotions;
};

/// YAML database for emotion expansions
class EmotionDatabase : public TypesafeCachedYamlDatabase<uint16_t, s_emotion_db> {
public:
	EmotionDatabase() : TypesafeCachedYamlDatabase("EMOTION_DB", 1) {}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
};

extern EmotionDatabase emotion_db;

// ============================================================
// Function declarations
// ============================================================

// Init / Finalize
void do_init_emotions(void);
void do_final_emotions(void);

// PC functions
void pc_use_emotion(map_session_data* const sd, const uint16_t ExpantionId, const uint16_t EmotionId);
void pc_buy_emotion_expantion(map_session_data* const sd, const uint16_t ExpantionId, const uint16_t ItemId, const uint8_t Amount);
void pc_load_emotion_expantion_list(map_session_data* const sd);

// CLIF functions (PACKETVER >= 20230925)
void clif_parse_emotion2(const int fd, map_session_data* const sd);
void clif_emotion2(block_list* const bl, const uint16_t ExpantionId, const uint16_t EmotionId);
void clif_emotion2_fail(map_session_data* const sd, const uint16_t ExpantionId, const uint16_t EmotionId, const EEmotionStatus Status);
void clif_parse_emotion2_expantion(const int fd, map_session_data* const sd);
void clif_emotion2_expantion(map_session_data* const sd, const uint16_t ExpantionId, const bool bRented, const uint32_t RentEndTime);
void clif_emotion2_expantion_fail(map_session_data* const sd, const uint16_t ExpantionId, const EEmotionExpantionStatus Status);
void clif_emotion2_expantion_list(map_session_data* const sd, const std::vector<PACKET_ZC_EMOTION2_EXPANTION_LIST_SUB>& List);

#endif // EMOTION_UI_PACKET_HPP
