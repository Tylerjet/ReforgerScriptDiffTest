class SCR_CampaignTutorialStage59Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage59 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_BUILDING_START");
		m_fWaypointHeightOffset = 1.3;
		m_bCheckWaypoint = false;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildModeOpen", duration: -1);
		IEntity truck = IEntity.Cast(GetGame().GetWorld().FindEntityByName("EastSupplyTruck"));	
		
		if (!truck)
			return;
		
		array<EntitySlotInfo> slotInfos = {};
		EntitySlotInfo.GetSlotInfos(truck, slotInfos);
		
		foreach (EntitySlotInfo slotInfo: slotInfos)
		{
			IEntity slotEntity = slotInfo.GetAttachedEntity();
			
			if (!slotEntity)
				continue;
			
			SCR_CampaignSuppliesComponent supplyComp = SCR_CampaignSuppliesComponent.Cast(slotEntity.FindComponent(SCR_CampaignSuppliesComponent));
			
			if (supplyComp)
			{
				supplyComp.AddSupplies(500);
				continue;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return IsBuildingModeOpen();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("WP_DRIVING_15"));
		m_TutorialComponent.StageReset_PrepareChotain();
	}
};