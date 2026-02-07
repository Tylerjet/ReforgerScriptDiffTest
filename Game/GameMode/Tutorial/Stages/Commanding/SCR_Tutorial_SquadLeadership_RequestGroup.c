[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_RequestGroupClass : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_SquadLeadership_RequestGroup : SCR_BaseTutorialStage
{
	protected ResourceName m_sRequestEntityPrefab = "{3BF36BDEEB33AEC9}Prefabs/Groups/BLUFOR/Group_US_SentryTeam.et";
	protected string m_sSpawnedEntityName = "REQUESTING_GROUP";
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
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
		SCR_AIGroup group = SCR_AIGroup.Cast(ent);
		if (group)
			group.RemoveWaypoint(group.GetCurrentWaypoint());	
		
		m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_bFinished;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SquadLeadership_RequestGroup()
	{
		m_TutorialComponent.GetOnEntitySpawned().Remove(OnEntitySpawned);
	}
}