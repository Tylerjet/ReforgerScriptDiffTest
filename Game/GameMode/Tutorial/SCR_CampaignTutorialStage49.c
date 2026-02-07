class SCR_CampaignTutorialStage49Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage49 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_REQUESTING_TRUCK");
		m_fConditionCheckPeriod = 1;
		m_fWaypointHeightOffset = 1.2;
		m_bCheckWaypoint = false;
		SCR_CampaignMilitaryBaseComponent mainBaseUS = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("MainBaseChotain").FindComponent(SCR_CampaignMilitaryBaseComponent));
		mainBaseUS.AlterSupplyIncomeTimer(-float.MAX);
		mainBaseUS.AddSupplies(1000);
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_RequestingVehicle" + CreateString("#AR-KeybindEditor_MultiSelection","SelectAction"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		IEntity nearbyEntities;
		GetGame().GetWorld().QueryEntitiesBySphere(m_Player.GetOrigin(), 30, m_TutorialComponent.FindSupplyTruck, null, EQueryEntitiesFlags.DYNAMIC);
		return (m_TutorialComponent.GetSupplyTruckComponent() != null);
	}
};