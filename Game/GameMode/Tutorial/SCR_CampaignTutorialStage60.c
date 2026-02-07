class SCR_CampaignTutorialStage60Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage60 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_BUILD_SERVICE");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		m_fConditionCheckPeriod = 1;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingDepot", duration: -1);
		GetGame().GetCallqueue().CallLater(DelayedPopup, 4000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Tickets", 10, "", "", "", "");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		SCR_CampaignBase chotainBase = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain"));
		SCR_CampaignServiceComponent service = chotainBase.GetBaseService(ECampaignServicePointType.LIGHT_VEHICLE_DEPOT);
		
		if (service)
		{
			if (m_TutorialComponent.GetSupplyTruckComponent() && m_TutorialComponent.GetSupplyTruckComponent().GetSupplies() >= 200)
				m_TutorialComponent.FinishStage(this, SCR_ECampaignTutorialStage.CONFLICT_BOARD_TRUCK);
			else
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("WP_DRIVING_15"));
		m_TutorialComponent.StageReset_PrepareChotain();
	}
};