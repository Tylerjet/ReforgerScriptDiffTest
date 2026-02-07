[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_SpawnEntity_1Class : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_CombatEngineering_SpawnEntity_1 : SCR_BaseTutorialStage
{
	protected ResourceName m_sRequestEntityPrefab = "{F54F833E747C1B77}Prefabs/Vehicles/Wheeled/M923A1/Tutorial_Engineering_Truck.et";
	protected string m_sSpawnedEntityName = "BUILDING_VEHICLE";
	protected SCR_Waypoint m_SuggestedWP;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_SuggestedWP = RegisterWaypoint("WP_SUGGESTED_VEHICLE_SPAWN");
		
		m_TutorialComponent.GetOnEntitySpawned().Insert(OnEntitySpawned);
	}
	//------------------------------------------------------------------------------------------------
	//TODO: I could probably just cache last spawned entity in TutorialComponent
	protected void OnEntitySpawned(IEntity ent)
	{
		if (!ent)
			return;
		
		if (ent.GetPrefabData().GetPrefabName() != m_sRequestEntityPrefab)
			return;
		
		ent.SetName(m_sSpawnedEntityName);
		
		m_bFinished = true;
		
		SlotManagerComponent slotMan = SlotManagerComponent.Cast(ent.FindComponent(SlotManagerComponent));
		if (!slotMan)
			return;
		
		EntitySlotInfo slot = slotMan.GetSlotByName("EngineerBox");
		if (!slot)
			return;
		
		IEntity box = slot.GetAttachedEntity();
		if (box)
			m_TutorialComponent.BlockBuildingModeAccess(box, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_CombatEngineering_SpawnEntity_1()
	{
		m_TutorialComponent.GetOnEntitySpawned().Remove(OnEntitySpawned);
	}
}