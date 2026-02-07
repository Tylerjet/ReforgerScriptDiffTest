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
		
		SCR_HintManagerComponent.GetInstance().GetOnHintShow().Remove(CheckGMHint);
		SCR_HintManagerComponent.GetInstance().GetOnHintShow().Insert(CheckGMHint);
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingDepot_GM", duration: -1);
		
		GetGame().GetCallqueue().Remove(OverrideGMHint);
		GetGame().GetCallqueue().CallLater(OverrideGMHint, 250, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		SCR_CampaignMilitaryBaseComponent chotainBase = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain").FindComponent(SCR_CampaignMilitaryBaseComponent));
		SCR_ServicePointComponent service = chotainBase.GetServiceByType(SCR_EServicePointType.LIGHT_VEHICLE_DEPOT);
		SCR_ServicePointComponent serviceHeavy = chotainBase.GetServiceByType(SCR_EServicePointType.HEAVY_VEHICLE_DEPOT);
		
		return (service != null || serviceHeavy != null);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("WP_DRIVING_15"));
		m_TutorialComponent.StageReset_PrepareChotain();
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckGMHint(SCR_HintUIInfo info, bool isSilent)
	{
		if (!info)
			return;
		
		if (info.GetType() != EHint.UNDEFINED)
		{
			OverrideGMHint();
			GetGame().GetCallqueue().CallLater(OverrideGMHint, 1);
		}
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
		
		if (SCR_HintManagerComponent.GetInstance())
			SCR_HintManagerComponent.GetInstance().GetOnHintShow().Remove(CheckGMHint);
	}
};