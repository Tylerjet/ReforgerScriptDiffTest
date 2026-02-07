//------------------------------------------------------------------------------------------------
class SCR_RespawnSystemComponentClass: RespawnSystemComponentClass
{
};

//! Scripted implementation that handles spawning and respawning of players.
//! Should be attached to a GameMode entity.
[ComponentEditorProps(icon: HYBRID_COMPONENT_ICON)]
class SCR_RespawnSystemComponent : RespawnSystemComponent
{
	[Attribute("0", uiwidget: UIWidgets.CheckBox, "Skip faction and loadout selection for faster debugging.\nHas effect only in Workbench!", category: "Respawn System")]
	protected bool m_bDebugSkipSelection;
	
	[Attribute("1", uiwidget: UIWidgets.CheckBox, category: "Respawn System")]
	protected bool m_bEnableRespawn;

	const string LOG_HEAD = "[RespawnSystemComponent]:";

	// Instance of this component
	private static SCR_RespawnSystemComponent s_Instance = null;
	
	protected ref ScriptInvoker m_OnPlayerFactionChanged = new ScriptInvoker();

	// The parent of this entity which should be a gamemode
	protected SCR_BaseGameMode m_pGameMode;
	// Parent entity's rpl component
	protected RplComponent m_pRplComponent;

	// Map of all respawn infos, key is playerID
	[RplProp(onRplName: "HandlePlayerRespawnInfos")]
	protected ref array<ref SCR_PlayerRespawnInfo> m_mPlayerRespawnInfos = new array<ref SCR_PlayerRespawnInfo>();

	// Helper items
	[RplProp()]
	protected ref array<int> m_aFactionPlayerCount = {};
	[RplProp()]
	protected ref array<int> m_aLoadoutPlayerCount = {};
	[RplProp()]
	protected ref array<int> m_aSpawnPointsPlayerCount = {};

	// Custom respawn variables
	protected string m_sCustomRespawnPrefab;
	protected bool m_bCustomRespawn;
	protected vector m_vCustomRespawnPos;
	protected vector m_vCustomRespawnRot;
	protected GenericEntity m_CustomSpawnedEntity;

	// Preload
	protected ref SimplePreload m_Preload;
	
	protected ref ScriptInvoker Event_OnRespawnEnabledChanged = new ref ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	//! Returns an instance of RespawnSystemComponent
	static SCR_RespawnSystemComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode pGameMode = GetGame().GetGameMode();
			if (pGameMode)
				s_Instance = SCR_RespawnSystemComponent.Cast(pGameMode.FindComponent(SCR_RespawnSystemComponent));
		}

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns either a valid Faction of local player character or null
	static Faction GetLocalPlayerFaction(IEntity player = null)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(player);
		if (character)
			return character.GetFaction();

		SCR_RespawnSystemComponent respawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		if (respawnSystemComponent)
			return respawnSystemComponent.GetPlayerFaction(SCR_PlayerController.GetLocalPlayerId());
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Access to replication component
	RplComponent GetRplComponent()
	{
		return m_pRplComponent;
	}

	//------------------------------------------------------------------------------------------------
	// Convenience checking for validity of input in intarray
	protected bool IsIntArrayIndexValid(notnull array<int> inArray, int index)
	{
		return (index >= 0 && index < inArray.Count());
	}

	//------------------------------------------------------------------------------------------------
	// Called when a spawn is requested
	// Asks the gamemode with PickPlayerSpawnPoint query expecting to get a spawn point
	// at which the player should be spawned
	protected override GenericEntity RequestSpawn(int playerId)
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		SCR_ReconnectComponent reconComp = SCR_ReconnectComponent.GetInstance();
		if (!reconComp || !reconComp.IsReconnectEnabled() || !reconComp.IsInReconnectList(playerId) )
		{
		}
		else 
		{
			IEntity ent = reconComp.ReturnControlledEntity(playerId);
			if (ent)
				return GenericEntity.Cast(ent);
		}

		// Catch illicit requests,
		// TODO@AS:
		// TODO@LK:
		// We should probably make it so RequestRespawn()
		// is not even called from client if !CanPlayerRespawn(playerId)
		// and only resort to this as a safety measure
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode && !gameMode.CanPlayerRespawn(playerId) && !m_bCustomRespawn)
		{
			Print("Requested spawn denied! GameMode returned false in CanPlayerRespawn() for playerId=" + playerId, LogLevel.WARNING);
			return null;
		}
		
		if (m_bCustomRespawn)
		{
			m_CustomSpawnedEntity = DoSpawn(m_sCustomRespawnPrefab, m_vCustomRespawnPos, m_vCustomRespawnRot);
			if (!m_CustomSpawnedEntity)
				return null;

			FactionAffiliationComponent affiliationComp = FactionAffiliationComponent.Cast(m_CustomSpawnedEntity.FindComponent(FactionAffiliationComponent));
			if (affiliationComp)
			{
				Faction faction = affiliationComp.GetAffiliatedFaction();
				if (faction)
					DoSetPlayerFaction(playerId, GetFactionIndex(faction));
			}

			return m_CustomSpawnedEntity;
		}

		SCR_BasePlayerLoadout loadout = GetPlayerLoadout(playerId);
		if (!loadout)
		{
			Print(LOG_HEAD+" No valid entity to spawn could be returned in RequestSpawn. Are there valid loadouts for the target player faction?", LogLevel.ERROR);
			return null;
		}

		SCR_SpawnPoint spawnPoint = GetPlayerSpawnPoint(playerId);
		if (!spawnPoint)
		{
			Print(LOG_HEAD+" No valid spawn point available in RequestSpawn. Player will not spawn!", LogLevel.ERROR);
			return null;
		}
		
		SCR_PlayerSpawnPoint playerSpawnPoint = SCR_PlayerSpawnPoint.Cast(spawnPoint);
		if (playerSpawnPoint)
		{
			if (!CanSpawnOnPlayerSpawnPoint(playerSpawnPoint))
			{
				string playerFactionKey;
				Faction faction = GetPlayerFaction(playerId);
				if (faction)
					playerFactionKey = faction.GetFactionKey();
				
				SCR_SpawnPoint nearestSpawnPoint = FindNearestAvailableSpawnPoint(playerSpawnPoint.GetOrigin(), playerFactionKey, playerId);
				if (nearestSpawnPoint)
					spawnPoint = nearestSpawnPoint;
			}
		}
		
		vector spawnPosition = vector.Zero;
		vector spawnRotation = vector.Zero;
		if (spawnPoint)
		{
			spawnPoint.GetPositionAndRotation(spawnPosition, spawnRotation);
			m_pGameMode.OnSpawnPointUsed(spawnPoint, playerId);
		}

		GenericEntity spawned = DoSpawn(loadout.GetLoadoutResource(), spawnPosition, spawnRotation);
		loadout.OnLoadoutSpawned(spawned, playerId);
		if (spawnPoint)
			spawnPoint.EOnPlayerSpawn(spawned);
		
		return spawned;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SpawnPoint FindNearestAvailableSpawnPoint(vector origin, string playerFactionKey, int playerId)
	{
		array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPointsForFaction(playerFactionKey);
		
		if (!spawnPoints)
			return null;
		
		float currentDistance;
		float minDistance = float.MAX;
		RplId rplId;
		SCR_PlayerSpawnPoint playerSpawnPoint;
		int availableSpawnPointIndex = -1;
		for (int i = 0, count = spawnPoints.Count(); i < count; i++)
		{
			if (!spawnPoints[i])
				continue;
			
			rplId = SCR_SpawnPoint.GetSpawnPointRplId(spawnPoints[i]);
			
			if (!CanSetSpawnPoint(playerId, rplId))
				continue;
			
			playerSpawnPoint = SCR_PlayerSpawnPoint.Cast(spawnPoints[i]);
			if (playerSpawnPoint && !CanSpawnOnPlayerSpawnPoint(playerSpawnPoint))
				continue;
			
			currentDistance = vector.DistanceSq(origin, spawnPoints[i].GetOrigin());
			if (currentDistance < minDistance)
			{
				availableSpawnPointIndex = i;
			}
		}
		
		if (availableSpawnPointIndex > -1)
			return spawnPoints[availableSpawnPointIndex];
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool HasFreeSlot(IEntity entity)
	{
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(entity.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return false;
		
		array<BaseCompartmentSlot> compartments = {};
		compartmentManager.GetCompartments(compartments);
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (!compartment.IsCompartmentAccessible())
				continue;
			
			if (compartment.GetOccupant())
				continue;
			
			return true;
		}
		
		//--- Scan compartments in child entities (ToDo: Remove once GetCompartments is recursive)
		bool childHasFreeSlot;
		IEntity child = entity.GetChildren();
		while (child)
		{
			childHasFreeSlot = HasFreeSlot(child);
			if (childHasFreeSlot)
				return true;
			
			child = child.GetSibling();
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool CanSpawnOnPlayerSpawnPoint(notnull SCR_PlayerSpawnPoint playerSpawnPoint)
	{
		IEntity targetPlayer = playerSpawnPoint.GetTargetPlayer();
		if (!targetPlayer)
			return true;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(targetPlayer);
		if (!character || !character.IsInVehicle())
			return true;
		
		CompartmentAccessComponent compartmentAccessComponent = character.GetCompartmentAccessComponent();
		if (!compartmentAccessComponent)
			return true;
		
		BaseCompartmentSlot slot = BaseCompartmentSlot.Cast(compartmentAccessComponent.GetCompartment());
		if (!slot)
			return true;
		
		int slotID;
		IEntity vehicle = slot.GetVehicle(slotID);
		if (!vehicle)
			return true;
		
		if (!HasFreeSlot(vehicle))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnPlayerFactionChanged()
	{
		return m_OnPlayerFactionChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	// Perform respawn using custom prefab
	// NOTE: temporary hack for 14546, please use with caution
	// TODO(koudelkaluk): remove me once there's a proper solution
	GenericEntity CustomRespawn(int playerId, string prefab, vector position, vector rotation = vector.Zero)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!pc)
			return null;

		//--- Kill the current character, we don't want it to idle around
		IEntity currentEntity = pc.GetMainEntity();
		if (currentEntity)
		{
			SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(currentEntity.FindComponent(SCR_CharacterControllerComponent));
			if (characterController)
				characterController.ForceDeath();
		}

		m_sCustomRespawnPrefab = prefab;
		m_vCustomRespawnPos = position;
		m_vCustomRespawnRot = rotation;

		m_bCustomRespawn = true;
		pc.RequestRespawn();
		m_bCustomRespawn = false;

		return m_CustomSpawnedEntity;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Verifies that provided loadout belongs to player assigned faction if faction manager is used.
	*/
	bool CanSetLoadout(int playerId, int loadoutIndex)
	{
		// Unassign loadout
		if (loadoutIndex == SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
			return true;

		// Cannot assign invalid loadout
		SCR_BasePlayerLoadout loadout = GetLoadoutByIndex(loadoutIndex);
		if (!loadout)
			false;

		// Loadout is non existant
		// No faction manager; thus we skip all faction checks
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return true;

		// Player has no faction, cannot assign faction loadout
		Faction playerFaction = GetPlayerFaction(playerId);
		if (!playerFaction)
			return false;

		// Loadout is not faction based
		SCR_FactionPlayerLoadout factionLoadout = SCR_FactionPlayerLoadout.Cast(loadout);
		if (!factionLoadout)
			return false;

		// Compare whether loadout faction matches the predicate
		return factionLoadout.GetFactionKey() == playerFaction.GetFactionKey();
	}

	//------------------------------------------------------------------------------------------------
	// Called from SCR_RespawnComponent
	void DoSetPlayerLoadout(int playerId, int loadoutIndex)
	{
		// Verify that loadout can be set
		if (!CanSetLoadout(playerId, loadoutIndex))
		{
			ERespawnSelectionResult res = ERespawnSelectionResult.ERROR_FORBIDDEN;
			// In case no player controller is present, respawn component will not be present either,
			// can happen in case of disconnection
			SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
			if (respawnComponent)
				respawnComponent.AcknowledgePlayerLoadoutSet(res);
			return;
		}

		SCR_BasePlayerLoadout loadOut = GetPlayerLoadout(playerId);

		int oldIndex = GetLoadoutIndex(loadOut);
		if (oldIndex != loadoutIndex)
		{
			if (oldIndex != SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
			{
				if (IsIntArrayIndexValid(m_aLoadoutPlayerCount, oldIndex))
					m_aLoadoutPlayerCount[oldIndex] = m_aLoadoutPlayerCount[oldIndex]-1;
			}

			if (IsIntArrayIndexValid(m_aLoadoutPlayerCount, loadoutIndex))
				m_aLoadoutPlayerCount[loadoutIndex] = m_aLoadoutPlayerCount[loadoutIndex]+1;
			else if (loadoutIndex != SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
				Print("Provided Loadout index in " + this.ToString() + " is out of array bounds!", LogLevel.ERROR);
		}

#ifdef RESPAWN_COMPONENT_VERBOSE
		Print("SCR_RespawnSystemComponent::DoSetPlayerLoadout(playerId: "+playerId+", loadoutIndex: "+loadoutIndex+")");
#endif
		RpcDo_SetPlayerLoadout(playerId, loadoutIndex);
		Rpc(RpcDo_SetPlayerLoadout, playerId, loadoutIndex);

		Replication.BumpMe();

		ERespawnSelectionResult res = ERespawnSelectionResult.OK;
		// In case no player controller is present, respawn component will not be present either,
		// can happen in case of disconnection
		SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
		if (respawnComponent)
			respawnComponent.AcknowledgePlayerLoadoutSet(res);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Verifies that provided faction can be assigned to provided player.
	*/
	bool CanSetFaction(int playerId, int factionIndex)
	{
		if (factionIndex == SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
			return true;

		// Disallow invalid factions
		Faction faction = GetFactionByIndex(factionIndex);
		if (!faction)
			return false;

		SCR_Faction scriptedFaction = SCR_Faction.Cast(faction);	
			
		//Verify that faction is playable Faction. If not then it checks if the spawned player has GM rights
		if (scriptedFaction && !scriptedFaction.IsPlayable())
		{
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (!core)
				return false;
		
			//Check if has GM rights if not return false
			SCR_EditorManagerEntity editorManager = core.GetEditorManager(playerId);
			return editorManager && !editorManager.IsLimited(); 
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Called from SCR_RespawnComponent
	void DoSetPlayerFaction(int playerId, int factionIndex)
	{
		// Verify that faction makes sense
		if (!CanSetFaction(playerId, factionIndex))
		{
			ERespawnSelectionResult res = ERespawnSelectionResult.ERROR_FORBIDDEN;
			// In case no player controller is present, respawn component will not be present either,
			// can happen in case of disconnection
			SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
			if (respawnComponent)
				respawnComponent.AcknowledgePlayerFactionSet(res);
			return;
		}

		Faction faction = GetPlayerFaction(playerId);

		int oldIndex = GetFactionIndex(faction);
		if (oldIndex != factionIndex)
		{
			if (oldIndex != SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
			{
				if (IsIntArrayIndexValid(m_aFactionPlayerCount, oldIndex))
				{
					m_aFactionPlayerCount[oldIndex] = m_aFactionPlayerCount[oldIndex]-1;
					Replication.BumpMe();
				}
			}

			if (IsIntArrayIndexValid(m_aFactionPlayerCount, factionIndex))
			{
				m_aFactionPlayerCount[factionIndex] = m_aFactionPlayerCount[factionIndex]+1;
				Replication.BumpMe();
			}
			else if (factionIndex != SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
				Print("Provided Faction index in " + this.ToString() + " is out of array bounds!", LogLevel.ERROR);
		}

#ifdef RESPAWN_COMPONENT_VERBOSE
		Print("SCR_RespawnSystemComponent::DoSetPlayerFaction(playerId: "+playerId+", factionIndex: "+factionIndex+")");
#endif
		m_OnPlayerFactionChanged.Invoke(playerId, factionIndex);
		RpcDo_SetPlayerFaction(playerId, factionIndex);
		Rpc(RpcDo_SetPlayerFaction, playerId, factionIndex);

		Replication.BumpMe();

		ERespawnSelectionResult res = ERespawnSelectionResult.OK;
		// In case no player controller is present, respawn component will not be present either,
		// can happen in case of disconnection
		SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
		if (respawnComponent)
			respawnComponent.AcknowledgePlayerFactionSet(res);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Verifies that provided spawnPoint belongs to player assigned faction if faction manager is used.
	*/
	bool CanSetSpawnPoint(int playerId, RplId spawnPointId)
	{
		if (spawnPointId == SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX || spawnPointId == RplId.Invalid())
			return true;

		if (!spawnPointId.IsValid())
			return false;

		// Spawn point existance check
		SCR_SpawnPoint spawnPoint = GetSpawnPointByIdentity(spawnPointId);
		if (!spawnPoint)
			return false;

		// Spawn point can't be faction based without faction manager
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return true;

		// Player has no faction, cannot assign any faction spawn points
		Faction playerFaction = GetPlayerFaction(playerId);
		if (!playerFaction)
			return false;

		// Compare whether loadout faction matches the predicate
		return spawnPoint.GetFactionKey() == playerFaction.GetFactionKey();
	}

	//------------------------------------------------------------------------------------------------
	// Called from SCR_RespawnComponent
	void DoSetPlayerSpawnPoint(int playerId, RplId spawnPointIdentity)
	{
		/*
		SCR_SpawnPoint spawnPoint = GetPlayerSpawnPoint(playerId);
		int oldIndex = GetSpawnPointIndex(spawnPoint);
		if (oldIndex != spawnPointIndex)
		{
			if (oldIndex != SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
			{
				if (IsIntArrayIndexValid(m_aSpawnPointsPlayerCount, oldIndex))
					m_aSpawnPointsPlayerCount[oldIndex] = m_aSpawnPointsPlayerCount[oldIndex]-1;
			}

			// Ensure our list contains right amount of elements
			while (m_aSpawnPointsPlayerCount.Count() < spawnPointIndex + 1)
				m_aSpawnPointsPlayerCount.Insert(0);

			if (IsIntArrayIndexValid(m_aSpawnPointsPlayerCount, spawnPointIndex))
				m_aSpawnPointsPlayerCount[spawnPointIndex] = m_aSpawnPointsPlayerCount[spawnPointIndex]+1;
			else if (spawnPointIndex != SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
				Print("Provided SpawnPoint index in " + this.ToString() + " is out of array bounds!", LogLevel.ERROR);
		}*/

		if (!CanSetSpawnPoint(playerId, spawnPointIdentity))
		{
			ERespawnSelectionResult res = ERespawnSelectionResult.ERROR_FORBIDDEN;
			// In case no player controller is present, respawn component will not be present either,
			// can happen in case of disconnection
			SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
			if (respawnComponent)
				respawnComponent.AcknowledgePlayerSpawnPointSet(res);
			return;
		}

#ifdef RESPAWN_COMPONENT_VERBOSE
		int id = spawnPointIdentity;
		Print("SCR_RespawnSystemComponent::DoSetPlayerSpawnPoint(playerId: "+playerId+", spawnPointIdentity: "+id+")");
#endif
		RpcDo_SetPlayerSpawnPoint(playerId, spawnPointIdentity);
		Rpc(RpcDo_SetPlayerSpawnPoint, playerId, spawnPointIdentity);

		Replication.BumpMe();

		ERespawnSelectionResult res = ERespawnSelectionResult.OK;
		// In case no player controller is present, respawn component will not be present either,
		// can happen in case of disconnection
		SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
		if (respawnComponent)
			respawnComponent.AcknowledgePlayerSpawnPointSet(res);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetPlayerLoadout(int playerId, int loadoutIndex)
	{
#ifdef RESPAWN_COMPONENT_VERBOSE
		Print("SCR_RespawnSystemComponent::RpcDo_SetPlayerLoadout(playerId: "+playerId+", loadoutIndex: "+loadoutIndex+")");
#endif
		SetPlayerLoadout(playerId, loadoutIndex);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetPlayerFaction(int playerId, int factionIndex)
	{
#ifdef RESPAWN_COMPONENT_VERBOSE
		Print("SCR_RespawnSystemComponent::RpcDo_SetPlayerFaction(playerId: "+playerId+", factionIndex: "+factionIndex+")");
#endif
		// Reset loadouts and spawn points as these might be faction specific.
		SetPlayerLoadout(playerId, -1);
		SetPlayerSpawnPoint(playerId, RplId.Invalid());

		SetPlayerFaction(playerId, factionIndex);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetPlayerSpawnPoint(int playerId, RplId spawnPointIdentity)
	{
#ifdef RESPAWN_COMPONENT_VERBOSE
		int id = spawnPointIdentity;
		Print("SCR_RespawnSystemComponent::RpcDo_SetPlayerSpawnPoint(playerId: "+playerId+", spawnPointIdentity: "+id+")");
#endif
		SetPlayerSpawnPoint(playerId, spawnPointIdentity);
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_PlayerRespawnInfo> GetPlayerRespawnInfoMap()
	{
		return m_mPlayerRespawnInfos;
	}

	//------------------------------------------------------------------------------------------------
	void HandlePlayerRespawnInfos()
	{
		// if (RplSession.Mode() == RplMode.Dedicated)
		// 	return;

		// int plId = SCR_PlayerController.GetLocalPlayerId();
		// SCR_PlayerRespawnInfo info = FindPlayerRespawnInfo(plId);
		// if (info || plId > 0)
		// 	m_bInfoRegistered = true;

#ifdef RESPAWN_COMPONENT_VERBOSE
		//Print("SCR_RespawnSystemComponent::HandlePlayerRespawnInfos - playerId: " + plId + ", found: " + (info != null));

		for (int i = 0; i < m_mPlayerRespawnInfos.Count(); i++)
		{
			SCR_PlayerRespawnInfo pInfo = m_mPlayerRespawnInfos[i];
			Print("  item " + i + ", playerId: " + pInfo.GetPlayerID());
		}
#endif
	}

	//------------------------------------------------------------------------------------------------
	int GetFactionPlayerCount(Faction faction)
	{
		if (!faction)
			return 0;

		int index = GetFactionIndex(faction);
		if (index != SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
			return m_aFactionPlayerCount[index];

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	int GetLoadoutPlayerCount(SCR_BasePlayerLoadout loadout)
	{
		if (!loadout)
			return 0;

		int index = GetLoadoutIndex(loadout);
		if (index != SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
			return m_aLoadoutPlayerCount[index];

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	int GetSpawnPointPlayerCount(SCR_SpawnPoint spawnPoint)
	{
		if (!spawnPoint)
			return 0;

		int index = GetSpawnPointIndex(spawnPoint);
		if (index != SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
			return m_aSpawnPointsPlayerCount[index];

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	void SetPlayerLoadout(int playerId, int loadoutIndex)
	{
		SCR_PlayerRespawnInfo playerRespawnInfo = FindPlayerRespawnInfo(playerId);
		if (!playerRespawnInfo)
			playerRespawnInfo = RegisterPlayerRespawnInfo(playerId);

		SCR_BasePlayerLoadout loadout = GetLoadoutByIndex(loadoutIndex);

		if (playerRespawnInfo)
			playerRespawnInfo.SetPlayerLoadoutIndex(loadoutIndex);

		if (m_pGameMode)
			m_pGameMode.HandleOnLoadoutAssigned(playerId, loadout);
	}

	//------------------------------------------------------------------------------------------------
	void SetPlayerFaction(int playerId, int factionIndex)
	{
		SCR_PlayerRespawnInfo playerRespawnInfo = FindPlayerRespawnInfo(playerId);
		if (!playerRespawnInfo)
			playerRespawnInfo = RegisterPlayerRespawnInfo(playerId);

		Faction faction = GetFactionByIndex(factionIndex);

		if (playerRespawnInfo)
			playerRespawnInfo.SetPlayerFactionIndex(factionIndex);

		if (m_pGameMode)
			m_pGameMode.HandleOnFactionAssigned(playerId, faction);
	}


	//------------------------------------------------------------------------------------------------
	void SetPlayerSpawnPoint(int playerId, RplId spawnPointIdentity)
	{
		SCR_PlayerRespawnInfo playerRespawnInfo = FindPlayerRespawnInfo(playerId);
		if (!playerRespawnInfo)
			playerRespawnInfo = RegisterPlayerRespawnInfo(playerId);

		SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.GetSpawnPointByRplId(spawnPointIdentity);
		int id = spawnPointIdentity;

		if (playerRespawnInfo)
			playerRespawnInfo.SetPlayerSpawnPointIdentity(spawnPointIdentity);

		if (m_pGameMode)
			m_pGameMode.HandleOnSpawnPointAssigned(playerId, spawnPoint);
	}


	//------------------------------------------------------------------------------------------------
	void OnPlayerConnected(int playerId)
	{
		RegisterPlayerRespawnInfo(playerId);
		//Rpc(RpcAsk_SyncPlayerRespawnInfos);
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected(int playerId)
	{
		// Clean previous loadout and faction
		if (!m_pRplComponent || !m_pRplComponent.IsProxy())
		{
			DoSetPlayerLoadout(playerId, SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX);
			DoSetPlayerFaction(playerId, SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX);
			DoSetPlayerSpawnPoint(playerId, SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX);
		}

		UnregisterPlayerRespawnInfo(playerId);
		// todo: send rpc to delete
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Tries to find PlayerRespawnInfo for corresponding playerId
	//! Returns a the PlayerRespawnInfo or null if none
	SCR_PlayerRespawnInfo FindPlayerRespawnInfo(int playerId)
	{
		int length = m_mPlayerRespawnInfos.Count();
		for (int i = 0; i < length; i++)
		{
			SCR_PlayerRespawnInfo pInfo = m_mPlayerRespawnInfos[i];
			if (pInfo.GetPlayerID() == playerId)
				return pInfo;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns faction corresponding to the provided index or null if none exists
	Faction GetFactionByIndex(int factionIndex)
	{
		if (factionIndex == SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
			return null;

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return null;

		return factionManager.GetFactionByIndex(factionIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Return faction index or -1 if not existant
	int GetFactionIndex(Faction faction)
	{
		if (!faction)
			return SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX;

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX;

		return factionManager.GetFactionIndex(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Try to get player faction from PlayerRespawnInfo
	//! If faction index is within valid bounds, return Faction otherwise null
	Faction GetPlayerFaction(int playerId)
	{
		SCR_PlayerRespawnInfo playerRespawnInfo = FindPlayerRespawnInfo(playerId);
		if (!playerRespawnInfo)
			return null;

		return GetFactionByIndex(playerRespawnInfo.GetPlayerFactionIndex());
	}

	//------------------------------------------------------------------------------------------------
	//! Returns loadout corresponding to the provided index or null if none exists
	SCR_BasePlayerLoadout GetLoadoutByIndex(int loadoutIndex)
	{
		if (loadoutIndex == SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
			return null;

		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		if (!loadoutManager)
			return null;

		return loadoutManager.GetLoadoutByIndex(loadoutIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Try to get player loadout from PlayerRespawnInfo
	//! If loadout index is within valid bounds, return Faction otherwise null
	SCR_BasePlayerLoadout GetPlayerLoadout(int playerId)
	{
		SCR_PlayerRespawnInfo playerRespawnInfo = FindPlayerRespawnInfo(playerId);
		if (!playerRespawnInfo)
			return null;

		int loadoutIndex = playerRespawnInfo.GetPlayerLoadoutIndex();
		return GetLoadoutByIndex(loadoutIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Return faction index or -1 if not existant
	int GetLoadoutIndex(SCR_BasePlayerLoadout loadout)
	{
		if (!loadout)
			return SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX;

		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		if (!loadoutManager)
			return SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX;

		return loadoutManager.GetLoadoutIndex(loadout);
	}

	//------------------------------------------------------------------------------------------------
	//! Return spawn point identity or RplId.Invalid() if not existant
	SCR_SpawnPoint GetSpawnPointByIdentity(RplId spawnPointIdentity)
	{
		return SCR_SpawnPoint.GetSpawnPointByRplId(spawnPointIdentity);
	}

	//------------------------------------------------------------------------------------------------
	//! Try to get player loadout from PlayerRespawnInfo
	//! If loadout index is within valid bounds, return Faction otherwise null
	SCR_SpawnPoint GetPlayerSpawnPoint(int playerId)
	{
		SCR_PlayerRespawnInfo playerRespawnInfo = FindPlayerRespawnInfo(playerId);
		if (!playerRespawnInfo)
			return null;

		RplId spawnPointIdentity = playerRespawnInfo.GetPlayerSpawnPointIdentity();
		SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.GetSpawnPointByRplId(spawnPointIdentity);

		return spawnPoint;
	}

	//------------------------------------------------------------------------------------------------
	//! Return spawn point index or -1 if not existant
	int GetSpawnPointIndex(SCR_SpawnPoint spawnPoint)
	{
		if (!spawnPoint)
			return SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX;

		return SCR_SpawnPoint.GetSpawnPointIndex(spawnPoint);
	}

	//------------------------------------------------------------------------------------------------
	//! If playerRespawnInfo for given playerId doesn't exist it is created and ptr returned
	//! If playerRespawnInfo for given playerId already exists, it's returned instead
	protected SCR_PlayerRespawnInfo RegisterPlayerRespawnInfo(int playerId)
	{
		// PlayerInfo already exists, return it straight away
		SCR_PlayerRespawnInfo info = FindPlayerRespawnInfo(playerId);
		if (info)
		{
#ifdef RESPAWN_COMPONENT_VERBOSE
			Print("SCR_RespawnSystemComponent::RegisterPlayerRespawnInfo - already exists, player:\n\tplayerId: " + info.GetPlayerID() + "\n\tfactionId: " + info.GetPlayerFactionIndex());

			for (int i = 0; i < m_mPlayerRespawnInfos.Count(); i++)
			{
				SCR_PlayerRespawnInfo pInfo = m_mPlayerRespawnInfos[i];
				Print("  item " + i + ", playerId: " + pInfo.GetPlayerID());
			}
#endif
			return info;
		}

		// PlayerInfo does not exist, create new one
		SCR_PlayerRespawnInfo targetPlayerInfo = new ref SCR_PlayerRespawnInfo();
		targetPlayerInfo.SetPlayerID(playerId);

#ifdef RESPAWN_COMPONENT_VERBOSE
		Print("SCR_RespawnSystemComponent::RegisterPlayerRespawnInfo - Registering player:\n\tplayerId: " + targetPlayerInfo.GetPlayerID() + "\n\tfactionId: " + targetPlayerInfo.GetPlayerFactionIndex());

		for (int i = 0; i < m_mPlayerRespawnInfos.Count(); i++)
		{
			SCR_PlayerRespawnInfo pInfo = m_mPlayerRespawnInfos[i];
			Print("  item " + i + ", playerId: " + pInfo.GetPlayerID());
		}
#endif

		// Insert PlayerInfo to info list and bump replication
		m_mPlayerRespawnInfos.Insert(targetPlayerInfo);
		HandlePlayerRespawnInfos();
		Replication.BumpMe();

		return targetPlayerInfo;
	}

	//------------------------------------------------------------------------------------------------
	//! Removes playerRespawnInfo for given playerId from the map
	protected void UnregisterPlayerRespawnInfo(int playerId)
	{
		SCR_PlayerRespawnInfo targetPlayerInfo = FindPlayerRespawnInfo(playerId);
		if (targetPlayerInfo)
		{
#ifdef RESPAWN_COMPONENT_VERBOSE
		Print("SCR_RespawnSystemComponent::UnregisterPlayerRespawnInfo - Removing player:\n\tplayerId: " + playerId + "\n\tfactionId: " + targetPlayerInfo.GetPlayerFactionIndex());
#endif

			m_mPlayerRespawnInfos.RemoveItem(targetPlayerInfo);
			Replication.BumpMe();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! UI management
	static MenuBase OpenRespawnMenu()
	{
		MenuManager pMenuManager = GetGame().GetMenuManager();
		if (!pMenuManager)
			return null;

		return pMenuManager.OpenMenu(ChimeraMenuPreset.RespawnSuperMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! Close all menus operated by Respawn System
	static void CloseRespawnMenu()
	{
		MenuManager pMenuManager = GetGame().GetMenuManager();
		if (!pMenuManager)
			return;

		MenuBase menu = pMenuManager.FindMenuByPreset(ChimeraMenuPreset.RespawnSuperMenu);
		if (menu)
			pMenuManager.CloseMenu(menu);
	}

	//------------------------------------------------------------------------------------------------
	//! Simple getter for other
	static bool IsRespawnMenuOpened()
	{
		MenuManager pMenuManager = GetGame().GetMenuManager();
		if (!pMenuManager)
			return false;

		return (pMenuManager.FindMenuByPreset(ChimeraMenuPreset.RespawnSuperMenu) != null);
	}

	//------------------------------------------------------------------------------------------------
	//! Close all menus operated by Respawn System
	static void ToggleRespawnMenu()
	{
		if (IsRespawnMenuOpened())
			CloseRespawnMenu();
		else
			OpenRespawnMenu();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set respawn enabled Server only
	\param enableSpawning set respawn enabled or not
	*/
	void ServerSetEnableRespawn(bool enableSpawning)
	{
		if (!Replication.IsServer() || enableSpawning == m_bEnableRespawn)
			return;
		
		SetEnableRespawnBroadcast(enableSpawning);
		Rpc(SetEnableRespawnBroadcast, enableSpawning);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetEnableRespawnBroadcast(bool enableSpawning)
	{
		m_bEnableRespawn = enableSpawning;
		Event_OnRespawnEnabledChanged.Invoke(m_bEnableRespawn);		
	}


	//------------------------------------------------------------------------------------------------
	/*!
	Get respawn enabled
	\return bool m_bEnableRespawn
	*/
	bool IsRespawnEnabled()
	{
		return m_bEnableRespawn;
	}
	
	/*!
	Get on respawn changed script invoker
	\return ScriptInvoker Event_OnRespawnEnabledChanged which is called when server enables or disables respawn
	*/
	ScriptInvoker GetOnRespawnEnabledChanged()
	{
		return Event_OnRespawnEnabledChanged;
	}

	/*!
		Initialize all arrays of player counts per item.
	*/
	protected void InitializePlayerCounts()
	{
		ArmaReforgerScripted game = GetGame();

		// Initialize factions
		FactionManager factionManager = game.GetFactionManager();
		if (factionManager)
		{
			array<Faction> buff = {};
			int count = factionManager.GetFactionsList(buff);

			// When initializing insert 0 players per faction, as
			// nobody is selected yet
			for (int i = 0; i < count; i++)
				m_aFactionPlayerCount.Insert(0);
		}

		// Initialize loadouts
		SCR_LoadoutManager loadoutManager = game.GetLoadoutManager();
		if (loadoutManager)
		{
			int count = loadoutManager.GetLoadoutCount();
			// When initializing insert 0 players per loadout, as
			// nobody is selected yet
			for (int i = 0; i < count; i++)
				m_aLoadoutPlayerCount.Insert(0);
		}

		// Initialize spawn points
		array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPoints();
		if (spawnPoints)
		{
			int count = spawnPoints.Count();
			for (int i = 0; i < count; i++)
				m_aSpawnPointsPlayerCount.Insert(0);
		}

		// Sync changes
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		m_pGameMode = SCR_BaseGameMode.Cast(owner);
		if (m_pGameMode)
		{
			// Use replication of the parent
			m_pRplComponent = RplComponent.Cast(m_pGameMode.FindComponent(RplComponent));

			if (m_pGameMode.IsMaster())
				InitializePlayerCounts();
			// Initialize items on the server

		}
		else
			// If parent is not gamemode, print an error
			Print("SCR_RespawnSystemComponent has to be attached to a SCR_BaseGameMode (or inherited) entity!", LogLevel.ERROR);
		
		
		if (GetGame().InPlayMode())
		{
			// Validate faction manager
			FactionManager factionManager = GetGame().GetFactionManager();
			if (factionManager)
			{
				array<Faction> _ = {};
				int factionsCount = factionManager.GetFactionsList(_);
	
				if (factionsCount <= 0)
				{
					Debug.Error("No faction(s) found in FactionManager, SCR_RespawnSystemComponent will malfunction!");
					Print("SCR_RespawnSystemComponent found FactionManager, but no Factions are defined!", LogLevel.ERROR);
				}
			}
			else
			{
				Debug.Error("No FactionManager found, SCR_RespawnSystemComponent will malfunction!");
				Print("SCR_RespawnSystemComponent could not find a FactionManager, is one present in the world?", LogLevel.ERROR);
			}
	
			// Validate loadout manager
			SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
			if (loadoutManager)
			{
				int loadoutCount = loadoutManager.GetLoadoutCount();
				if (loadoutCount <= 0)
				{
					Debug.Error("No loadout(s) found in SCR_LoadoutManager, SCR_RespawnSystemComponent will malfunction!");
					Print("SCR_RespawnSystemComponent found LoadoutManager, but no loadouts are defined!", LogLevel.ERROR);
				}
			}
			else
			{
				Debug.Error("No SCR_LoadoutManager found, SCR_RespawnSystemComponent will malfunction!");
				Print("SCR_RespawnSystemComponent could not find a LoadoutManager, is one present in the world?", LogLevel.ERROR);
			}
		}
	}
	
	protected override event bool OnRplSave(ScriptBitWriter w)
    {	
		w.WriteBool(m_bEnableRespawn);
		
		return super.OnRplSave(w);
	}
	
	protected override event bool OnRplLoad(ScriptBitReader r)
    {
		bool enableRespawn;
		r.ReadBool(enableRespawn);
		
		SetEnableRespawnBroadcast(enableRespawn);
		
		return super.OnRplLoad(r);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_RespawnSystemComponent()
	{
		s_Instance = null;
		m_pRplComponent = null;

		if (m_mPlayerRespawnInfos)
		{
			m_mPlayerRespawnInfos.Clear();
			m_mPlayerRespawnInfos = null;
		}

		if (m_aFactionPlayerCount)
		{
			m_aFactionPlayerCount.Clear();
			m_aFactionPlayerCount = null;
		}

		if (m_aLoadoutPlayerCount)
		{
			m_aLoadoutPlayerCount.Clear();
			m_aLoadoutPlayerCount = null;
		}

		if (m_aSpawnPointsPlayerCount)
		{
			m_aSpawnPointsPlayerCount.Clear();
			m_aSpawnPointsPlayerCount = null;
		}
	}
};