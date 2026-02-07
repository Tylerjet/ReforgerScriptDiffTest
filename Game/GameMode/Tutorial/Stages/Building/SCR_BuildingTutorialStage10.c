[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage10Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage10 : SCR_BaseCampaignTutorialArlandStage
{
	protected Vehicle m_SupplyTruck;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("FIA_SupplyDepot");
		m_TutorialComponent.SetWaypointMiscImage("CUSTOM", true);
		m_bCheckWaypoint = false;
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("Building_Exit", true);
		HintOnVoiceOver();
	
		m_SupplyTruck = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("BuildingSupplyTruck"));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SupplyTruck)
			return false;

		return m_SupplyTruck.GetPilot() != m_Player;
	}
};