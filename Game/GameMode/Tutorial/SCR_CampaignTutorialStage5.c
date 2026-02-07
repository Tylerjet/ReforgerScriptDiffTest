class SCR_CampaignTutorialStage5Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage5 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointHeightOffset = 0.2;
		RegisterWaypoint("WP_WIREMESH");
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Crawling" + CreateString("#AR-Keybind_Prone", "CharacterProne", "CharacterJump"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		CharacterControllerComponent comp = CharacterControllerComponent.Cast(m_Player.FindComponent(CharacterControllerComponent));
					
		if (comp)
			return comp.GetStance() == ECharacterStance.PRONE;
		else
			return true;
	}
};