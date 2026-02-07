class SCR_CampaignTutorialStage80Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage80 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fConditionCheckPeriod = 1;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_CompassNorth", duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		float angle = Math.RAD2DEG * m_Player.GetCharacterController().GetInputContext().GetAimingAngles()[0];
		return (Math.AbsFloat(angle) < 3);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_CaptureLaruns();
		m_TutorialComponent.StageReset_HandleRespawnRadios();
	}
};