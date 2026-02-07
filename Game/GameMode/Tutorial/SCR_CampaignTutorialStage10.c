class SCR_CampaignTutorialStage10Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage10 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WALL");
		m_fWaypointCompletionRadius = 5;
		m_fWaypointHeightOffset = 0.3;
		
		if (!m_TutorialComponent.GetWas3rdPersonViewUsed())
			GetGame().GetCallqueue().CallLater(DelayedPopup, 2000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Camera", 7, "", "", "<color rgba='226,168,79,200'><shadow mode='image' color='0,0,0' size='1' offset='1,1' opacity = '0.5'><action name = 'SwitchCameraType'/></shadow></color>", "");
				
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Walls" + CreateString("#AR-Keybind_JumpVaultClimb","CharacterJump"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.CLIMB);
	}
};