class SCR_CampaignTutorialStage6Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage6 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_VAULT");
		m_fWaypointCompletionRadius = 5;
		m_fWaypointHeightOffset = 0.2;
		GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowCustomHint, 3000, false, "#AR-Tutorial_Hint_Vaulting" + CreateString("#AR-Keybind_JumpVaultClimb","CharacterJump"), "", -1, false, EFieldManualEntryId.NONE, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.CLIMB);
	}
};