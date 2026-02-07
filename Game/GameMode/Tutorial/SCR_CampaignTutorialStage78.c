class SCR_CampaignTutorialStage78Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage78 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 33;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_Signal_Text", duration: 20, isTimerVisible: true);
		GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowCustomHint, 20500, false, "#AR-Tutorial_Hint_NavigationIntro", "", 12, false, EFieldManualEntryId.NONE, true);
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (campaign)
			campaign.SetIsTutorial(false);
		
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
				
		if (hudManager)
		{
			SCR_XPInfoDisplay display = SCR_XPInfoDisplay.Cast(hudManager.FindInfoDisplay(SCR_XPInfoDisplay));
			
			if (display)
				display.AllowShowingInfo(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_CaptureLaruns();
		m_TutorialComponent.StageReset_HandleRespawnRadios();
	}
};