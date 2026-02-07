[EntityEditorProps(category: "Campaign/BuildingManagerEntity", description: "Building manager entity")]
class SCR_CampaignBuildingManagerEntityClass : GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingManagerEntity : GenericEntity
{

	protected static SCR_CampaignBuildingManagerEntity s_BuildingManagerEnt;
	
	[Attribute("1", UIWidgets.EditBox, "Composition cost multiplier"), RplProp()] 
	private float m_fCostMultiplier;
	
	[Attribute("1", UIWidgets.EditBox, "Composition refund multiplier"), RplProp()] 
	private float m_fRefundMultiplier;
	
	//------------------------------------------------------------------------------------------------
	static SCR_CampaignBuildingManagerEntity GetInstance()
	{
		return s_BuildingManagerEnt;
	}
	
	//------------------------------------------------------------------------------------------------
	// Set the composition cost multiplier
	void SetCostMultiplier(float costMultiplier)
	{
		m_fCostMultiplier = costMultiplier;
		
		Replication.BumpMe();
		RpcDo_UpdateCostMultiplier(m_fCostMultiplier);
		Rpc(RpcDo_UpdateCostMultiplier, m_fCostMultiplier);
	}
	
	//------------------------------------------------------------------------------------------------
	// Set the composition refund multiplier
	void SetRefundMultiplier(float refundMultiplier)
	{
		m_fRefundMultiplier = refundMultiplier;
		
		Replication.BumpMe();
		RpcDo_UpdateRefundMultiplier(m_fRefundMultiplier);
		Rpc(RpcDo_UpdateRefundMultiplier, m_fRefundMultiplier);
	}

	//------------------------------------------------------------------------------------------------
	// Get the composition cost multiplier
	float GetCostMultiplier()
	{
		return m_fCostMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	// Get the composition refund multiplier
	float GetRefundMultiplier()
	{
		return m_fRefundMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	// Send RPC to all clients to update a cost multiplier
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_UpdateCostMultiplier(float costMultiplier)
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)	
			return;
		
		SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(player.FindComponent(SCR_CampaignBuildingComponent));
		if (buildComp)
			buildComp.SetCostMultiplier(costMultiplier);
		
		//Notification
		int notificationInt = costMultiplier * 1000;
		SCR_NotificationsComponent.SendLocal(ENotification.BUILD_COST_MULTI_CHANGED, notificationInt);
	}
	
	//------------------------------------------------------------------------------------------------
	// Send RPC to all clients to update a refund multiplier
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_UpdateRefundMultiplier(float costMultiplier)
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)	
			return;
		
		SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(player.FindComponent(SCR_CampaignBuildingComponent));
		if (buildComp)
			buildComp.SetRefundMultiplier(costMultiplier);
		
		//Notification
		int notificationInt = costMultiplier * 1000;
		SCR_NotificationsComponent.SendToEveryone(ENotification.BUILD_REFUND_MULTI_CHANGED, notificationInt);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignBuildingManagerEntity(IEntitySource src, IEntity parent)
	{
		SetFlags(EntityFlags.ACTIVE, true);
		s_BuildingManagerEnt = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignBuildingManagerEntity()
	{
	}

};
