[EntityEditorProps(insertable: false)]
class SCR_HeliCourse_stage1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_HeliCourse_stage1 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		m_TutorialComponent.StageReset_Helicopter();
		
		m_fDuration = 6;
		m_bConditionPassCheck = true;
		
		GetGame().GetCallqueue().CallLater(PlaySoundSystem, 2000, false, "Heli_Start", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
};