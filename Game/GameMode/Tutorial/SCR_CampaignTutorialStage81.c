class SCR_CampaignTutorialStage81Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage81 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fConditionCheckPeriod = 1;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_CompassAzimuth" + CreateString("#AR-Keybind_CompassADS","GadgetADS"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		float angle = m_Player.GetCharacterController().GetAimingAngles()[0];
		return ((angle > 137) && (angle < 143));
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_CaptureLaruns();
		m_TutorialComponent.StageReset_HandleRespawnRadios();
	}
};