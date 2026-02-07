[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Base for gamemode scripted component.", color: "0 0 255 255")]
class SCR_BaseGameModeComponentClass: ScriptComponentClass
{
};


//------------------------------------------------------------------------------------------------
//! Interface for game mode extending components.
//! Must be attached to a GameMode entity.
class SCR_BaseGameModeComponent : ScriptComponent
{
	//! The game mode entity this component is attached to.
	protected SCR_BaseGameMode m_pGameMode;

	//------------------------------------------------------------------------------------------------
	/*!
		\return Returns game mode this component is attached to.
	*/
	SCR_BaseGameMode GetGameMode()
	{
		return m_pGameMode;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called on all machines when the world ends.
	*/
	void OnGameEnd()
	{
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called when game mode state changes called on all machines.
		\param state New state game mode transitioned into.
	*/
	void OnGameStateChanged(SCR_EGameModeState state)
	{
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called when game mode ends.
		\param data End game data with game logic.
	*/
	void OnGameModeEnd(SCR_GameModeEndData data)
	{
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player is connected. Server-only.
		\param playerId PlayerId of connected player.
	*/
	void OnPlayerConnected(int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called on every machine after a player is registered (identity, name etc.). Always called after OnPlayerConnected.
		\param playerId PlayerId of registered player.
	*/
	void OnPlayerRegistered(int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player is disconnected.
		\param playerId PlayerId of disconnected player.
	*/
	void OnPlayerDisconnected(int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player is spawned.
		\param playerId PlayerId of spawned player.
		\param controlledEntity Spawned entity for this player.
	*/
	void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Check if entity is actual player, or just possessed one (e.g., Game Master-controlled AI or unmanned vehicle)
		\param player Assumed player entity
		\param[out] playerID Value to be filled with player ID
		\return True if the value was set
	*/
	bool GetPlayerID(IEntity entity, out int playerID)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player gets killed.
		\param playerId PlayerId of victim player.
		\param player Entity of victim player if any.
		\param killer Entity of killer instigator if any.
	*/
	void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{

	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player gets deleted.
		\param playerId Player ID
		\param player Player entity
	*/
	void OnPlayerDeleted(int playerId, IEntity player)
	{

	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called when player role changes.
		\param playerId Player whose role is being changed.
		\param roleFlags Roles as a flags
	*/
	void OnPlayerRoleChange(int playerId, EPlayerRole roleFlags)
	{
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called once loading of all entities of the world have been finished (still within the loading)
		\param world Loaded world
	*/
	void OnWorldPostProcess(World world)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! What happens when a player is assigned a lodout
	void HandleOnLoadoutAssigned(int playerID, SCR_BasePlayerLoadout assignedLoadout)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! What happens when a player is assigned a faction
	void HandleOnFactionAssigned(int playerID, Faction assignedFaction)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! What happens when a player is assigned a spawn point
	void HandleOnSpawnPointAssigned(int playerID, SCR_SpawnPoint spawnPoint)
	{
	}

	//------------------------------------------------------------------------------------------------
	/*
		When a controllable entity is spawned, this event is raised.
		\param entity Spawned entity that raised this event
	*/
	void OnControllableSpawned(IEntity entity)
	{
	}

	//------------------------------------------------------------------------------------------------
	/*
		When a controllable entity is destroyed, this event is raised.
		Entity is destroyed when DamageManager.OnStateChanged -> EDamageState.Destroyed
		\param entity Destroyed entity that raised this event
		\param instigator Instigator entity that destroyed our victim
	*/
	void OnControllableDestroyed(IEntity entity, IEntity instigator)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	/*
		Prior to a controllable entity being DELETED, this event is raised.
		Controllable entity is such that has BaseControllerComponent and can be
		possessed either by a player, an AI or stay unpossessed.
		\param entity Entity about to be deleted
	*/
	void OnControllableDeleted(IEntity entity)
	{
	}

	//------------------------------------------------------------------------------------------------
	void SCR_BaseGameModeComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_pGameMode = SCR_BaseGameMode.Cast(ent);
		if (!m_pGameMode)
		{
			string message = string.Format("%1 is attached to entity '%2' type=%3, required type=%4! This is not allowed!", Type().ToString(), ent.GetName(), ent.ClassName(), "SCR_BaseGameMode");
			Debug.Error(message);
			Print(message);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_BaseGameModeComponent()
	{
	}
};
