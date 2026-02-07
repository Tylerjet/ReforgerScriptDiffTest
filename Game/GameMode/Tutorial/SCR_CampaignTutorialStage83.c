class SCR_CampaignTutorialStage83Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage83 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_MAP_MOVE");
		m_fWaypointCompletionRadius = 30;
		m_fConditionCheckPeriod = 1;
		m_bShowWaypoint = false;
		GenericEntity WP_cottage = GenericEntity.Cast(GetGame().GetWorld().FindEntityByName("WP_CONFLICT_COMPAS_MOVE"));
		GenericEntity WP_mobileHQ = GenericEntity.Cast(GetGame().GetWorld().FindEntityByName("WP_CONFLICT_MAP_MOVE"));
		SCR_MapDescriptorComponent cottageDescr = SCR_MapDescriptorComponent.Cast(WP_cottage.FindComponent(SCR_MapDescriptorComponent));
		SCR_MapDescriptorComponent mobileTruckDescr = SCR_MapDescriptorComponent.Cast(WP_mobileHQ.FindComponent(SCR_MapDescriptorComponent));
		cottageDescr.Item().SetVisible(true);
		mobileTruckDescr.Item().SetVisible(true);
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MHQLocation", duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_CaptureLaruns();
		m_TutorialComponent.StageReset_HandleRespawnRadios();
	}
};