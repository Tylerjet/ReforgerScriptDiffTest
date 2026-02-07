class SCR_CampaignTutorialStage79Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage79 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Compass" + CreateString("#AR-Keybind_Compass","GadgetCompass"), duration: -1);
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (campaign)
			campaign.SetIsTutorial(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		SCR_GadgetManagerComponent gadgetComponent = SCR_GadgetManagerComponent.Cast(m_Player.FindComponent(SCR_GadgetManagerComponent));
		IEntity compas = gadgetComponent.GetHeldGadget();
		IEntity compasGadget = gadgetComponent.GetGadgetByType(EGadgetType.COMPASS);
		return (compas == compasGadget);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_CaptureLaruns();
		m_TutorialComponent.StageReset_HandleRespawnRadios();
	}
};