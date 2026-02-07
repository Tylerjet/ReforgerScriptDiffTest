[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_Place_AntennaClass : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_Seizing_Place_Antenna : SCR_BaseTutorialStage
{
	protected ResourceName m_sRequestEntityPrefab = "{F7DC8BB193BCAF44}PrefabsEditable/Auto/Compositions/Slotted/SlotFlatSmall/E_Antenna_S_US_01.et";
	//protected SCR_PreviewEntityEditorComponent m_PreviewEntityComponent;
	protected string m_sSpawnedEntityName = "BUILDING_ANTENNA";
	protected bool m_bEntitySpawned;
	protected SCR_Waypoint m_SuggestedWP;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		//m_SuggestedWP = RegisterWaypoint("WP_SUGGESTED_VEHICLE_SPAWN");
		
		m_TutorialComponent.GetOnEntitySpawned().Insert(OnEntitySpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	//TODO: I could probably just cache last spawned entity in TutorialComponent
	protected void OnEntitySpawned(IEntity ent)
	{
		if (!ent)
			return;
		
		m_bEntitySpawned = ent.GetPrefabData().GetPrefabName() == m_sRequestEntityPrefab;
		if (m_bEntitySpawned)
			ent.SetName(m_sSpawnedEntityName);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_bEntitySpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Seizing_Place_Antenna()
	{
		m_TutorialComponent.GetOnEntitySpawned().Remove(OnEntitySpawned);
	}
}