class SCR_CampaignTutorialStage85Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage85 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_RadioInfo" + CreateString("#AR-Keybind_VONMenu", "VONMenu"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		BaseRadioComponent radioComp = m_TutorialComponent.GetPlayerRadio();
		if (!radioComp)
			return false;

		BaseTransceiver tsv = radioComp.GetTransceiver(0);
		if (!tsv)
			return false;

		return tsv.GetFrequency() == DESIRED_FREQUENCY;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_CaptureLaruns();
		m_TutorialComponent.StageReset_HandleRespawnRadios();
	}
};