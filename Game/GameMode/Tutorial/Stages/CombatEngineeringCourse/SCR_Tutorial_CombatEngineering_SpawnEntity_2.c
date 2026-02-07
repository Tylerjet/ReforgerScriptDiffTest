[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_SpawnEntity_2Class : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_CombatEngineering_SpawnEntity_2 : SCR_BaseTutorialStage
{
	protected ResourceName m_sRequestEntityPrefab = "{6E9DCCF936BF17B9}PrefabsEditable/Auto/Compositions/Slotted/SlotFlatSmall/E_MachineGunNest_S_US_02.et";
	protected string m_sSpawnedEntityName = "BUILDING_GUNNEST";
	protected bool m_bEntitySpawned;
	protected SCR_Waypoint m_SuggestedWP;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_SuggestedWP = RegisterWaypoint("WP_SUGGESTED_GUNPOST_SPAWN");
		
		PlayNarrativeCharacterStage("COMBATENGINEERING_Instructor_E", 2);
		
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
	void ~SCR_Tutorial_CombatEngineering_SpawnEntity_2()
	{
		m_TutorialComponent.GetOnEntitySpawned().Remove(OnEntitySpawned);
	}
}