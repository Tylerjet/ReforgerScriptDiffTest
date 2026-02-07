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
		
		GetGame().GetCallqueue().Remove(OverrideGMHint);
		GetGame().GetCallqueue().CallLater(OverrideGMHint, 250, true);
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingDepot_GM", duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		SCR_CampaignBase chotainBase = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain"));
		SCR_CampaignServiceComponent service = chotainBase.GetBaseService(ECampaignServicePointType.LIGHT_VEHICLE_DEPOT);
		SCR_CampaignServiceComponent serviceHeavy = chotainBase.GetBaseService(ECampaignServicePointType.HEAVY_VEHICLE_DEPOT);
		
		return (service != null || serviceHeavy != null);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("WP_DRIVING_15"));
		m_TutorialComponent.StageReset_PrepareChotain();
	}
	
	//------------------------------------------------------------------------------------------------
	void OverrideGMHint()
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingDepot_GM", duration: -1, isSilent: true);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTutorialStage60()
	{
		GetGame().GetCallqueue().Remove(OverrideGMHint);
	}
};