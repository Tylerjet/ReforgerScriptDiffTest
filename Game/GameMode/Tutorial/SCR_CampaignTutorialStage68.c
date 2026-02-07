class SCR_CampaignTutorialStage68Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage68 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_BUILDING_QUIT");
		m_bCheckWaypoint = false;
		m_TutorialComponent.SetWaypointTruckPosition(SCR_ETutorialSupplyTruckWaypointMode.BACK);
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildModeClose_GM" + CreateString("#AR-KeybindEditor_Interface", "EditorToggle"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !IsBuildingModeOpen();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckCheckpointPos"));
	}
};