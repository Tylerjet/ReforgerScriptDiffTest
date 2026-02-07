[EntityEditorProps(category: "GameScripted/DeployableItems", description: "")]
class SCR_RestrictedDeployableSpawnPointComponentClass : SCR_BaseDeployableSpawnPointComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! Deployable spawn point with configurable conditions
class SCR_RestrictedDeployableSpawnPointComponent : SCR_BaseDeployableSpawnPointComponent
{
	[Attribute(defvalue: "1")]
	protected int m_iMaxSpawnPointsPerGroup;

	[Attribute(defvalue: "10")]
	protected int m_iMaxRespawns;

	[Attribute(defvalue: "1", desc: "Check if bases are nearby before deploying")]
	protected bool m_bQueryBases;

	[Attribute(defvalue: "0", desc: "Check if spawn points are nearby before deploying")]
	protected bool m_bQuerySpawnPoints;

	[Attribute(defvalue: "0", desc: "Check if characters are nearby before deploying")]
	protected bool m_bQueryCharacters;

	[Attribute(defvalue: "300.0", desc: "Query radius for military bases")]
	protected float m_fQueryRadiusBases;

	[Attribute(defvalue: "250.0", desc: "Query radius for other existing spawn points")]
	protected float m_fQueryRadiusSpawnPoints;

	[Attribute(defvalue: "100.0", desc: "Query radius for enemy characters")]
	protected float m_fQueryRadiusCharacters;

	[Attribute(defvalue: "0", desc: "Query all military bases, not just Main Operating Bases")]
	protected bool m_bQueryAllBases;

	[Attribute(defvalue: "0")]
	protected bool m_bIgnoreEnemyBases;

	[Attribute(defvalue: "0")]
	protected bool m_bIgnoreEnemySpawnPoints;

	[Attribute(defvalue: "0")]
	protected bool m_bIgnoreEnemyCharacters;

	[Attribute(defvalue: "0")]
	protected bool m_bUnlockActionsForEnemyFactions;

	[Attribute(defvalue: "0")]
	protected bool m_bAllowAllGroupsToSpawn;

	[Attribute(defvalue: "0")]
	protected bool m_bUnlockActionsForAllGroups;

	[Attribute(defvalue: "1", desc: "Play audio cue once spawn point can/cannot be deployed")]
	protected bool m_bPlaySoundOnZoneEntered;

	[Attribute(defvalue: "1", desc: "Show notification once spawn point can/cannot be deployed")]
	protected bool m_bShowNotificationOnZoneEntered;

	[Attribute(defvalue: "1.0", desc: "Rate at which spawn point will check if it can be deployed or not")]
	protected float m_fUpdateRate;
	
	[Attribute(defvalue: "#AR-DeployableSpawnPoints_UserAction_OutsideDeployArea")]
	protected string m_sOutsideDeployAreaMessage;
	
	[Attribute(defvalue: "#AR-DeployableSpawnPoints_UserAction_DeployLimitReached")]
	protected string m_sDeployLimitReachedMessage;
	
	[Attribute(defvalue: "#AR-DeployableSpawnPoints_UserAction_NoGroupJoined")]
	protected string m_sNoGroupJoinedMessage;

	[RplProp()]
	protected int m_iGroupID = -1;
	
	[RplProp()]
	protected bool m_bIsOutsideExclusionZone;
	
	protected bool m_bIsGroupLimitReached;
	protected bool m_bNoGroupJoined;
	
	protected static ref array<int> s_aActiveDeployedSpawnPointGroupIDs = {};

	protected int m_iRespawnCount;

	protected bool m_bSpawnLimitReached;
	protected bool m_bIsWornByPlayer;

	protected float m_fTimeSinceUpdate;

#ifdef ENABLE_DIAG
	protected static ref array<ref Shape> s_aDebugShapes = {};
#endif

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_AddSpawnPointGroupBroadcast(int groupID)
	{
		s_aActiveDeployedSpawnPointGroupIDs.Insert(groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_RemoveSpawnPointGroupBroadcast(int groupID)
	{
		s_aActiveDeployedSpawnPointGroupIDs.RemoveItem(groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RPC_PlaySoundOnZoneEnteredBroadcast(bool entered)
	{
		SoundComponent soundComp = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (!soundComp)
			return;

		if (entered)
			soundComp.SoundEvent(SCR_SoundEvent.SOUND_DEPLOYED_RADIO_ENTER_ZONE);
		else
			soundComp.SoundEvent(SCR_SoundEvent.SOUND_DEPLOYED_RADIO_EXIT_ZONE);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when there are no entities in the specified area that could prevent deploying
	protected bool EntityQuery(notnull array<IEntity> entities, notnull Faction spawnPointFaction, vector spawnPointOrigin)
	{
		if (entities.IsEmpty())
			return true;

		float queryRadius = 100; // Default 100 meters
		bool isBase, isSpawnPoint, isCharacter;

		if (SCR_MilitaryBaseComponent.Cast(entities[0].FindComponent(SCR_MilitaryBaseComponent)))
		{
			queryRadius = m_fQueryRadiusBases;
			isBase = true;
		}
		else if (SCR_SpawnPoint.Cast(entities[0]))
		{
			queryRadius = m_fQueryRadiusSpawnPoints;
			isSpawnPoint = true;
		}
		else if (SCR_ChimeraCharacter.Cast(entities[0]))
		{
			queryRadius = m_fQueryRadiusCharacters;
			isCharacter = true;
		}

		float radiusSq = queryRadius * queryRadius;

		foreach (IEntity e : entities)
		{
			vector origin = e.GetOrigin();	
					
#ifdef ENABLE_DIAG
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_DEPLOYABLE_SPAWNPOINTS_ENABLE_DIAG))
				s_aDebugShapes.Insert(Shape.CreateSphere(Color.RED, ShapeFlags.WIREFRAME, origin, queryRadius));
#endif			
			
			float distanceToItemSq = vector.DistanceSq(spawnPointOrigin, origin);
			bool isWithinRadius = distanceToItemSq < radiusSq;

			if (!isWithinRadius)
				continue;

			SCR_Faction faction = SCR_Faction.Cast(SCR_Faction.GetEntityFaction(e));
			if (!faction)
				return false;

			bool isFriendlyFaction = faction.DoCheckIfFactionFriendly(spawnPointFaction);

			if (isBase)
			{					
				if (isFriendlyFaction)
					return false;

				if (!isFriendlyFaction && !m_bIgnoreEnemyBases)
					return false;
			}
			else if (isSpawnPoint)
			{			
				if (isFriendlyFaction)
					return false;

				if (!isFriendlyFaction && !m_bIgnoreEnemySpawnPoints)
					return false;
			}
			else if (isCharacter)
			{			
				if (!isFriendlyFaction && !m_bIgnoreEnemyCharacters)
					return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanActionBeShown(notnull IEntity userEntity, bool checkCanDeploy, bool checkFaction = true, bool checkGroupID = true)
	{
		if (!s_bDeployableSpawnPointsEnabled)
			return false;

		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(userEntity.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliation)
			return false;

		Faction affiliatedFaction = factionAffiliation.GetAffiliatedFaction();
		if (!affiliatedFaction)
			return false;

		FactionKey userFactionKey = affiliatedFaction.GetFactionKey();
		if (!userFactionKey)
			return false;

		if (userFactionKey != m_FactionKey && !m_bUnlockActionsForEnemyFactions && checkFaction)
			return false;

		SCR_PossessingManagerComponent possessingManagerComp = SCR_PossessingManagerComponent.GetInstance();
		if (!possessingManagerComp)
			return false;

		int userID = possessingManagerComp.GetIdFromControlledEntity(userEntity);

		SCR_PlayerControllerGroupComponent playerControllerGroupComp = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(userID);
		if (!playerControllerGroupComp)
			return false;

		int userGroupID = playerControllerGroupComp.GetGroupID();

		if (userFactionKey == m_FactionKey && userGroupID != m_iGroupID && !m_bUnlockActionsForAllGroups && checkGroupID)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanBeDeployedAtPosition(vector position, IEntity userEntity, out int notification = -1)
	{
		array<IEntity> entities = {};

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return false;

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByKey(m_FactionKey));

		// Check for nearby bases and prevent deploy if there are any
		if (m_bQueryBases)
		{
			SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
			if (!baseManager)
				return false;

			array<SCR_MilitaryBaseComponent> baseComponents = {};

			baseManager.GetBases(baseComponents);

			foreach (SCR_MilitaryBaseComponent baseComponent : baseComponents)
			{
				if (m_bQueryAllBases)
				{
					entities.Insert(baseComponent.GetOwner());
					continue;
				}

				SCR_CampaignMilitaryBaseComponent campaignBaseComponent = SCR_CampaignMilitaryBaseComponent.Cast(baseComponent);
				if (!campaignBaseComponent || !campaignBaseComponent.IsHQ())
					continue;

				entities.Insert(baseComponent.GetOwner());
			}

			if (!EntityQuery(entities, faction, position))
			{
				notification = ENotification.DEPLOYABLE_SPAWNPOINTS_ZONE_EXITED;
				return false;
			}
		}

		// Check for nearby spawnpoints and prevent deploy if there are any
		if (m_bQuerySpawnPoints)
		{
			entities = {};

			array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPoints();

			foreach (SCR_SpawnPoint spawnPoint : spawnPoints)
			{
				if (!SCR_PlayerRadioSpawnPoint.Cast(spawnPoint))
					entities.Insert(spawnPoint);
			}

			if (!EntityQuery(entities, faction, position))
			{
				notification = ENotification.DEPLOYABLE_SPAWNPOINTS_ZONE_EXITED;
				return false;
			}
		}

		// Check for nearby enemy characters and prevent deploy if there are any
		if (m_bQueryCharacters)
		{
			entities = {};

			array<SCR_ChimeraCharacter> characters = SCR_CharacterRegistrationComponent.GetChimeraCharacters();
			if (!characters)
				return false;

			foreach (SCR_ChimeraCharacter character : characters)
			{
				if (character == userEntity || character.GetCharacterController().IsDead())
					continue;

				entities.Insert(character);
			}

			if (!EntityQuery(entities, faction, position))
			{
				notification = ENotification.DEPLOYABLE_SPAWNPOINTS_ZONE_EXITED;
				return false;
			}
		}

		notification = ENotification.DEPLOYABLE_SPAWNPOINTS_ZONE_ENTERED;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsDeployLimitReachedLocal()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;
		
		SCR_AIGroup group = groupsManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!group)
		{
			m_bNoGroupJoined = true;
			return false;
		}
		
		m_bNoGroupJoined = false;
		return group.GetDeployedRadioCount() >= m_iMaxSpawnPointsPerGroup;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if deploy is possible, then call super.Deploy()
	override void Deploy(IEntity userEntity)
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;
		
		int userID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(userEntity);
		
		SCR_PlayerControllerGroupComponent playerControllerGroupComp = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(userID);
		if (!playerControllerGroupComp)
			return;

		m_iGroupID = playerControllerGroupComp.GetGroupID();		
		Replication.BumpMe();
				
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup playerGroup = groupsManager.FindGroup(m_iGroupID);
		if (!playerGroup)
			return;
		
		playerGroup.IncreaseDeployedRadioCount();

		super.Deploy(userEntity);

		if (!m_SpawnPoint)
			return;

		SCR_RestrictedDeployableSpawnPoint restrictedSpawnPoint = SCR_RestrictedDeployableSpawnPoint.Cast(m_SpawnPoint);
		if (!restrictedSpawnPoint)
			return;

		restrictedSpawnPoint.SetRespawnCount(m_iRespawnCount);
		restrictedSpawnPoint.SetMaxRespawns(m_iMaxRespawns);

		restrictedSpawnPoint.SetAllowAllGroupsToSpawn(m_bAllowAllGroupsToSpawn);
		restrictedSpawnPoint.SetGroupID(m_iGroupID);
	}

	//------------------------------------------------------------------------------------------------
	//! Cache respawn count; then call super.Dismantle()
	override void Dismantle(IEntity userEntity = null)
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		SCR_RestrictedDeployableSpawnPoint restrictedSpawnPoint = SCR_RestrictedDeployableSpawnPoint.Cast(m_SpawnPoint);
		if (!restrictedSpawnPoint)
			return;

		m_iRespawnCount = restrictedSpawnPoint.GetRespawnCount();

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup playerGroup = groupsManager.FindGroup(m_iGroupID);
		if (!playerGroup)
			return;
		
		playerGroup.DecreaseDeployedRadioCount();
		
		m_iGroupID = -1; // reset groupID		
		Replication.BumpMe();

		super.Dismantle(userEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Display amount of respawns left - called from SCR_ShowItemInfo.PerformAction
	void ShowInfo(notnull IEntity userEntity)
	{
		SCR_PossessingManagerComponent possessingManagerComp = SCR_PossessingManagerComponent.GetInstance();
		if (!possessingManagerComp)
			return;

		int userID = possessingManagerComp.GetIdFromControlledEntity(userEntity);
		int respawnsLeft = m_iMaxRespawns - m_iRespawnCount;

		if (!m_bIsDeployed || m_bAllowAllGroupsToSpawn)
		{
			SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_DISPLAY_RESPAWN_COUNT, respawnsLeft, m_iMaxRespawns);
			return;
		}

		SCR_PlayerControllerGroupComponent playerControllerGroupComp = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(userID);
		if (!playerControllerGroupComp)
			return;

		int userGroupID = playerControllerGroupComp.GetGroupID();

		if (userGroupID != m_iGroupID)
		{
			SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_DISPLAY_GROUP, m_iGroupID);
			return;
		}

		SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_DISPLAY_RESPAWN_COUNT, respawnsLeft, m_iMaxRespawns);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] reason
	//! \return
	bool CanDeployBePerformed(out string reason)
	{
		if (m_bNoGroupJoined)
			reason = m_sNoGroupJoinedMessage;
		else if (m_bIsGroupLimitReached)
			reason = m_sDeployLimitReachedMessage;
		else if (!m_bIsOutsideExclusionZone)
			reason = m_sOutsideDeployAreaMessage;
		
		return m_bIsOutsideExclusionZone && !m_bIsGroupLimitReached && !m_bNoGroupJoined;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanDeployBeShown(notnull IEntity userEntity)
	{
		if (!CanActionBeShown(userEntity, true, checkGroupID : false))
			return false;

		return !m_bIsDeployed;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanDismantleBeShown(notnull IEntity userEntity)
	{
		if (!CanActionBeShown(userEntity, false, checkFaction : false))
			return false;

		return m_bIsDeployed;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] userEntity
	//! \return
	bool CanInfoBeShown(notnull IEntity userEntity)
	{
		return CanActionBeShown(userEntity, !m_bIsDeployed, checkGroupID : false);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetAllowAllGroupsToSpawn()
	{
		return m_bAllowAllGroupsToSpawn;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIgnoreEnemyCharacters()
	{
		return m_bIgnoreEnemyCharacters;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetGroupID()
	{
		return m_iGroupID;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetQueryRadiusCharacters()
	{
		return m_fQueryRadiusCharacters;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] respawnCount
	void SetRespawnCount(int respawnCount)
	{
		m_iRespawnCount = respawnCount;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupRemoved(SCR_AIGroup group)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup itemGroup = groupsManager.FindGroup(m_iGroupID);

		if (group == itemGroup)
			Dismantle(); // Dismantle item if removed group is the same as the item group; otherwise deployed item would be unusable
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		super.Update(owner, timeSlice);	

		if (m_fTimeSinceUpdate < m_fUpdateRate)
		{
			m_fTimeSinceUpdate += timeSlice;
			return;
		}
		
		m_fTimeSinceUpdate = 0;

		//timed execution
		
#ifdef ENABLE_DIAG
		if (m_bIsWornByPlayer)
			s_aDebugShapes.Clear();
#endif
		
		if (!s_bDeployableSpawnPointsEnabled)
			return;
		
		m_bIsGroupLimitReached = IsDeployLimitReachedLocal();
		
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;
		
		//server code

		InventoryItemComponent item = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (!item)
			return;
		
		InventoryStorageSlot parentSlot = item.GetParentSlot();
		if (!parentSlot)
		{
			m_bIsWornByPlayer = false;
			return;
		}
		
		IEntity parentEntity;
		while (parentSlot)
		{
			parentEntity = parentSlot.GetStorage().GetOwner();
			parentSlot = parentSlot.GetStorage().GetParentSlot();
		}

		if (!parentEntity || !SCR_ChimeraCharacter.Cast(parentEntity))
		{
			m_bIsWornByPlayer = false;
			return;
		}

		int userID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(parentEntity);
		if (userID <= 0)
		{
			m_bIsWornByPlayer = false;
			return;
		}
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(parentEntity.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliation)
			return;

		Faction affiliatedFaction = factionAffiliation.GetAffiliatedFaction();
		if (!affiliatedFaction)
			return;

		FactionKey userFactionKey = affiliatedFaction.GetFactionKey();
		if (!userFactionKey)
			return;

		if (userFactionKey != m_FactionKey)
		{
			m_bIsWornByPlayer = false;
			return;
		}

		m_bIsWornByPlayer = true;

		int notification;
		bool canBeDeployedAtPos = CanBeDeployedAtPosition(GetOwner().GetOrigin(), parentEntity, notification);

		if (m_bIsOutsideExclusionZone == canBeDeployedAtPos)
			return;
		
		m_bIsOutsideExclusionZone = canBeDeployedAtPos;
		Replication.BumpMe();

		if (m_bShowNotificationOnZoneEntered && notification > -1)
			SCR_NotificationsComponent.SendToPlayer(userID, notification);

		if (m_bEnableSounds && m_bPlaySoundOnZoneEntered)
		{
			RPC_PlaySoundOnZoneEnteredBroadcast(canBeDeployedAtPos);
			Rpc(RPC_PlaySoundOnZoneEnteredBroadcast, canBeDeployedAtPos);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		ConnectToDeployableSpawnPointSystem();

		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		//first position check for items that are pre-placed in editor
		int dummyNotification;
		m_bIsOutsideExclusionZone = CanBeDeployedAtPosition(GetOwner().GetOrigin(), null, dummyNotification);
		Replication.BumpMe();

		groupsManager.GetOnPlayableGroupRemoved().Insert(OnGroupRemoved);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromDeployableSpawnPointSystem();
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		groupsManager.GetOnPlayableGroupRemoved().Remove(OnGroupRemoved);
		
		if (m_bIsDeployed && Replication.IsServer())
		{			
			SCR_AIGroup playerGroup = groupsManager.FindGroup(m_iGroupID);
			if (!playerGroup)
				return;
			
			playerGroup.DecreaseDeployedRadioCount();
		}
		
#ifdef ENABLE_DIAG
		if (m_bIsWornByPlayer)
			s_aDebugShapes.Clear();
#endif
		
		super.OnDelete(owner);
	}
}
