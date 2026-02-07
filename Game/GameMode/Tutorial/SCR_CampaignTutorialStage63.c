class SCR_CampaignTutorialStage63Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage63 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_BUILD_BUNKER");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		GetGame().GetCallqueue().CallLater(DelayedPopup, 20000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_BlockedSlots", 12, "", "", "", "");
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingBunker" + CreateString("#AR-ControlsHint_BuildingCompositionRotate","BuildingPreviewRotationUp","BuildingPreviewRotationDown"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		SCR_SiteSlotEntity bunker_slot = SCR_SiteSlotEntity.Cast(GetGame().GetWorld().FindEntityByName("buildingSlot"));	
						
		if (bunker_slot && bunker_slot.IsOccupied())
		{
			m_TutorialComponent.FinishStage(this, SCR_ECampaignTutorialStage.CONFLICT_BUILDING_QUIT);
		}
		else
		{
			SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(m_Player.FindComponent(SCR_CampaignBuildingComponent));
			
			if (buildComp && !buildComp.IsBuilding())
				m_TutorialComponent.FinishStage(this, SCR_ECampaignTutorialStage.CONFLICT_TURN_ON_BUILDING);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckBunkerPos"));
		m_TutorialComponent.StageReset_PrepareChotain();
	}
};