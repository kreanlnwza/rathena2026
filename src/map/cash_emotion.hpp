// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef CASH_EMOTION_HPP
#define CASH_EMOTION_HPP
#include <common/database.hpp>

/** Cash Emoji catalog. Author: V!be Coding [kreanlnwza] AI Assistant (Codex) */
struct s_cash_emotion_pack {
	uint16 id = 0;
	uint16 currency = 0; // Resolved from the Currency AegisName while loading.
	int32 price = 0;
	std::string registry;
	std::vector<uint16> emotions;
};

class CashEmotionDatabase : public TypesafeYamlDatabase<uint16, s_cash_emotion_pack> {
private:
	bool invalid_rows = false;
	uint64 parsePack(const ryml::NodeRef& node);
	void onLoadFailure() override { this->invalid_rows = true; }
public:
	CashEmotionDatabase() : TypesafeYamlDatabase("CASH_EMOTION_DB", 1) {}
	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
	// Stage and validate a replacement before touching the active catalog.
	bool load();
	bool reload();
};
extern CashEmotionDatabase cash_emotion_db;
#endif // CASH_EMOTION_HPP
