class SCR_AmbientVehicleSpawnPointComponentClass : ScriptComponentClass
{
}

class SCR_AmbientVehicleSpawnPointComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.EditBox, "How often will the vehicle respawn when destroyed. (seconds, 0 = no respawn)", "0 inf 1")]
	protected int m_iRespawnPeriod;

	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to optionally include to random spawn. If you want to spawn everything, you can leave it out empty.", "", ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aIncludedEditableEntityLabels;

	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to exclude from random spawn.", "", ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aExcludedEditableEntityLabels;

	[Attribute("0", desc: "If true, only assets with ALL of provided included labels will be used.")]
	protected bool m_bRequireAllIncludedLabels;

	protected static const int SPAWNING_RADIUS = 5;				//m, check empty space on a spawnpoint with this radius

	protected bool m_bDepleted;
	protected bool m_bFirstSpawnDone;
	protected bool m_bSpawnProcessed;

	protected int m_iID;

	protected WorldTimestamp m_fRespawnTimestamp;
	protected WorldTimestamp m_fDespawnTimer;

	protected ResourceName m_sPrefab;

	protected Vehicle m_Vehicle;

	protected Faction m_SavedFaction;

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRespawnPeriod()
	{
		return m_iRespawnPeriod;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] ID
	void SetID(int ID)
	{
		m_iID = ID;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetID()
	{
		return m_iID;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] depleted
	void SetIsDepleted(bool depleted)
	{
		m_bDepleted = depleted;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsDepleted()
	{
		return m_bDepleted;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsFirstSpawnDone()
	{
		return m_bFirstSpawnDone;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsSpawnProcessed()
	{
		return m_bSpawnProcessed;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] time
	void SetDespawnTimer(WorldTimestamp time)
	{
		m_fDespawnTimer = time;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	WorldTimestamp GetDespawnTimer()
	{
		return m_fDespawnTimer;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] timestamp
	void SetRespawnTimestamp(WorldTimestamp timestamp)
	{
		m_fRespawnTimestamp = timestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	WorldTimestamp GetRespawnTimestamp()
	{
		return m_fRespawnTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	Vehicle GetSpawnedVehicle()
	{
		return m_Vehicle;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return the created vehicle
	Vehicle SpawnVehicle()
	{
		SCR_FactionAffiliationComponent comp = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));

		if (!comp)
			return null;

		SCR_Faction faction = SCR_Faction.Cast(comp.GetAffiliatedFaction());

		if (!faction)
			faction = SCR_Faction.Cast(comp.GetDefaultAffiliatedFaction());

		if (faction != m_SavedFaction || (!faction && m_sPrefab.IsEmpty()))
			Update(faction);

		if (m_sPrefab.IsEmpty())
			return null;

		Resource prefab = Resource.Load(m_sPrefab);

		if (!prefab || !prefab.IsValid())
			return null;

		vector pos;
		bool spawnEmpty = SCR_WorldTools.FindEmptyTerrainPosition(pos, GetOwner().GetOrigin(), SPAWNING_RADIUS, SPAWNING_RADIUS);

		if (!spawnEmpty)
		{
#ifdef WORKBENCH
			Print("SCR_AmbientVehicleSpawnPointComponent: FindEmptyTerrainPosition failed at " + GetOwner().GetOrigin().ToString(), LogLevel.WARNING);
#endif

			// In case this spawnpoint is blocked from the start, don't process it anymore
			// Prevents unexpected behavior such as vehicles spawning on a spot where a service composition has been built and after a session load dismantled 
			if (!m_bFirstSpawnDone)
				m_bDepleted = true;

			return null;
		}

		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		GetOwner().GetTransform(params.Transform);

		m_Vehicle = Vehicle.Cast(GetGame().SpawnEntityPrefab(prefab, null, params));
		m_fRespawnTimestamp = null;
		m_bFirstSpawnDone = true;
		m_bSpawnProcessed = true;

		if (!m_Vehicle)
			return null;

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

		return m_Vehicle;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicle
	void OnVehicleDestroyed(IEntity vehicle)
	{
		m_Vehicle = null;

		if (m_iRespawnPeriod > 0)
		{
			ChimeraWorld world = GetOwner().GetWorld();
			m_fRespawnTimestamp = world.GetServerTimestamp().PlusSeconds(m_iRespawnPeriod);
		}
		else
			m_bDepleted = true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void DespawnVehicle()
	{
		m_fDespawnTimer = null;
		m_bSpawnProcessed = false;

		if (!m_Vehicle)
			return;

		RplComponent.DeleteRplEntity(m_Vehicle, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void Update(SCR_Faction faction)
	{
		m_SavedFaction = faction;
		SCR_EntityCatalog entityCatalog;

		if (faction)
		{
			entityCatalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		}
		else
		{
			SCR_EntityCatalogManagerComponent comp = SCR_EntityCatalogManagerComponent.GetInstance();

			if (!comp)
				return;

			entityCatalog = comp.GetEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		}

		if (!entityCatalog)
			return;

		array<SCR_EntityCatalogEntry> data = {};
		entityCatalog.GetFullFilteredEntityListWithLabels(data, m_aIncludedEditableEntityLabels, m_aExcludedEditableEntityLabels, m_bRequireAllIncludedLabels);

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
		SCR_AmbientVehicleSystem manager = SCR_AmbientVehicleSystem.GetInstance();

		if (!manager)
			return;

		manager.RegisterSpawnpoint(this);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_AmbientVehicleSpawnPointComponent()
	{
		SCR_AmbientVehicleSystem manager = SCR_AmbientVehicleSystem.GetInstance();

		if (!manager)
			return;

		manager.UnregisterSpawnpoint(this);
	}
}
