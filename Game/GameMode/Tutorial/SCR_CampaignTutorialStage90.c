class SCR_CampaignTutorialStage90Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage90 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SeizeHQ", duration: 60);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		SCR_CampaignMilitaryBaseComponent HQ = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("MainBaseLevie").FindComponent(SCR_CampaignMilitaryBaseComponent));
				
		if (HQ)
			return (HQ.GetFaction() == SCR_GameModeCampaign.GetInstance().GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
		else
			return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_CaptureLaruns();
		m_TutorialComponent.StageReset_DeployMHQ();
	}
};