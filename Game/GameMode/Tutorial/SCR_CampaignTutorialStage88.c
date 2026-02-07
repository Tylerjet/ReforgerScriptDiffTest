class SCR_CampaignTutorialStage88Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage88 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MHQDeploy", duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return (SCR_GameModeCampaign.GetInstance().GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetMobileAssembly() != null);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_CaptureLaruns();
		m_TutorialComponent.StageReset_HandleRespawnRadios();
	}
};