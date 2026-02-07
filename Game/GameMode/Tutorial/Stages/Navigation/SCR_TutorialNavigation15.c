[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation15Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation15 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_fConditionCheckPeriod = 1;

		SCR_MapEntity.GetOnMapOpen().Remove(m_TutorialComponent.OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Remove(m_TutorialComponent.OnMapClose);
		SCR_MapEntity.GetOnMapOpen().Insert(m_TutorialComponent.OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(m_TutorialComponent.OnMapClose);
		
		PlaySoundSystem("Navigation_OrientationOpenMap");
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.GetIsMapOpen();
	}
};