// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

class SkillElementalIntegration : public StatusSkillImpl {
public:
	SkillElementalIntegration();
};

class SkillElementalIntegrationAttack : public SkillImplRecursiveDamageSplash {
public:
	SkillElementalIntegrationAttack(e_skill skill_id, int32 elemental_id);

	void calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const override;

private:
	int32 elemental_id_;
};
