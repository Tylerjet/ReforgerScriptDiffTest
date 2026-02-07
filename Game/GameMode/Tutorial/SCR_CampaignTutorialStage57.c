class SCR_CampaignTutorialStage57Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage57 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_15");
		m_fWaypointCompletionRadius = 20;
		m_fWaypointHeightOffset = 0;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SuppliesUnload", duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(m_WaypointEntity);
		m_TutorialComponent.StageReset_ProcessTruck();
	}
};