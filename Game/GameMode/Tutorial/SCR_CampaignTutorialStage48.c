class SCR_CampaignTutorialStage48Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage48 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 12;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MotorPool", "", 12, isTimerVisible: true);
	}
};