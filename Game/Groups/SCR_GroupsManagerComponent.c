[EntityEditorProps(category: "GameScripted/Groups", description: "Player groups manager, attach to game mode entity!.")]
class SCR_GroupsManagerComponentClass : SCR_BaseGameModeComponentClass
{
};

typedef set<Faction> FactionHolder;

//------------------------------------------------------------------------------------------------
class SCR_GroupsManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute()]
	protected ResourceName m_sDefaultGroupPrefab;
	
	[Attribute("1000")]
	protected int m_iPlayableGroupFrequencyOffset;
	
	[Attribute("38000")]
	protected int m_iPlayableGroupFrequencyMin;
	
	[Attribute("54000")]
	protected int m_iPlayableGroupFrequencyMax;
	
	//this is changed only localy and doesnt replicate
	protected bool m_bConfirmedByPlayer;
	
	protected static SCR_GroupsManagerComponent s_Instance;
	
#ifdef DEBUG_GROUPS
	static Widget s_wDebugLayout;
#endif
	
	protected ref ScriptInvoker m_OnPlayableGroupCreated = new ScriptInvoker();
	protected ref ScriptInvoker m_OnPlayableGroupRemoved = new ScriptInvoker();
	protected int m_iLatestGroupID = 0;
	protected ref map<Faction, ref array<SCR_AIGroup>> m_mPlayableGroups = new map<Faction, ref array<SCR_AIGroup>>();
	protected ref map<Faction, int> m_mPlayableGroupFrequencies = new map<Faction, int>();
	protected ref map<int, ref FactionHolder> m_mUsedFrequenciesMap = new map<int, ref FactionHolder>();
	protected ref array<SCR_AIGroup> m_aDeletionQueue = {};
	protected int m_iMovingPlayerToGroupID = -1;
	
	//------------------------------------------------------------------------
	static SCR_GroupsManagerComponent GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------
	ScriptInvoker GetOnPlayableGroupCreated()
	{
		return m_OnPlayableGroupCreated;
	}
	
	//------------------------------------------------------------------------
	protected bool IsProxy()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComponent)
			return false;
		
		return rplComponent.IsProxy();
	}
	
	//------------------------------------------------------------------------
	int MovePlayerToGroup(int playerID, int previousGroupID, int newGroupID)
	{
		m_iMovingPlayerToGroupID = newGroupID;
		SCR_AIGroup previousGroup = FindGroup(previousGroupID);
		if (previousGroup)
			previousGroup.RemovePlayer(playerID);
		
		SCR_AIGroup newGroup = FindGroup(newGroupID);
		if (newGroup)
		{
			if (newGroup.IsFull())
			{
				m_iMovingPlayerToGroupID = -1;
				return -1;
			}
			
			newGroup.AddPlayer(playerID);
			m_iMovingPlayerToGroupID = -1;
			return newGroupID;
		}
		else
		{
			m_iMovingPlayerToGroupID = -1;
			return -1;
		}
	}
	
	//------------------------------------------------------------------------
	int AddPlayerToGroup(int groupID, int playerID)
	{
		SCR_AIGroup group = FindGroup(groupID);
		if (!group)
			return -1;
		
		if (group.IsFull())
			return -1;
		
		group.AddPlayer(playerID);
		return groupID;
	}
	
	//------------------------------------------------------------------------
	void SetGroupLeader(int groupID, int playerID)
	{
		SCR_AIGroup group = FindGroup(groupID);
		if (!group)
			return;
		group.SetGroupLeader(playerID);
	}
	
	//------------------------------------------------------------------------
	void SetPrivateGroup(int groupID, bool isPrivate)
	{
		SCR_AIGroup group = FindGroup(groupID);
		if (!group)
			return;
		group.SetPrivate(isPrivate);
	}
	
	//------------------------------------------------------------------------
	SCR_AIGroup FindGroup(int groupID)
	{
		array<SCR_AIGroup> groups = GetAllPlayableGroups();
		for (int i = groups.Count() - 1; i >= 0; i--)
		{
			if (groups[i].GetGroupID() == groupID)
				return groups[i];
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------
	ScriptInvoker GetOnPlayableGroupRemoved()
	{
		return m_OnPlayableGroupRemoved;
	}
	
	//------------------------------------------------------------------------
	// Returns group by player id
	SCR_AIGroup GetPlayerGroup(int playerID)
	{
		SCR_RespawnSystemComponent respawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnSystemComponent)
			return null;
		
		Faction faction = respawnSystemComponent.GetPlayerFaction(playerID);
		if (!faction)
			return null;
		
		array<SCR_AIGroup> playableGroups = GetPlayableGroupsByFaction(faction);
		if (!playableGroups)
			return null;
		
		for (int i = playableGroups.Count() - 1; i >= 0; i--)
		{
			if (playableGroups[i].IsPlayerInGroup(playerID))
				return playableGroups[i];
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------
	bool IsPlayerInAnyGroup(int playerID)
	{
		array<SCR_AIGroup> playableGroups = GetAllPlayableGroups();
		for (int i = playableGroups.Count() - 1; i >= 0; i--)
		{
			if (playableGroups[i].IsPlayerInGroup(playerID))
				return true;
		}
		
		return false;
	}
	
#ifdef DEBUG_GROUPS
	//------------------------------------------------------------------------
	void UpdateDebugUI()
	{
		if (!s_wDebugLayout)
			s_wDebugLayout = GetGame().GetWorkspace().CreateWidgets("{661C199F3D59BB24}UI/layouts/Debug/Groups.layout");
		
		VerticalLayoutWidget groupsLayout = VerticalLayoutWidget.Cast(s_wDebugLayout.FindAnyWidget("GroupsLayout"));
		if (!groupsLayout)
			return;
		
		//Clear entries
		while (groupsLayout.GetChildren())
			groupsLayout.GetChildren().RemoveFromHierarchy();
		
		array<SCR_AIGroup> playableGroups = GetAllPlayableGroups();
		for (int i = playableGroups.Count() - 1; i >= 0; i--)
		{
			Widget groupEntry = GetGame().GetWorkspace().CreateWidgets("{898A1945FD29FC71}UI/layouts/Debug/GroupsGroupEntry.layout", groupsLayout);
			RichTextWidget groupName = RichTextWidget.Cast(groupEntry.FindAnyWidget("GroupName"));
			if (groupName)
				groupName.SetText(playableGroups[i].GetGroupID().ToString() + ": " + playableGroups[i].GetCallsignSingleString());
			
			array<int> playerIDs = playableGroups[i].GetPlayerIDs();
			int playersCount = playerIDs.Count();
			
			for (int j = 0; j < playersCount; j++)
			{
				HorizontalLayoutWidget characterEntry = HorizontalLayoutWidget.Cast(groupEntry.GetWorkspace().CreateWidgets("{952C36C11AF74465}UI/layouts/Debug/GroupsCharacterEntry.layout", groupEntry));
				TextWidget playerNameWidget = TextWidget.Cast(characterEntry.FindAnyWidget("CharacterName"));
				if (!playerNameWidget)
					continue;
				
				playerNameWidget.SetText(GetGame().GetPlayerManager().GetPlayerName(playerIDs[j]));
			}
		}
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	array<SCR_AIGroup> GetPlayableGroupsByFaction(Faction faction)
	{
		return m_mPlayableGroups.Get(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_AIGroup> GetAllPlayableGroups()
	{
		array<SCR_AIGroup> allGroups = new array<SCR_AIGroup>();
		array<SCR_AIGroup> currentGroups = new array<SCR_AIGroup>();
		for (int i = m_mPlayableGroups.Count() - 1; i >= 0; i--)
		{
			currentGroups = m_mPlayableGroups.GetElement(i);
			
			for (int j = currentGroups.Count() - 1; j >= 0; j--)
			{
				allGroups.Insert(currentGroups[j]);
			}
		}
		
		return allGroups;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AssignGroupFrequency(notnull SCR_AIGroup group)
	{
		int frequency = 0;
		Faction groupFaction = group.GetFaction();

		frequency = GetFreeFrequency(groupFaction);
		if (frequency == -1)
			return;
		
		ClaimFrequency(frequency, groupFaction);
		group.SetGroupFrequency(frequency);
	}
	
	//------------------------------------------------------------------------------------------------
	void DeleteGroupDelayed(SCR_AIGroup group)
	{
		m_aDeletionQueue.Insert(group);
		SetEventMask(GetOwner(), EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnGroupPlayerRemoved(SCR_AIGroup group, int playerID)
	{
		// This script should only run on the server
		if (IsProxy())
			return;
		
		// Is empty?
		if (group.GetPlayerCount() > 0)
			return;
		
		// Yes, can we delete it?
		array<SCR_AIGroup> playableGroups = GetPlayableGroupsByFaction(group.GetFaction());
		if (!playableGroups)
			return;
		
		DeleteGroupDelayed(group);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnGroupPlayerAdded(SCR_AIGroup group, int playerID)
	{
		if (IsProxy())
			return;
		
		// Is group full?
		if (group.GetPlayerCount() < group.GetMaxMembers())
			return; // Group not full, we don't have to make any new groups
		
		// Group was full, we need to see if we have any other not full group
		Faction faction = group.GetFaction();
		if (!faction)
			return;
		
		array<SCR_AIGroup> groups = GetPlayableGroupsByFaction(faction);
		if (!groups)
			return;
		
		// No groups found for faction?
		int groupsCount = groups.Count();
		if (groupsCount == 0)
		{
			// Anyway we create a new one
			CreateNewPlayableGroup(faction);
			return;
		}
		
		for (int i = groupsCount - 1; i >= 0; i--)
		{
			// We are checking for full groups if at least one group is not full, we return, we don't have to create new one
			if (groups[i].GetPlayerCount() < group.GetMaxMembers())
				return;
		}
		
		// All the groups were full, create new one
		CreateNewPlayableGroup(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns a gadget of EGadgetType.RADIO
	private BaseRadioComponent GetCommunicationDevice(notnull IEntity controlledEntity)
	{
		SCR_GadgetManagerComponent gagdetManager = SCR_GadgetManagerComponent.Cast(controlledEntity.FindComponent(SCR_GadgetManagerComponent));
		if (!gagdetManager)
			return null;
		
		IEntity radio = gagdetManager.GetGadgetByType(EGadgetType.RADIO); //variable purpose = debug
		if (!radio)
			return null;
		
		return BaseRadioComponent.Cast(radio.FindComponent(BaseRadioComponent));
	}	
	
	//------------------------------------------------------------------------
	private void TuneAgentsRadio(AIAgent agentEntity)
	{
		if (!agentEntity)
			return;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(agentEntity.GetParentGroup());
		if (!group)
			return;
		
		BaseRadioComponent radio = GetCommunicationDevice(agentEntity.GetControlledEntity());
		
		if (radio)
			radio.SetFrequency(group.GetGroupFrequency());
	}
	
	//------------------------------------------------------------------------------------------------
	void OnGroupAgentAdded(AIAgent child)
	{
		GetGame().GetCallqueue().CallLater(TuneAgentsRadio, 1, false, child);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnGroupAgentRemoved(SCR_AIGroup group, AIAgent child)
	{
		return; //For now we don't delete empty groups
		
		if (!group)
			return;
		
		// Is group empty?
		if (group.GetAgentsCount() > 0)
			return;
		
		// Group is empty, can we delete it?
		Faction faction = group.GetFaction();
		if (!faction)
			return;
		
		array<SCR_AIGroup> groups = GetPlayableGroupsByFaction(faction);
		if (!groups)
			return;
		
		// No groups found for faction?
		int groupsCount = groups.Count();
		if (groupsCount == 0)
			return;
		
		// If we find any other group, which is empty, we delete this group
		for (int i = groupsCount - 1; i >= 0; i--)
		{
			if (groups[i] == group)
				continue;
			
			if (groups[i].GetAgentsCount() == 0)
			{
				DeleteAndUnregisterGroup(group);
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void DeleteAndUnregisterGroup(notnull SCR_AIGroup group)
	{
		ReleaseFrequency(group.GetGroupFrequency(), group.GetFaction());
		UnregisterGroup(group);
		delete group;
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterGroup(SCR_AIGroup group)
	{
		Faction faction = group.GetFaction();
		if (!faction)
			return;
		
		array<SCR_AIGroup> groups = GetPlayableGroupsByFaction(faction);
		if (!groups)
			return;
		
		if (groups.Find(group) >= 0)
			groups.RemoveItem(group);
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterGroup(SCR_AIGroup group)
	{
		array<SCR_AIGroup> groups;
		if (!m_mPlayableGroups.Find(group.GetFaction(), groups))
		{
			// No array found, let's make one
			groups = new array<SCR_AIGroup>();
			m_mPlayableGroups.Insert(group.GetFaction(), groups);
		}
		
		if (groups.Find(group) < 0)
			groups.Insert(group);
		
		group.GetOnAgentAdded().Insert(OnGroupAgentAdded);
		group.GetOnAgentRemoved().Insert(OnGroupAgentRemoved);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetGroupFaction(RplId groupID, int factionIndex)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Faction faction = factionManager.GetFactionByIndex(factionIndex);
		if (!faction)
			return;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(Replication.FindItem(groupID));
		if (group)
			group.SetFaction(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AssignGroupID(SCR_AIGroup group)
	{
		group.SetGroupID(m_iLatestGroupID);
		m_iLatestGroupID++;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is called only on the server (authority)
	void OnPlayerFactionChanged(int playerID, int factionIndex)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Faction faction = factionManager.GetFactionByIndex(factionIndex);
		if (!faction)
			return;
		SCR_AIGroup newPlayerGroup = GetFirstNotFullForFaction(faction);
		if (!newPlayerGroup)
			newPlayerGroup = CreateNewPlayableGroup(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on clients (proxies) to notice a playable group has been created
	void OnGroupCreated(SCR_AIGroup group)
	{
		m_OnPlayableGroupCreated.Invoke(group);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIGroup TryFindEmptyGroup(notnull Faction faction)
	{
		array<SCR_AIGroup> factionGroups = GetPlayableGroupsByFaction(faction);
		if (!factionGroups)
			return null;
		
		for (int i = factionGroups.Count() - 1; i >= 0; i--)
		{
			if (factionGroups[i].GetPlayerCount() == 0)
				return factionGroups[i];
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIGroup CreateNewPlayableGroup(Faction faction)
	{
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComp)
			return null;
		
		if (!rplComp.IsMaster())
			return null;
		
		Resource groupResource = Resource.Load(m_sDefaultGroupPrefab);
		if (!groupResource.IsValid())
			return null;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(groupResource, GetOwner().GetWorld()));
		
		group.SetFaction(faction);
		RegisterGroup(group);
		AssignGroupFrequency(group);
		AssignGroupID(group);
		
		m_OnPlayableGroupCreated.Invoke(group);
		
#ifdef DEBUG_GROUPS
		GetGame().GetCallqueue().CallLater(UpdateDebugUI, 1, false);
#endif
		
		return group;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetFirstNotFullForFaction(notnull Faction faction, SCR_AIGroup ownGroup = null, bool respectPrivate = false)
	{
		SCR_AIGroup group;
		array<SCR_AIGroup> factionGroups = GetPlayableGroupsByFaction(faction);
		if (!factionGroups)
			return group;
		
		for (int i = 0, count = factionGroups.Count(); i < count; i++)
		{
			if (!factionGroups[i].IsFull() && factionGroups[i] != ownGroup && (!respectPrivate || !factionGroups[i].IsPrivate()))
			{
				group = factionGroups[i];
				break;
			}
		}
		
		return group;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanCreateNewGroup(notnull Faction newGroupFaction)
	{
		FactionHolder factions = new FactionHolder();
		if (GetFreeFrequency(newGroupFaction) == -1)
			return false;
		
		if (TryFindEmptyGroup(newGroupFaction))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFreeFrequency(Faction frequencyFaction)
	{
		FactionHolder usedForFactions = new FactionHolder();
		
		int factionHQFrequency; // Don't assign this frequency to any of the groups
		SCR_MilitaryFaction militaryFaction = SCR_MilitaryFaction.Cast(frequencyFaction);
		if (militaryFaction)
			factionHQFrequency = militaryFaction.GetFactionRadioFrequency();
		
		int minFrequencyAvailable = m_iPlayableGroupFrequencyMin;
		
		while (minFrequencyAvailable <= m_iPlayableGroupFrequencyMax)
		{
			if (minFrequencyAvailable == factionHQFrequency)
			{
				minFrequencyAvailable += m_iPlayableGroupFrequencyOffset;
				continue; // We cannot assign this frequency to any group, it is used by the factions HQ
			}
			
			if (!m_mUsedFrequenciesMap.Find(minFrequencyAvailable, usedForFactions))
				break; // No assigned frequencies for this faction yet
			
			if (usedForFactions.Find(frequencyFaction) == -1)
				break; // Unused frequency found
			
			minFrequencyAvailable += m_iPlayableGroupFrequencyOffset;
		}
		
		if (minFrequencyAvailable <= m_iPlayableGroupFrequencyMax)
			return minFrequencyAvailable;
		
		Print("Ran out of frequencies for groups", LogLevel.WARNING);
		return -1;
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ClaimFrequency(int frequency, Faction faction)
	{
		FactionHolder usedForFactions = new FactionHolder();
		FactionHolder factions = new FactionHolder();
			
		if (!m_mUsedFrequenciesMap.Find(frequency, usedForFactions))
		{
			factions.Insert(faction);
			m_mUsedFrequenciesMap.Insert(frequency, factions);
			return;
		}
		if (usedForFactions.Find(faction) == -1)
		{
			usedForFactions.Insert(faction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ReleaseFrequency(int frequency, Faction faction)
	{
		if (!faction || !frequency)
			return;
		FactionHolder factions = new FactionHolder();
		if (m_mUsedFrequenciesMap.Find(frequency, factions))
		{
			if (factions.Count() <= 1 && factions.Find(faction) != -1)
				m_mUsedFrequenciesMap.Remove(frequency);
			else 
			{
				int factionIdx = factions.Find(faction);
				if (factionIdx >= 0 && factionIdx < factions.Count())
					factions.Remove(factionIdx);
			}
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestSetGroupSlave(RplId compID, RplId slaveID)
	{
		RPC_DoSetGroupSlave(compID, slaveID);
		//Call next method later to be sure that SCR_AIGroup was replicated to the client succesfully (temporary solution)
		GetGame().GetCallqueue().CallLater(RpcWrapper, 2000, false, compID, slaveID);
	}
	
	//------------------------------------------------------------------------------------------------
	void RpcWrapper(RplId compID, RplId slaveID)
	{
		Rpc(RPC_DoSetGroupSlave, compID, slaveID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetGroupSlave(RplId masterGroupID, RplId slaveGroupID)
	{
		SCR_AIGroup masterGroup, group;
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(masterGroupID));
		if (!rplComp)
			return;
		
		masterGroup = SCR_AIGroup.Cast(rplComp.GetEntity());
		if (!masterGroup)
			return;
		
		rplComp = RplComponent.Cast(Replication.FindItem(slaveGroupID));
		if (!rplComp)
			return;
		
		group = SCR_AIGroup.Cast(rplComp.GetEntity());
		if (!group)
			return;
		
		masterGroup.SetSlave(group);
		group.GetOnAgentRemoved().Insert(OnAIMemberRemoved);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoRemoveAIMemberFromGroup(RplId groupRplCompID, RplId aiCharacterComponentID)
	{		
		SCR_ChimeraCharacter AIMember;
		array<SCR_ChimeraCharacter> AIMembers;
		GetAIMembers(groupRplCompID, aiCharacterComponentID, AIMembers, AIMember);
		if (!AIMembers || !AIMember)
			return;
		
		AIMembers.RemoveItem(AIMember);
	}
	
	//------------------------------------------------------------------------------------------------
	void GetAIMembers(RplId groupRplCompID, RplId aiCharacterComponentID, out array<SCR_ChimeraCharacter> members, out SCR_ChimeraCharacter AIMember)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(aiCharacterComponentID));
		if (!rplComp)
			return;

		AIMember = SCR_ChimeraCharacter.Cast(rplComp.GetEntity());
		if (!AIMember)
			return;
		
		rplComp = RplComponent.Cast(Replication.FindItem(groupRplCompID));
		if (!rplComp)
			return;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(rplComp.GetEntity());
		if (!group)
			return;
		
		members = group.GetAIMembers();
	}
	
	//------------------------------------------------------------------------------------------------
	void AskRemoveAiMemberFromGroup(RplId groupRplCompID, RplId aiCharacterComponentID)
	{
		RPC_DoRemoveAIMemberFromGroup(groupRplCompID, aiCharacterComponentID);
		Rpc(RPC_DoRemoveAIMemberFromGroup, groupRplCompID, aiCharacterComponentID);
	}
	
	//------------------------------------------------------------------------------------------------
	void AskAddAiMemberToGroup(RplId groupRplCompID, RplId aiCharacterComponentID)
	{
		RPC_DoAddAIMemberToGroup(groupRplCompID, aiCharacterComponentID);
		Rpc(RPC_DoAddAIMemberToGroup, groupRplCompID, aiCharacterComponentID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoAddAIMemberToGroup(RplId groupRplCompID, RplId aiCharacterComponentID)
	{
		SCR_ChimeraCharacter AIMember;
		array<SCR_ChimeraCharacter> AIMembers;
		GetAIMembers(groupRplCompID, aiCharacterComponentID, AIMembers, AIMember);
		if (!AIMembers || !AIMember)
			return;
		
		AIMembers.Insert(AIMember);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnAIMemberRemoved(SCR_AIGroup group, AIAgent agent)
	{
		if (!group || !agent)
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(agent.GetControlledEntity());
		if (!character)
			return;
		
		RplId groupCompRplID, characterCompRplID;
		RplComponent rplComp = RplComponent.Cast(group.FindComponent(RplComponent));
		if (!rplComp)
			return;
		
		groupCompRplID = rplComp.Id();
		
		rplComp = RplComponent.Cast(character.FindComponent(RplComponent));
		if (!rplComp)
			return;

		characterCompRplID = rplComp.Id();
		AskRemoveAiMemberFromGroup(groupCompRplID, characterCompRplID);
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetConfirmedByPlayer()
	{
		return m_bConfirmedByPlayer;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetConfirmedByPlayer(bool isConfirmed)
	{
		m_bConfirmedByPlayer = isConfirmed;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		DeleteGroups();
	}
	
	//------------------------------------------------------------------------------------------------
	void DeleteGroups()
	{
		for (int i = m_aDeletionQueue.Count() - 1; i >= 0; i--)
		{
			DeleteAndUnregisterGroup(m_aDeletionQueue[i]);
			m_aDeletionQueue.Remove(i);
		}
		
		ClearEventMask(GetOwner(), EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		owner.SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_AIGroup.GetOnPlayerAdded().Insert(OnGroupPlayerAdded);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(OnGroupPlayerRemoved);
		
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (respawnSystem)
			respawnSystem.GetOnPlayerFactionChanged().Insert(OnPlayerFactionChanged);
		m_bConfirmedByPlayer = false;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_GroupsManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!s_Instance)
			s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_GroupsManagerComponent()
	{
#ifdef DEBUG_GROUPS
		if (s_wDebugLayout)
			s_wDebugLayout.RemoveFromHierarchy();
#endif
	}

};
