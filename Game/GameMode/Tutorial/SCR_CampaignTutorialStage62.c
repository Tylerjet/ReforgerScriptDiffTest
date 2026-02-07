class SCR_CampaignTutorialStage62Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage62 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_BOARD_TRUCK");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BoardTruck", duration: -1);
		m_TutorialComponent.SetWaypointTruckPosition(SCR_ETutorialSupplyTruckWaypointMode.DRIVER);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterInVehicle(ECompartmentType.Pilot);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("WP_DRIVING_15"));
		m_TutorialComponent.StageReset_PrepareChotain();
	}
};