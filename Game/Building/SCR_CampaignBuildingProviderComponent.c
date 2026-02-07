[EntityEditorProps(category: "GameScripted/Building", description: "Component attached to a provider, responsible for basic provider behaviour.")]
class SCR_CampaignBuildingProviderComponentClass : ScriptComponentClass
{

};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingProviderComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.CheckBox, "Can the building mode at this provider executed only via user action?")]
	protected bool m_bUserActionActivationOnly;

	[Attribute("50", UIWidgets.EditBox, "Building radius")]
	protected float m_fBuildingRadius;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Minimal rank that allows player to use the provider to build structures.", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_iRank;

	protected vector m_vProviderOrigin;
	protected vector m_vNewProviderOrigin;
	protected bool m_bProviderIsMoving;
	protected bool m_bProviderMoveCheckRunning;

	protected ref array<int> m_aActiveUsersIDs = {};
	protected ref array<int> m_aAvailableUsersIDs = {};

	protected static ref ScriptInvoker s_OnProviderCreated = new ScriptInvoker();

	protected const float MOVING_CHECK_DISTANCE_SQ = 10;
	protected const float STOPPING_CHECK_DISTANCE_SQ = 0.5;
	protected const int MOVING_CHECK_PERIOD = 1000;
	protected const int STOPPING_CHECK_PERIOD = 2000;

	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetAccessRank()
	{
		return m_iRank;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetBuildingRadius()
	{
		return m_fBuildingRadius;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (System.IsConsoleApp() || !GetGame().GetPlayerController())
			SetOnProviderFactionChangedEvent();
	}

	//------------------------------------------------------------------------------------------------
	//! Add an ID of a user on the list of those who are currently in use of provider
	void AddNewActiveUser(int userID)
	{
		m_aActiveUsersIDs.Insert(userID);
		SetOnPlayerDeathActiveUserEvent(userID);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove an ID of a user from the list of those who are currently in use of provider
	void RemoveActiveUser(int userID)
	{
		m_aActiveUsersIDs.RemoveItem(userID);
	}

	//------------------------------------------------------------------------------------------------
	//! Return the array of users id and it's count.
	int GetActiveUsers(out notnull array<int> users)
	{
		users.Copy(m_aActiveUsersIDs);
		return users.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool ContainActiveUsers(int playerId)
	{
		return m_aActiveUsersIDs.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void AddNewAvailableUser(int userID)
	{
		m_aAvailableUsersIDs.Insert(userID);
		SetOnPlayerDeathAvailableUserEvent(userID);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveAvailableUser(int userID)
	{
		m_aAvailableUsersIDs.RemoveItem(userID);
	}

	//------------------------------------------------------------------------------------------------
	int GetAvailableUsers(out array<int> users)
	{
		users.Copy(m_aAvailableUsersIDs);
		return users.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool ContainAvailableUsers(int playerId)
	{
		return m_aAvailableUsersIDs.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Requesting a building mode. If trigger exist (was spawned with provider, because "Y" can be used to enter the mode, it will open the mode directly. If not, it 1st spawn the building area trigger.
	void RequestBuildingMode(int playerID, bool UserActionUsed)
	{
		RequestEnterBuildingMode(playerID, UserActionUsed);
	}

	//------------------------------------------------------------------------------------------------
	void RequestEnterBuildingMode(int playerID, bool UserActionUsed)
	{
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		SetOnPlayerConsciousnessChanged(playerID);
		SetOnPlayerTeleported(playerID);
		editorManager.GetOnOpened().Insert(BuildingModeCreated);
		networkComponent.RequestEnterBuildingMode(GetOwner(), playerID, m_bUserActionActivationOnly, UserActionUsed);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOnPlayerTeleported(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;	
		
		SCR_PlayerTeleportedFeedbackComponent playerTeleportComponent = SCR_PlayerTeleportedFeedbackComponent.Cast(playerController.FindComponent(SCR_PlayerTeleportedFeedbackComponent));
		if (!playerTeleportComponent)
			return;
		
		playerTeleportComponent.GetOnPlayerTeleported().Insert(PlayerTeleported);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOnPlayerConsciousnessChanged(int playerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return;
		
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(player.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
			eventHandlerManager.RegisterScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged, false, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnConsciousnessChanged(bool conscious)
	{
		if (conscious)
			return;
		
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		networkComponent.RemoveEditorMode(SCR_PlayerController.GetLocalPlayerId(), GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayerTeleported(SCR_EditableCharacterComponent character, bool isLongFade, SCR_EPlayerTeleportedReason teleportReason)
	{
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		networkComponent.RemoveEditorMode(SCR_PlayerController.GetLocalPlayerId(), GetOwner());
		
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveOnConsciousnessChanged()
	{
		IEntity ent = SCR_PlayerController.GetLocalControlledEntity();
		if (!ent)
			return;
		
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(ent.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
			eventHandlerManager.RemoveScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged, false);
	}
	
	
	//------------------------------------------------------------------------------------------------
	// Insert a method called when the provider faction is changed. For an example base is taken by an enemy.
	void SetOnProviderFactionChangedEvent()
	{
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
		if (!factionComponent)
			return;

		factionComponent.GetOnFactionChanged().Insert(OnBaseOwnerChanged);
	}

	//------------------------------------------------------------------------------------------------
	void SetOnPlayerDeathActiveUserEvent(int userID)
	{
		SCR_CharacterControllerComponent comp = GetCharacterControllerComponent(userID);
		if (!comp)
			return;

		comp.m_OnPlayerDeathWithParam.Insert(OnActiveUserDeath);
	}

	//------------------------------------------------------------------------------------------------
	void SetOnPlayerDeathAvailableUserEvent(int userID)
	{
		SCR_CharacterControllerComponent comp = GetCharacterControllerComponent(userID);
		if (!comp)
			return;

		comp.m_OnPlayerDeathWithParam.Insert(OnAvailableUserDeath);
	}

	//------------------------------------------------------------------------------------------------
	void OnActiveUserDeath(SCR_CharacterControllerComponent characterControllerComponent)
	{
		RemoveActiveUser(GetPlayerIdFromCharacterController(characterControllerComponent));
	}

	//------------------------------------------------------------------------------------------------
	void OnAvailableUserDeath(SCR_CharacterControllerComponent characterControllerComponent)
	{
		RemoveAvailableUser(GetPlayerIdFromCharacterController(characterControllerComponent));
	}

	//------------------------------------------------------------------------------------------------
	void BuildingModeCreated()
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		editorManager.GetOnOpened().Remove(BuildingModeCreated);
		editorManager.GetOnClosed().Insert(OnModeClosed);
	}

	//------------------------------------------------------------------------------------------------
	void OnModeClosed()
	{
		RemoveOnModeClosed();
		RemoveOnConsciousnessChanged();
		RemoveOnPlayerTeleported();
		
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		networkComponent.RemoveEditorMode(SCR_PlayerController.GetLocalPlayerId(), GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnProviderCreated()
	{
		return s_OnProviderCreated;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveOnModeClosed()
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		editorManager.GetOnClosed().Remove(OnModeClosed);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveOnPlayerTeleported()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;	
		
		SCR_PlayerTeleportedFeedbackComponent playerTeleportComponent = SCR_PlayerTeleportedFeedbackComponent.Cast(playerController.FindComponent(SCR_PlayerTeleportedFeedbackComponent));
		if (!playerTeleportComponent)
			return;
		
		playerTeleportComponent.GetOnPlayerTeleported().Remove(PlayerTeleported);
	}

	//------------------------------------------------------------------------------------------------
	bool GetUserActionInitOnly()
	{
		return m_bUserActionActivationOnly;
	}

	//------------------------------------------------------------------------------------------------
	//! GetNetworkManager
	protected SCR_CampaignBuildingNetworkComponent GetNetworkManager()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return null;

		return SCR_CampaignBuildingNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignBuildingNetworkComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Get Local Editor manager
	protected SCR_EditorManagerEntity GetEditorManager()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		return core.GetEditorManager();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Editor manager by player ID
	protected SCR_EditorManagerEntity GetEditorManagerByID(int playerId)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		return core.GetEditorManager(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CharacterControllerComponent GetCharacterControllerComponent(int playerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return null;

		return SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected int GetPlayerIdFromCharacterController(SCR_CharacterControllerComponent characterControllerComponent)
	{
		if (!characterControllerComponent)
			return -1;

		IEntity ent = characterControllerComponent.GetOwner();
		if (!ent)
			return -1;

		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
	}

	//------------------------------------------------------------------------------------------------
	//! Does player faction match the provider faction
	// Method called by na user action to make sure it can be shown only to the players of the same faction as the base belong to.
	bool IsPlayerFactionSame(notnull IEntity player)
	{
		Faction playerFaction = GetEntityFaction(player);
		if (!playerFaction)
			return false;

		IEntity provider = SCR_EntityHelper.GetMainParent(GetOwner(), true);
		if (!provider)
			return false;

		Faction owningFaction = GetEntityFaction(provider);
		if (!owningFaction)
			return false;

		return playerFaction == owningFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	protected Faction GetEntityFaction(notnull IEntity ent)
	{
		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		if (!factionComp)
			return null;

		Faction faction = factionComp.GetAffiliatedFaction();
		if (!faction)
			faction = factionComp.GetDefaultAffiliatedFaction();

		return faction;
	}

	//------------------------------------------------------------------------------------------------
	//! Method triggered when owning faction of provider has changed.
	protected void OnBaseOwnerChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction)
	{
		// ToDo: In the future if the trigger activation will be enabled again, here I have to use a trigger range to find all entities which should be added and remove those on the list now.
		RemoveActiveUsers();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns provider supply component
	SCR_CampaignSuppliesComponent GetSuppliesComponent()
	{
		return SCR_CampaignSuppliesComponent.Cast(GetOwner().FindComponent(SCR_CampaignSuppliesComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when the provider was destroyed or deleted to remove a provider.
	void RemoveActiveUsers()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		bool isActiveUser;
		foreach (int playerId : m_aAvailableUsersIDs)
		{
			isActiveUser = false;

			SCR_EditorManagerEntity editorManager = GetEditorManagerByID(playerId);
			if (!editorManager)
				return;
			
			SCR_EditorModeEntity modeEntity = SCR_EditorModeEntity.Cast(editorManager.FindModeEntity(EEditorMode.BUILDING));
			if (!modeEntity)
				return;
						
			if (m_aActiveUsersIDs.Contains(playerId))
			{
				RemoveActiveUser(playerId);
				isActiveUser = true;
			}

			buildingManagerComponent.RemoveProvider(playerId, this, isActiveUser);
			RemoveAvailableUser(playerId);
		}

		RemoveCheckProviderMove();
	}

	//------------------------------------------------------------------------------------------------
	void RemoveCheckProviderMove()
	{
		if (IsProviderDynamic())
			GetGame().GetCallqueue().Remove(CheckProviderMove);
	}

	//------------------------------------------------------------------------------------------------
	void SetCheckProviderMove()
	{
		if (IsProviderDynamic())
			GetGame().GetCallqueue().CallLater(CheckProviderMove, MOVING_CHECK_PERIOD, true);
	}

	//------------------------------------------------------------------------------------------------
	void CheckProviderMove()
	{
		m_vNewProviderOrigin = GetOwner().GetOrigin();
		if (m_vProviderOrigin == vector.Zero)
		{
			m_vProviderOrigin = m_vNewProviderOrigin;
			return;
		}

		if (vector.DistanceSqXZ(m_vNewProviderOrigin, m_vProviderOrigin) > MOVING_CHECK_DISTANCE_SQ)
		{
			RemoveActiveUsers();
		}

		m_vProviderOrigin = m_vNewProviderOrigin;
	}

	//------------------------------------------------------------------------------------------------
	private bool IsProviderDynamic()
	{
		Physics ph = GetOwner().GetPhysics();
		return ph && ph.IsDynamic();
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		s_OnProviderCreated.Invoke();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Destructor
	void ~SCR_CampaignBuildingProviderComponent()
	{		
		if (!m_aAvailableUsersIDs.IsEmpty())
			RemoveActiveUsers();
	}
};
