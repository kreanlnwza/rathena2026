// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "cash_emotion.hpp"
#include <algorithm>
#include <charconv>
#include <common/showmsg.hpp>
#include "itemdb.hpp"
#include "map.hpp"
#include "script.hpp"

// Author: V!be Coding [kreanlnwza] AI Assistant (Codex)
CashEmotionDatabase cash_emotion_db;

const std::string CashEmotionDatabase::getDefaultLocation() {
	return std::string(db_path) + "/cash_emotion_db.yml";
}

bool CashEmotionDatabase::load() {
	return this->reload();
}

bool CashEmotionDatabase::reload() {
	CashEmotionDatabase replacement;
	if (!replacement.YamlDatabase::load() || replacement.invalid_rows) {
		ShowWarning("Cash Emoji database reload rejected; previous catalog retained.\n");
		return false;
	}
	this->data.swap(replacement.data);
	return true;
}

uint64 CashEmotionDatabase::parseBodyNode(const ryml::NodeRef& node) {
	const uint64 result = this->parsePack(node);
	if (result == 0)
		this->invalid_rows = true;
	return result;
}

uint64 CashEmotionDatabase::parsePack(const ryml::NodeRef& node) {
	if (!node.is_map()) {
		this->invalidWarning(node, "Cash Emoji entry must be a mapping.\n");
		return 0;
	}
	for (const std::string field : {"Id", "Currency", "Price", "Registry"}) {
		if (this->nodeExists(node, field)) {
			const auto& value = node[c4::to_csubstr(field)];
			if (!value.has_val() || value.val_is_null()) {
				this->invalidWarning(node, "Cash Emoji %s must be a scalar value.\n", field.c_str());
				return 0;
			}
		}
	}
	uint32 id;
	if (!this->asUInt32(node, "Id", id))
		return 0;
	if (id == 0 || id > UINT16_MAX) {
		this->invalidWarning(node, "Cash Emoji Id must be 1..65535.\n");
		return 0;
	}
	const auto existing = this->find(static_cast<uint16>(id));
	if (existing == nullptr && this->size() >= 1000) {
		this->invalidWarning(node, "Cash Emoji catalog is limited to 1000 packs.\n");
		return 0;
	}
	if (existing == nullptr && !this->nodesExist(node, {"Price", "Registry", "Emotions"}))
		return 0;
	// Imports override individual fields. Invalid replacements cannot mutate
	// existing entries, and any invalid row rejects the entire staged reload.
	auto pack = existing ? std::make_shared<s_cash_emotion_pack>(*existing) : std::make_shared<s_cash_emotion_pack>();
	pack->id = static_cast<uint16>(id);
	if (!existing)
		pack->currency = ITEMID_NYANGVINE_FRUIT;
	if (this->nodeExists(node, "Currency")) {
		std::string name;
		if (!this->asString(node, "Currency", name))
			return 0;
		uint32 currency = 0;
		const auto parsed = std::from_chars(name.data(), name.data() + name.size(), currency);
		// Keep numeric IDs compatible with existing import overrides.
		if (parsed.ec != std::errc() || parsed.ptr != name.data() + name.size()) {
			const auto item = item_db.search_aegisname(name.c_str());
			if (!item) {
				this->invalidWarning(node, "Unknown Cash Emoji Currency AegisName '%s'.\n", name.c_str());
				return 0;
			}
			currency = item->nameid;
		}
		if (currency == 0 || currency > UINT16_MAX) {
			this->invalidWarning(node, "Cash Emoji Currency must be an item ID in 1..65535.\n");
			return 0;
		}
		pack->currency = static_cast<uint16>(currency);
	}
	if (!item_db.exists(pack->currency)) {
		this->invalidWarning(node, "Cash Emoji Currency item %u does not exist.\n", pack->currency);
		return 0;
	}
	if (this->nodeExists(node, "Price") && !this->asInt32(node, "Price", pack->price))
		return 0;
	if (pack->price < 1 || pack->price > UINT8_MAX) {
		this->invalidWarning(node, "Cash Emoji Price must be 1..255 (client quote is one byte).\n");
		return 0;
	}
	if (this->nodeExists(node, "Registry") && !this->asString(node, "Registry", pack->registry))
		return 0;
	const auto identifier = [](char c) {
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
	};
	if (pack->registry.size() < 2 || pack->registry.size() > 32 || pack->registry[0] != '#' ||
		!std::all_of(pack->registry.begin() + 1, pack->registry.end(), identifier)) {
		this->invalidWarning(node, "Cash Emoji Registry must be a single-# numeric account variable, 2..32 characters.\n");
		return 0;
	}
	for (const auto& entry : *this) {
		if (entry.first != pack->id && entry.second->registry == pack->registry) {
			this->invalidWarning(node, "Cash Emoji Registry '%s' is already assigned to pack %u.\n", pack->registry.c_str(), entry.first);
			return 0;
		}
	}
	if (this->nodeExists(node, "Emotions")) {
		const auto& emotions = node["Emotions"];
		if (!emotions.is_seq() || emotions.num_children() == 0 || emotions.num_children() > 1000) {
			this->invalidWarning(emotions, "Cash Emoji Emotions must be a nonempty list of at most 1000 IDs.\n");
			return 0;
		}
		pack->emotions.clear();
		for (const auto& emotion : emotions) {
			int64 value = 0;
			if (!emotion.has_val() || emotion.val_is_null() || emotion.val().len == 0) {
				this->invalidWarning(emotion, "Cash Emoji emotion must be an ET_* constant or integer ID.\n");
				return 0;
			}
			const auto text = emotion.val();
			if (text.len > 3 && text.str[0] == 'E' && text.str[1] == 'T' && text.str[2] == '_') {
				const std::string name(text.str, text.len);
				if (!script_get_constant(name.c_str(), &value)) {
					this->invalidWarning(emotion, "Unknown Cash Emoji constant '%s'.\n", name.c_str());
					return 0;
				}
			} else {
				const auto parsed = std::from_chars(text.str, text.str + text.len, value);
				if (parsed.ec != std::errc() || parsed.ptr != text.str + text.len) {
					this->invalidWarning(emotion, "Cash Emoji emotion must be a known ET_* constant or integer ID.\n");
					return 0;
				}
			}
			if (value < 0 || value > UINT16_MAX || value == 34) {
				this->invalidWarning(emotion, "Cash Emoji emotion must be 0..65535, excluding reserved mute ID 34.\n");
				return 0;
			}
			if (std::find(pack->emotions.begin(), pack->emotions.end(), value) != pack->emotions.end()) {
				this->invalidWarning(emotion, "Duplicate Cash Emoji emotion ID %u.\n", static_cast<uint32>(value));
				return 0;
			}
			pack->emotions.push_back(static_cast<uint16>(value));
		}
	}
	this->put(pack->id, pack);
	return 1;
}
