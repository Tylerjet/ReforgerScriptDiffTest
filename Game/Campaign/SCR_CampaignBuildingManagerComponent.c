[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Base for gamemode scripted component.")]
class SCR_CampaignBuildingManagerComponentClass : SCR_BaseGameModeComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Interface for game mode extending components.
//! Must be attached to a GameMode entity.
class SCR_CampaignBuildingManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab of trigger spawned on server to activate a building mode when player enters its range.", "et")]
	protected ResourceName m_sFreeRoamBuildingServerTrigger;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab of trigger spawned only on clients, to visualize the building area and allow player build only within its radius.", "et")]
	protected ResourceName m_sFreeRoamBuildingClientTrigger;
	
	
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	protected EEditableEntityBudget m_BudgetType;
	
	[Attribute("25",  "Refund percentage", "")]
	protected int m_iCompositionRefundPercentage;	
	protected SCR_EditableEntityCore m_EntityCore;
	protected IEntity m_TemporaryProvider;
	protected RplComponent m_RplComponent;
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetServerTriggerResourceName()
	{
		return m_sFreeRoamBuildingServerTrigger;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetClientTriggerResourceName()
	{
		return m_sFreeRoamBuildingClientTrigger;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTemporaryProvider(IEntity ent)
	{
		m_TemporaryProvider = ent;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetTemporaryProvider()
	{
		return m_TemporaryProvider;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetProviderEntity(IEntity ownerEntity, out SCR_CampaignSuppliesComponent suppliesComponent)
	{
		SCR_CampaignBuildingCompositionComponent campaignCompositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(ownerEntity.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!campaignCompositionComponent)
			return false;
		
		IEntity providerEntity;
		providerEntity = campaignCompositionComponent.GetProviderEntity();
		if (!providerEntity)
		{			
			providerEntity = GetTemporaryProvider();
			SetTemporaryProvider(null);
		}

		if (!providerEntity)
			return false;
		
		suppliesComponent = SCR_CampaignSuppliesComponent.Cast(providerEntity.FindComponent(SCR_CampaignSuppliesComponent));
		return suppliesComponent != null;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get supplies from the composition cost that gets refunded on removal of composition
	\return Percentage of supplies refunded
	*/
	int GetCompositionRefundPercentage()
	{
		return m_iCompositionRefundPercentage;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntityCoreBudgetUpdated(EEditableEntityBudget entityBudget, int originalBudgetValue, int budgetChange, int updatedBudgetValue, SCR_EditableEntityComponent entity)
	{
		if (IsProxy())
			return;
		
		if (entityBudget != m_BudgetType)
			return;
		
		IEntity entityOwner = entity.GetOwnerScripted();
		SCR_CampaignSuppliesComponent suppliesComponent;
		IEntity providerEntity;
		if (!GetProviderEntity(entityOwner, suppliesComponent))
			return;
		
		if (budgetChange < 0)
			budgetChange = Math.Round(budgetChange * (m_iCompositionRefundPercentage / 100));

		suppliesComponent.AddSupplies(-budgetChange);
	}
		
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (GetGameMode().IsMaster())
			m_EntityCore.Event_OnEntityBudgetUpdated.Insert(OnEntityCoreBudgetUpdated);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (m_EntityCore && GetGameMode().IsMaster())
			m_EntityCore.Event_OnEntityBudgetUpdated.Remove(OnEntityCoreBudgetUpdated);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------	
	void GetEditorMode(int playerID, notnull IEntity provider, bool UserActionActivationOnly = false, bool UserActionUsed = false)
	{		
		SCR_EditorManagerEntity editorManager = GetEditorManagerEntity(playerID);
		if (!editorManager)
			return;
		
		SCR_EditorModeEntity modeEntity = SCR_EditorModeEntity.Cast(editorManager.FindModeEntity(EEditorMode.BUILDING));
		if (!modeEntity)
			modeEntity = editorManager.CreateEditorMode(EEditorMode.BUILDING, false);
		
		if (!modeEntity)
			return;
				
		SetEditorMode(editorManager, modeEntity, playerID, provider, UserActionActivationOnly, UserActionUsed);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void SetEditorMode(notnull SCR_EditorManagerEntity editorManager, notnull SCR_EditorModeEntity modeEntity, int playerID, notnull IEntity provider, bool userActionActivationOnly = false, bool userActionUsed = false)
	{
		SCR_CampaignBuildingEditorComponent buildingComponent = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!buildingComponent)
			return;
				
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;
		
		providerComponent.AddNewAvailableUser(playerID);
		if (userActionUsed)
			providerComponent.AddNewActiveUser(playerID);
		
		buildingComponent.AddProviderEntityEditorComponent(providerComponent);
				
		if (!editorManager.IsOpened())
			editorManager.SetCurrentMode(EEditorMode.BUILDING);
		
		if (userActionActivationOnly || userActionUsed)
			ToggleEditorMode(editorManager);
				
		// events
		SetOnPlayerDeathEvent(playerID);
		SetOnProviderDestroyedEvent(provider);
		providerComponent.SetCheckProviderMove();
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void SetOnPlayerDeathEvent(int playerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return;
		
		SCR_CharacterControllerComponent comp = SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
		if (!comp)
			return;
		
		comp.m_OnPlayerDeathWithParam.Insert(OnPlayerDeath);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void RemoveOnPlayerDeathEvent(int playerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return;
		
		SCR_CharacterControllerComponent comp = SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
		if (!comp)
			return;
		
		comp.m_OnPlayerDeathWithParam.Remove(OnPlayerDeath);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetOnProviderDestroyedEvent(IEntity provider)
	{		
		SCR_HitZoneContainerComponent hitZoneContainerComponent = SCR_HitZoneContainerComponent.Cast(provider.FindComponent(SCR_HitZoneContainerComponent));
		if (!hitZoneContainerComponent)
			return;
		
		ScriptedHitZone zone = ScriptedHitZone.Cast(hitZoneContainerComponent.GetDefaultHitZone());
		if (zone)
			zone.GetOnDamageStateChanged().Insert(OnProviderDestroyed);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveOnProviderDestroyedEvent(IEntity provider)
	{		
		SCR_HitZoneContainerComponent hitZoneContainerComponent = SCR_HitZoneContainerComponent.Cast(provider.FindComponent(SCR_HitZoneContainerComponent));
		if (!hitZoneContainerComponent)
			return;
		
		ScriptedHitZone zone = ScriptedHitZone.Cast(hitZoneContainerComponent.GetDefaultHitZone());
		if (zone)
			zone.GetOnDamageStateChanged().Remove(OnProviderDestroyed);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void OnPlayerDeath(SCR_CharacterControllerComponent characterController)
	{
		if (!characterController)
			return;
		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(characterController.GetOwner());
		
		SCR_EditorManagerEntity editorManager = GetEditorManagerEntity(playerID);
		if (!editorManager)
			return;
		
		if (editorManager.IsOpened())
			ToggleEditorMode(editorManager);
		
		RemoveEditorMode(playerID);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Method called when the provider was destroyed.
	protected void OnProviderDestroyed(ScriptedHitZone hitZone)
	{
		if (!hitZone)
			return;
		
		if (hitZone.GetDamageState() != EDamageState.DESTROYED)
			return;
						
		IEntity provider = hitZone.GetOwner();
		if (!provider)
			return;
		
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;
		
		array<int> playersIDs = {};
		providerComponent.GetAvailableUsers(playersIDs);
		foreach (int playerId : playersIDs)
		{
			if (providerComponent.ContainActiveUsers(playerId))
			{
				RemoveProvider(playerId, providerComponent, true);
				providerComponent.RemoveActiveUser(playerId);
			}
			
			providerComponent.RemoveAvailableUser(playerId);
		}
	}
		
	//------------------------------------------------------------------------------------------------	
	// removes player Id from list of active and avalilable users, return true if the user was on the list of active users
	bool RemovePlayerIdFromProvider(int playerID, SCR_CampaignBuildingProviderComponent providerComponent)
	{				
		bool isActiveUser = providerComponent.ContainActiveUsers(playerID);
			
		providerComponent.RemoveActiveUser(playerID);
		providerComponent.RemoveAvailableUser(playerID);
		
		return isActiveUser;
	}
	
	//------------------------------------------------------------------------------------------------	
	void RemoveProvider(int playerID, SCR_CampaignBuildingProviderComponent providerComponent, bool isActiveUser)
	{
		SCR_EditorManagerEntity editorManager = GetEditorManagerEntity(playerID);
		if (!editorManager)
			return;
		
		SCR_EditorModeEntity modeEntity = SCR_EditorModeEntity.Cast(editorManager.FindModeEntity(EEditorMode.BUILDING));
		if (!modeEntity)
			return;
		
		SCR_CampaignBuildingEditorComponent editorComponent = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!editorComponent)
			return;
		
		if (isActiveUser && editorManager.IsOpened())
			editorManager.Close();
		
		editorComponent.RemoveProviderEntityEditorComponent(providerComponent);
		
		IEntity provider = providerComponent.GetOwner();
		if (provider)
			RemoveOnProviderDestroyedEvent(providerComponent.GetOwner());
		
		providerComponent.RemoveCheckProviderMove();
		RemoveOnPlayerDeathEvent(playerID);
		
		// if it was a last provider and forced provider doesn't exist, remove the mode completely. 
		if (!editorComponent.GetProviderComponent())
			RemoveEditorMode(playerID);
	}
		
	//------------------------------------------------------------------------------------------------
	protected void ToggleEditorMode(notnull SCR_EditorManagerEntity editorManager)
	{		
		editorManager.Toggle();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveEditorMode(int playerID)
	{	
		SCR_EditorManagerEntity editorManager = GetEditorManagerEntity(playerID);
		if (!editorManager)
			return;
		
		SCR_EditorModeEntity editorModeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!editorModeEntity)
			return;

		SCR_EntityHelper.DeleteEntityAndChildren(editorModeEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_EditorManagerEntity GetEditorManagerEntity(int playerID)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;
		
		return core.GetEditorManager(playerID);
	}
};
