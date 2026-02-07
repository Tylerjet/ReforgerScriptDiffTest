[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical7Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical7: SCR_BaseCampaignTutorialArlandStage
{
	protected Vehicle m_Jeep;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_bCheckWaypoint = false;

		RegisterWaypoint("AccidentJeep");
	
		m_Jeep = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("AccidentJeep"));
		m_TutorialComponent.SetWaypointMiscImage("HEAL", true);
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("FirstAid_ExtractUncon", true);
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------	
	override protected bool GetIsFinished()
	{		
		return m_Jeep && !m_Jeep.GetPilot();
	}
};