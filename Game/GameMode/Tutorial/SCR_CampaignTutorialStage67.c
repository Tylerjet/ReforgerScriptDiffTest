class SCR_CampaignTutorialStage67Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage67 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_BUILDING_CHECKPOINT");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingCheckpointActual", duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		SCR_SiteSlotEntity checkpoint_slot = SCR_SiteSlotEntity.Cast(GetGame().GetWorld().FindEntityByName("buildingSlotCheckpoint"));	
		return checkpoint_slot.IsOccupied();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckCheckpointPos"));
		m_TutorialComponent.GetSupplyTruckComponent().AddSupplies(500);
	}
};