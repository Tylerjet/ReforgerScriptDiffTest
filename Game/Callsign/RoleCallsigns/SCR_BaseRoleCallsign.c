[BaseContainerProps(), BaseContainerCustomStringTitleField("Do not use!")]
class SCR_BaseRoleCallsign
{
	[Attribute()]
	protected LocalizedString m_sRoleName;
	
	/*!
	Checks if character can be assigned a role (or a specific role if roleCallsignIndex is given a value)
	\param character AIAgent of character to assign callsign
	\param group SCR_AIGroup group of which character is a part of
	\param occupiedRoles roles already occupied for the group the character is part of
	\param[out] roleCallsignIndex the return index of the found character role. Can be given a value to look for a specific role to assign
	\return bool true if callsign found, else it returns false
	*/
	bool IsValidRole(AIAgent character, SCR_AIGroup group, map<int, AIAgent> occupiedRoles, out int roleCallsignIndex)
	{		
		return character
		&& (roleCallsignIndex < 0 || roleCallsignIndex == GetRoleIndex()) // If looking for specific role
		&& (!occupiedRoles || !occupiedRoles.Contains(GetRoleIndex())); // Check if role is already taken
	}

	
	/*!
	Get Role name
	\return string role localized name
	*/
	string GetRoleName()
	{
		return m_sRoleName;
	}
	
	/*!
	Get Role index
	\return int role index
	*/
	int GetRoleIndex()
	{
		return -1;
	}
};


