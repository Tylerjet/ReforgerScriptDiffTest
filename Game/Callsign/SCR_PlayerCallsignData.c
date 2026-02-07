/*!
Callsign Info. used for Company, Platoon and Squad. It currently holds the callsign name
*/
class SCR_PlayerCallsignData
{
	protected Faction m_Faction;
	protected int m_iCompanyIndex;
	protected int m_iPlatoonIndex;
	protected int m_iSquadIndex;
	protected int m_iCharacterIndex = 1;
	
	/*!
	Assigns a player callsign using faction settings
	\param faction of player
	\param callsignManager to get functions
	*/ 
	void SCR_PlayerCallsignData(Faction faction, SCR_CallsignManagerComponent callsignManager)
	{
		SetPlayerCallsign(faction, callsignManager);
	}
	
	/*!
	Assigns a player callsign using faction settings
	\param faction of player
	\param callsignManager to get functions
	*/ 
	void SetPlayerCallsign(Faction faction, SCR_CallsignManagerComponent callsignManager)
	{		
		MakePlayerCallsignAvailible(callsignManager);
		
		m_Faction = faction;
		callsignManager.AssignCallGroupsign(faction, m_iCompanyIndex, m_iPlatoonIndex, m_iSquadIndex); 
	}
	
	/*!
	Assigns a random player callsign no matter what the faction settings are
	\param faction of player
	\param callsignManager to get functions
	*/ 
	void RandomizeCallsign(Faction faction, SCR_CallsignManagerComponent callsignManager)
	{
		MakePlayerCallsignAvailible(callsignManager);
			
		m_Faction = faction;
		callsignManager.AssignRandomGroupCallsigns(faction, m_iCompanyIndex, m_iPlatoonIndex, m_iSquadIndex); 
	}
	
	/*!
	Make assigned callsign availible again
	\param callsignManager to get functions
	*/
	void MakePlayerCallsignAvailible(SCR_CallsignManagerComponent callsignManager)
	{
		if (m_Faction && m_iCompanyIndex >= 0)
			callsignManager.MakeGroupCallsignAvailible(m_Faction, m_iCompanyIndex, m_iPlatoonIndex, m_iSquadIndex);
	}
	
	/*!
	Returns player callsigns
	\param[out] companyIndex assigned
	\param[out] platoonIndex assigned
	\param[out] squadIndex assigned
	\param[out] characterIndex assigned
	\return returns false if callsign was not assigned
	*/ 
	bool GetPlayerCallsignIndexes(out int company, out int platoon, out int squad, out int character)
	{
		if (m_iCompanyIndex < 0)
			return false;
		
		company = m_iCompanyIndex;
		platoon = m_iPlatoonIndex;
		squad = m_iSquadIndex;
		character = m_iCharacterIndex;

		return true;
	}
	
	/*!
	Get faction of the player callsign
	\return Faction
	*/ 
	Faction GetFaction()
	{
		//On player spawn call this to see if faction changed
		return m_Faction;
	}
};