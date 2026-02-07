[EntityEditorProps(category: "GameScripted/Groups", description: "This component should be attached to player controller and is used by groups to send requests to server.")]
class SCR_PlayerControllerGroupComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerControllerGroupComponent : ScriptComponent
{
	protected int m_iGroupID = -1;
	// Map with playerID and list of groups the player was invited to
	protected ref map<int, ref array<int>> m_mPlayerInvitesToGroups;
	protected ref ScriptInvoker<int, int> m_OnInviteReceived = new ScriptInvoker<int, int>();
	protected ref ScriptInvoker<int> m_OnInviteAccepted = new ScriptInvoker<int>();
	protected ref ScriptInvoker<int> m_OnInviteCancelled = new ScriptInvoker<int>();

	protected int m_iUISelectedGroupID = -1;
	protected int m_iGroupInviteID = -1;
	protected int m_iGroupInviteFromPlayerID = -1;
	protected string m_sGroupInviteFromPlayerName; //Player name is saved to get the name of the one who invited even if that player left the server
	
	//------------------------------------------------------------------------------------------------
	static SCR_PlayerControllerGroupComponent GetPlayerControllerComponent(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return null;
		
		return SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_PlayerControllerGroupComponent GetLocalPlayerControllerGroupComponent()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return null;
		
		return SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	int GetGroupID()
	{
		return m_iGroupID;
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestCreateGroup()
	{
		Rpc(RPC_AskCreateGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestKickPlayer(int playerID)
	{
		Rpc(RPC_AskKickPlayer, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestPromoteLeader(int playerID)
	{
		Rpc(RPC_AskPromoteLeader, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestPrivateGroupChange(int playerID, bool isPrivate)
	{
		Rpc(RPC_ChangePrivateGroup, playerID, isPrivate);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerID()
	{
		PlayerController playerController = PlayerController.Cast(GetOwner());
		if (!playerController)
			return -1;
		
		return playerController.GetPlayerId();
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanPlayerJoinGroup(int playerID, notnull SCR_AIGroup group)
	{
		// First we check the player is in the faction of the group
		SCR_RespawnSystemComponent respawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		if (respawnSystemComponent && respawnSystemComponent.GetPlayerFaction(playerID) != group.GetFaction())
			return false;
		
		// Groups manager doesn't exist, no point in continuing, cannot join
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;
		
		// Cannot join a full group
		if (group.IsFull())
			return false;
		
		// Cannot join the group we are in already
		if (groupsManager.GetPlayerGroup(playerID) == group)
			return false;
		
		//cannot join private group
		if (group.IsPrivate())
			return false;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPlayerLeader(int playerID, notnull SCR_AIGroup group)
	{
		return playerID == group.GetLeaderID();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPlayerLeaderOwnGroup()
	{
		SCR_PlayerControllerGroupComponent groupPlayerController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupPlayerController)
			return false;
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return false;
		
		SCR_AIGroup playerGroup;
		
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		playerGroup = groupManager.GetPlayerGroup(playerID);
		if (!playerGroup)
			return false;
		
		return IsPlayerLeader(playerID, playerGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanInvitePlayer(int playerID)
	{
		// Our group id is not valid -> cannot invite anyone
		if (m_iGroupID < 0)
			return false;
		
		// Groups manager doesn't exist, no point in continuing, cannot invite
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;
		
		// We get our group
		SCR_AIGroup group = groupsManager.GetPlayerGroup(GetPlayerID());
		
		if (!group)
			return false;
		
		// Check if the player can join us
		if (!CanPlayerJoinGroup(playerID, group))
			return false;
		
		// Already invited this player, cannot invite again
		if (WasAlreadyInvited(playerID))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool WasAlreadyInvited(int playerID)
	{
		// The map is not initialized -> didn't invite anyone yet
		if (!m_mPlayerInvitesToGroups)
			return false;
		
		// We didn't invite this player to any group yet
		if (!m_mPlayerInvitesToGroups.Contains(playerID))
			return false;
		
		// If our group is in the array of invites for this player, we return true (already invited)
		// Otherwise we return false (wasn't invited yet)
		return m_mPlayerInvitesToGroups.Get(playerID).Contains(m_iGroupID);
	}
	
	//------------------------------------------------------------------------------------------------
	void InvitePlayer(int playerID)
	{
		// When group id is not valid, return
		if (m_iGroupID < 0)
			return;
		
		// Init map if not initialized yet
		if (!m_mPlayerInvitesToGroups)
			m_mPlayerInvitesToGroups = new map<int, ref array<int>>();
		
		// Init array of groups the playerID was invited to if not initialized yet
		if (!m_mPlayerInvitesToGroups.Contains(playerID))
			m_mPlayerInvitesToGroups.Insert(playerID, {});
		
		// We already invited this player to our group, don't invite again
		if (m_mPlayerInvitesToGroups.Get(playerID).Contains(m_iGroupID))
			return;
		
		// We didn't invite this player to our group yet
		// Get an array of all the groups the local player invited the other player to
		array<int> invitedGroups = m_mPlayerInvitesToGroups.Get(playerID);
		
		// Invite the player and log the invitation
		Rpc(RPC_AskInvitePlayer, playerID);
		invitedGroups.Insert(m_iGroupID);
	}
	
	//------------------------------------------------------------------------------------------------
	void InviteThisPlayer(int groupID, int fromPlayerID)
	{
		Rpc(RPC_DoInvitePlayer, groupID, fromPlayerID)
	}
	
	//------------------------------------------------------------------------------------------------
	void AcceptInvite()
	{
		if (m_iGroupInviteID >= 0)
		{
			RequestJoinGroup(m_iGroupInviteID);
			m_iGroupInviteID = -1;
			m_OnInviteAccepted.Invoke();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnInviteReceived()
	{
		return m_OnInviteReceived;
	}
	
	void OnGroupDeleted(SCR_AIGroup group)
	{
		if (!group)
			return; 
		
		//in case of the selected group being deleted, remove it from selected
		if (group.GetGroupID() == GetSelectedGroupID())
			SetSelectedGroupID(-1);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RPC_DoInvitePlayer(int groupID, int fromPlayerID)
	{
		m_iGroupInviteID = groupID;
		m_iGroupInviteFromPlayerID = fromPlayerID;
		
		//Save player name so it can be obtained even if the player left
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (playerManager)
			m_sGroupInviteFromPlayerName = playerManager.GetPlayerName(fromPlayerID);
		
		m_OnInviteReceived.Invoke(groupID, fromPlayerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskInvitePlayer(int playerID)
	{
		PlayerController invitedPlayer = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!invitedPlayer)
			return;
		
		SCR_PlayerControllerGroupComponent invitedPlayerGroupComponent = SCR_PlayerControllerGroupComponent.Cast(invitedPlayer.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!invitedPlayerGroupComponent)
			return;
		
		invitedPlayerGroupComponent.InviteThisPlayer(m_iGroupID, GetPlayerID());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskCreateGroup()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_RespawnSystemComponent respawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnSystemComponent)
			return;
		
		Faction faction = respawnSystemComponent.GetPlayerFaction(GetPlayerID());
		if (!faction)
			return;
		
		// We check if there is any empty group already for our faction
		if (groupsManager.TryFindEmptyGroup(faction))
			return;
		
		// No empty group found, we allow creation of new group
		SCR_AIGroup newGroup = groupsManager.CreateNewPlayableGroup(faction);
		
		// No new group was created, return
		if (!newGroup)
			return;
		
		// New group sucessfully created
		// The player should be automatically added/moved to it
		RPC_AskJoinGroup(newGroup.GetGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskKickPlayer(int playerID)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		//requesting player is not leader of the targets group, do nothing
		if (!group.IsPlayerLeader(GetPlayerID()))
			return;
		
		SCR_AIGroup newGroup = groupsManager.GetFirstNotFullForFaction(group.GetFaction(), group, true);
		if (!newGroup)
			newGroup = groupsManager.CreateNewPlayableGroup(group.GetFaction());
				
		if (!newGroup)
			return;
		playerGroupController.RequestJoinGroup(newGroup.GetGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskPromoteLeader(int playerID)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		if (!group.IsPlayerLeader(GetPlayerID()))
			return;
		
		groupsManager.SetGroupLeader(group.GetGroupID(), playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_ChangePrivateGroup(int playerID, bool isPrivate)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		if (!group.IsPlayerLeader(GetPlayerID()))
			return;
		
		groupsManager.SetPrivateGroup(group.GetGroupID(), isPrivate);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestJoinGroup(int groupID)
	{
		Rpc(RPC_AskJoinGroup, groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RPC_DoChangeGroupID(int groupID)
	{
		m_iGroupID = groupID;
		if (groupID == m_iGroupInviteID)
		{
			//reset the invite if player manually joined the group he is invited into
			m_iGroupInviteID = -1;
			m_OnInviteCancelled.Invoke();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskJoinGroup(int groupID)
	{
		// Trying to join the same group, reject.
		if (groupID == m_iGroupID)
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		int groupIDAfter;
		if (m_iGroupID != -1)
			groupIDAfter = groupsManager.MovePlayerToGroup(GetPlayerID(), m_iGroupID, groupID);
		else
			groupIDAfter = groupsManager.AddPlayerToGroup(groupID, GetPlayerID());
		
		if (groupIDAfter != m_iGroupID)
		{
			m_iGroupID = groupIDAfter;
			Rpc(RPC_DoChangeGroupID, groupIDAfter);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool InitiateComponents(int playerID, out SCR_GroupsManagerComponent groupsManager, out SCR_PlayerControllerGroupComponent playerGroupController , out SCR_AIGroup group)
	{
		groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;
		
		playerGroupController = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(playerID);
		if (!playerGroupController)
			return false;
		
		group = groupsManager.GetPlayerGroup(playerID);
		if (!group)
			return false;
		return true;
	}
	//------------------------------------------------------------------------------------------------
	int GetSelectedGroupID()
	{
		return m_iUISelectedGroupID;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnInviteAccepted()
	{
		return m_OnInviteAccepted;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnInviteCancelled()
	{
		return m_OnInviteCancelled;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetGroupInviteID()
	{
		return m_iGroupInviteID;
	}	
	
	//------------------------------------------------------------------------------------------------
	int GetGroupInviteFromPlayerID()
	{
		return m_iGroupInviteFromPlayerID;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetGroupInviteFromPlayerName()
	{
		return m_sGroupInviteFromPlayerName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSelectedGroupID(int groupID)
	{
		m_iUISelectedGroupID = groupID;
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestSetCustomGroupDescription(int groupID, string desc)
	{
		Rpc(RPC_AskSetCustomDescription, groupID, desc);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetCustomDescription(int groupID, string desc)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();

		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		group.SetCustomDescription(desc);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestSetCustomGroupName(int groupID, string name)
	{
		Rpc(RPC_AskSetCustomName, groupID, name);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetCustomName(int groupID, string name)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		group.SetCustomName(name);
	}
	
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		groupsManager.GetOnPlayableGroupRemoved().Insert(OnGroupDeleted);
	}
};
