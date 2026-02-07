[EntityEditorProps(insertable: false)]
class SCR_HeliCourse_stage13Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_HeliCourse_stage13 : SCR_BaseCampaignTutorialArlandStage
{
	protected Vehicle m_Helicopter;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_Helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		
		PlaySoundSystem("Heli_GetOut", true);
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Helicopter.GetPilot() != m_Player;
	}
};