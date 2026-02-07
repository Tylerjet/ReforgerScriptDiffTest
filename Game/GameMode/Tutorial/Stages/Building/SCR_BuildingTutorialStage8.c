[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage8Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage8 : SCR_BaseCampaignTutorialArlandStage
{
	protected Vehicle m_SupplyTruck;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{		
		m_SupplyTruck = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("BuildingSupplyTruck"));
		if (!m_SupplyTruck)
			return;
		
		m_bCheckWaypoint = false;
		RegisterWaypoint(m_SupplyTruck);
		m_TutorialComponent.SetWaypointMiscImage("GETIN", true);
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("Building_Truck");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SupplyTruck)
			return false;
		
		return m_SupplyTruck.GetPilot() == m_Player;
	}
};