class SCR_CampaignTutorialStage52Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage52 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_10");
		m_fWaypointCompletionRadius = 20;
		m_fWaypointHeightOffset = 0;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SuppliesDrive", duration: 10);
		GetGame().GetCallqueue().CallLater(DelayedPopup, 10500, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Reinforcements", 15, "", "", "", "");
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