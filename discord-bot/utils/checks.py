import discord
import config
from discord import app_commands

def is_admin(interaction: discord.Interaction) -> bool:
    if not interaction.guild:
        return False
    
    user = interaction.user
    # Server owner or Administrator permission always passes
    if user.guild_permissions.administrator or user.id == interaction.guild.owner_id:
        return True
        
    if config.ADMIN_ROLE_ID:
        has_role = any(r.id == config.ADMIN_ROLE_ID for r in user.roles)
        if not has_role:
            # Debug log (optional: remove after fix)
            print(f"[DEBUG] User {user} (ID: {user.id}) does not have Admin Role {config.ADMIN_ROLE_ID}")
            print(f"[DEBUG] User Roles: {[r.id for r in user.roles]}")
        return has_role
        
    return False

def admin_check():
    """Check for Admin / Game Master rights"""
    async def predicate(interaction: discord.Interaction) -> bool:
        if is_admin(interaction):
            return True
        await interaction.response.send_message('❌ เฉพาะ **Game Master** หรือ **Admin** เท่านั้นที่ใช้คำสั่งนี้ได้', ephemeral=True)
        return False
    return app_commands.check(predicate)

def is_player(interaction: discord.Interaction) -> bool:
    if not interaction.guild:
        return True
    
    # If no role restricted in config, allow everyone
    if not config.PLAYER_ROLE_ID and not config.VERIFIED_ROLE_ID:
        return True
    
    user_role_ids = [r.id for r in interaction.user.roles]
    if config.PLAYER_ROLE_ID and config.PLAYER_ROLE_ID in user_role_ids:
        return True
    if config.VERIFIED_ROLE_ID and config.VERIFIED_ROLE_ID in user_role_ids:
        return True
    
    # Admins can always use player commands
    if is_admin(interaction):
        return True
        
    return False

def player_check():
    """Check for general player rights (e.g. Adventurer role)"""
    async def predicate(interaction: discord.Interaction) -> bool:
        if is_player(interaction):
            return True
        
        role_name = "นักผจญภัย"
        await interaction.response.send_message(f'❌ คุณต้องมีมียศ **{role_name}** ก่อนจึงจะใช้คำสั่งนี้ได้', ephemeral=True)
        return False
    return app_commands.check(predicate)
