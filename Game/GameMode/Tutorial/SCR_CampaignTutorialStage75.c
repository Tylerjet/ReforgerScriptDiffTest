class SCR_CampaignTutorialStage75Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage75 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_18");
		m_fWaypointCompletionRadius = 20;
		m_fWaypointHeightOffset = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(m_WaypointEntity);
		m_TutorialComponent.MoveInVehicle("SupplyTruck", ECompartmentType.Pilot);
	}
};