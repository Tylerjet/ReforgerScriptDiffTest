[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPluginSpawnPoint : SCR_ScenarioFrameworkPlugin
{
	[Attribute("0", desc: "Find empty position for spawning within given radius. When none is found, entity position will be used.")]
	float m_fSpawnRadius;

	[Attribute("US", UIWidgets.EditBox, "Determines which faction can spawn on this spawn point."), RplProp(onRplName: "OnSetFactionKey")]
	string m_sFaction;

	[Attribute("0")]
	bool m_bShowInDeployMapOnly;

	[Attribute("0", desc: "Use custom timer when deploying on this spawn point. Takes the remaining respawn time from SCR_TimedSpawnPointComponent")]
	bool m_bTimedSpawnPoint;
	
	[Attribute()]
	ref SCR_UIInfo m_Info;
	
	[Attribute("0", desc: "Allow usage of Spawn Positions in range")]
	bool m_bUseNearbySpawnPositions;

	[Attribute("100", desc: "Spawn position detection radius, in metres")]
	float m_fSpawnPositionUsageRange;

	[Attribute("0", desc: "Additional respawn time (in seconds) when spawning on this spawn point"), RplProp()]
	float m_fRespawnTime;
	
	[Attribute(UIWidgets.Auto, desc: "What to do once Spawn Point is used",)]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnSpawnPointUsed;

	IEntity m_Asset;

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		if (!object)
			return;

		super.Init(object);
		m_Asset = object.GetSpawnedEntity();
		if (!m_Asset)
			return;
		
		SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.Cast(m_Asset);
		if (!spawnPoint)
			return;
		
		spawnPoint.SetSpawnRadius(m_fSpawnRadius);
		spawnPoint.SetFactionKey(m_sFaction);
		spawnPoint.SetVisibleInDeployMapOnly(m_bShowInDeployMapOnly);
		spawnPoint.SetIsTimed(m_bTimedSpawnPoint);
		
		if (m_Info)
		{
			spawnPoint.LinkInfo(m_Info);
			spawnPoint.SetSpawnPointName(m_Info.GetName());
		}
		
		spawnPoint.SetUseNearbySpawnPositions(m_bUseNearbySpawnPositions);
		spawnPoint.SetSpawnPositionRange(m_fSpawnPositionUsageRange);
		spawnPoint.SetRespawnTime(m_fRespawnTime);
		
		spawnPoint.GetOnSpawnPointFinalizeSpawn().Insert(OnFinalizeSpawnDone_S);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFinalizeSpawnDone_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		if (!data || !m_Asset)
			return;
		
		SCR_SpawnPointSpawnData spawnPointData = SCR_SpawnPointSpawnData.Cast(data);
		if (!spawnPointData)
			return;
		
		SCR_SpawnPoint spawnPoint = spawnPointData.GetSpawnPoint();
		if (!spawnPoint)
			return;
		
		if (spawnPoint != m_Asset)
			return;
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnSpawnPointUsed)
		{
			action.OnActivate(entity);
		}
	}
}