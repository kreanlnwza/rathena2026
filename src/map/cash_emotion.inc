/**
 * Cash Emoji protocol for the locally verified 2026 MAIN client.
 * Author: V!be Coding [kreanlnwza] AI Assistant (Codex)
 * Catalog: effective CashEmotionList.lua and CashEmotionListDefine.lua.
 * Uses existing account registry and inventory save paths; no new SQL schema.
 */
// BF6 with timezone replaces BEF from 2023-09-20. Runtime verified on 2026 MAIN.
#if PACKETVER_MAIN_NUM >= 20230920
namespace cash_emotion {
static int64 ownership_key(const s_cash_emotion_pack& pack) {
	return add_str(pack.registry.c_str());
}

static bool owned(const map_session_data& sd, const s_cash_emotion_pack& pack) {
	return pc_readaccountreg(&sd, ownership_key(pack)) != 0;
}

static bool valid_emotion(const s_cash_emotion_pack& pack, uint16 emotion) {
	return std::find(pack.emotions.begin(), pack.emotions.end(), emotion) != pack.emotions.end();
}

static bool basic_skill(const map_session_data& sd) {
	return !battle_config.basic_skill_check || pc_checkskill(&sd, NV_BASIC) >= 2 || pc_checkskill(&sd, SU_BASIC_SKILL) >= 1;
}

static void initialize(map_session_data& sd) {
	uint16 length = 17;
	for (const auto& entry : cash_emotion_db)
		if (owned(sd, *entry.second))
			length += 7;
	const int32 fd = sd.fd;
	WFIFOHEAD(fd, length);
	memset(WFIFOP(fd, 0), 0, length);
	WFIFOW(fd, 0) = 0x0bf6;
	WFIFOW(fd, 2) = length;
	WFIFOL(fd, 4) = static_cast<uint32>(time(nullptr));
	WFIFOW(fd, 8) = 0; // UTC; client converts to its local timezone.
	WFIFOW(fd, 10) = 0; // Basic pack, permanent, available to every player.
	uint16 offset = 17;
	for (const auto& entry : cash_emotion_db) {
		const auto& pack = *entry.second;
		if (owned(sd, pack)) {
			WFIFOW(fd, offset) = pack.id; // Permanent: flag and expiry zero.
			offset += 7;
		}
	}
	WFIFOSET(fd, length);
}

static void purchase_failure(map_session_data& sd, uint16 pack, uint8 result) {
	WFIFOHEAD(sd.fd, 5);
	WFIFOW(sd.fd, 0) = 0x0bee;
	WFIFOW(sd.fd, 2) = pack;
	WFIFOB(sd.fd, 4) = result;
	WFIFOSET(sd.fd, 5);
}

static void use_failure(map_session_data& sd, uint16 pack, uint16 emotion, uint8 result) {
	WFIFOHEAD(sd.fd, 7);
	WFIFOW(sd.fd, 0) = 0x0beb;
	WFIFOW(sd.fd, 2) = pack;
	WFIFOW(sd.fd, 4) = emotion;
	WFIFOB(sd.fd, 6) = result;
	WFIFOSET(sd.fd, 7);
}

static void broadcast(map_session_data& sd, uint16 pack, uint16 emotion) {
	uint8 packet[10] = {};
	WBUFW(packet, 0) = 0x0bea;
	WBUFL(packet, 2) = sd.id;
	WBUFW(packet, 6) = pack;
	WBUFW(packet, 8) = emotion;
	clif_send(packet, sizeof(packet), &sd, AREA);
}
} // namespace cash_emotion

static void clif_parse_cash_emotion_buy(int32 fd, map_session_data* sd) {
	if (sd == nullptr)
		return;
	using namespace cash_emotion;
	const uint16 requested_pack = RFIFOW(fd, 2);
	const auto pack = cash_emotion_db.find(requested_pack);
	// The final byte is the price shown by the client, not a sale-type flag.
	// Reject stale/mismatched quotes; deduction always uses our catalog price.
	if (pack == nullptr || RFIFOW(fd, 4) != pack->currency || RFIFOB(fd, 6) != pack->price) {
		// Result 3 means already purchased under another sale type in this client.
		purchase_failure(*sd, requested_pack, 255);
		return;
	}
	if (!sd->vars_ok || !chrif_isconnected() || pc_cant_act(sd) || pc_isdead(sd)) {
		purchase_failure(*sd, requested_pack, 255);
		return;
	}
	if (!basic_skill(*sd)) {
		purchase_failure(*sd, requested_pack, 4);
		return;
	}
	if (owned(*sd, *pack)) {
		purchase_failure(*sd, requested_pack, 2);
		return;
	}
	int32 amount = 0;
	for (int32 i = 0; i < MAX_INVENTORY; ++i) {
		if (sd->inventory.u.items_inventory[i].nameid == pack->currency) {
			if (sd->inventory_data[i] == nullptr || sd->inventory.u.items_inventory[i].amount <= 0) {
				purchase_failure(*sd, requested_pack, 255);
				return;
			}
			amount += sd->inventory.u.items_inventory[i].amount;
		}
	}
	if (amount < pack->price) {
		purchase_failure(*sd, requested_pack, 0);
		return;
	}
	// The map loop serializes requests. Mark ownership before consuming so
	// duplicate requests cannot spend twice; loaded registry is required above.
	if (!pc_setaccountreg(sd, ownership_key(*pack), 1)) {
		purchase_failure(*sd, requested_pack, 255);
		return;
	}
	int32 remaining = pack->price;
	for (int32 i = 0; i < MAX_INVENTORY && remaining > 0; ++i) {
		const auto& item = sd->inventory.u.items_inventory[i];
		if (item.nameid != pack->currency)
			continue;
		const int32 take = std::min<int32>(remaining, item.amount);
		// Indices and quantities were checked in the same map-loop callback.
		pc_delitem(sd, i, take, 0, 0, LOG_TYPE_CASH);
		remaining -= take;
	}
	chrif_save(sd, CSAVE_INVENTORY);
	WFIFOHEAD(fd, 9);
	WFIFOW(fd, 0) = 0x0bed;
	WFIFOW(fd, 2) = requested_pack;
	WFIFOB(fd, 4) = 0;
	WFIFOL(fd, 5) = 0;
	WFIFOSET(fd, 9);
}

static void clif_parse_cash_emotion_use(int32 fd, map_session_data* sd) {
	using namespace cash_emotion;
	const uint16 pack = RFIFOW(fd, 2);
	const uint16 emotion = RFIFOW(fd, 4);
	const auto definition = cash_emotion_db.find(pack);
	if (definition == nullptr || !valid_emotion(*definition, emotion) || !owned(*sd, *definition)) {
		use_failure(*sd, pack, emotion, 1);
		return;
	}
	if (!basic_skill(*sd) || (sd->state.block_action & PCBLOCK_EMOTION)) {
		use_failure(*sd, pack, emotion, 2);
		return;
	}
	const auto now = time(nullptr);
	if (sd->emotionlasttime + 1 >= now)
		return;
	sd->emotionlasttime = now;
	if (battle_config.idletime_option & IDLE_EMOTION)
		sd->idletime = last_tick;
	if (battle_config.hom_idle_no_share && sd->hd && (battle_config.idletime_hom_option & IDLE_EMOTION))
		sd->idletime_hom = last_tick;
	if (battle_config.mer_idle_no_share && sd->md && (battle_config.idletime_mer_option & IDLE_EMOTION))
		sd->idletime_mer = last_tick;
	broadcast(*sd, pack, emotion);
}
#endif
