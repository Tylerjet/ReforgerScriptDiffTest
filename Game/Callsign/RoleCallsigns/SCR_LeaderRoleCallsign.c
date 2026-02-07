[BaseContainerProps(), BaseContainerCustomStringTitleField("Leader")]
class SCR_LeaderRoleCallsign: SCR_BaseRoleCallsign
{
	override bool IsValidRole(AIAgent character, SCR_AIGroup group, map<int, AIAgent> occupiedRoles, out int roleCallsignIndex)
	{
		if (!super.IsValidRole(character, group, occupiedRoles, roleCallsignIndex))
			return false;
		
		if (!group)
			return false;
		
		if (group.GetLeaderAgent() == character)
		{
			roleCallsignIndex = GetRoleIndex();
			return true;
		}
	
		return false;
	}
	
	override int GetRoleIndex()
	{
		return ERoleCallsign.SQUAD_LEADER;
	}
};