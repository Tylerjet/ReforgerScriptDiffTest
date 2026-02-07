class SCR_CampaignTutorialStage51Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage51 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_SUPPLY_DEPOT");
		m_bCheckWaypoint = false;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SuppliesLoad", duration: -1);
		m_TutorialComponent.SetWaypointTruckPosition(SCR_ETutorialSupplyTruckWaypointMode.BACK);
		SCR_CampaignBase mainBaseUS = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("MainBaseChotain"));
		mainBaseUS.AddSupplies(2000);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.GetSupplyTruckComponent().GetSupplies() == m_TutorialComponent.GetSupplyTruckComponent().GetSuppliesMax();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckDepotPos"));
	}
};