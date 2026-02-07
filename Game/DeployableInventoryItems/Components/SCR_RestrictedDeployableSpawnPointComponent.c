enum SCR_ESpawnPointBudgetType
{
	NONE = 0,
	SUPPLIES = 1,
	SPAWNTICKET = 2
}

[EntityEditorProps(category: "GameScripted/DeployableItems", description: "")]
class SCR_RestrictedDeployableSpawnPointComponentClass : SCR_BaseDeployableSpawnPointComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! Deployable spawn point with configurable conditions
class SCR_RestrictedDeployableSpawnPointComponent : SCR_BaseDeployableSpawnPointComponent
{
	// Setup
	[Attribute("{5A258632A5C32E48}Prefabs/MP/Spawning/ItemSpecifics/RestrictedDeployableSpawnPoint_Radio_Supplies.et", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", category: "Setup")]
	protected ResourceName m_sSpawnPointPrefabSupplies;
	
	// General	
	[Attribute(defvalue: "1", desc: "Play audio cue once spawn point can/cannot be deployed", category: "General")]
	protected bool m_bPlaySoundOnZoneEntered;

	[Attribute(defvalue: "1", desc: "Show notification once spawn point can/cannot be deployed", category: "General")]
	protected bool m_bShowNotificationOnZoneEntered;
	
	[Attribute(defvalue: "1", category: "General")]
	protected int m_iMaxSpawnPointsPerGroup;
	
	// Respawning	
	[Attribute(defvalue: SCR_ESpawnPointBudgetType.SUPPLIES.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ESpawnPointBudgetType), category: "Respawning"), RplProp()]
	protected SCR_ESpawnPointBudgetType m_eRespawnBudgetType;
	
	[Attribute(defvalue: "100", category: "Respawning")]
	protected float m_fSuppliesValue;

	[Attribute(defvalue: "5", category: "Respawning")]
	protected int m_iMaxRespawns;
	
	[Attribute(defvalue: "0", category: "Respawning")]
	protected bool m_bAllowAllGroupsToSpawn;
	
	[Attribute(defvalue: "0", category: "Respawning")]
	protected bool m_bAllowCustomLoadouts;

	// Queries
	[Attribute(defvalue: "1.0", desc: "Rate at which spawn point will check if it can be deployed or not", category: "Queries")]
	protected float m_fUpdateRate;
	
	[Attribute(defvalue: "1", desc: "Check if bases are nearby before deploying", category: "Queries")]
	protected bool m_bQueryBases;

	[Attribute(defvalue: "0", desc: "Check if spawn points are nearby before deploying", category: "Queries")]
	protected bool m_bQuerySpawnPoints;

	[Attribute(defvalue: "0", desc: "Check if characters are nearby before deploying", category: "Queries")]
	protected bool m_bQueryCharacters;

	[Attribute(defvalue: "300.0", desc: "Query radius for military bases", category: "Queries")]
	protected float m_fQueryRadiusBases;

	[Attribute(defvalue: "250.0", desc: "Query radius for other existing spawn points", category: "Queries")]
	protected float m_fQueryRadiusSpawnPoints;

	[Attribute(defvalue: "100.0", desc: "Query radius for enemy characters", category: "Queries")]
	protected float m_fQueryRadiusCharacters;

	[Attribute(defvalue: "0", desc: "Query all military bases, not just Main Operating Bases", category: "Queries")]
	protected bool m_bQueryAllBases;

	[Attribute(defvalue: "0", category: "Queries")]
	protected bool m_bIgnoreEnemyBases;

	[Attribute(defvalue: "0", category: "Queries")]
	protected bool m_bIgnoreEnemySpawnPoints;

	[Attribute(defvalue: "0", category: "Queries")]
	protected bool m_bIgnoreEnemyCharacters;

	// User actions
	[Attribute(defvalue: "0", category: "User Actions")]
	protected bool m_bUnlockActionsForEnemyFactions;

	[Attribute(defvalue: "0", category: "User Actions")]
	protected bool m_bUnlockActionsForAllGroups;
	
	[Attribute(defvalue: "#AR-DeployableSpawnPoints_UserAction_OutsideDeployArea", category: "User Actions")]
	protected string m_sOutsideDeployAreaMessage;
	
	[Attribute(defvalue: "#AR-DeployableSpawnPoints_UserAction_DeployLimitReached", category: "User Actions")]
	protected string m_sDeployLimitReachedMessage;
	
	[Attribute(defvalue: "#AR-DeployableSpawnPoints_UserAction_NoGroupJoined", category: "User Actions")]
	protected string m_sNoGroupJoinedMessage;

	[RplProp()]
	protected int m_iGroupID = -1;
	
	[RplProp()]
	protected bool m_bIsOutsideExclusionZone;
	
	protected SCR_AIGroup m_LocalPlayerGroup;
	
	protected static ref array<int> s_aActiveDeployedSpawnPointGroupIDs = {};
	
	protected bool m_bIsGroupLimitReached;
	protected bool m_bNoGroupJoined;
	protected bool m_bSpawnLimitReached;
	protected bool m_bIsWornByPlayer;
	
	protected int m_iRespawnCount;
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
				if (!isFriendlyFaction && !m_bIgnoreEnemyBases)
					return false;
			}
			else if (isSpawnPoint)
			{
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
	protected bool CanActionBeShown(notnull IEntity userEntity, bool checkFaction, bool checkGroupID)
	{
		if (!m_bDeployableSpawnPointsEnabled)
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
	protected bool CanBeDeployedAtPosition(vector position, notnull IEntity userEntity, out int notification = -1)
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
				if (!baseComponent)
					continue;
				
				if (baseComponent.GetFaction() != faction)
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
				if (!spawnPoint)
					continue;
				
				if (!SCR_PlayerRadioSpawnPoint.Cast(spawnPoint) && spawnPoint.GetFactionKey() != faction.GetFactionKey())
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
				if (!character)
					continue;
				
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
		if (!m_LocalPlayerGroup)
			return false;

		return m_LocalPlayerGroup.GetDeployedRadioCount() >= m_iMaxSpawnPointsPerGroup;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected SCR_DeployableSpawnPoint CreateSpawnPoint()
	{
		if (m_sSpawnPointPrefab.IsEmpty() || m_sSpawnPointPrefabSupplies.IsEmpty())
			return null;
		
		Resource resource; 
		if (m_eRespawnBudgetType == SCR_ESpawnPointBudgetType.SUPPLIES)
			resource = Resource.Load(m_sSpawnPointPrefabSupplies);
		else
			resource = Resource.Load(m_sSpawnPointPrefab);
		
		if (!resource.IsValid())
			return null;

		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = m_aOriginalTransform;
		params.TransformMode = ETransformMode.WORLD;
		
		return SCR_DeployableSpawnPoint.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params));
	}

	//------------------------------------------------------------------------------------------------
	//! Check if deploy is possible, then call super.Deploy()
	override void Deploy(IEntity userEntity, bool reload = false)
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

		super.Deploy(userEntity, reload);

		if (!m_SpawnPoint)
			return;

		SCR_RestrictedDeployableSpawnPoint restrictedSpawnPoint = SCR_RestrictedDeployableSpawnPoint.Cast(m_SpawnPoint);
		if (!restrictedSpawnPoint)
			return;
		
		restrictedSpawnPoint.SetAllowAllGroupsToSpawn(m_bAllowAllGroupsToSpawn);
		restrictedSpawnPoint.SetGroupID(m_iGroupID);
		restrictedSpawnPoint.SetBudgetType(m_eRespawnBudgetType);
		
		restrictedSpawnPoint.SetRespawnCount(m_iRespawnCount);
		restrictedSpawnPoint.SetMaxRespawns(m_iMaxRespawns);
		restrictedSpawnPoint.SetLoadoutAllowed(m_bAllowCustomLoadouts);

		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(m_SpawnPoint.FindComponent(SCR_ResourceComponent));
		if (resourceComponent)
		{
			SCR_ResourceContainer container = resourceComponent.GetContainer(EResourceType.SUPPLIES);
			if (!container)
				return;
		
			container.SetResourceValue(m_fSuppliesValue);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Cache respawn count; then call super.Dismantle()
	override void Dismantle(IEntity userEntity = null, bool reload = false)
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;
		
		if (m_SpawnPoint)
		{
			SCR_RestrictedDeployableSpawnPoint restrictedSpawnPoint = SCR_RestrictedDeployableSpawnPoint.Cast(m_SpawnPoint);
			if (!restrictedSpawnPoint)
				return;
			
			m_iRespawnCount = restrictedSpawnPoint.GetRespawnCount();
			
			SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(m_SpawnPoint.FindComponent(SCR_ResourceComponent));
			if (resourceComponent)
			{
				SCR_ResourceContainer container = resourceComponent.GetContainer(EResourceType.SUPPLIES);
				if (!container)
					return;
				
				m_fSuppliesValue = container.GetResourceValue();
			}
		}

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup playerGroup = groupsManager.FindGroup(m_iGroupID);
		if (!playerGroup)
			return;
		
		playerGroup.DecreaseDeployedRadioCount();
		
		m_iGroupID = -1; // reset groupID		
		Replication.BumpMe();

		super.Dismantle(userEntity, reload);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Display amount of respawns left - called from SCR_ShowDeployableSpawnPointInfo.PerformAction
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
	//! Toggles spawning with custom loadout
	void ToggleSpawningWithLoadout(notnull IEntity userEntity)
	{
		SCR_RestrictedDeployableSpawnPoint restrictedSpawnPoint = SCR_RestrictedDeployableSpawnPoint.Cast(m_SpawnPoint);
		if (!restrictedSpawnPoint)
			return;
		
		m_bAllowCustomLoadouts = !m_bAllowCustomLoadouts;		
		restrictedSpawnPoint.SetLoadoutAllowed(m_bAllowCustomLoadouts);
		
		SCR_PossessingManagerComponent possessingManagerComp = SCR_PossessingManagerComponent.GetInstance();
		if (!possessingManagerComp)
			return;

		int userID = possessingManagerComp.GetIdFromControlledEntity(userEntity);
		
		if (m_bAllowCustomLoadouts)
			SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_LOADOUTS_ALLOWED);
		else
			SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_LOADOUTS_BANNED);		
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] reason
	//! \return
	bool CanDeployBePerformed(out string reason)
	{
		m_bIsGroupLimitReached = IsDeployLimitReachedLocal();
		
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
		if (!CanActionBeShown(userEntity, true, false))
			return false;

		return !m_bIsDeployed;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanDismantleBeShown(notnull IEntity userEntity)
	{
		if (!CanActionBeShown(userEntity, false, false))
			return false;

		return m_bIsDeployed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] userEntity
	//! \return
	bool CanInfoBeShown(notnull IEntity userEntity)
	{
		return m_eRespawnBudgetType == SCR_ESpawnPointBudgetType.SPAWNTICKET && CanActionBeShown(userEntity, true, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] userEntity
	//! \return
	bool CanOpenStorageBeShown(notnull IEntity userEntity)
	{
		if (m_eRespawnBudgetType != SCR_ESpawnPointBudgetType.SUPPLIES || !CanActionBeShown(userEntity, true, true))
			return false;

		return m_bIsDeployed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] userEntity
	//! \return
	bool CanToggleLoadoutSpawningBeShown(notnull IEntity userEntity)
	{
		IEntity ownerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iItemOwnerID);
		if (!ownerEntity || ownerEntity != userEntity)
			return false;
		
		if (m_eRespawnBudgetType != SCR_ESpawnPointBudgetType.SUPPLIES || !CanActionBeShown(userEntity, true, true))
			return false;

		return m_bIsDeployed;
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
	//! \param[in] maxRespawns
	void SetMaxRespawns(int maxRespawns)
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;
		
		m_iMaxRespawns = maxRespawns;
		Reload();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] budgetType
	void SetBudgetType(SCR_ESpawnPointBudgetType budgetType)
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;
		
		m_eRespawnBudgetType = budgetType;
		Replication.BumpMe();
		Reload();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnGroupChanged(int groupID)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		m_LocalPlayerGroup = groupsManager.FindGroup(groupID);
		if (!m_LocalPlayerGroup)
			m_bNoGroupJoined = true;
		else		
			m_bNoGroupJoined = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupRemoved(SCR_AIGroup group)
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup itemGroup = groupsManager.FindGroup(m_iGroupID);

		if (group == itemGroup)
			Dismantle(); // Dismantle item if removed group is the same as the item group; otherwise deployed item would be unusable
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSpawnPointBudgetTypeChanged(SCR_ESpawnPointBudgetType budgetType)
	{
		SetBudgetType(budgetType);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSpawnPointTicketAmountChanged(int tickets)
	{
		SetMaxRespawns(tickets);
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
		
		if (!m_bDeployableSpawnPointsEnabled)
			return;
		
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
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		if (playerSpawnPointManager)
		{
			playerSpawnPointManager.GetOnSpawnPointBudgetTypeChanged().Insert(OnSpawnPointBudgetTypeChanged);
			playerSpawnPointManager.GetOnSpawnPointTicketAmountChanged().Insert(OnSpawnPointTicketAmountChanged);
		}
		
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!groupComponent)
			return;
		
		groupComponent.GetOnGroupChanged().Insert(OnGroupChanged);
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		groupsManager.GetOnPlayableGroupRemoved().Insert(OnGroupRemoved);
		m_LocalPlayerGroup = groupsManager.GetPlayerGroup(playerController.GetPlayerId());
		
		m_bIsGroupLimitReached = IsDeployLimitReachedLocal();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromDeployableSpawnPointSystem();
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		groupsManager.GetOnPlayableGroupRemoved().Remove(OnGroupRemoved);
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		if (playerSpawnPointManager)
		{
			playerSpawnPointManager.GetOnSpawnPointBudgetTypeChanged().Remove(OnSpawnPointBudgetTypeChanged);
			playerSpawnPointManager.GetOnSpawnPointTicketAmountChanged().Remove(OnSpawnPointTicketAmountChanged);
		}
		
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
		
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!groupComponent)
			return;
		
		groupComponent.GetOnGroupChanged().Remove(OnGroupChanged);
		
		super.OnDelete(owner);
	}
}
