[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation11Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation11 : SCR_BaseCampaignTutorialArlandStage
{
	float m_fAngle;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fConditionCheckPeriod = 1;
		
		IEntity lighthousePos = GetGame().GetWorld().FindEntityByName("LighthousePos");
		m_fAngle = m_TutorialComponent.GetEntityCompassAngle(lighthousePos);
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("Navigation_CompassLighthouse", true);
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.IsMyAngleInRange(m_fAngle, 5);
	}
};