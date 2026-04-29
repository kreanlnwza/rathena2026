// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Atcommands
 * Place the body of custom atcommands in this file.
 * Format:
 *	ACMD_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//ACMD_FUNC(newcommand)
//{
//	clif_displaymessage(fd, "It works!");
//	clif_specialeffect(&sd->bl, EF_HEARTCASTING, AREA);
//	return 0;
//}

/**
 * @guildskillpoint - Add/remove guild skill points
 * Usage: @guildskillpoint <amount>
 * @author V!be Coding [kreanlnwza] AI Assistant (Antigravity)
 */
ACMD_FUNC(guildskillpoint)
{
	int32 point = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || (point = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please enter a number (usage: @guildskillpoint <number of points>).");
		return -1;
	}

	auto &guild_info = sd->guild;

	if (sd->status.guild_id <= 0 || guild_info == nullptr) {
		clif_displaymessage(fd, msg_txt(sd, 43)); // You're not in a guild.
		return -1;
	}

	int32 current = guild_info->guild.skill_point;
	int32 new_point = current + point;

	// Prevent negative skill points
	if (new_point < 0)
		new_point = 0;

	if (new_point == current) {
		if (point < 0)
			clif_displaymessage(fd, msg_txt(sd, 41)); // Unable to decrease the number/value.
		else
			clif_displaymessage(fd, msg_txt(sd, 149)); // Unable to increase the number/value.
		return -1;
	}

	intif_guild_change_basicinfo(guild_info->guild.guild_id, GBI_SKILLPOINT, &new_point, sizeof(new_point));

	sprintf(atcmd_output, "Guild skill points changed. (%d -> %d)", current, new_point);
	clif_displaymessage(fd, atcmd_output);
	return 0;
}

/**
 * @guildallskill - Learn all guild skills to max level
 * Usage: @guildallskill
 * @author V!be Coding [kreanlnwza] AI Assistant (Antigravity)
 */
ACMD_FUNC(guildallskill)
{
	nullpo_retr(-1, sd);

	if (sd->status.guild_id <= 0 || sd->guild == nullptr) {
		clif_displaymessage(fd, msg_txt(sd, 43)); // You're not in a guild.
		return -1;
	}

	uint16 max_levels[MAX_GUILDSKILL];

	for (int32 i = 0; i < MAX_GUILDSKILL; i++) {
		max_levels[i] = guild_skill_get_max(GD_SKILLBASE + i);
	}

	intif_guild_change_basicinfo(sd->guild->guild.guild_id, GBI_ALLSKILL, max_levels, sizeof(max_levels));

	clif_displaymessage(fd, "All guild skills have been maxed out.");
	return 0;
}

/**
 * Enchantgrade random-option helpers
 * @gradedown <inventory index>
 *     Downgrade the item's enchantgrade by one. Removes the option added
 *     at the current grade plus the 5th-option slot.
 * @addopt5 <inventory index> <random option group name>
 *     Roll the 5th random option from the given group into slot 4.
 *     Requires grade = A and slots 0..3 filled.
 * @rerollopt <inventory index> <slot> <random option group name>
 *     Reroll any single option slot in place using the given group.
 *
 * Author: V!be Coding [kreanlnwza] AI Assistant (Antigravity)
 **/
ACMD_FUNC(gradedown)
{
	char msg[CHAT_SIZE_MAX];
	int32 index = -1;

	if( !message || !*message || sscanf( message, "%d", &index ) < 1 || index < 0 ){
		clif_displaymessage( fd, "Usage: @gradedown <inventory index>" );
		return -1;
	}

	if( index >= MAX_INVENTORY ){
		safesnprintf( msg, sizeof( msg ), "@gradedown: Invalid inventory index %d (max %d).", index, MAX_INVENTORY - 1 );
		clif_displaymessage( fd, msg );
		return -1;
	}

	struct item& it = sd->inventory.u.items_inventory[index];

	if( it.nameid == 0 || it.amount < 1 ){
		clif_displaymessage( fd, "@gradedown: No item at that inventory index." );
		return -1;
	}

	if( it.enchantgrade <= ENCHANTGRADE_NONE ){
		clif_displaymessage( fd, "@gradedown: Item is already at grade None." );
		return -1;
	}

	uint8 old_grade = it.enchantgrade;
	size_t grade_slot = static_cast<size_t>( old_grade - 1 );

	log_pick_pc( sd, LOG_TYPE_COMMAND, -1, &it );
	clif_delitem( *sd, index, 1, 0 );

	it.option[grade_slot].id = 0;
	it.option[grade_slot].value = 0;
	it.option[grade_slot].param = 0;

	it.option[MAX_ITEM_RDM_OPT - 1].id = 0;
	it.option[MAX_ITEM_RDM_OPT - 1].value = 0;
	it.option[MAX_ITEM_RDM_OPT - 1].param = 0;

	it.enchantgrade = old_grade - 1;

	log_pick_pc( sd, LOG_TYPE_COMMAND, 1, &it );
	clif_additem( sd, index, 1, 0 );

	safesnprintf( msg, sizeof( msg ), "@gradedown: Item at index %d downgraded.", index );
	clif_displaymessage( fd, msg );
	return 0;
}

ACMD_FUNC(addopt5)
{
	char msg[CHAT_SIZE_MAX];
	char group_name[NAME_LENGTH];
	int32 index = -1;

	if( !message || !*message || sscanf( message, "%d %23s", &index, group_name ) < 2 || index < 0 ){
		clif_displaymessage( fd, "Usage: @addopt5 <inventory index> <random option group name>" );
		return -1;
	}

	if( index >= MAX_INVENTORY ){
		safesnprintf( msg, sizeof( msg ), "@addopt5: Invalid inventory index %d.", index );
		clif_displaymessage( fd, msg );
		return -1;
	}

	struct item& it = sd->inventory.u.items_inventory[index];

	if( it.nameid == 0 || it.amount < 1 ){
		clif_displaymessage( fd, "@addopt5: No item at that inventory index." );
		return -1;
	}

	if( it.enchantgrade != ENCHANTGRADE_A ){
		clif_displaymessage( fd, "@addopt5: Item must be at grade A." );
		return -1;
	}

	for( size_t i = 0; i < MAX_ITEM_RDM_OPT - 1; i++ ){
		if( it.option[i].id == 0 ){
			clif_displaymessage( fd, "@addopt5: Slots 0..3 must all be filled first." );
			return -1;
		}
	}
	if( it.option[MAX_ITEM_RDM_OPT - 1].id != 0 ){
		clif_displaymessage( fd, "@addopt5: Slot 4 already has an option. Use @rerollopt to overwrite." );
		return -1;
	}

	uint16 group_id;
	if( !random_option_group.option_get_id( group_name, group_id ) ){
		safesnprintf( msg, sizeof( msg ), "@addopt5: Unknown random option group \"%s\".", group_name );
		clif_displaymessage( fd, msg );
		return -1;
	}

	std::shared_ptr<s_random_opt_group> group = random_option_group.find( group_id );
	if( group == nullptr ){
		clif_displaymessage( fd, "@addopt5: Random option group lookup failed." );
		return -1;
	}

	log_pick_pc( sd, LOG_TYPE_COMMAND, -1, &it );
	clif_delitem( *sd, index, 1, 0 );

	bool ok = enchantgrade_reroll_option_slot( it, *group, MAX_ITEM_RDM_OPT - 1 );

	log_pick_pc( sd, LOG_TYPE_COMMAND, 1, &it );
	clif_additem( sd, index, 1, 0 );

	clif_displaymessage( fd, ok ? "@addopt5: Slot 4 rolled." : "@addopt5: Roll failed." );
	return ok ? 0 : -1;
}

ACMD_FUNC(rerollopt)
{
	char msg[CHAT_SIZE_MAX];
	char group_name[NAME_LENGTH];
	int32 index = -1;
	int32 slot = -1;

	if( !message || !*message || sscanf( message, "%d %d %23s", &index, &slot, group_name ) < 3 || index < 0 || slot < 0 ){
		clif_displaymessage( fd, "Usage: @rerollopt <inventory index> <slot> <random option group name>" );
		return -1;
	}

	if( index >= MAX_INVENTORY ){
		safesnprintf( msg, sizeof( msg ), "@rerollopt: Invalid inventory index %d.", index );
		clif_displaymessage( fd, msg );
		return -1;
	}

	if( slot >= MAX_ITEM_RDM_OPT ){
		safesnprintf( msg, sizeof( msg ), "@rerollopt: Invalid slot %d (max %d).", slot, MAX_ITEM_RDM_OPT - 1 );
		clif_displaymessage( fd, msg );
		return -1;
	}

	struct item& it = sd->inventory.u.items_inventory[index];

	if( it.nameid == 0 || it.amount < 1 ){
		clif_displaymessage( fd, "@rerollopt: No item at that inventory index." );
		return -1;
	}

	uint16 group_id;
	if( !random_option_group.option_get_id( group_name, group_id ) ){
		safesnprintf( msg, sizeof( msg ), "@rerollopt: Unknown random option group \"%s\".", group_name );
		clif_displaymessage( fd, msg );
		return -1;
	}

	std::shared_ptr<s_random_opt_group> group = random_option_group.find( group_id );
	if( group == nullptr ){
		clif_displaymessage( fd, "@rerollopt: Random option group lookup failed." );
		return -1;
	}

	log_pick_pc( sd, LOG_TYPE_COMMAND, -1, &it );
	clif_delitem( *sd, index, 1, 0 );

	bool ok = enchantgrade_reroll_option_slot( it, *group, static_cast<size_t>( slot ) );

	log_pick_pc( sd, LOG_TYPE_COMMAND, 1, &it );
	clif_additem( sd, index, 1, 0 );

	clif_displaymessage( fd, ok ? "@rerollopt: Slot rerolled." : "@rerollopt: Reroll failed." );
	return ok ? 0 : -1;
}
