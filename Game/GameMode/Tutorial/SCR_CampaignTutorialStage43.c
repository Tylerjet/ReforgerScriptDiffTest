class SCR_CampaignTutorialStage43Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage43 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_TOUR_HQ");
		m_fWaypointCompletionRadius = 5;
		
		string hintString = "#AR-Tutorial_Hint_Antenna <h1 align='center' scale='4'><color rgba='34,196,244,255'><image set='{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset' name='Slot_Antenna'/></color></h1>";
		
		SCR_HintManagerComponent.ShowCustomHint(hintString, "", -1);
	}
};