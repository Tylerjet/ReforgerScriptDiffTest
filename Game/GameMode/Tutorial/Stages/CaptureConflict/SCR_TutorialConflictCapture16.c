[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture16Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture16 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("Conflict_RadioInfo");
		HintOnVoiceOver();
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
};