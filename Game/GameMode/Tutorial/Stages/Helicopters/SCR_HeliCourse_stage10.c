[EntityEditorProps(insertable: false)]
class SCR_HeliCourse_stage10Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_HeliCourse_stage10 : SCR_BaseCampaignTutorialArlandStage
{
	protected SCR_HelicopterControllerComponent m_HelicopterController;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;
		
		m_HelicopterController = SCR_HelicopterControllerComponent.Cast(helicopter.FindComponent(SCR_HelicopterControllerComponent));
		PlaySoundSystem("Heli_TurnOffAutohover");
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_HelicopterController)
			return false;
		
		return !m_HelicopterController.GetAutohoverEnabled();
	}
};