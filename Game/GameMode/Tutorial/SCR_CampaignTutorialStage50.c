class SCR_CampaignTutorialStage50Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage50 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_SUPPLY_DEPOT");
		m_fWaypointCompletionRadius = 10;
		m_fWaypointHeightOffset = 0;
		
		string hintString = "#AR-Tutorial_Hint_SupplyDepot <h1 align='center' scale='4'><color rgba='34,196,244,255'><image set='{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset' name='Slot_Supplies'/></color></h1>";
		
		SCR_HintManagerComponent.ShowCustomHint(hintString, duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck();
	}
};