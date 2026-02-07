class SCR_CampaignTutorialStage34Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage34 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_5");
		m_fWaypointCompletionRadius = 20;
		m_fWaypointHeightOffset = 0;
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
		
		if (accessComp)
		{
			accessComp.GetOnCompartmentLeft().Remove(m_TutorialComponent.OnJeepLeft);
			accessComp.GetOnCompartmentLeft().Insert(m_TutorialComponent.OnJeepLeft);
		}
		
		SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Tutorial_Popup_Title-UC", 20, text2: "#AR-Tutorial_Popup_SkipStageJeep", category: SCR_EPopupMsgFilter.TUTORIAL);
		GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowCustomHint, 2000, false, "#AR-Tutorial_Hint_Stopping" + CreateString("#AR-Keybind_Exit","GetOut"), "", 20, false, EFieldManualEntryId.NONE, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_MoveInJeep(m_WaypointEntity);
		m_TutorialComponent.StageReset_RegisterJeepSkip();
	}
};