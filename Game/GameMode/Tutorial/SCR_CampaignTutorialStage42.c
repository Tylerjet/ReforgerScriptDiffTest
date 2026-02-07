class SCR_CampaignTutorialStage42Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage42 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_TOUR_ANTENNA");
		m_fWaypointCompletionRadius = 5;
		
		//string hintString = "#AR-Tutorial_Hint_Barracks_Alternative <h1 align='center' scale='4'><color rgba='34,196,244,255'><image set='{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset' name='Slot_Barracks'/></color></h1>";
		string hintString = "#AR-Tutorial_Hint_Barracks <h1 align='center' scale='4'><color rgba='34,196,244,255'><image set='{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset' name='Slot_Barracks'/></color></h1>";
	
		SCR_HintManagerComponent.ShowCustomHint(hintString, "", -1);
	}
};