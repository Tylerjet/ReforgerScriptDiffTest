[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical19Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical19 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 8;
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		m_TutorialComponent.SetStagesComplete(2, true);
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("FirstAid_End", true);
	}
};