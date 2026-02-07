#include "scripts/Game/config.c"
//------------------------------------------------------------------------------------------------
class SCR_AmbientVehicleSpawnPointComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_AmbientVehicleSpawnPointComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.EditBox, "How often will the vehicle respawn when destroyed. (seconds, 0 = no respawn)", "0 inf 1")]
	protected int m_iRespawnPeriod;
	
	[Attribute("1")]
	protected bool m_bIncludeArmedVehicles;
	
	protected static const int SPAWNING_RADIUS = 5;				//m, check empty space on a spawnpoint with this radius
	
	protected bool m_bDepleted;
	protected bool m_bFirstSpawnDone;
	
	protected int m_iID;
	
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fRespawnTimestamp;
	protected float m_fDespawnTimer = -1;
	#else
	protected WorldTimestamp m_fRespawnTimestamp;
	protected WorldTimestamp m_fDespawnTimer;
	#endif
	
	protected ResourceName m_sPrefab;
	
	protected Vehicle m_Vehicle;
	
	protected Faction m_SavedFaction;
	
	//------------------------------------------------------------------------------------------------
	int GetRespawnPeriod()
	{
		return m_iRespawnPeriod;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetID(int ID)
	{
		m_iID = ID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetID()
	{
		return m_iID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsDepleted(bool depleted)
	{
		m_bDepleted = depleted;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsDepleted()
	{
		return m_bDepleted;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsFirstSpawnDone()
	{
		return m_bFirstSpawnDone;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	void SetDespawnTimer(float time)
	#else
	void SetDespawnTimer(WorldTimestamp time)
	#endif
	{
		m_fDespawnTimer = time;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetDespawnTimer()
	#else
	WorldTimestamp GetDespawnTimer()
	#endif
	{
		return m_fDespawnTimer;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	void SetRespawnTimestamp(float timestamp)
	#else
	void SetRespawnTimestamp(WorldTimestamp timestamp)
	#endif
	{
		m_fRespawnTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetRespawnTimestamp()
	#else
	WorldTimestamp GetRespawnTimestamp()
	#endif
	{
		return m_fRespawnTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	Vehicle GetSpawnedVehicle()
	{
		return m_Vehicle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnVehicle()
	{
		SCR_FactionAffiliationComponent comp = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
		
		if (!comp)
			return;
		
		SCR_Faction faction = SCR_Faction.Cast(comp.GetAffiliatedFaction());
		
		if (!faction)
			faction = SCR_Faction.Cast(comp.GetDefaultAffiliatedFaction());
		
		if (faction != m_SavedFaction)
			Update(faction);
		
		if (m_sPrefab.IsEmpty())
			return;
		
		Resource prefab = Resource.Load(m_sPrefab);
		
		if (!prefab || !prefab.IsValid())
			return;

		vector pos;
		bool spawnEmpty = SCR_WorldTools.FindEmptyTerrainPosition(pos, GetOwner().GetOrigin(), SPAWNING_RADIUS, SPAWNING_RADIUS);
		
		if (!spawnEmpty)
			return;

		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		GetOwner().GetTransform(params.Transform);
		
		m_Vehicle = Vehicle.Cast(GetGame().SpawnEntityPrefab(prefab, null, params));
		#ifndef AR_CAMPAIGN_TIMESTAMP
		m_fRespawnTimestamp = 0;
		#else
		m_fRespawnTimestamp = null;
		#endif
		m_bFirstSpawnDone = true;
		
		if (!m_Vehicle)
			return;

		CarControllerComponent_SA carController = CarControllerComponent_SA.Cast(m_Vehicle.FindComponent(CarControllerComponent_SA));
		
		// Activate handbrake so the vehicles don't go downhill on their own when spawned
		if (carController)
			carController.SetPersistentHandBrake(true);
		
		Physics physicsComponent = m_Vehicle.GetPhysics();
		
		// Snap to terrain
		if (physicsComponent)
			physicsComponent.SetVelocity("0 -1 0");
		
		EventHandlerManagerComponent handler = EventHandlerManagerComponent.Cast(m_Vehicle.FindComponent(EventHandlerManagerComponent));
		
		if (handler)
			handler.RegisterScriptHandler("OnDestroyed", this, OnVehicleDestroyed);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnVehicleDestroyed(IEntity vehicle)
	{
		m_Vehicle = null;

		if (m_iRespawnPeriod > 0)
		#ifndef AR_CAMPAIGN_TIMESTAMP
			m_fRespawnTimestamp = Replication.Time() + (m_iRespawnPeriod * 1000);
		#else
		{
			ChimeraWorld world = GetOwner().GetWorld();
			m_fRespawnTimestamp = world.GetServerTimestamp().PlusSeconds(m_iRespawnPeriod);
		}
		#endif
		else
			m_bDepleted = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void DespawnVehicle()
	{
		#ifndef AR_CAMPAIGN_TIMESTAMP
		m_fDespawnTimer = -1;
		#else
		m_fDespawnTimer = null;
		#endif
		
		if (!m_Vehicle)
			return;
		
		RplComponent.DeleteRplEntity(m_Vehicle, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Update(SCR_Faction faction)
	{	
		m_SavedFaction = faction;
		
		if (!faction)
			return;
		
		SCR_EntityCatalog entityCatalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		
		if (!entityCatalog)
			return;
		
		array<SCR_EntityCatalogEntry> data = {};
		
		if (m_bIncludeArmedVehicles)
			entityCatalog.GetEntityList(data);
		else
			entityCatalog.GetEntityListExcludingLabel(EEditableEntityLabel.TRAIT_ARMED, data);
		
		if (data.IsEmpty())
			return;
		
		Math.Randomize(-1);
		m_sPrefab = (data.GetRandomElement().GetPrefab());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		
		if (!factionComponent)
		{
			Print("SCR_AmbientVehicleSpawnPointComponent: SCR_FactionAffiliationComponent not found on owner entity. Vehicle spawning will not be available.", LogLevel.WARNING);
			return;
		}
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode() || Replication.IsClient())
			return;

		SCR_AmbientVehiclesManager.GetInstance().RegisterSpawnpoint(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_AmbientVehicleSpawnPointComponent()
	{
		SCR_AmbientVehiclesManager manager = SCR_AmbientVehiclesManager.GetInstance(false);
		
		if (manager)
			manager.UnregisterSpawnpoint(this);
	}
};
