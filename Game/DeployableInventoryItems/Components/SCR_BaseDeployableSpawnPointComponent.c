[EntityEditorProps(category: "GameScripted/DeployableItems", description: "")]
class SCR_BaseDeployableSpawnPointComponentClass : SCR_BaseDeployableInventoryItemComponentClass
{
}

//! Base class which all deployable spawn points / radios inherit from
class SCR_BaseDeployableSpawnPointComponent : SCR_BaseDeployableInventoryItemComponent
{
	protected static ref array<SCR_BaseDeployableSpawnPointComponent> s_aActiveDeployedSpawnPoints = {};
	
	protected static ref ScriptInvokerInt s_OnSpawnPointDismantled;

	[Attribute("{84680168E273774F}Prefabs/MP/Spawning/DeployableItemSpawnPoint_Base.et")]
	protected ResourceName m_sSpawnPointPrefab;

	[Attribute()]
	protected ResourceName m_sReplacementPrefab;

	[Attribute()]
	protected FactionKey m_FactionKey;
	
	[Attribute(defvalue: "1")]
	protected bool m_bEnableSounds;

	protected SCR_DeployableSpawnPoint m_SpawnPoint;

	protected IEntity m_ReplacementEntity;

	protected vector m_aOriginalTransform[4];
	
	protected static bool s_bDeployableSpawnPointsEnabled;
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerInt GetOnSpawnPointDismantled()
	{
		if (!s_OnSpawnPointDismantled)
			s_OnSpawnPointDismantled = new ScriptInvokerInt();
		
		return s_OnSpawnPointDismantled;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetTransformBroadcast(vector transform[4])
	{
		GetOwner().SetTransform(transform);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RPC_PlaySoundOnDeployBroadcast(bool deploy)
	{
		SoundComponent soundComp = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (!soundComp)
			return;
		
		if (deploy)
			soundComp.SoundEvent(SCR_SoundEvent.SOUND_DEPLOYED_RADIO_DEPLOY);
		else
			soundComp.SoundEvent(SCR_SoundEvent.SOUND_DEPLOYED_RADIO_UNDEPLOY);
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleRadioChatter(bool enable)
	{
		SignalsManagerComponent signalsManagerComp = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));
		if (!signalsManagerComp)
			return;

		int radioChatter = signalsManagerComp.FindSignal("DeployableRadioChatter");

		if (enable)
			signalsManagerComp.SetSignalValue(radioChatter, 1);
		else
			signalsManagerComp.SetSignalValue(radioChatter, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompositionDestroyed(IEntity instigator)
	{
		Dismantle(instigator);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnRplDeployed()
	{
		if (m_bEnableSounds)
			ToggleRadioChatter(m_bIsDeployed);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns replacement composition and attaches owner entity to it - called from SCR_DeployItemBaseAction.PerformAction
	//! \param[in] userEntity
	override void Deploy(IEntity userEntity = null)
	{
		//~ Not allowed to deploy
		if (!s_bDeployableSpawnPointsEnabled)
			return;
		
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		vector transform[4];
		IEntity owner = GetOwner();
		SCR_TerrainHelper.GetTerrainBasis(owner.GetOrigin(), transform, GetGame().GetWorld(), false, new TraceParam());

		m_aOriginalTransform = transform;

		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = transform;
		params.TransformMode = ETransformMode.WORLD;

		Resource resource = Resource.Load(m_sReplacementPrefab);
		if (!resource.IsValid())
			return;

		m_ReplacementEntity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
		if (!m_ReplacementEntity)
			return;

		RplComponent rplCompReplacementEntity = RplComponent.Cast(m_ReplacementEntity.FindComponent(RplComponent));
		if (!rplCompReplacementEntity)
			return;

		SCR_DeployableInventoryItemReplacementComponent replacementComp = SCR_DeployableInventoryItemReplacementComponent.Cast(m_ReplacementEntity.FindComponent(SCR_DeployableInventoryItemReplacementComponent));
		if (!replacementComp)
			return;

		vector compositionTransform[4], combinedTransform[4];
		replacementComp.GetItemTransform(compositionTransform);
		Math3D.MatrixMultiply4(transform, compositionTransform, combinedTransform);

		RPC_SetTransformBroadcast(combinedTransform);
		Rpc(RPC_SetTransformBroadcast, combinedTransform);

		replacementComp.GetOnCompositionDestroyed().Insert(OnCompositionDestroyed);

		resource = Resource.Load(m_sSpawnPointPrefab);
		if (!resource.IsValid())
			return;

		m_SpawnPoint = SCR_DeployableSpawnPoint.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params));
		if (!m_SpawnPoint)
			return;

		RplComponent rplCompSpawnpoint = RplComponent.Cast(m_SpawnPoint.FindComponent(RplComponent));
		if (!rplCompSpawnpoint)
			return;

		m_SpawnPoint.SetDeployableSpawnPointComponent(this);
		m_SpawnPoint.SetFactionKey(m_FactionKey);

		m_bIsDeployed = true;
		Replication.BumpMe();

		if (userEntity)
			m_iItemOwnerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(userEntity);

		s_aActiveDeployedSpawnPoints.Insert(this);
		
		if (m_bEnableSounds)
		{
			ToggleRadioChatter(true);		
			RPC_PlaySoundOnDeployBroadcast(true);
			Rpc(RPC_PlaySoundOnDeployBroadcast, true);
		}

		auto garbageSystem = SCR_GarbageSystem.GetByEntityWorld(owner);
		if (garbageSystem)
			garbageSystem.Withdraw(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete replacement composition and spawnpoint and set position of owner entity to it's original position - called from SCR_DismantleItemBaseAction.PerformAction
	//! \param[in] userEntity
	override void Dismantle(IEntity userEntity = null)
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		RplComponent rplComp;
		if (m_SpawnPoint)
		{
			rplComp = RplComponent.Cast(m_SpawnPoint.FindComponent(RplComponent));
			if (!rplComp)
				return;

			rplComp.DeleteRplEntity(m_SpawnPoint, false);
		}

		if (m_ReplacementEntity)
		{
			rplComp = RplComponent.Cast(m_ReplacementEntity.FindComponent(RplComponent));
			if (!rplComp)
				return;

			rplComp.DeleteRplEntity(m_ReplacementEntity, false);
		}

		RPC_SetTransformBroadcast(m_aOriginalTransform);
		Rpc(RPC_SetTransformBroadcast, m_aOriginalTransform);

		m_bIsDeployed = false;
		Replication.BumpMe();

		m_iItemOwnerID = -1;

		s_aActiveDeployedSpawnPoints.RemoveItem(this);
		
		int userID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(userEntity);		
		if (s_OnSpawnPointDismantled)
			s_OnSpawnPointDismantled.Invoke(userID);
		
		if (m_bEnableSounds)
		{
			ToggleRadioChatter(false);		
			RPC_PlaySoundOnDeployBroadcast(false);
			Rpc(RPC_PlaySoundOnDeployBroadcast, false);
		}

		IEntity owner = GetOwner();
		auto garbageSystem = SCR_GarbageSystem.GetByEntityWorld(owner);
		if (garbageSystem)
			garbageSystem.Insert(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnSpawnPointDeployingEnabledChanged(bool enabled)
	{
		s_bDeployableSpawnPointsEnabled = enabled;
		
		if (enabled)
			return;
		
		foreach (SCR_BaseDeployableSpawnPointComponent spawnPointComp : s_aActiveDeployedSpawnPoints)
		{
			spawnPointComp.Dismantle();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static array<SCR_BaseDeployableSpawnPointComponent> GetActiveDeployedSpawnPoints()
	{
		return s_aActiveDeployedSpawnPoints;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_SpawnPoint GetSpawnPoint()
	{
		return m_SpawnPoint;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] timeSlice
	void Update(float timeSlice);
	
	//------------------------------------------------------------------------------------------------
	protected void ConnectToDeployableSpawnPointSystem()
	{
		World world = GetOwner().GetWorld();
		SCR_DeployableSpawnPointSystem updateSystem = SCR_DeployableSpawnPointSystem.Cast(world.FindSystem(SCR_DeployableSpawnPointSystem));
		if (!updateSystem)
			return;

		updateSystem.Register(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromDeployableSpawnPointSystem()
	{
		World world = GetOwner().GetWorld();
		SCR_DeployableSpawnPointSystem updateSystem = SCR_DeployableSpawnPointSystem.Cast(world.FindSystem(SCR_DeployableSpawnPointSystem));
		if (!updateSystem)
			return;

		updateSystem.Unregister(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);	
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		if (!playerSpawnPointManager)
		{
			OnSpawnPointDeployingEnabledChanged(true);
			return;
		}
		
		OnSpawnPointDeployingEnabledChanged(playerSpawnPointManager.IsDeployingSpawnPointsEnabled());
		playerSpawnPointManager.GetOnSpawnPointDeployingEnabledChanged().Insert(OnSpawnPointDeployingEnabledChanged);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_BaseDeployableSpawnPointComponent()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
		{
			SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
			if (playerSpawnPointManager)
				playerSpawnPointManager.GetOnSpawnPointDeployingEnabledChanged().Remove(OnSpawnPointDeployingEnabledChanged);
		}
		
		if (m_bIsDeployed && Replication.IsServer())
			s_aActiveDeployedSpawnPoints.RemoveItem(this);
	}
}
