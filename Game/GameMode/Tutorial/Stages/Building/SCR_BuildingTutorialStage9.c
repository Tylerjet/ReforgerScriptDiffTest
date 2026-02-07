[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage9Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage9 : SCR_BaseCampaignTutorialArlandStage
{
	protected IEntity m_SupplyTruck, m_SupplyDepot;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{			
		m_fWaypointCompletionRadius = 10;
		RegisterWaypoint("FIA_SupplyDepot");
		m_TutorialComponent.SetWaypointMiscImage("GETOUT", true);
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("Building_GetIn");
	
		m_SupplyTruck = GetGame().GetWorld().FindEntityByName("BuildingSupplyTruck");
		m_SupplyDepot = GetGame().GetWorld().FindEntityByName("FIA_SupplyDepot");
		
		GetGame().GetCallqueue().CallLater(PlaySoundSystem, 20000, false, "Building_DriveHint", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SupplyDepot)
			return false;
		
		return vector.DistanceSq(m_SupplyTruck.GetOrigin(), m_SupplyDepot.GetOrigin()) <= 100;
	}
};