class SCR_CampaignTutorialStage0Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage0 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 9;
		GetGame().GetCallqueue().CallLater(DelayedPopup, 2000, false, "#AR-Tutorial_ScenarioName-UC", "#AR-Tutorial_ScenarioDescription", 6, "", "", "", "");
	}
};