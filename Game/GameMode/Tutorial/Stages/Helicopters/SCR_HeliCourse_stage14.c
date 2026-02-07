[EntityEditorProps(insertable: false)]
class SCR_HeliCourse_stage14Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_HeliCourse_stage14 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		m_fDuration = 10;
		m_TutorialComponent.SetStagesComplete(6, true);
		PlaySoundSystem("Heli_End", true);
	}
};