class SCR_CampaignTutorialStage53Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage53 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_11");
		m_fWaypointCompletionRadius = 20;
		m_fWaypointHeightOffset = 0;
		SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Tutorial_Popup_Title-UC", 20, text2: "#AR-Tutorial_Popup_SkipStageTruck", category: SCR_EPopupMsgFilter.TUTORIAL);
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
		
		if (accessComp)
		{
			accessComp.GetOnCompartmentLeft().Remove(m_TutorialComponent.OnSupplyTruckLeft);
			accessComp.GetOnCompartmentLeft().Insert(m_TutorialComponent.OnSupplyTruckLeft);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(m_WaypointEntity);
		m_TutorialComponent.StageReset_ProcessTruck();
	}
};