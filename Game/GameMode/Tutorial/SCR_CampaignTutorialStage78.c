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
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (campaign)
			campaign.SetIsTutorial(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_CaptureLaruns();
		m_TutorialComponent.StageReset_HandleRespawnRadios();
	}
};