class SCR_CampaignTutorialStage39Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage39 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
		if (accessComp)
			accessComp.GetOnCompartmentLeft().Remove(m_TutorialComponent.OnJeepLeft);
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Dismounting" + CreateString("#AR-Keybind_Exit","GetOut"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !(m_Player.IsInVehicle());
	}
};