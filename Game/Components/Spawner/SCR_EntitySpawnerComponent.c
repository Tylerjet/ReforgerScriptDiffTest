class SCR_EntitySpawnerComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! Component allowing user to request spawning entities using asset list
class SCR_EntitySpawnerComponent : ScriptComponent
{	
	[Attribute(desc: "List of entities that can be spawned", category: "Entity Spawner")]
	protected ref SCR_EntityAssetList m_EntityAssetList;
	
	[Attribute("2", desc: "Range to check if a spawned entity is occupying the spawner", params: "0 inf 1", category: "Entity Spawner")]
	protected int m_iEntitySpawnRange;
	
	[Attribute(defvalue: "0 0 0", uiwidget: UIWidgets.Coords, desc: "Local spawn position offset coords from owner", params: "inf inf purpose=coords space=entity anglesVar=m_vSpawnOffsetRotation", category: "Entity Spawner")]
	protected vector m_vSpawnOffsetPosition;
	
	[Attribute("0 0 0", UIWidgets.Coords, desc: "Rotational offset", params: "inf inf purpose=angles space=entity coordsVar=m_vSpawnOffsetPosition", category: "Entity Spawner")]
	protected vector m_vSpawnOffsetRotation;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Should spawner check Triggers in hierarchy for additional spawn positions?", category: "Entity Spawner")]
	protected bool m_bUseTriggers;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Should spawner check radius around parent for spawn positions?", category: "Entity Spawner")]
	protected bool m_bUseRadius;
	
	[Attribute("15", desc: "Spawn position radius", category: "Entity Spawner")]
	protected float m_fRadiusSize;
	
	[Attribute("5", desc: "Radius spawn position search size (should be bit more than biggest entity spawned in spawner)", category: "Entity Spawner")]
	protected float m_fRadiusSpawnZoneSize;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Should be wrecks on spawn positions removed?", category: "Entity Spawner")]
	protected bool m_bDeleteWrecks;
	
	[Attribute("-1", desc: "Disabled if -1, Used for spawn entity costs.", category: "Entity Spawner")]
	protected float m_fSpawnerSupplies;
	
	[Attribute("", UIWidgets.Flags, "", category: "Entity Spawner", enums: ParamEnumArray.FromEnum(SCR_EEntityType))]
	protected SCR_EEntityType m_eSupportedEntities;
	
	protected IEntity m_SpawnedEntity;
	protected RplComponent m_RplComponent;
	protected ref array<IEntity> m_aEntitiesOnSpawnPosition = {};

	protected ref ScriptInvoker Event_OnEntitySpawned = new ScriptInvoker(); //~ Sends Spawned IEntity
	protected ref ScriptInvoker Event_OnSpawnerSuppliesChanged = new ScriptInvoker(); //~ Sends Spawned prev and new spawn supplies
	
	static const vector m_vHeightOffset = "0 0.03 0";
	
	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableTriggerUsage(bool enableTriggers)
	{
		m_bUseTriggers = enableTriggers;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanUseTriggers()
	{
		return m_bUseTriggers;
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableRadiusUsage(bool enableRadius)
	{
		m_bUseRadius = enableRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanUseRadius()
	{
		return m_bUseRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true, if entity can be spawned on this spawner
	bool GetIsEntityAllowed(SCR_EntityInfo entityInfo)
	{
		if (entityInfo && entityInfo.GetEntityType() & m_eSupportedEntities)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Initiate spawn
	\param index of entity
	\param entity initiating the spawn
	*/
	void InitiateSpawn(int index, int userId)
	{
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(userId);
		if (!user)
			return;
		
		if (CanSpawn(index, user) == SCR_EntityRequestDeniedReason.CAN_SPAWN)
			PerformSpawn(index, user);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Checks if the given index has an entity assigned
	\param index of entity
	\return bool returns false if no entity is assigned
	*/
	SCR_EntityInfo GetEntryAtIndex(int index)
	{
		if (!m_EntityAssetList)
    		return null;

		return m_EntityAssetList.GetEntryAtIndex(index);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Checks if entity can be spawned
	\param index of entity
	\param entity initiating the spawn
	\return bool returns false if entity cannot be spawned
	*/
	int CanSpawn(int entityIndex = -1, IEntity user = null)
	{
		if (!m_EntityAssetList)
			return SCR_EntityRequestDeniedReason.NOT_AVAILABLE;
		
		SCR_EntityInfo spawnInfo = m_EntityAssetList.GetEntryAtIndex(entityIndex);
		
		if (!GetIsEntityAllowed(spawnInfo))
			return SCR_EntityRequestDeniedReason.NOT_AVAILABLE;
		
		if (spawnInfo && spawnInfo.GetCost() > GetSpawnerSupplies())
			return SCR_EntityRequestDeniedReason.NOT_ENOUGH_SUPPLIES;
		
		return SCR_EntityRequestDeniedReason.CAN_SPAWN;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Gets script invoker on entity spawns.
	Invoker will send over spawned IEntity 
	\return ScriptInvoker Event when entity is spawned
	*/
	ScriptInvoker GetOnEntitySpawned()
	{
		return Event_OnEntitySpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	// Send notification to player requesting spawn
	protected void SendNotification(int msgId, notnull IEntity user, int assetId = -1)
	{
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!playerController)
			return;
		
		SCR_PlayerEntitySpawnerRequestComponent reqComponent = SCR_PlayerEntitySpawnerRequestComponent.Cast(playerController.FindComponent(SCR_PlayerEntitySpawnerRequestComponent));
		if (!reqComponent)
			return;
		
		reqComponent.SendPlayerFeedback(msgId, assetId);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Spawn the actual entity
	\param index of entity
	\param entity initiating the spawn
	*/
	protected void PerformSpawn(int index = -1, IEntity user = null)
	{
		if (IsProxy())
			return;
		
		m_SpawnedEntity = null;
		
		SCR_EntityInfo spawnInfo;
		array<SCR_EntityInfo> assetList = {};
		
		if (!m_EntityAssetList || m_EntityAssetList.GetAssetList(assetList) <= 0)
		{
			Print("Asset list is null or empty! Cannot spawn!", LogLevel.ERROR);
			return;
		}
		else if (index == -1)
		{
			spawnInfo = assetList.GetRandomElement();
		}
		else if (index >= 0 && index < assetList.Count())
		{
			spawnInfo = assetList.Get(index);
		}
		
		ResourceName spawnResource = string.Empty;
		if (spawnInfo)
		{
			spawnResource = spawnInfo.GetPrefab();
		}
		
		Resource resource = Resource.Load(spawnResource);
		if (!resource || !resource.IsValid())
		{
			Print("Resource is null or empty! Cannot spawn!", LogLevel.ERROR);
			return;
		}
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		
		//Check spawn position if it is clean
		if (!IsSpawnPositionClean(resource))
		{			
			if (m_bUseTriggers)
			{
				IEntity trigger = GetEmptyTrigger();
				if (trigger)
					trigger.GetTransform(params.Transform);
			}
			
			if (m_bUseRadius && params.Transform[3] == vector.Zero)
			{
				vector pos;
				if (SCR_WorldTools.FindEmptyTerrainPosition(pos, GetOwner().GetOrigin(), m_fRadiusSize, m_fRadiusSpawnZoneSize))
					params.Transform[3] = pos;
			}
			
			if (params.Transform[3] == vector.Zero)
			{
				SendNotification(SCR_EntityRequestDeniedReason.NOT_ENOUGH_SPACE, user);
    			return;
			}
		}
		else
		{
			GetSpawnTransform(params.Transform);
		}
		
		m_SpawnedEntity = GetGame().SpawnEntityPrefab(resource, GetOwner().GetWorld(), params);
		if (!m_SpawnedEntity)
			return;
		
		AddSpawnerSupplies(-spawnInfo.GetCost());
		Event_OnEntitySpawned.Invoke(m_SpawnedEntity);
		
		//Send notification to player, whom requested the vehicle
		SendNotification(SCR_EntityRequestDeniedReason.CAN_SPAWN, user, index);
		
		Physics physicsComponent = m_SpawnedEntity.GetPhysics();
		if (!physicsComponent)
			return;
		
		m_SpawnedEntity.GetPhysics().SetVelocity("0 -0.1 0"); // Make the entity copy the terrain properly
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calculates the spawn tranformation matrix for the object
	protected void GetSpawnTransform(out vector outMat[4])
	{
		vector localMat[4], parentMat[4];
		GetOwner().GetWorldTransform(parentMat);
		Math3D.AnglesToMatrix(m_vSpawnOffsetRotation, localMat);
		localMat[3] = m_vSpawnOffsetPosition;
		
		Math3D.MatrixMultiply4(parentMat, localMat, outMat);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true, if spawn position is empty
	bool IsSpawnPositionClean(Resource resource)
	{
		if (!resource || !resource.IsValid())
			return false;
		
		ref EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		GetSpawnTransform(params.Transform);
		
		//Currently, material is still used for preview, even thought it shouldn't be seen, as function doesn't work correctly without it
		SCR_PrefabPreviewEntity previewEntity = SCR_PrefabPreviewEntity.Cast(SCR_PrefabPreviewEntity.SpawnPreviewFromPrefab(resource, "SCR_PrefabPreviewEntity", GetOwner().GetWorld(), params, "{56EBF5038622AC95}Assets/Conflict/CanBuild.emat"));
		if (!previewEntity)
			return false;
		
		TraceOBB paramOBB = new TraceOBB();
		Math3D.MatrixIdentity3(paramOBB.Mat);
		vector currentMat[4];
		previewEntity.GetTransform(currentMat);
		paramOBB.Mat[0] = currentMat[0];
		paramOBB.Mat[1] = currentMat[1];
		paramOBB.Mat[2] = currentMat[2];
		paramOBB.Start = currentMat[3] + m_vHeightOffset;
		paramOBB.Flags = TraceFlags.ENTS;
		paramOBB.Exclude = previewEntity;
		paramOBB.LayerMask = EPhysicsLayerPresets.Projectile;
		previewEntity.GetPreviewBounds(paramOBB.Mins, paramOBB.Maxs);
		
		GetGame().GetWorld().TracePosition(paramOBB, null);
		SCR_EntityHelper.DeleteEntityAndChildren(previewEntity);
		
		//If tracePosition found colliding entity, further checks will be done to determine whether can be actually something spawned
		if (!paramOBB.TraceEnt)
			return true;
		
		m_aEntitiesOnSpawnPosition.Clear();
		GetGame().GetWorld().QueryEntitiesByOBB(paramOBB.Mins, paramOBB.Maxs, currentMat, EntitySearchCallback);
			
		DamageManagerComponent comp;
		GarbageManager garbageMan = GetGame().GetGarbageManager();
			
		//Goes through all entities found through query. While dead bodies are ignored, vehicle wrecks are removed, if m_bDeleteWrecks is enabled
		for (int i = m_aEntitiesOnSpawnPosition.Count()-1; i >= 0; i--)
		{
			comp = DamageManagerComponent.Cast(m_aEntitiesOnSpawnPosition[i].FindComponent(DamageManagerComponent));
			
			if (ChimeraCharacter.Cast(m_aEntitiesOnSpawnPosition[i]) && comp.IsDestroyed())
			{
				m_aEntitiesOnSpawnPosition.Remove(i);
				continue;
			}
			
			if (m_aEntitiesOnSpawnPosition[i].IsInherited(Vehicle) && comp.IsDestroyed() && m_bDeleteWrecks)
			{
				if (garbageMan && garbageMan.IsInserted(m_aEntitiesOnSpawnPosition[i]))
					garbageMan.Withdraw(m_aEntitiesOnSpawnPosition[i]);
				
				SCR_EntityHelper.DeleteEntityAndChildren(m_aEntitiesOnSpawnPosition[i]);
				m_aEntitiesOnSpawnPosition.Remove(i);
			}	
		}
			
		if (!m_aEntitiesOnSpawnPosition.IsEmpty())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used to get all characters and vehicles obstructing spawn position
	protected bool EntitySearchCallback(IEntity ent)
	{	
		if (!ent.FindComponent(DamageManagerComponent))
			return true;
			
		m_aEntitiesOnSpawnPosition.Insert(ent);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Checks if the spawned entity is within range
	protected bool IsSpawnedEntityInRange()
	{
		return m_SpawnedEntity && vector.Distance(m_SpawnedEntity.GetOrigin(), GetOwner().GetOrigin()) < m_iEntitySpawnRange);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns empty trigger in hierarchy
	BaseGameTriggerEntity GetEmptyTrigger()
	{
		IEntity child = GetOwner().GetChildren();
		BaseGameTriggerEntity trigger;
		while (child)
		{
			if (child.Type() == BaseGameTriggerEntity)
			{
				trigger = BaseGameTriggerEntity.Cast(child);
				trigger.QueryEntitiesInside();
				array<IEntity> inside = new array<IEntity>();
				int cntInside = trigger.GetEntitiesInside(inside);
				
				if (cntInside != 0 && m_bDeleteWrecks)
				{
					GarbageManager garbageMan = GetGame().GetGarbageManager();
					foreach (IEntity ent : inside)
					{
						if (!ent)
							continue;
						
						DamageManagerComponent comp = DamageManagerComponent.Cast(ent.FindComponent(DamageManagerComponent));
							
						if (!comp || !comp.IsDestroyed())
							continue;
							
						if (garbageMan && garbageMan.IsInserted(ent))
							garbageMan.Withdraw(ent);
							
						SCR_EntityHelper.DeleteEntityAndChildren(ent);
					}
					
					cntInside = trigger.GetEntitiesInside(inside);
				}
				
				if (cntInside == 0)
					return trigger;
			}
			
			child = child.GetSibling();
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get supplies left for spawner
	\return float spawn supplies
	*/
	float GetSpawnerSupplies()
	{
		return m_fSpawnerSupplies;
	}
	
	/*!
	Set the spawner supplies value
	\param newValue new value to replace existing
	*/
	void SetSpawnerSupplies(float newValue)
	{
		Event_OnSpawnerSuppliesChanged.Invoke(m_fSpawnerSupplies, newValue);
		m_fSpawnerSupplies = newValue;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add specific amount of supplies 
	void AddSpawnerSupplies(float newValue)
	{
		Event_OnSpawnerSuppliesChanged.Invoke(m_fSpawnerSupplies, newValue);
		m_fSpawnerSupplies = m_fSpawnerSupplies + newValue;
	}
	
	/*!
	Add/remove given amount from spawner
	\param addRemove The value to add/remove
	*/
	void ChangeSpawnerSupplies(float addRemove)
	{
		Event_OnSpawnerSuppliesChanged.Invoke(m_fSpawnerSupplies, m_fSpawnerSupplies + addRemove);
		m_fSpawnerSupplies += addRemove;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!m_RplComponent)
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_EntitySpawnerComponent()
	{
		if (SCR_Global.IsEditMode())
			return;
	}
}

enum SCR_EntityRequestDeniedReason
	{
		CAN_SPAWN = 0,
		CAN_SPAWN_TRIGGER = 1,
		NOT_ENOUGH_SUPPLIES = 2,
		NOT_ENOUGH_SPACE = 3,
		NOT_AVAILABLE = 4,
		RANK_LOW = 5,
		COOLDOWN = 6
	};
