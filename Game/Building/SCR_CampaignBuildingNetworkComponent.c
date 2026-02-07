[EntityEditorProps(category: "GameScripted/Building", description: "Handles client > server communication in Building. Should be attached to PlayerController.")]
class SCR_CampaignBuildingNetworkComponentClass : ScriptComponentClass
{
}
	
class SCR_CampaignBuildingNetworkComponent : ScriptComponent
{	
	//------------------------------------------------------------------------------------------------
	void RemoveEditorMode(int playerID, IEntity provider)
	{
		RplComponent comp = RplComponent.Cast(provider.FindComponent(RplComponent));
		if (!comp)
			return;
		
		Rpc(RpcAsk_RemoveEditorMode, playerID, comp.Id());
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestEnterBuildingMode(IEntity provider, int playerID, bool UserActionActivationOnly, bool UserActionUsed)
	{
		RplComponent providerRplComp = RplComponent.Cast(provider.FindComponent(RplComponent));
		if (!providerRplComp)
			return;
		
		Rpc(RpcAsk_RequestEnterBuildingMode, providerRplComp.Id(), playerID, UserActionActivationOnly, UserActionUsed);
	}
					
	//------------------------------------------------------------------------------------------------
	//! Delete given player building mode.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RemoveEditorMode(int playerID, RplId compId)
	{			
		IEntity provider = GetProviderFormRplId(compId);
		if (!provider)
			return;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;
		
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;
		
		bool isActiveUser = buildingManagerComponent.RemovePlayerIdFromProvider(playerID, providerComponent);
		buildingManagerComponent.RemoveProvider(playerID, providerComponent, isActiveUser);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Spawn a trigger as a child of the provider entity.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RequestEnterBuildingMode(RplId rplProviderId, int playerID, bool UserActionActivationOnly, bool UserActionUsed)
	{
		IEntity provider = GetProviderFormRplId(rplProviderId);
		if (!provider)
			return;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;
		
		buildingManagerComponent.GetEditorMode(playerID, provider, UserActionActivationOnly, UserActionUsed);
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetProviderFormRplId(RplId rplProviderId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplProviderId));
		if (!rplComp)
			return null;
		
		return rplComp.GetEntity();
	}
}