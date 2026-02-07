[EntityEditorProps(category: "GameScripted/Building", description: "Handles client > server communication in Building. Should be attached to PlayerController.")]
class SCR_CampaignBuildingNetworkComponentClass : ScriptComponentClass
{
}
	
class SCR_CampaignBuildingNetworkComponent : ScriptComponent
{	
	RplComponent m_test;
	//------------------------------------------------------------------------------------------------
	 void CreateEditorMode(int playerID, notnull SCR_FreeCampaignBuildingTrigger trigger)
	{
		RplComponent comp = RplComponent.Cast(trigger.FindComponent(RplComponent));
		if (!comp)
			return;

		Rpc(RpcAsk_CreateEditorMode, playerID, comp.Id());
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveEditorMode(int playerID)
	{
		Rpc(RpcAsk_RemoveEditorMode, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddProviderEditorMode(int playerID, notnull IEntity provider)
	{
		RplComponent comp = RplComponent.Cast(provider.FindComponent(RplComponent));
		if (!comp)
			return;
		
		Rpc(RpcAsk_AddProviderEditorMode, playerID,  comp.Id());
	}
	
	//------------------------------------------------------------------------------------------------
	void AddForcedProviderEditorMode(int playerID, notnull IEntity forcedProvider)
	{
		RplComponent comp = RplComponent.Cast(forcedProvider.FindComponent(RplComponent));
		if (!comp)
			return;
		
		Rpc(RpcAsk_AddForcedProviderEditorMode, playerID,  comp.Id());
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveForcedProviderEditorMode(int playerID)
	{		
		Rpc(RpcAsk_RemoveForcedProviderEditorMode, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveProviderEditorMode(int playerID, notnull IEntity provider)
	{
		RplComponent comp = RplComponent.Cast(provider.FindComponent(RplComponent));
		if (!comp)
			return;
		
		Rpc(RpcAsk_RemoveProviderEditorMode, playerID,  comp.Id());
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeSupply(int supplyValue, IEntity provider, IEntity entityOwner)
	{
		RplComponent comp = RplComponent.Cast(provider.FindComponent(RplComponent));
		if (!comp)
			return;
		
		RplComponent compEntityOwner = RplComponent.Cast(entityOwner.FindComponent(RplComponent));
		if (!compEntityOwner)
			return;
		
		Rpc(RpcAsk_ChangeSupply, supplyValue, comp.Id(), compEntityOwner.Id());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_AddProviderEditorMode(int playerID, RplId rplCompId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplCompId));
		if (!rplComp)
			return;
		
		IEntity provider = rplComp.GetEntity();
		if (!provider)
			return;

		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager(playerID);
		if (!editorManager)
			return;
		
		SCR_EditorModeEntity modeEntity = SCR_EditorModeEntity.Cast(editorManager.FindModeEntity(EEditorMode.BUILDING));
		if (!modeEntity)
			return;
		
		SCR_CampaignBuildingEditorComponent editorComponent = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!editorComponent)
			return;
		
		editorComponent.AddProviderEntityEditorComponent(provider);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_AddForcedProviderEditorMode(int playerID, RplId rplCompId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplCompId));
		if (!rplComp)
			return;
		
		IEntity provider = rplComp.GetEntity();
		if (!provider)
			return;

		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager(playerID);
		if (!editorManager)
			return;
		
		SCR_EditorModeEntity modeEntity = SCR_EditorModeEntity.Cast(editorManager.FindModeEntity(EEditorMode.BUILDING));
		if (!modeEntity)
			return;
		
		SCR_CampaignBuildingEditorComponent editorComponent = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!editorComponent)
			return;
		
		editorComponent.SetForcedProvider(provider);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RemoveForcedProviderEditorMode(int playerID)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager(playerID);
		if (!editorManager)
			return;
		
		SCR_EditorModeEntity modeEntity = SCR_EditorModeEntity.Cast(editorManager.FindModeEntity(EEditorMode.BUILDING));
		if (!modeEntity)
			return;
		
		SCR_CampaignBuildingEditorComponent editorComponent = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!editorComponent)
			return;
		
		editorComponent.SetForcedProvider(null);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RemoveProviderEditorMode(int playerID, RplId rplCompId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplCompId));
		if (!rplComp)
			return;
		
		IEntity provider = rplComp.GetEntity();
		if (!provider)
			return;

		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager(playerID);
		if (!editorManager)
			return;
		
		SCR_EditorModeEntity modeEntity = SCR_EditorModeEntity.Cast(editorManager.FindModeEntity(EEditorMode.BUILDING));
		if (!modeEntity)
			return;
		
		SCR_CampaignBuildingEditorComponent editorComponent = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!editorComponent)
			return;
		
		editorComponent.RemoveProviderEntityEditorComponent(provider);
		
		// if it was a last provider, remove the mode completely. 
		if (editorComponent.GetProviderEntitiesCount() == 0)
			RpcAsk_RemoveEditorMode(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Enable building mode
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CreateEditorMode(int playerID, RplId rplCompId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplCompId));
		if (!rplComp)
			return;
		
		IEntity trigger = rplComp.GetEntity();
		if (!trigger)
			return;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager(playerID);
		if (!editorManager)
			return;
		
		SCR_EditorModeEntity editorMode = editorManager.CreateEditorMode(EEditorMode.BUILDING, false);
		if (!editorMode)
			return;
				
		SCR_CampaignBuildingEditorComponent buildingComponent = SCR_CampaignBuildingEditorComponent.Cast(editorMode.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!buildingComponent)
			return;
		
		IEntity provider = trigger.GetParent();
		if (!provider)
			return;
		
		buildingComponent.AddProviderEntityEditorComponent(provider);
		
		if (!editorManager.IsOpened())
			editorManager.SetCurrentMode(EEditorMode.BUILDING);

		SCR_PlayerController playerControler = SCR_PlayerController.Cast(GetOwner());
		if (!playerControler)
			return;
		
		IEntity ent = playerControler.GetControlledEntity();
		if (!ent)
			return;
		
		SCR_CharacterControllerComponent comp = SCR_CharacterControllerComponent.Cast(ent.FindComponent(SCR_CharacterControllerComponent));
		if (!comp)
			return;
		
		comp.m_OnPlayerDeath.Insert(OnPlayerDeath);
		
		SCR_CampaignSuppliesComponent supplyComp = SCR_CampaignSuppliesComponent.Cast(trigger.GetParent().FindComponent(SCR_CampaignSuppliesComponent));
		if (!supplyComp)
			return;
		supplyComp.m_OnSuppliesTruckDeleted.Insert(OnProviderDeleted);
	}
			
	//------------------------------------------------------------------------------------------------
	//! Disable building mode
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RemoveEditorMode(int playerID)
	{	
 		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager(playerID);
		if (!editorManager)
			return;
		
		SCR_EditorModeEntity editorModeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!editorModeEntity)
			return;
		
		SCR_EntityHelper.DeleteEntityAndChildren(editorModeEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change a supply value on proivder
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ChangeSupply(int supply, RplId rplCompId, RplId rplCompEntityOwnerId)
	{
		RplComponent rplCompEntityOwner = RplComponent.Cast(Replication.FindItem(rplCompEntityOwnerId));
		if (!rplCompEntityOwnerId)
			return;
		
		// if we already send an RPC with this entity to change budget, drop it.
		if (m_test != rplCompEntityOwner)
			m_test = rplCompEntityOwner;
		else
			return;
		
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplCompId));
		if (!rplComp)
			return;
		
		IEntity provider = rplComp.GetEntity();
		if (!provider)
			return;
		
		SCR_CampaignSuppliesComponent supplyComp = SCR_CampaignSuppliesComponent.Cast(provider.FindComponent(SCR_CampaignSuppliesComponent));
		if (!supplyComp)	
			return;
		
		supplyComp.AddSupplies(supply);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerDeath()
	{
		SCR_PlayerController playerControler = SCR_PlayerController.Cast(GetOwner());
		if (!playerControler)
			return;
		
		RpcAsk_RemoveEditorMode(playerControler.GetPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	//ToDo: Don't remove the mode completely, just a provider from the array.
	protected void OnProviderDeleted(IEntity owner)
	{		
		SCR_PlayerController playerControler = SCR_PlayerController.Cast(GetOwner());
		if (!playerControler)
			return;
		
		RpcAsk_RemoveEditorMode(playerControler.GetPlayerId());
	}
}