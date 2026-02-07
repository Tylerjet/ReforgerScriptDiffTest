class SCR_CampaignTutorialStage65Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage65 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_BUILDING_QUIT");
		m_fWaypointHeightOffset = 0;
		m_fWaypointCompletionRadius = 20;
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingCheckpoint", duration: -1, isSilent: true);
		GetGame().GetCallqueue().CallLater(DelayedPopup, 5000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Hint_Bunkers", 12, "", "", "", "");
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckBunkerPos"));
		m_TutorialComponent.StageReset_PrepareChotain();
	}
};