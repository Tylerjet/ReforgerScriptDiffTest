[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical6Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical6: SCR_BaseCampaignTutorialArlandStage
{
	protected Vehicle m_Ambulance;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointCompletionRadius = 10;
		m_fWaypointHeightOffset = 0;
		
		RegisterWaypoint("WP_MEDICAL_4");
		
		m_Ambulance = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("Ambulance"));
		m_TutorialComponent.SetWaypointMiscImage("GETOUT", true);
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("FirstAid_Park", true);
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Ambulance && !m_Ambulance.GetPilot();
	}
};