class SCR_CampaignTutorialStage67Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage67 : SCR_BaseCampaignTutorialStage
{
	//protected int m_iTruckSupplies;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_BUILDING_CHECKPOINT");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		/*m_fConditionCheckPeriod = 1;
		
		GetGame().GetCallqueue().Remove(OverrideGMHint);
		GetGame().GetCallqueue().CallLater(OverrideGMHint, 250, true);*/
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingCheckpointActual", duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		/*SCR_CampaignSuppliesComponent comp = m_TutorialComponent.GetSupplyTruckComponent();
		
		if (!comp)
			return false;
		
		if (m_iTruckSupplies == 0)
			m_iTruckSupplies = comp.GetSupplies();
		
		return comp.GetSupplies() < m_iTruckSupplies;*/
		
		SCR_SiteSlotEntity checkpoint_slot = SCR_SiteSlotEntity.Cast(GetGame().GetWorld().FindEntityByName("buildingSlotCheckpoint"));	
		return checkpoint_slot.IsOccupied();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckCheckpointPos"));
		m_TutorialComponent.GetSupplyTruckComponent().AddSupplies(500);
	}
	
	//------------------------------------------------------------------------------------------------
	/*void OverrideGMHint()
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingCheckpointActual", duration: -1, isSilent: true);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTutorialStage67()
	{
		GetGame().GetCallqueue().Remove(OverrideGMHint);
	}*/
};