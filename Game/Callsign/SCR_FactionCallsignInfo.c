/*!
Stores the Callsign information data for the faction.
*/
[BaseContainerProps()]
class SCR_FactionCallsignInfo
{
	[Attribute(defvalue: "1", desc: "If true then callsigns will be assigned at random rather then trying to keep callsigns close")]
	protected bool m_bIsAssignedRandomly;
	
	[Attribute()]
	protected ref array<ref SCR_CallsignInfo> m_aCompanyNames;
	
	[Attribute(defvalue: "4", desc: "Index of overflow companies. Starting with the given index, these companies will only be assigned if all company indexes above are taken")]
	protected int m_iCompanyOverflowIndex;
	
	[Attribute()]
	protected ref array<ref SCR_CallsignInfo> m_aPlatoonNames;
	
	[Attribute()]
	protected ref array<ref SCR_CallsignInfo> m_aSquadNames;
	
	[Attribute(desc: "%1 = Company, %2 = Platoon, %3 = Squad")]
	protected LocalizedString m_sCallsignGroupFormat;
	
	[Attribute(desc: "%1 = Company, %2 = Platoon, %3 = Squad, %4 = Character")]
	protected LocalizedString m_sCallsignCharacterFormat;

	[Attribute(desc: "A character can have one role at the time and a group can only have one of each role. Note that roles are assigned in order, so make sure important roles (Such as leader) are on top of the list.")]
	protected ref array<ref SCR_BaseRoleCallsign> m_aCharacterRoleCallsigns;
	
	//[Attribute(defvalue: "%4", desc: "%1 = Company, %2 = Platoon, %3 = Squad, %4 = Character")]
	//protected LocalizedString m_sCallsignCharacterFormat;
	

	/*!
	Get callsign format
	\return string callsign Format
	*/
	string GetCallsignFormat(bool includeCharacter)
	{
		if (!includeCharacter)
			return m_sCallsignGroupFormat;
		else
			return m_sCallsignCharacterFormat;
	}
	
	/*!
	Get callsign format
	\return bool if callsigns are assigned in order or randomly
	*/
	bool GetIsAssignedRandomly()
	{
		return m_bIsAssignedRandomly;
	}
	
	/*!
	Get Company array
	\return array<ref SCR_CallsignInfo> companyArray
	*/
	void GetCompanyArray(notnull array<ref SCR_CallsignInfo> companyArray)
	{
		companyArray.Clear();
		
		foreach(SCR_CallsignInfo info: m_aCompanyNames)
			companyArray.Insert(info);
	}
	
	/*!
	Get Platoon array
	\return array<ref SCR_CallsignInfo> platoonArray
	*/
	void GetPlatoonArray(notnull array<ref SCR_CallsignInfo> platoonArray)
	{
		platoonArray.Clear();
		
		foreach(SCR_CallsignInfo info: m_aPlatoonNames)
			platoonArray.Insert(info);
	}
	
	/*!
	Get Squad array
	\return array<ref SCR_CallsignInfo> squadArray
	*/
	void GetSquadArray(notnull array<ref SCR_CallsignInfo> squadArray)
	{
		squadArray.Clear();
		
		foreach(SCR_CallsignInfo info: m_aSquadNames)
			squadArray.Insert(info);
	}
	
	/*!
	Get the company name using index
	\param index of company name
	\return string company name
	*/
	string GetCompanyCallsignName(int index)
	{
		if (index < 0 || index >= m_aCompanyNames.Count())
			return index.ToString();
		
		return m_aCompanyNames[index].GetCallsign();
	}
	
	/*!
	Gets the index at which overflow companies start
	\return int m_iCompanyOverflowIndex
	*/
	int GetCompanyOverflowIndex()
	{		
		return m_iCompanyOverflowIndex;
	}
	
	/*!
	Get the Platoon name using index
	\param index of Platoon name
	\return string Platoon name
	*/
	string GetPlatoonCallsignName(int index)
	{
		if (index < 0 || index >= m_aPlatoonNames.Count())
			return index.ToString();
		
		return m_aPlatoonNames[index].GetCallsign();
	}
	
	/*!
	Get the Squad name using index
	\param index of Squad name
	\return string Squad name
	*/
	string GetSquadCallsignName(int index)
	{
		if (index < 0 || index >= m_aSquadNames.Count())
			return index.ToString();
		
		return m_aSquadNames[index].GetCallsign();
	}
	
	/*!
	Get the character role callsign using index
	\param index of character role
	\return string character role callsign name
	*/
	string GetCharacterRoleCallsignName(int index)
	{
		if (!m_aCharacterRoleCallsigns.IsEmpty())
		{
			foreach(SCR_BaseRoleCallsign callsign: m_aCharacterRoleCallsigns)
			{
				if (callsign.GetRoleIndex() == index)
					return callsign.GetRoleName();
			}
		}
		
		Print(string.Format("Given Role index: '%1' does not exist in role data!", index.ToString()), LogLevel.ERROR);
		return index.ToString();
	}
	
	/*!
	Get random call sign
	\param[out] company random company name index
	\param[out] platoon random platoon name index
	\param[out] squad random squad name index
	*/
	bool GetRandomCallsign(out int company, out int platoon, out int squad)
	{	
		company = -1;
		platoon = -1;
		squad = -1;
		
		if (!m_aCompanyNames || m_aCompanyNames.Count() == 0 || !m_aPlatoonNames || m_aPlatoonNames.Count() == 0 || !m_aSquadNames || m_aSquadNames.Count() == 0)
			return false;
		
		//TODO: Make sure the same callsigns are never assigned twice
		company = Math.RandomInt(0, m_aCompanyNames.Count());
		platoon = Math.RandomInt(0, m_aPlatoonNames.Count());
		squad = Math.RandomInt(0, m_aSquadNames.Count());
		
		return true;
	}
	
	/*!
	Loops through all availible roles for the faction and grabs the first availible role for that character
	\param character AIAgent of character to assign callsign
	\param group SCR_AIGroup group of which character is a part of
	\param occupiedRoles roles already occupied for the group the character is part of
	\param[out] roleCallsignIndex the return index of the found character role. Can be given a value to look for a specific role to assign
	\return bool true if callsign found, else it returns false
	*/
	bool GetCharacterRoleCallsign(AIAgent character, SCR_AIGroup group, notnull map<int, AIAgent> occupiedRoles, out int roleCallsignIndex)
	{
		foreach (SCR_BaseRoleCallsign roleCallsign: m_aCharacterRoleCallsigns)
		{
			if (roleCallsign.IsValidRole(character, group, occupiedRoles, roleCallsignIndex))
				return true;
		}
	
		return false;
	}

	
	/*!
	Loops through all availible roles for the faction and grabs the first availible role for a GROUPLESS character
	\param character AIAgent of character to assign callsign
	\param[out] roleCallsignIndex the return index of the found character role. Can be given a value to look for a specific role to assign
	\return bool true if callsign found, else it returns false
	*/
	bool GetGrouplessCharacterRoleCallsign(AIAgent character, out int roleCallsignIndex)
	{
		foreach (SCR_BaseRoleCallsign roleCallsign: m_aCharacterRoleCallsigns)
		{
			if (roleCallsign.IsValidRole(character, null, null, roleCallsignIndex))
				return true;
		}
	
		return false;
	}
	
	/*!
	Loops through all availible roles for the faction and grabs the first availible role for a GROUPLESS character
	\param character IEntity, will get character AIAgent
	\param[out] roleCallsignIndex the return index of the found character role. Can be given a value to look for a specific role to assign
	\return bool true if callsign found, else it returns false
	*/
	bool GetGrouplessCharacterRoleCallsign(IEntity character, out int roleCallsignIndex)
	{
		AIControlComponent cc = AIControlComponent.Cast(character.FindComponent(AIControlComponent));
		AIAgent aIAgent;
		if (cc)
			aIAgent = cc.GetAIAgent();
		if (aIAgent)
			return GetGrouplessCharacterRoleCallsign(aIAgent, roleCallsignIndex);
		else
			return false;
	}
};