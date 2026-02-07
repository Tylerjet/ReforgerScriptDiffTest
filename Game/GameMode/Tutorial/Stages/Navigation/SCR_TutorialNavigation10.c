[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation10Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation10 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_fConditionCheckPeriod = 1;
		
		PlaySoundSystem("Navigation_CompassNorth", true);
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		float angle = Math.RAD2DEG * m_Player.GetCharacterController().GetInputContext().GetAimingAngles()[0];
		return (Math.AbsFloat(angle) < 3);
	}
};