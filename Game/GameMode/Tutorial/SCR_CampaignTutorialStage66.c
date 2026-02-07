class SCR_CampaignTutorialStage66Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage66 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_BUILDING_TO_FAR");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		m_TutorialComponent.SetWaypointTruckPosition(SCR_ETutorialSupplyTruckWaypointMode.BACK);
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildModeOpenTruck" + CreateString("#AR-KeybindEditor_Interface", "EditorToggle"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		/*SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		
		if (!core)
			return false;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		
		if (!editorManager)
		    return false;
		
		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		
		if (!modeEntity)
		    return false;
		
		return modeEntity.IsOpened();*/
		
		SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(m_Player.FindComponent(SCR_CampaignBuildingComponent));
		return buildComp.IsBuilding();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckBunkerPos"));
		m_TutorialComponent.StageReset_PrepareChotain();
	}
};