class SCR_CampaignTutorialStage1Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage1 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_ZIGZAG_1");
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_ObstacleCourseStart" + CreateString("#AR-Keybind_Move", "CharacterForward", "CharacterRight") + CreateString("#AR-KeybindEditor_LookRL","MouseX","MouseY"), duration: -1);
	}
};