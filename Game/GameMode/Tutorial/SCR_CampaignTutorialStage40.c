class SCR_CampaignTutorialStage40Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage40 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_TOUR_ARMORY");
		m_fWaypointCompletionRadius = 5;
		GetGame().GetCallqueue().Remove(SCR_HintManagerComponent.ShowCustomHint);
		GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowCustomHint, 1500, false, "#AR-Tutorial_Hint_MainBase", "", -1, false, EFieldManualEntryId.NONE, false);
	}
};