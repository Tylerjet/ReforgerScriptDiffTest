// Script File[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class SCR_CallsignCharacterComponentClass: SCR_CallsignBaseComponentClass
{
};

/*!
Component of assigning and storing squad names
*/
class SCR_CallsignCharacterComponent : SCR_CallsignBaseComponent
{
	//Server only
	protected bool m_bIsInGroup = true;
	protected bool m_bIsPlayer;
	
	//Broadcast
	protected int m_iCharacterCallsign = -1;
	protected int m_iRoleCallsign = -1;
	protected bool m_bAloneInGroup = false;
	protected int m_iFactioniD = -1;

	
	//======================================== GET CALLSIGN NAMES ========================================\\
	override bool GetCallsignNames(out string company, out string platoon, out string squad, out string character, out string format)
	{					
		int companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex, characterRoleCallsignIndex;
		character = string.Empty;
		
		if (!GetCallsignIndexes(companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex, characterRoleCallsignIndex))
			return false;
		
		//Get character callsign if not alone in group, or has a role and that role is not leader (Can't really be a leader of yourself)
		bool showCharacterCallsign = !GetIsCharacterAloneInGroup();
		
		//No callsign data
		if (!SetCallsignInfo())
		{
			company = companyCallsignIndex.ToString();
			platoon = platoonCallsignIndex.ToString();
			squad = squadCallsignIndex.ToString();
			character = characterCallsignIndex.ToString();
			format = "ERROR %1-%2-%3-%4";
			
			return true;
		}
		
		company = m_CallsignInfo.GetCompanyCallsignName(companyCallsignIndex);
		platoon = m_CallsignInfo.GetPlatoonCallsignName(platoonCallsignIndex);
		squad = m_CallsignInfo.GetSquadCallsignName(squadCallsignIndex);
		
		//Character callsign
		if (showCharacterCallsign)
		{	
			if (characterRoleCallsignIndex >= 0)
				character = m_CallsignInfo.GetCharacterRoleCallsignName(characterRoleCallsignIndex);
			else 
				character = characterCallsignIndex.ToString();
		}
		
		format = m_CallsignInfo.GetCallsignFormat(showCharacterCallsign);		
		return true;
	}
	
	//======================================== GET CALLSIGN INDEXES ========================================\\
	override bool GetCallsignIndexes(out int company, out int platoon, out int squad, out int character = -1, out int characterRole = -1)
	{
		if (m_iCompanyCallsign < 0 || m_iPlatoonCallsign < 0 || m_iSquadCallsign < 0)
		{
			company = -1;
			platoon = -1;
			squad = -1;
			return false;
		}	
		
		company = m_iCompanyCallsign;
		platoon = m_iPlatoonCallsign;
		squad = m_iSquadCallsign;
		
		GetCharacterAndRoleCallsign(character, characterRole);	
		return true;
	}
	
	
	//======================================== GROUP CALLSIGNS ========================================\\
	//---------------------------------------- Assign Character Group Callsign ----------------------------------------\\
	/*!
	Called by group, assigns a specific character group index and role (if any assigned) to character within specific group
	\param callsignIndex character group index
	\param roleCallsignIndex specific character role index
	\param callsignGroupComponent character group reference
	*/
	void AssignCharacterCallsign(Faction faction, int company, int platoon, int squad, int character, int role, bool isInGroup, bool aloneInGroup)
	{
		if (!m_bIsServer)
			return;
		
		//Make sure that if character wasn't in a group that the group callsign will be made availible again
		MakeGroupCallsignAvailible();
			
		//On Faction changed
		if (m_Faction != faction && faction != null)
		{
			FactionManager factionManager = GetGame().GetFactionManager();
			if (factionManager)
			{
				int factionId = factionManager.GetFactionIndex(faction);
				m_Faction = faction;
				Rpc(SetFactionBroadCast, factionId);	
			}
		}
		
		//Update in group. Used to make callsigns availible again on server if not in group
		m_bIsInGroup = isInGroup;
		
		if (company == m_iCompanyCallsign && m_iPlatoonCallsign == platoon && m_iSquadCallsign == squad && m_iCharacterCallsign == character && m_iRoleCallsign == role && m_bAloneInGroup == aloneInGroup) 
			return;
		
		//If group did not change
		if (company == m_iCompanyCallsign && m_iPlatoonCallsign == platoon && m_iSquadCallsign == squad && m_bAloneInGroup == aloneInGroup)
		{
			//If role did not change
			if (m_iRoleCallsign == role)
			{
				AssignCharacterCallsignBroadcast(character);
				Rpc(AssignCharacterCallsignBroadcast, character);	
			}
			//If role did change
			else
			{
				AssignCharacterAndRoleCallsignBroadcast(character, role);
				Rpc(AssignCharacterAndRoleCallsignBroadcast, character, role);	
			}
		}
		//If group did change but not role
		else if (m_iRoleCallsign == role)
		{
			AssignCallsignNoRoleBroadcast(company, platoon, squad, character, aloneInGroup);
			Rpc(AssignCallsignNoRoleBroadcast, company, platoon, squad, character, aloneInGroup);	
		}
		//If role and group changed
		else 
		{
			AssignCallsignBroadcast(company, platoon, squad, character, role, aloneInGroup);
			Rpc(AssignCallsignBroadcast, company, platoon, squad, character, role, aloneInGroup);	
		}
	}
	
	//---------------------------------------- Update role Callsign ----------------------------------------\\
	/*!
	Updates the assigned role
	\param roleCallsignIndex role index
	*/
	void UpdateCharacterRoleCallsign(int roleCallsignIndex)
	{
		if (!m_bIsServer)
			return;
		
		//If role did not change
		if (m_iRoleCallsign == roleCallsignIndex) 
			return;
		
		AssignRoleCallsignBroadcast(roleCallsignIndex);
		Rpc(AssignRoleCallsignBroadcast, roleCallsignIndex);
	}	
	
	/*!
	Updates the assigned callsign and role. Called when leader role is assigned as leader is always 1
	\param characterCallsign character callsign
	\param roleCallsignIndex role index
	*/
	void UpdateCharacterCallsignAndRole(int characterCallsign, int roleCallsignIndex)
	{
		if (!m_bIsServer)
			return;
		
		//If callsigns did not change
		if (m_iCharacterCallsign == characterCallsign && m_iRoleCallsign == roleCallsignIndex) 
			return;
		
		AssignCharacterAndRoleCallsignBroadcast(characterCallsign, roleCallsignIndex);
		Rpc(AssignCharacterAndRoleCallsignBroadcast, characterCallsign, roleCallsignIndex);
	}
	
	//---------------------------------------- Clear character and role Callsign ----------------------------------------\\
	/*!
	Called on server only before assigning new callsign to make sure everything is clear. Does not broadcast cleared values to players!
	*/
	override void ClearCallsigns()
	{
		super.ClearCallsigns();
		m_iCharacterCallsign = -1;
		m_iRoleCallsign = -1;
	}
	
	/*!
	Called on server only before assigning new role callsign to make sure everything is clear. Is not broadcast!
	*/
	void ClearCharacterRoleCallsign()
	{
		if (!m_bIsServer)
			return;
		
		m_iRoleCallsign = -1;
	}
	
	//======================================== CHARACTER HOLDS OWN GROUP CALLSIGN ========================================\\
	//---------------------------------------- Assign Both Character and Group Callsign ----------------------------------------\\
	protected void AssignGroupAndCharacterCallsign()
	{
		if (!m_bIsServer)
			return;
		
		if (!m_CallsignManager)
			return;
		
		int company, platoon, squad;
		m_CallsignManager.AssignCallGroupsign(m_Faction, company, platoon, squad);
		
		int roleIndex = -1;
		if (!SetCallsignInfo())
			return;

		//Assign role to groupless character
		m_CallsignInfo.GetGrouplessCharacterRoleCallsign(GetOwner(), roleIndex);
		AssignCharacterCallsign(m_Faction, company, platoon, squad, 1, roleIndex, false, true);
	}
	
	//======================================== PLAYER CALLSIGN ========================================\\
	//---------------------------------------- Assign Player Callsign ----------------------------------------\\
	//Get callsign from CallsignManager
	protected void AssignPlayerCallsign(int playerId)
	{
		m_bIsPlayer = true;
		int company, platoon, squad, character;
		
		m_CallsignManager.SystemGetPlayerCallsignIndexes(playerId, company, platoon, squad, character);
		
		int roleIndex = -1;
		if (!SetCallsignInfo())
			return;
		
		m_CallsignInfo.GetGrouplessCharacterRoleCallsign(GetOwner(), roleIndex);
		AssignCharacterCallsign(m_Faction, company, platoon, squad, character, roleIndex, false, true);
	}
	
	//======================================== BROAD CASTS ========================================\\
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void AssignCallsignBroadcast(int company, int platoon, int squad, int character, int role, bool aloneInGroup)
	{	
		m_iCompanyCallsign = company;
		m_iPlatoonCallsign = platoon;
		m_iSquadCallsign = squad;
		m_iCharacterCallsign = character;
		m_iRoleCallsign = role;
		m_bAloneInGroup = aloneInGroup;
		
		Event_OnCallsignChanged.Invoke(m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, m_iCharacterCallsign, m_iRoleCallsign);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void AssignCallsignNoRoleBroadcast(int company, int platoon, int squad, int character, bool aloneInGroup)
	{	
		m_iCompanyCallsign = company;
		m_iPlatoonCallsign = platoon;
		m_iSquadCallsign = squad;
		m_iCharacterCallsign = character;
		m_bAloneInGroup = aloneInGroup;
		
		Event_OnCallsignChanged.Invoke(m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, m_iCharacterCallsign, m_iRoleCallsign);
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void AssignCharacterAndRoleCallsignBroadcast(int character, int role)
	{	
		m_iCharacterCallsign = character;
		m_iRoleCallsign = role;
		
		Event_OnCallsignChanged.Invoke(m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, m_iCharacterCallsign, m_iRoleCallsign);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void AssignCharacterCallsignBroadcast(int character)
	{	
		m_iCharacterCallsign = character;
		
		Event_OnCallsignChanged.Invoke(m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, m_iCharacterCallsign, m_iRoleCallsign);
	}
		
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void AssignRoleCallsignBroadcast(int role)
	{	
		m_iRoleCallsign = role;
		
		Event_OnCallsignChanged.Invoke(m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, m_iCharacterCallsign, m_iRoleCallsign);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetAloneInGroupBroadcast(bool isAloneInGroup)
	{	
		m_bAloneInGroup = isAloneInGroup;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetFactionBroadCast(int factionID)
	{	
		m_iFactioniD = factionID;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Faction faction = factionManager.GetFactionByIndex(factionID);
		
		if (faction != m_Faction && faction != null)
		{
			m_Faction = faction;
			m_CallsignInfo = null;
		}	
	}
	
	
	
	//======================================== GETTERS ========================================\\
	/*!
	Returns if character is alone in group. ALso returns true if not in a group at all
	\return bool m_bAloneInGroup
	*/
	bool GetIsCharacterAloneInGroup()
	{
		return m_bAloneInGroup;	
	}
	
	//---------------------------------------- Assign new callsign on faction Changed ----------------------------------------\\
	//Todo: Currently not possible for individual characters
	protected void CharacterChangedFaction(Faction newFaction){
		MakeGroupCallsignAvailible(newFaction != null);

		if (!m_bIsInGroup && newFaction != null)
			AssignGroupAndCharacterCallsign();
	}
	
	//---------------------------------------- Make assigned group callsign availible again ----------------------------------------\\
	protected void MakeGroupCallsignAvailible(bool clearCallsign = true)
	{
		//Only make availible on server, if not part of group, if faction is assigned and if company callsign is assigned
		if (!m_bIsServer || m_bIsInGroup || !m_Faction || m_iCompanyCallsign < 0 || !m_CallsignManager)
			return;
		
		m_CallsignManager.MakeGroupCallsignAvailible(m_Faction, m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign);
		
		if (clearCallsign)
			m_iCompanyCallsign = -1;
	}
	
	//======================================== CHARACTER SPECIFIC CALLSIGN INDEXES ========================================\\
	//---------------------------------------- Get Character callsign ----------------------------------------\\
	/*
	Returns the character index (ignoring the role)
	\return int character group index
	*/
	int GetCharacterCallsignIndex()
	{
		return m_iCharacterCallsign;
	}
	
	//---------------------------------------- Get Character or role callsign ----------------------------------------\\
	/*
	Returns either the character index or, if a role is assigned, it will return the character role index instead
	\return int character group index or role index
	*/
	int GetCharacterOrRoleCallsignIndex()
	{
		if (m_iRoleCallsign >= 0)
			return m_iRoleCallsign;
		else
			return m_iCharacterCallsign;
	}

	//---------------------------------------- Get Character and role callsigns ----------------------------------------\\
	/*
	Returns both character index and character role index
	\param[out] character group index
	\param[out] role index
	*/
	void GetCharacterAndRoleCallsign(out int character, out int role)
	{
		character = m_iCharacterCallsign;
		role = m_iRoleCallsign;
	}
	
	//======================================== GET CHARACTER CALLSIGN NAME ========================================\\
	/*
	Returns the character callsign name, this either the character group index or the specific role name
	\param[out] characterCallsignName the character Callsign name in string
	\return bool if name was succesfully found
	*/
	bool GetCharacterCallsignName(out string characterCallsignName)
	{
		if(m_iCharacterCallsign == -1)
			return false;

		if (m_iRoleCallsign >= 0)
			characterCallsignName = m_CallsignInfo.GetCharacterRoleCallsignName(m_iRoleCallsign);
		else 
			m_iCharacterCallsign.ToString();

		return true;
	}	
	
	//======================================== INIT ========================================\\
	override void InitOnServer(IEntity owner)
	{
		// Always verify the pointer. The object could be deleted by the time it gets here.
		if (owner == null)
			return;
		
		super.InitOnServer(owner);
		
		AIAgent aIAgent;
		AIControlComponent cc = AIControlComponent.Cast(owner.FindComponent(AIControlComponent));
		if (cc)
			aIAgent = cc.GetAIAgent();
		
		if (aIAgent)
		{
			SCR_AIGroup aiGroup = SCR_AIGroup.Cast(aIAgent.GetParentGroup());
			
			//If has group then let group assign callsign
			if (aiGroup)							
				return;
		}
		
		//Not part of a group check if player
		int id = SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(GetOwner());
		
		if (id > -1)
		{
			AssignPlayerCallsign(id);
			return;
		}
		
		//Is not a player and not part of a group so should assign full callsign
		AssignGroupAndCharacterCallsign();
	}
	
	//======================================== RPL ========================================\\
	override bool RplSave(ScriptBitWriter writer)
    {	
        writer.WriteInt(m_iCompanyCallsign); 
        writer.WriteInt(m_iPlatoonCallsign);
		writer.WriteInt(m_iSquadCallsign);
		writer.WriteInt(m_iCharacterCallsign);
		writer.WriteInt(m_iRoleCallsign);
		writer.WriteInt(m_bAloneInGroup);
		writer.WriteInt(m_iFactioniD);
		
        return true;
    }
     
    override bool RplLoad(ScriptBitReader reader)
    {
		int company, platoon, squad, character, role, aloneInGroup, factionID;
		
        reader.ReadInt(company);
        reader.ReadInt(platoon);
		reader.ReadInt(squad);
        reader.ReadInt(character);
		reader.ReadInt(role);
		reader.ReadInt(aloneInGroup);
		reader.ReadInt(factionID);
		
		AssignCallsignBroadcast(company, platoon, squad, character, role, aloneInGroup);
		SetFactionBroadCast(factionID);
		
        return true;
    }
	
	//======================================== ON DESTROY ========================================\\
	void ~SCR_CallsignCharacterComponent()
	{
		if (!m_bIsServer || m_bIsInGroup || m_bIsPlayer || m_iCompanyCallsign < 0 || !m_Faction || !m_CallsignManager)
			return;
		
		m_CallsignManager.MakeGroupCallsignAvailible(m_Faction, m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign);
	}
};

