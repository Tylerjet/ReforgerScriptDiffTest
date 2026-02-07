[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation1 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_fDuration = 5;
		m_bConditionPassCheck = true;
		
		m_TutorialComponent.SpawnAsset("Navigation_car", "{5674FAEB9AB7BDD0}Prefabs/Vehicles/Wheeled/M998/M998.et");
		
		GetGame().GetCallqueue().CallLater(PlaySoundSystem, 2000, false, "Navigation_Start", false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
};