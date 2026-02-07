[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation14Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation14 : SCR_BaseCampaignTutorialArlandStage
{
	float m_fAngle;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fConditionCheckPeriod = 1;
		
		IEntity lighthousePos = GetGame().GetWorld().FindEntityByName("TowerPos");
		m_fAngle = m_TutorialComponent.GetEntityCompassAngle(lighthousePos);
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("Navigation_Compass_Village");
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.IsMyAngleInRange(m_fAngle, 5);
	}
};