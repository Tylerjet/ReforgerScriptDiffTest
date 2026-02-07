[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical1 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 5;
		m_bConditionPassCheck = true;
		
		m_TutorialComponent.ResetStage_Medical();
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		GetGame().GetCallqueue().CallLater(PlaySoundSystem, 2000, false, "FirstAid_Start", false);
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
};