[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class SCR_CallsignGroupComponentClass: SCR_CallsignBaseComponentClass
{
};

/*!
Component of assigning and storing squad names
*/
class SCR_CallsignGroupComponent : SCR_CallsignBaseComponent
{
	protected bool m_bGroupInitCalled;
	protected SCR_AIGroup m_Group;
	protected int m_iGroupSize = -1;
	
	protected const int iLEADER_ROLE_CHARACTER_CALLSIGN = 1;
	
	//Keeps track of character callsigns in group
	protected ref map<int, AIAgent> m_mCharacterCallsigns = new ref map<int, AIAgent>;
	
	//Keeps track of specific character role callsigns in group
	protected ref map<int, AIAgent> m_mCharacterRoleCallsigns = new ref map<int, AIAgent>;
	
	//======================================== GET CALLSIGN NAMES ========================================\\
	override bool GetCallsignNames(out string company, out string platoon, out string squad, out string character, out string format)
	{	
		int companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, emptyCharacter;
		character = string.Empty;
		
		if (!GetCallsignIndexes(companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, emptyCharacter))
			return false;
		
		//No callsign data
		if (!SetCallsignInfo())
		{
			company = companyCallsignIndex.ToString();
			platoon = platoonCallsignIndex.ToString();
			squad = squadCallsignIndex.ToString();
			
			format = "ERROR %1-%2-%3";
			
			return true;
		}
		
		company = m_CallsignInfo.GetCompanyCallsignName(companyCallsignIndex);
		platoon = m_CallsignInfo.GetPlatoonCallsignName(platoonCallsignIndex);
		squad = m_CallsignInfo.GetSquadCallsignName(squadCallsignIndex);
		
		
		format = m_CallsignInfo.GetCallsignFormat(false);		
		return true;
	}
	
	//======================================== GET CALLSIGN INDEXES ========================================\\
	override bool GetCallsignIndexes(out int company, out int platoon, out int squad, out int character = -1, out int characterRole = -1)
	{
		character = -1;
		
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
		
		return true;
	}
	
	//======================================== ASSIGN GROUP CALLSIGN ========================================\\
	//TODO: Callsigns should have logic as to how they are assigned as now there can be duplicants and there is no structure (eg Alpha-Red-1 should be followed by Alpha-Red-2) 
	protected void AssignGroupCallsign()
	{		
		if (!m_CallsignManager)
			return;
		
		int company, platoon, squad;
		m_CallsignManager.AssignCallGroupsign(m_Faction, company, platoon, squad);
		
		AssignCallsignBroadcast(company, platoon, squad);
		Rpc(AssignCallsignBroadcast, company, platoon, squad);
	}

	//======================================== BROAD CAST ========================================\\
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void AssignCallsignBroadcast(int company, int platoon, int squad)
	{	
		if (company == m_iCompanyCallsign && m_iPlatoonCallsign == platoon && m_iSquadCallsign == squad) return;
		
		m_iCompanyCallsign = company;
		m_iPlatoonCallsign = platoon;
		m_iSquadCallsign = squad;
		
		Event_OnCallsignChanged.Invoke(m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, -1, -1);
	}
	
	//======================================== ON LEADER ASSIGNER ========================================\\
	//Called if group is created by editor to get correct faction
	protected void OnLeaderAssigned(AIAgent currentLeader, AIAgent prevLeader)
	{	
		//Group init
		if (!m_bGroupInitCalled)
		{
			if (!currentLeader)
				return;
			
			IEntity leader = currentLeader.GetControlledEntity();
			FactionInit(SCR_EditableEntityComponent.GetEditableEntity(leader).GetFaction());
		}
		else {
			ReplaceLeaderRole(currentLeader, prevLeader);
		}
	}
	
	//---------------------------------------- Replace leader Callsign ----------------------------------------\\
	//If leader was replaced and both characters are still alive and in the same group
	//Note this only works if the actual leader is replaced in the SCR_AIGroup component, it will not override the callsign if it is not a leader
	protected void ReplaceLeaderRole(AIAgent currentLeader, AIAgent prevLeader)
	{
		if (!m_bIsServer)
			return;
		
		//~ Make sure both agents are alive, not the same character and in the same group
		if (currentLeader == null || prevLeader == null || currentLeader == prevLeader || currentLeader.GetParentGroup() != prevLeader.GetParentGroup())
			return;
		
		SCR_CallsignCharacterComponent prevLeaderCallsignComponent = SCR_CallsignCharacterComponent.Cast(prevLeader.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));
		if (!prevLeaderCallsignComponent)
			return;
		
		int prevLeaderCharacterCallsign, prevLeaderRoleCallsign
		prevLeaderCallsignComponent.GetCharacterAndRoleCallsign(prevLeaderCharacterCallsign, prevLeaderRoleCallsign);
		
		if (prevLeaderRoleCallsign != ERoleCallsign.SQUAD_LEADER)
			return;
		
		//~ Have system assign a new character callsign to the leader. Will change character callsign from 1 and assign a new role if any
		AssignCharacterCallsign(prevLeader, prevLeaderCallsignComponent);
		
		//~ Make leader role availible so it is auto assigned to the new leader
		MakeCharacterRoleAvailible(ERoleCallsign.SQUAD_LEADER);
	}
	
	//======================================== CHARACTERS ADDED/REMOVED FROM GROUP ========================================\\
	//---------------------------------------- On character added to group ----------------------------------------\\
	protected void OnCharacterAddedToGroup(AIAgent character)
	{
		if (!m_bGroupInitCalled)
			return;
		
		SCR_CallsignCharacterComponent characterCallsignComponent = SCR_CallsignCharacterComponent.Cast(character.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));
				
		if (characterCallsignComponent)
			AssignCharacterCallsign(character, characterCallsignComponent);
		
		m_iGroupSize = m_Group.GetAgentsCount();
		
		//No longer one character in group so update callsigns (This includes squad leader)
		if (m_iGroupSize == 2)
			UpdateAllCharacterRoleCallsigns();
	}
	
	//---------------------------------------- On character removed from group ----------------------------------------\\
	protected void OnCharacterRemovedFromGroup(SCR_AIGroup group, AIAgent character)
	{		
		if (!m_bGroupInitCalled)
			return;

		SCR_CallsignCharacterComponent characterCallsignComponent = SCR_CallsignCharacterComponent.Cast(character.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));

		if (!characterCallsignComponent)
			return;
		
		int characterIndex, roleIndex;
		characterCallsignComponent.GetCharacterAndRoleCallsign(characterIndex, roleIndex);
		
		//Make character callsign availible
		if (m_mCharacterCallsigns.Contains(characterIndex))
			m_mCharacterCallsigns.Remove(characterIndex);
		
		MakeCharacterRoleAvailible(roleIndex);
			
		if (m_Group)
			m_iGroupSize = m_Group.GetAgentsCount();
		
		//Only one character in group so make sure squad leader is not assigned
		if (m_iGroupSize == 1)
			UpdateAllCharacterRoleCallsigns();
	}
	
	//======================================== ASSIGN CHARACTER CALLSIGN  ========================================\\
	//---------------------------------------- Assign character Callsign ----------------------------------------\\
	protected void AssignCharacterCallsign(AIAgent character, SCR_CallsignCharacterComponent characterCallsignComponent, int characterCallsign = -1)
	{
		if (!m_CallsignInfo)
			return;
		
		//Assign first availible callsign
		if (characterCallsign < 0)
		{
			characterCallsign = 1;

			while (m_mCharacterCallsigns.Contains(characterCallsign))
			{
				characterCallsign++;
			}
		}
		
		//Assign specific role callsign
		int roleIndex = -1;
		if (m_CallsignInfo.GetCharacterRoleCallsign(character, m_Group, m_mCharacterRoleCallsigns, roleIndex))
		{		
			m_mCharacterRoleCallsigns.Set(roleIndex, character);
		}
		
		//Assign callsign
		characterCallsignComponent.AssignCharacterCallsign(m_Faction, m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, characterCallsign, roleIndex, true, m_iGroupSize <= 1);
		
		if (characterCallsign >= 0)
			m_mCharacterCallsigns.Set(characterCallsign, character);
	}
	
	//======================================== CHARACTER ROLE CALLSIGNS  ========================================\\
	//---------------------------------------- Make character roll callsign availible ----------------------------------------\\
	protected void MakeCharacterRoleAvailible(int roleIndex)
	{
		if (roleIndex < 0)
			return;
		
		//Make character role callsign availible
		if (m_mCharacterRoleCallsigns.Contains(roleIndex))
			m_mCharacterRoleCallsigns.Remove(roleIndex);
		
		array<AIAgent> agents = new array<AIAgent>;
		SCR_CallsignCharacterComponent characterCallsignComponent;
		
		int characterCallsign, currentRoleCallsign;
		bool roleReassigned = false;
		
		m_Group.GetAgents(agents);
		
		//Go over each character to check if role can be reassigned to another character
		foreach (AIAgent agent: agents)
		{
			characterCallsignComponent = SCR_CallsignCharacterComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));
			
			if (!characterCallsignComponent)
				continue;
			
			characterCallsignComponent.GetCharacterAndRoleCallsign(characterCallsign, currentRoleCallsign);
			
			//Do not assign roles to characters that already have a role (Unless squad leader)
			if (currentRoleCallsign >= 0 && roleIndex != ERoleCallsign.SQUAD_LEADER)
				continue;
			
			if (m_CallsignInfo.GetCharacterRoleCallsign(agent, m_Group, m_mCharacterRoleCallsigns, roleIndex))
			{
				//~ Update role
				if (roleIndex != ERoleCallsign.SQUAD_LEADER)
				{
					characterCallsignComponent.UpdateCharacterRoleCallsign(roleIndex);
				}
				//~ Replace leader
				else 
				{
					characterCallsignComponent.UpdateCharacterCallsignAndRole(iLEADER_ROLE_CHARACTER_CALLSIGN, roleIndex);
					
					if (m_mCharacterCallsigns.Contains(characterCallsign))
						m_mCharacterCallsigns.Remove(characterCallsign);
					
					m_mCharacterCallsigns.Set(iLEADER_ROLE_CHARACTER_CALLSIGN, agent);
				}
				
				//~ Update role map
				m_mCharacterRoleCallsigns.Set(roleIndex, agent);
				
				roleReassigned = true;
				break;
			}
		}
	
		//~ Had a prev role make it availible again (Only called if new role assigned is Squad leader else it will never reach this)
		if (roleReassigned && currentRoleCallsign >= 0)
			MakeCharacterRoleAvailible(currentRoleCallsign);
	}
	
	//======================================== INIT ========================================\\
	//---------------------------------------- Group Init/ Faction changed ----------------------------------------\\
	//On Group init and faction change
	void FactionInit(Faction faction, bool isFactionChange = false)
	{
		Faction prevFaction = m_Faction;	
		m_Faction = faction;
		
		//Server only
		if (!m_bIsServer)
			return;
		
		if (!SetCallsignInfo())
			return;
		
		//Faction is changed so make current callsign availible again
		if (isFactionChange)
		{
			if (!prevFaction || !m_CallsignManager)
				return;
			
			if (m_iCompanyCallsign >= 0)
			{
				m_CallsignManager.MakeGroupCallsignAvailible(prevFaction, m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign);
			}
			//Clears assigned callsign
			ClearCallsigns();
			
			//Clear character callsigns
			m_mCharacterCallsigns.Clear();
			m_mCharacterRoleCallsigns.Clear();
		}
		
		if (!SetCallsignInfo())
			return;
		
		m_bGroupInitCalled = true;
		
		//Assign group callsign
		AssignGroupCallsign();

		array<AIAgent> agents = new array<AIAgent>;
		SCR_CallsignCharacterComponent characterCallsignComponent;
		
		m_Group.GetAgents(agents);
		m_iGroupSize = agents.Count();
		
		//If callsigns are assigned at random randomize group as well on init
		array<int> availibleRandomCallsigns = new array<int>;
		if (m_CallsignInfo.GetIsAssignedRandomly())
		{
			int count = m_iGroupSize;
			
			while(count > 0)
			{
				availibleRandomCallsigns.Insert(count);
				count--;
			}
		}
		
		//Assign callsigns for each character in group
		foreach (AIAgent agent: agents)
		{
			characterCallsignComponent = SCR_CallsignCharacterComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));
			characterCallsignComponent.ClearCallsigns();
			
			if (characterCallsignComponent)
				AssignCharacterCallsign(agent, characterCallsignComponent);
			
			//~ Replace line above if you want character callsigns to also be randomized
			/*{
				if (m_CallsignInfo.GetIsAssignedRandomly())
				{
					Math.Randomize(-1);
					int randomIndex = Math.RandomInt(0, availibleRandomCallsigns.Count());
					int randomCharacterCallsign = availibleRandomCallsigns[randomIndex];
					availibleRandomCallsigns.Remove(randomIndex);
					
					AssignCharacterCallsign(agent, characterCallsignComponent, randomCharacterCallsign);
				}
				else 
				{
					AssignCharacterCallsign(agent, characterCallsignComponent);
				}
			}*/
		}
	}
	
	protected void OnFactionChanged(Faction faction)
	{
		FactionInit(faction, m_Faction && m_Faction != faction);
	}
	
	protected void UpdateAllCharacterRoleCallsigns()
	{
		m_mCharacterRoleCallsigns.Clear();
		array<AIAgent> agents = new array<AIAgent>;
		SCR_CallsignCharacterComponent characterCallsignComponent;
		
		if (m_Group)
			m_Group.GetAgents(agents);
		
		foreach (AIAgent agent: agents)
		{
			characterCallsignComponent = SCR_CallsignCharacterComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));
			
			if (characterCallsignComponent)
			{
				int characterCallsign, roleCallsign;
				characterCallsignComponent.GetCharacterAndRoleCallsign(characterCallsign, roleCallsign);
				characterCallsignComponent.ClearCharacterRoleCallsign();
				AssignCharacterCallsign(agent, characterCallsignComponent, characterCallsign);
			}
		}
	
	}
	
	//---------------------------------------- Server Init ----------------------------------------\\
	//If server assign callsign
	override void InitOnServer(IEntity owner)
	{
		// Always verify the pointer. The object could be deleted by the time it gets here.
		if (owner == null)
			return;
		
		super.InitOnServer(owner);
		
		m_Group = SCR_AIGroup.Cast(owner);		
		if (!m_Group) 
			return;
		
		//If leader assigned then init group, else wait until leader is assigned
		IEntity leader = m_Group.GetLeaderEntity();
		Faction groupFaction;
		if (leader)
			groupFaction = SCR_EditableEntityComponent.GetEditableEntity(leader).GetFaction();
		else
			groupFaction = m_Group.GetFaction();
		
		if (groupFaction)
			FactionInit(groupFaction);
				
		//If characters are added/removed from group
		m_Group.GetOnAgentAdded().Insert(OnCharacterAddedToGroup);
		m_Group.GetOnAgentRemoved().Insert(OnCharacterRemovedFromGroup);
		m_Group.GetOnFactionChanged().Insert(OnFactionChanged);
		m_Group.GetOnLeaderChanged().Insert(OnLeaderAssigned);
	}

	//======================================== RPL ========================================\\
	override bool RplSave(ScriptBitWriter writer)
    {	
        writer.WriteInt(m_iCompanyCallsign); 
        writer.WriteInt(m_iPlatoonCallsign);
		writer.WriteInt(m_iSquadCallsign);
		
        return true;
    }
     
    override bool RplLoad(ScriptBitReader reader)
    {
		int company, platoon, squad, groupSize;
		
        reader.ReadInt(company);
        reader.ReadInt(platoon);
		reader.ReadInt(squad);
		
		AssignCallsignBroadcast(company, platoon, squad);
		
        return true;
    }
	
	//======================================== ON DESTROY ========================================\\
	void ~SCR_CallsignGroupComponent()
	{
		if (!m_bIsServer || m_iCompanyCallsign < 0 || !m_Faction || !m_CallsignManager)
			return;
		
		m_CallsignManager.MakeGroupCallsignAvailible(m_Faction, m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign);
	}
};
