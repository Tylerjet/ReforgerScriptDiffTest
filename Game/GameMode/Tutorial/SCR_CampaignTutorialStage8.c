class SCR_CampaignTutorialStage8Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage8 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WINDOW");
		m_fWaypointCompletionRadius = 5;
		m_fWaypointHeightOffset = 0.6;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Window" + CreateString("#AR-Keybind_JumpVaultClimb","CharacterJump"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.CLIMB);
	}
};