class SCR_CampaignTutorialStage14Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage14 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_LADDER_DOWN");
		m_fWaypointCompletionRadius = 3;
		m_fWaypointHeightOffset = 0;
		
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_LadderDown" + CreateString("#AR-Keybind_SpeedAnalog","CharacterSpeedAnalog"), duration: -1);	
		else
			SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_LadderDown", duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.LADDER);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInputDeviceChanged(bool switchedToKeyboard)
	{
		if (switchedToKeyboard)
			SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_LadderDown" + CreateString("#AR-Keybind_SpeedAnalog","CharacterSpeedAnalog"), duration: -1, isSilent: true);	
		else
			SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_LadderDown", duration: -1, isSilent: true);
	}
};