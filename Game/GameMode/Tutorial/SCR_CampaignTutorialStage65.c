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
		/*m_fWaypointCompletionRadius = 20;
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingCheckpoint", duration: -1, isSilent: true);
		GetGame().GetCallqueue().CallLater(DelayedPopup, 5000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Hint_Bunkers", 12, "", "", "", "");*/
		
		m_bCheckWaypoint = false;
		string hintString = "#AR-Tutorial_Hint_BuildingCheckpoint <h1 align='center' scale='4'><color rgba='34,196,244,255'><image set='{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset' name='Slot_Roadblock'/></color></h1>";
		SCR_HintManagerComponent.ShowCustomHint(hintString, duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(m_Player.FindComponent(SCR_CampaignBuildingComponent));
		SCR_SiteSlotEntity checkpoint_slot = SCR_SiteSlotEntity.Cast(GetGame().GetWorld().FindEntityByName("buildingSlotCheckpoint"));
		
		if (checkpoint_slot.IsOccupied())
			m_TutorialComponent.FinishStage(this, SCR_ECampaignTutorialStage.CONFLICT_BUILDING_QUIT2);	
		else
			return (!buildComp || !buildComp.IsBuilding());
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckBunkerPos"));
		m_TutorialComponent.StageReset_PrepareChotain();
	}
};