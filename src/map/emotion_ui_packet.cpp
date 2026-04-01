// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Emotion UI Packet System - Implementation (PACKETVER >= 20230925)
 * Implements database loading, PC emotion functions, and CLIF packet handlers
 * for the new emotion expansion UI system.
 * Author: V!be Coding [kreanlnwza] AI Assistant (Claude Sonnet 4.6)
 **/

#include "emotion_ui_packet.hpp"

#include <cstring>
#include <ctime>

#include <common/core.hpp>       // db_path
#include <common/nullpo.hpp>     // nullpo_retv
#include <common/random.hpp>     // rnd()
#include <common/socket.hpp>     // WFIFOHEAD, WFIFOP, WFIFOSET, RFIFOP
#include <common/timer.hpp>      // last_tick
#include <common/utilities.hpp>  // util::vector_exists

using namespace rathena;

#include "battle.hpp"    // battle_config
#include "clif.hpp"      // clif_send, AREA, emotion_type, ET_DICE1, ET_DICE6, ET_MAX
#include "log.hpp"       // LOG_TYPE_CONSUME
#include "map.hpp"       // map_session_data (via pc.hpp), block_list
#include "pc.hpp"        // pc_checkskill, pc_readglobalreg, pc_setglobalreg, pc_search_inventory, pc_delitem
#include "script.hpp"    // script_get_constant, add_str

// ============================================================
// EmotionDatabase instance
// ============================================================

EmotionDatabase emotion_db;

// ============================================================
// EmotionDatabase methods
// ============================================================

const std::string EmotionDatabase::getDefaultLocation() {
	return std::string(db_path) + "/emotion_db.yml";
}

uint64_t EmotionDatabase::parseBodyNode(const ryml::NodeRef& Node)
{
	int16_t Id;
	if (!this->asInt16(Node, "Id", Id))
		return 0;

	std::shared_ptr<s_emotion_db> EmotionsInfo = this->find(Id);
	const bool bExists = EmotionsInfo != nullptr;
	if (!bExists) {
		EmotionsInfo = std::make_shared<s_emotion_db>();
		EmotionsInfo->Id = Id;
	}

	if (this->nodeExists(Node, "Price")) {
		uint16_t Price = 0;
		if (!this->asUInt16(Node, "Price", Price))
			return 0;
		if (Price > MAX_AMOUNT) {
			this->invalidWarning(Node["Price"], "Emotion expantion '%hu' amount it is too high, capping it to MAX_AMOUNT...\n", Id);
			Price = MAX_AMOUNT;
		}
		EmotionsInfo->Price = Price;
	} else {
		EmotionsInfo->Price = 0;
	}

	if (this->nodeExists(Node, "Type")) {
		uint16_t Type = 0;
		if (!this->asUInt16(Node, "Type", Type))
			return 0;
		if (Type > 1) {
			this->invalidWarning(Node["Type"], "Emotion expantion '%hu' type it is invalid, capping it to 1...\n", Id);
			Type = 1;
		}
		EmotionsInfo->Type = Type;
	} else {
		EmotionsInfo->Type = 0;
	}

	if (this->nodeExists(Node, "SaleStart")) {
		uint32_t SaleStart = 0;
		if (!this->asUInt32(Node, "SaleStart", SaleStart))
			return 0;
		if (SaleStart != 0 && (SaleStart < 20020000 || SaleStart > 29990000)) {
			this->invalidWarning(Node["SaleStart"], "Emotion expantion '%hu' sale start it is invalid, capping it to 0...\n", Id);
			SaleStart = 0;
		}
		EmotionsInfo->SaleStart = SaleStart;
	} else {
		EmotionsInfo->SaleStart = 0;
	}

	if (this->nodeExists(Node, "SaleEnd")) {
		uint32_t SaleEnd = 0;
		if (!this->asUInt32(Node, "SaleEnd", SaleEnd))
			return 0;
		if (SaleEnd != 0 && (SaleEnd < 20020000 || SaleEnd > 29990000 || EmotionsInfo->SaleStart > SaleEnd)) {
			this->invalidWarning(Node["SaleEnd"], "Emotion expantion '%hu' sale start it is invalid, capping it to 0...\n", Id);
			EmotionsInfo->SaleStart = 0;
			SaleEnd = 0;
		}
		EmotionsInfo->SaleEnd = SaleEnd;
	} else {
		EmotionsInfo->SaleEnd = 0;
	}

	if (this->nodeExists(Node, "SaleRentalPeriod")) {
		uint32_t SaleRentalPeriod = 0;
		if (!this->asUInt32(Node, "SaleRentalPeriod", SaleRentalPeriod))
			return 0;
		EmotionsInfo->SaleRentalPeriod = SaleRentalPeriod * 60u * 60u * 24u;
	} else {
		EmotionsInfo->SaleRentalPeriod = 0;
	}

	for (const ryml::NodeRef& EmotionsNode : Node["Emotions"]) {
		std::string EmotionName;
		c4::from_chars(EmotionsNode.val(), &EmotionName);

		int64_t EmotionId = 0;
		script_get_constant(EmotionName.c_str(), &EmotionId);

		if (EmotionId < 0 || EmotionId >= ET_MAX) {
			this->invalidWarning(Node["Emotions"], "Invalid emotion constant with name '%s'.\n", EmotionName.c_str());
			return 0;
		}

		EmotionsInfo->Emotions.push_back(static_cast<emotion_type>(EmotionId));
	}

	if (!bExists)
		this->put(Id, EmotionsInfo);

	return 1;
}

// ============================================================
// Init / Final
// ============================================================

void do_init_emotions(void) {
	emotion_db.load();
}

void do_final_emotions(void) {
	emotion_db.clear();
}

// ============================================================
// PC functions
// ============================================================

void pc_use_emotion(map_session_data* const sd, const uint16_t Id, const uint16_t EmotionId)
{
	nullpo_retv(sd);

	if (battle_config.basic_skill_check != 0 && pc_checkskill(sd, NV_BASIC) < 2 && pc_checkskill(sd, SU_BASIC_SKILL) < 1) {
		clif_emotion2_fail(sd, Id, EmotionId, EMSG_EMOTION_USE_FAIL_SKILL_LEVEL);
		return;
	}

	if (sd->emotionlasttime + 1 >= time(nullptr)) {
		sd->emotionlasttime = time(nullptr);
		clif_emotion2_fail(sd, Id, EmotionId, EMSG_EMOTION_EXPANTION_USE_FAIL_UNKNOWN);
		return;
	}
	sd->emotionlasttime = time(nullptr);

	if (battle_config.idletime_option & IDLE_EMOTION)
		sd->idletime = last_tick;
	if (battle_config.hom_idle_no_share && sd->hd && battle_config.idletime_hom_option & IDLE_EMOTION)
		sd->idletime_hom = last_tick;
	if (battle_config.mer_idle_no_share && sd->md && battle_config.idletime_mer_option & IDLE_EMOTION)
		sd->idletime_mer = last_tick;

	if (sd->state.block_action & PCBLOCK_EMOTION) {
		clif_emotion2_fail(sd, Id, EmotionId, EMSG_EMOTION_EXPANTION_USE_FAIL_UNKNOWN);
		return;
	}

	if (battle_config.client_reshuffle_dice && EmotionId >= ET_DICE1 && EmotionId <= ET_DICE6) {
		const uint16_t DiceEmotionId = static_cast<uint16_t>(rnd() % 6 + ET_DICE1);
		clif_emotion2(sd, 0, DiceEmotionId);
		return;
	}

	std::shared_ptr<s_emotion_db> EmotionsInfo = emotion_db.find(Id);
	if (EmotionsInfo == nullptr) {
		clif_emotion2_fail(sd, Id, EmotionId, EMSG_EMOTION_EXPANTION_USE_FAIL_UNKNOWN);
		return;
	}

	if (!util::vector_exists(EmotionsInfo->Emotions, static_cast<emotion_type>(EmotionId))) {
		clif_emotion2_fail(sd, Id, EmotionId, EMSG_EMOTION_EXPANTION_USE_FAIL_UNKNOWN);
		return;
	}

	if (EmotionsInfo->Id != 0) {
		char Buffer[32];
		const char* FormatString = nullptr;

		FormatString = EmotionsInfo->Type ? "emotion_%hu" : "#emotion_%hu";
		memset(Buffer, '\0', sizeof(Buffer));
		sprintf(Buffer, FormatString, EmotionsInfo->Id);

		const bool bExpantionBought = static_cast<bool>(pc_readglobalreg(sd, add_str(Buffer)));
		if (!bExpantionBought) {
			clif_emotion2_fail(sd, Id, EmotionId, EMSG_EMOTION_EXPANTION_USE_FAIL_UNPURCHASED);
			return;
		}

		FormatString = EmotionsInfo->Type ? "emotion_expire_%hu" : "#emotion_expire_%hu";
		memset(Buffer, '\0', sizeof(Buffer));
		sprintf(Buffer, FormatString, EmotionsInfo->Id);

		const time_t CurrentTime = time(nullptr);
		const int64_t ExpireTime = pc_readglobalreg(sd, add_str(Buffer));
		if (EmotionsInfo->SaleRentalPeriod != 0 && CurrentTime > time_t(ExpireTime)) {
			clif_emotion2_fail(sd, Id, EmotionId, EMSG_EMOTION_EXPANTION_USE_FAIL_DATE);
			return;
		}
	}

	clif_emotion2(sd, Id, EmotionId);
}

void pc_buy_emotion_expantion(map_session_data* const sd, const uint16_t Id, const uint16_t ItemId, const uint8_t Amount)
{
	nullpo_retv(sd);

	if (battle_config.basic_skill_check != 0 && pc_checkskill(sd, NV_BASIC) < 2 && pc_checkskill(sd, SU_BASIC_SKILL) < 1) {
		clif_emotion2_expantion_fail(sd, Id, EMSG_EMOTION_EXPANTION_FAIL_UNKNOWN);
		return;
	}

	std::shared_ptr<s_emotion_db> EmotionsInfo = emotion_db.find(Id);
	if (EmotionsInfo == nullptr) {
		clif_emotion2_expantion_fail(sd, Id, EMSG_EMOTION_EXPANTION_FAIL_UNKNOWN);
		return;
	}

	const time_t CurrentTime = time(nullptr);
	if (EmotionsInfo->SaleEnd != 0 && EmotionsInfo->SaleEnd < static_cast<uint32_t>(CurrentTime)) {
		clif_emotion2_expantion_fail(sd, Id, EMSG_EMOTION_EXPANTION_FAIL_DATE);
		return;
	}
	if (EmotionsInfo->SaleStart != 0 && EmotionsInfo->SaleStart > static_cast<uint32_t>(CurrentTime)) {
		clif_emotion2_expantion_fail(sd, Id, EMSG_NOT_YET_SALE_START_TIME);
		return;
	}

	char Buffer[32];
	const char* FormatString = EmotionsInfo->Type ? "emotion_%hu" : "#emotion_%hu";
	memset(Buffer, '\0', sizeof(Buffer));
	sprintf(Buffer, FormatString, EmotionsInfo->Id);

	const bool bExpantionBought = static_cast<bool>(pc_readglobalreg(sd, add_str(Buffer)));
	if (bExpantionBought) {
		clif_emotion2_expantion_fail(sd, Id, EMSG_EMOTION_EXPANTION_FAIL_ALREADY_BUY);
		return;
	}

	if (ItemId != 6909) {
		clif_emotion2_expantion_fail(sd, Id, EMSG_EMOTION_EXPANTION_FAIL_UNKNOWN);
		return;
	}
	if (EmotionsInfo->Price != Amount) {
		clif_emotion2_expantion_fail(sd, Id, EMSG_EMOTION_EXPANTION_FAIL_UNKNOWN);
		return;
	}

	const int32_t NyangvineIndex = pc_search_inventory(sd, ItemId);
	if (NyangvineIndex < 0 || sd->inventory.u.items_inventory[NyangvineIndex].amount < Amount) {
		clif_emotion2_expantion_fail(sd, Id, EMSG_EMOTION_EXPANTION_NOT_ENOUGH_NYANGVINE);
		return;
	}

	pc_delitem(sd, NyangvineIndex, Amount, 0, 0, LOG_TYPE_CONSUME);
	pc_setglobalreg(sd, add_str(Buffer), 1);

	if (EmotionsInfo->SaleRentalPeriod != 0) {
		FormatString = EmotionsInfo->Type ? "emotion_expire_%hu" : "#emotion_expire_%hu";
		memset(Buffer, '\0', sizeof(Buffer));
		sprintf(Buffer, FormatString, EmotionsInfo->Id);

		const int64_t ExpireTime = static_cast<int64_t>(CurrentTime) + static_cast<int64_t>(EmotionsInfo->SaleRentalPeriod);
		pc_setglobalreg(sd, add_str(Buffer), ExpireTime);
		clif_emotion2_expantion(sd, Id, true, static_cast<uint32_t>(ExpireTime));
		return;
	}

	clif_emotion2_expantion(sd, Id, false, 0);
}

void pc_load_emotion_expantion_list(map_session_data* const sd)
{
	nullpo_retv(sd);

	char Buffer1[32];
	char Buffer2[32];
	const char* FormatString = nullptr;

	std::vector<PACKET_ZC_EMOTION2_EXPANTION_LIST_SUB> EmotionExpantionList;
	for (const std::pair<uint16_t, std::shared_ptr<s_emotion_db>>& EmotionPair : emotion_db) {
		const uint16_t& ExpantionId = EmotionPair.first;
		const std::shared_ptr<s_emotion_db>& EmotionsInfo = EmotionPair.second;

		FormatString = EmotionsInfo->Type ? "emotion_%hu" : "#emotion_%hu";
		memset(Buffer1, '\0', sizeof(Buffer1));
		sprintf(Buffer1, FormatString, EmotionsInfo->Id);

		FormatString = EmotionsInfo->Type ? "emotion_expire_%hu" : "#emotion_expire_%hu";
		memset(Buffer2, '\0', sizeof(Buffer2));
		sprintf(Buffer2, FormatString, EmotionsInfo->Id);

		const bool bRental = EmotionsInfo->SaleRentalPeriod != 0;
		const time_t CurrentTime = time(nullptr);
		const bool bExpantionBought = static_cast<bool>(pc_readglobalreg(sd, add_str(Buffer1)));
		const int64_t ExpireTime = pc_readglobalreg(sd, add_str(Buffer2));

		if (bExpantionBought && bRental && CurrentTime > static_cast<time_t>(ExpireTime)) {
			pc_setglobalreg(sd, add_str(Buffer1), 0);
			pc_setglobalreg(sd, add_str(Buffer2), 0);
			continue;
		}

		if (bExpantionBought) {
			PACKET_ZC_EMOTION2_EXPANTION_LIST_SUB sub{};
			sub.ExpantionId = ExpantionId;
			sub.Rented = bRental ? 1 : 0;
			sub.Timestamp = static_cast<uint32_t>(ExpireTime);
			EmotionExpantionList.push_back(sub);
		}
	}

	clif_emotion2_expantion_list(sd, EmotionExpantionList);
}

// ============================================================
// CLIF packet handlers (PACKETVER >= 20230925)
// ============================================================

void clif_parse_emotion2(const int fd, map_session_data* const sd)
{
#if (PACKETVER_MAIN_NUM >= 20230925)
	nullpo_retv(sd);
	const PACKET_CZ_REQ_EMOTION2* const Packet = reinterpret_cast<const PACKET_CZ_REQ_EMOTION2*>(RFIFOP(fd, 0));
	pc_use_emotion(sd, Packet->ExpantionId, Packet->EmotionId);
#endif
}

void clif_emotion2(block_list* const bl, const uint16_t ExpantionId, const uint16_t EmotionId)
{
#if (PACKETVER_MAIN_NUM >= 20230925)
	nullpo_retv(bl);
	PACKET_ZC_EMOTION2 Packet = {};
	Packet.PacketType = HEADER_ZC_EMOTION2;
	Packet.GID = bl->id;
	Packet.ExpantionId = ExpantionId;
	Packet.EmotionId = EmotionId;
	clif_send(&Packet, sizeof(PACKET_ZC_EMOTION2), bl, AREA);
#endif
}

void clif_emotion2_fail(map_session_data* const sd, const uint16_t ExpantionId, const uint16_t EmotionId, const EEmotionStatus Status)
{
#if (PACKETVER_MAIN_NUM >= 20230925)
	nullpo_retv(sd);
	const int fd = sd->fd;
	WFIFOHEAD(fd, sizeof(PACKET_ZC_EMOTION2_FAIL));
	PACKET_ZC_EMOTION2_FAIL* const Packet = reinterpret_cast<PACKET_ZC_EMOTION2_FAIL*>(WFIFOP(fd, 0));
	Packet->PacketType = HEADER_ZC_EMOTION2_FAIL;
	Packet->ExpantionId = ExpantionId;
	Packet->EmotionId = EmotionId;
	Packet->Status = static_cast<uint8_t>(Status);
	WFIFOSET(fd, sizeof(PACKET_ZC_EMOTION2_FAIL));
#endif
}

void clif_parse_emotion2_expantion(const int fd, map_session_data* const sd)
{
#if (PACKETVER_MAIN_NUM >= 20230925)
	nullpo_retv(sd);
	const PACKET_CZ_REQ_EMOTION2_EXPANTION* const Packet = reinterpret_cast<const PACKET_CZ_REQ_EMOTION2_EXPANTION*>(RFIFOP(fd, 0));
	pc_buy_emotion_expantion(sd, Packet->ExpantionId, Packet->ItemId, Packet->Amount);
#endif
}

void clif_emotion2_expantion(map_session_data* const sd, const uint16_t ExpantionId, const bool bRented, const uint32_t RentEndTime)
{
#if (PACKETVER_MAIN_NUM >= 20230925)
	nullpo_retv(sd);
	const int fd = sd->fd;
	WFIFOHEAD(fd, sizeof(PACKET_ZC_EMOTION2_EXPANTION));
	PACKET_ZC_EMOTION2_EXPANTION* const Packet = reinterpret_cast<PACKET_ZC_EMOTION2_EXPANTION*>(WFIFOP(fd, 0));
	Packet->PacketType = HEADER_ZC_EMOTION2_EXPANTION;
	Packet->ExpantionId = ExpantionId;
	Packet->bRented = bRented ? 1 : 0;
	Packet->Timestamp = RentEndTime;
	WFIFOSET(fd, sizeof(PACKET_ZC_EMOTION2_EXPANTION));
#endif
}

void clif_emotion2_expantion_fail(map_session_data* const sd, const uint16_t ExpantionId, const EEmotionExpantionStatus Status)
{
#if (PACKETVER_MAIN_NUM >= 20230925)
	nullpo_retv(sd);
	const int fd = sd->fd;
	WFIFOHEAD(fd, sizeof(PACKET_ZC_EMOTION2_EXPANTION_FAIL));
	PACKET_ZC_EMOTION2_EXPANTION_FAIL* const Packet = reinterpret_cast<PACKET_ZC_EMOTION2_EXPANTION_FAIL*>(WFIFOP(fd, 0));
	Packet->PacketType = HEADER_ZC_EMOTION2_EXPANTION_FAIL;
	Packet->ExpantionId = ExpantionId;
	Packet->Status = static_cast<uint8_t>(Status);
	WFIFOSET(fd, sizeof(PACKET_ZC_EMOTION2_EXPANTION_FAIL));
#endif
}

void clif_emotion2_expantion_list(map_session_data* const sd, const std::vector<PACKET_ZC_EMOTION2_EXPANTION_LIST_SUB>& List)
{
#if (PACKETVER_MAIN_NUM >= 20230925)
	nullpo_retv(sd);
	const int fd = sd->fd;
	const size_t PacketTotalSize = sizeof(PACKET_ZC_EMOTION2_EXPANTION_LIST) + sizeof(PACKET_ZC_EMOTION2_EXPANTION_LIST_SUB) * List.size();
	WFIFOHEAD(fd, static_cast<int>(PacketTotalSize));
	PACKET_ZC_EMOTION2_EXPANTION_LIST* const Packet = reinterpret_cast<PACKET_ZC_EMOTION2_EXPANTION_LIST*>(WFIFOP(fd, 0));
	Packet->PacketType = HEADER_ZC_EMOTION2_EXPANTION_LIST;
	Packet->Timestamp = static_cast<uint32_t>(time(nullptr));
	Packet->Timezone = 540;
	for (size_t Num = 0; Num < List.size(); ++Num)
		Packet->List[Num] = List[Num];
	Packet->PacketLength = static_cast<uint16_t>(PacketTotalSize);
	WFIFOSET(fd, static_cast<int>(PacketTotalSize));
#endif
}
