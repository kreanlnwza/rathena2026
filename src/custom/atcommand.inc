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
