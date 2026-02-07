/**
@defgroup GameMode Game Mode
Game modes.
*/
class SCR_BaseGameModeClass: BaseGameModeClass
{
};

/// @ingroup GameMode
//------------------------------------------------------------------------------------------------
/*
											SCR_BaseGameMode

Brief:
	SCR_BaseGameMode is the skeleton of all game modes which connects some of the critical aspects of spawning.

Important:
	A. Components and expansion:
		1. SCR_BaseGameMode automatically gathers all attached components of SCR_BaseGameModeComponent type.
		These can be used to receive events from the game mode and expand game mode functionality in modular blocks.

		2. SCR_RespawnSystemComponent is a component that allows to store and select player faction, loadout and spawn points
		This system is closely tied to game modes and also requires FactionManager or LoadoutManager in the world.

		3. SCR_RespawnHandlerComponent is a component that provides a game mode with a way of respawning.
		These can be either very generic (automatic, menu selection) and interchangable or completely game mode specific.

		4. Additional components like
			SCR_RespawnTimerComponent: That allows to add a timer between each respawn
			SCR_ScoringSystemComponent: That allows handling of score when a player kills or is killed


	If you want to make a system which only requires to hook on game mode events, SCR_BaseGameMode is most likely the way to go.
	Inherit the component and implement methods which you require. If you need to have a very game specific implementation that
	cannot work standalone you will have to inherit SCR_BaseGameMode and plug the communication yourself.


	B. Instances and logic:
		1. SCR_BaseGameMode is a unique entity in the world, there can currently be only one game mode.
		2. SCR_RespawnSystemComponent is a unique component attached to the game mode, there can currently be only one respawn system.
		3. SCR_RespawnHandlerComponent is a unique component attached to the game mode, there can currently be only one respawn handler.
		4. SCR_RespawnComponent is a component attached to individual PlayerController instances.
			Due to the nature of PlayerController, only the server can access all RespawnComponent(s).
			The local client will always be only to access their local RespawnComponent.

			For the local component you can use GetGame().GetPlayerController().GetRespawnComponent();
			For other clients as the authority you can use GetGame().GetPlayerManager().GetPlayerRespawnComponent( int playerId );

			This component serves as communication between the authority and client for the SCR_RespawnSystemComponent.
			If you need to request a faction, loadout or spawn point selection as the client, navigate through the component.

			If you need to SET a faction, loadout or spawn point as the AUTHORITY, use SCR_RespawnSystemComponent's
			DoSetPlayerFaction, DoSetPlayerLoadout and DoSetPlayerSpawnPoint method(s).

			If you need to convert object instances to their id/indices, you can use SCR_RespawnSystemComponent's
			GetFactionIndex, GetLoadoutIndex, GetSpawnPointIndex method(s) and/or
			GetFactionByIndex, GetLoadoutByIndex, GetSpawnPointByIdentity.


	C. Game State
		There are three game states in total that you can use, two of which are optional.
			1. Pre-game:
				This is a state that can be enabled before the game mode itself starts, to for example await connecting players,
				have a warmup period, etc. See: m_fPreGamePeriod

				Setting m_fPreGamePeriod to -1 allows you to automatically check for
				SCR_BaseGameMode.CanStartGameMode() periodically, thus allowing to implement custom logic. (e.g. min player count)

				Setting m_fPreGamePeriod to 0 makes the game start in the pre-game period
				and await for the authority to call SCR_BaseGameMode.StartGameMode() manually.

				Setting m_fPreGamePeriod above n>0 makes the game mode await n seconds before starting automatically.

			2. Game
				This is the core game loop.

				Setting m_fTimeLimit to 0 disables time limit and the game can only be terminated manually
				from the authority via SCR_BaseGameMode.EndGameMode().

				Setting m_fTimeLimit to n>n enables automatic time limit, ending the game automatically in n seconds.

			3. Post-game
				This is your game-over state, you can automatically transition into new world, restart the session,
				have player voting, display scoreboard or end-game screens, as desired.

				In addition this transition carries along SCR_GameModeEndData, providing the authority and all
				clients with additional data that can notify the gamemode about e.g. win condition, or end reason
				as implemented per game mode.

		You can always retrieve current game state by calling GetState() (see SCR_GameModeState)
		or IsRunning() if you're interested in the core game loop only.

		On each state change SCR_BaseGameMode.OnGameStateChanged() is called on both the server and all clients.
		In additionaly as implemented in this class it will automatically call OnGameModeStart() and OnGameModeEnd()
		based on the state the game is transitioning into.
*/
//------------------------------------------------------------------------------------------------

class SCR_BaseGameMode : BaseGameMode
{
	#define GAME_MODE_DEBUG

	#ifdef GAME_MODE_DEBUG
		static bool s_DebugRegistered = false;
	#endif

	const static string WB_GAME_MODE_CATEGORY = "Game Mode";

	protected ref ScriptInvoker Event_OnGameStart = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnGameEnd = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnPlayerAuditSuccess = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnPlayerAuditFail = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnPlayerConnected = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnPlayerRegistered = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnPlayerDisconnected = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnPlayerSpawned = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnPlayerKilled = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnPlayerDeleted = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnPlayerRoleChange = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnWorldPostProcess = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnControllableSpawned = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnControllableDestroyed = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnControllableDeleted = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnGameModeEnd = new ref ScriptInvoker();

	//-----------------------------------------
	//
	// World Editor attributes meant for Debug or Testing
	//
	[Attribute("0", uiwidget: UIWidgets.Flags, "Test Game Flags for when you run mission via WE.", "", ParamEnumArray.FromEnum(EGameFlags), WB_GAME_MODE_CATEGORY)]
	protected EGameFlags m_eTestGameFlags;

	[Attribute("1", uiwidget: UIWidgets.CheckBox, "When false, then Game mode need to handle its very own spawning. If true, then simple default logic is used to spawn and respawn automatically.", category: WB_GAME_MODE_CATEGORY)]
	protected bool m_bAutoPlayerRespawn;
	[Attribute("1", UIWidgets.Auto, desc: "Character bleeding rate multiplier", category: WB_GAME_MODE_CATEGORY)]
	private float m_fBleedingRateScale;
	//-----------------------------------------
	//
	// Game End Screen States info
	//

	//! Current state of this gamemode.
	[RplProp(onRplName: "OnGameStateChanged")]
	private SCR_EGameModeState m_eGameState = SCR_EGameModeState.PREGAME;

	//! End data provided by the server when game ends.
	[RplProp()]
	private ref SCR_GameModeEndData m_pGameEndData = new SCR_GameModeEndData();

	[Attribute("0", uiwidget: UIWidgets.Slider, "Time limit of game mode in seconds or 0 if none.", params: "0 864000 1", category: WB_GAME_MODE_CATEGORY)]
	protected float m_fTimeLimit;

	[Attribute("-1", uiwidget: UIWidgets.Slider, "Duration in seconds before the game mode launches. 0 = authority needs to manually start the gamemode, -1 = gamemode is started automatically and immediately!", params: "-1 864000 1", category: WB_GAME_MODE_CATEGORY)]
	protected float m_fPreGamePeriod;

	/*!
		Elapsed time from the beginning of the game mode in seconds.
		NOTE: This does not correspond to the length of session but rather to the length of current game mode loop.
	*/
	[RplProp(condition: RplCondition.NoOwner)]
	protected float m_fTimeElapsed;

	//! Interval of time synchronization in seconds
	protected float m_fTimeCorrectionInterval = 10.0;

	//! Last timestamp of sent time correction for the server.
	protected float m_fLastTimeCorrection;

	[Attribute("1", uiwidget: UIWidgets.CheckBox, "Disable player controls on PRE and POST game states.", category: WB_GAME_MODE_CATEGORY)]
	protected bool m_bAutoDisabledControls;

	//-----------------------------------------
	//
	// Required components
	//
	protected RplComponent m_RplComponent;
	protected SCR_RespawnSystemComponent m_pRespawnSystemComponent;
	protected SCR_RespawnHandlerComponent m_pRespawnHandlerComponent;
	protected SCR_BaseScoringSystemComponent m_ScoringSystemComponent;
	protected SCR_RespawnTimerComponent m_RespawnTimerComponent;

	/*!
		Additional game mode components attached to this gamemode where we dispatch all our game mode related events to.
	*/
	protected ref array<SCR_BaseGameModeComponent> m_aAdditionalGamemodeComponents = new ref array<SCR_BaseGameModeComponent>();

	//! Used on server to respawn player on their original position after reconnecting.
	protected ref map<int, vector> m_mPlayerSpawnPosition = new ref map<int, vector>();

	/*!
		Returns whether pregame state is set to automatic.
		If so it means that the game will start automatically once elapsed time > m_fPreGamePeriod by default
		or automatically once CanStartGameMode() returns true - this can be implemented in derived modes.
		\return True in case pregame mode is automatic.
	*/
	protected bool IsAutomaticPregameSet()
	{
		return m_fPreGamePeriod <= -1.0;
	}

	/*!
		Returns whether pregame state is set to manual.
		If so it meansa that the game will remain in pregame state indefinitely, until the server
		decides to call StartGameMode(), which then propagates to all connected clients.
		\return True in case pregame mode is manual.
	*/
	protected bool IsManualPregameSet()
		{
		return m_fPreGamePeriod > -1.0 && m_fPreGamePeriod <= 0.0;
		}

	/*!
		\return Returns true if any pregame mode is set (automatic or manual), false otherwise.
	*/
	protected bool IsPregameSet()
	{
		return IsAutomaticPregameSet() || IsManualPregameSet();
	}

	/*!
		Returns whether current game mode is running its game loop.
		\return True in case gamemode is running, false if it is over.
	*/
	bool IsRunning()
	{
		return m_eGameState == SCR_EGameModeState.GAME;
	}

	/*!
		Returns current game state. See SCR_GameModeState for more details.
		\return Current game state.
	*/
	SCR_EGameModeState GetState()
	{
		return m_eGameState;
	}

	/*!
		Returns whether the owner of this instance is authority or not.
	*/
	sealed bool IsMaster()
	{
		return (!m_RplComponent || m_RplComponent.IsMaster());
	}

	/*!
		Returns elapsed time from the start of this game in seconds.
		\return Elapsed time in seconds.
	*/
	float GetElapsedTime()
	{
		return m_fTimeElapsed;
	}

	/*!
		Returns the game duration from the start of game in seconds or 0 if none.
		\return Maximum time in seconds or 0 if infinite.
	*/
	float GetTimeLimit()
	{
		return m_fTimeLimit;
	}

	/*!
		Returns the remaining time of this game in seconds or -1 if none or undefined.
		\return Remaining time of this game.
	*/
	float GetRemainingTime()
	{
		float timeLimit = GetTimeLimit();
		if (!IsRunning() || timeLimit <= 0)
			return -1;

		float time = timeLimit - GetElapsedTime();
		if (time < 0)
			time = 0;

		return time;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called when game state changed, i.e. when game ends. Can be used to display post-scenario scoreboard
		and prepare for next match if any. See IsRunning() for new state.
	*/
	protected void OnGameStateChanged()
	{
		SCR_EGameModeState newState = GetState();
		Print("SCR_BaseGameMode::OnGameStateChanged= " + newState.ToString());

		switch (newState)
		{
			case SCR_EGameModeState.POSTGAME:
				OnGameModeEnd(m_pGameEndData);
				foreach (SCR_BaseGameModeComponent component : m_aAdditionalGamemodeComponents)
					component.OnGameModeEnd(m_pGameEndData);

				break;

			case SCR_EGameModeState.GAME:
				OnGameModeStart();
				break;
		}

		// Dispatch events
		foreach (SCR_BaseGameModeComponent component : m_aAdditionalGamemodeComponents)
			component.OnGameStateChanged(newState);
	}

	//------------------------------------------------------------------------------------------------
	void CachePlayerSpawnPosition(int playerID, vector position)
	{
		m_mPlayerSpawnPosition.Set(playerID, position);
	}

	/*!
		Returns respawn system component attached to this gamemode.
		\return Returns attached respawn system instance or null if none.
	*/
	SCR_RespawnSystemComponent GetRespawnSystemComponent()
	{
		return m_pRespawnSystemComponent;
	}

	/*!
		Returns respawn handler component attached to this gamemode.
		\return Returns attached respawn handler instance or null
	*/
	SCR_RespawnHandlerComponent GetRespawnHandlerComponent()
	{
		return m_pRespawnHandlerComponent;
	}

	/*!
		Returns scoring system component attached to this game mode or null if none.
		\return Returns attached scoring system instance or null if none.
	*/
	SCR_BaseScoringSystemComponent GetScoringSystemComponent()
	{
		return m_ScoringSystemComponent;
	}

	//------------------------------------------------------------------------------------------------
	// Called when a spawnpoint is used to respawn
	void OnSpawnPointUsed(SCR_SpawnPoint spawnPoint, int playerId)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Server-only call that starts current session.
		Replicates state to all clients, see GetState() and IsRunning().
	*/
	void StartGameMode()
	{
		if (!IsMaster())
			return;

		if (IsRunning())
		{
			Print("Trying to start a gamemode that is already running.", LogLevel.WARNING);
			return;
		}

		m_fTimeElapsed = 0.0;
		m_eGameState = SCR_EGameModeState.GAME;
		Replication.BumpMe();

		// Raise event for authority
		OnGameStateChanged();
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Server-only logic that implements whether we can transition from pre-game (if enabled)
		to game loop.  Only applicable when IsAutomaticPregameSet().
		Can be overriden to implement custom logic, e.g. minimum player count for each side.

		Does not apply to manual StartGameMode() call from the authority!

		\return Returns true in case pre-game can end and game can start or false otherwise.
	*/
	protected bool CanStartGameMode()
	{
		return m_fTimeElapsed >= m_fPreGamePeriod;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Server-only call that marks current session as complete.
		Replicates state to all clients, see GetState() and IsRunning().

		\param SCR_GameModeEndData the data this game mode ends with. When null, fallback to default is used.
		All machines can reach this data in the OnGameModeEnd(SCR_GameModeEndData) method.
	*/
	void EndGameMode(SCR_GameModeEndData endData)
	{
		if (!IsMaster())
			return;

		// Already over
		if (!IsRunning())
		{
			Print("Trying to end a gamemode that is not running.", LogLevel.WARNING);
			return;
		}

		if (endData == null)
			endData = SCR_GameModeEndData.CreateSimple();

		m_pGameEndData = endData;
		m_eGameState = SCR_EGameModeState.POSTGAME;
		Replication.BumpMe();

		// Raise event for authority
		OnGameStateChanged();
	}
	
	/*!
		Get end game data will return null if game has not ended
		\return SCR_GameModeEndData m_pGameEndData the data this game mode ends with
	*/
	SCR_GameModeEndData GetEndGameData()
	{
		if (m_eGameState != SCR_EGameModeState.POSTGAME)
			return null;
		else
			return m_pGameEndData;
	}

	ScriptInvoker GetOnGameStart()
	{
		return Event_OnGameStart;
	}
	ScriptInvoker GetOnGameEnd()
	{
		return Event_OnGameEnd;
	}
	ScriptInvoker GetOnPlayerAuditSuccess()
	{
		return Event_OnPlayerAuditSuccess;
	}
	ScriptInvoker GetOnPlayerAuditFail()
	{
		return Event_OnPlayerAuditFail;
	}
	ScriptInvoker GetOnPlayerConnected()
	{
		return Event_OnPlayerConnected;
	}
	ScriptInvoker GetOnPlayerRegistered()
	{
		return Event_OnPlayerRegistered;
	}
	ScriptInvoker GetOnPlayerDisconnected()
	{
		return Event_OnPlayerDisconnected;
	}
	ScriptInvoker GetOnPlayerSpawned()
	{
		return Event_OnPlayerSpawned;
	}
	ScriptInvoker GetOnPlayerKilled()
	{
		return Event_OnPlayerKilled;
	}
	ScriptInvoker GetOnPlayerDeleted()
	{
		return Event_OnPlayerDeleted;
	}
	ScriptInvoker GetOnPlayerRoleChange()
	{
		return Event_OnPlayerRoleChange;
	}
	ScriptInvoker GetOnWorldPostProcess()
	{
		return Event_OnWorldPostProcess;
	}
	ScriptInvoker GetOnControllableSpawned()
	{
		return Event_OnControllableSpawned;
	}
	ScriptInvoker GetOnControllableDestroyed()
	{
		return Event_OnControllableDestroyed;
	}
	ScriptInvoker GetOnControllableDeleted()
	{
		return Event_OnControllableDeleted;
	}
	/*!
	Get on game end script invoker called on every machine
	\return ScriptInvoker Event_OnGameModeEnd
	*/
	ScriptInvoker GetOnGameModeEnd()
	{
		return Event_OnGameModeEnd;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called on every machine when game mode starts.
		This can be immediate (if no pre-game period is set) or can happen after
		a certain delay, as deemed appropriate by the authority.
	*/
	protected void OnGameModeStart()
	{
		#ifdef GMSTATS
		if (IsGameModeStatisticsEnabled())
		{
			if (!SCR_GameModeStatistics.IsRecording())
				SCR_GameModeStatistics.StartRecording();
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called on every machine when game mode ends.
		This can be based on time limit or as deemed appropriate by the authority,
		e.g. after reaching certain score threshold and similar.
		\param SCR_GameModeEndData optional game mode end data received from the server.
	*/
	protected void OnGameModeEnd(SCR_GameModeEndData endData)
	{
		Event_OnGameModeEnd.Invoke(endData);

		#ifdef GMSTATS
		if (IsGameModeStatisticsEnabled())
		{
			if (SCR_GameModeStatistics.IsRecording())
				SCR_GameModeStatistics.StopRecording();
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnGameStart()
	{
		super.OnGameStart();
		Event_OnGameStart.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void OnGameEnd()
	{
		Event_OnGameEnd.Invoke();

		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.OnGameEnd();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess(int iPlayerID)
	{
	#ifdef RESPAWN_COMPONENT_VERBOSE
			Print("SCR_BaseGameMode::OnPlayerAuditSuccess - playerId: " + iPlayerID);
	#endif

		super.OnPlayerAuditSuccess(iPlayerID);
		Event_OnPlayerAuditSuccess.Invoke(iPlayerID);

		if (m_pRespawnSystemComponent)
			m_pRespawnSystemComponent.OnPlayerConnected(iPlayerID);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditFail(int iPlayerID)
	{
	#ifdef RESPAWN_COMPONENT_VERBOSE
		Print("SCR_BaseGameMode::OnPlayerAuditFail - playerId: " + iPlayerID);
	#endif

		super.OnPlayerAuditFail(iPlayerID);
		Event_OnPlayerAuditFail.Invoke(iPlayerID);

		if (m_pRespawnSystemComponent)
			m_pRespawnSystemComponent.OnPlayerConnected(iPlayerID);

	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player is connected. Server-only.
		\param playerId PlayerId of connected player.
	*/
	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);
		Event_OnPlayerConnected.Invoke(playerId);

		// TODO@LITERALLYANYONE: REMOVE THIS CRAP FOR THE LOVE OF GOD, THANKS
		// Wait for backend response if dedicated server is used and is not run without backend functionality
		if (m_pRespawnSystemComponent && (RplSession.Mode() != RplMode.Dedicated || System.IsCLIParam("nobackend")))
		{
		#ifdef RESPAWN_COMPONENT_VERBOSE
			Print("SCR_BaseGameMode::OnPlayerConnected - playerId: " + playerId);
		#endif

			m_pRespawnSystemComponent.OnPlayerConnected(playerId);
		}

		// Dispatch event to child components
		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.OnPlayerConnected(playerId);

		// DON'T! Leave that up to game mode respawning
		// SetPlayerRandomLoadout(playerId);

		#ifdef TREE_DESTRUCTION
		int count = SCR_DestructibleTree.DestructibleTrees.Count();
		for (int i = 0; i < count; i++)
		{
			SCR_DestructibleTree.DestructibleTrees[i].OnPlayerConnected();
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player is disconnected.
		\param playerId PlayerId of disconnected player.
	*/
	protected override void OnPlayerDisconnected(int playerId)
	{
		super.OnPlayerDisconnected(playerId);
		Event_OnPlayerDisconnected.Invoke(playerId);

		if (m_pRespawnSystemComponent)
			m_pRespawnSystemComponent.OnPlayerDisconnected(playerId);

		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.OnPlayerDisconnected(playerId);
		
		if (IsMaster())
		{
			IEntity controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (controlledEntity)
				RplComponent.DeleteRplEntity(controlledEntity, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called on every machine after a player is registered (identity, name etc.). Always called after OnPlayerConnected.
		\param playerId PlayerId of registered player.
	*/
	protected override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);
		Event_OnPlayerRegistered.Invoke(playerId);

		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.OnPlayerRegistered(playerId);
		
		#ifdef GMSTATS
		if (IsGameModeStatisticsEnabled())
			SCR_GameModeStatistics.RecordConnection(playerId, GetGame().GetPlayerManager().GetPlayerName(playerId));
		#endif
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player is spawned.
		\param playerId PlayerId of spawned player.
		\param controlledEntity Spawned entity for this player.
	*/
	protected override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);
		Event_OnPlayerSpawned.Invoke(playerId, controlledEntity);

		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.OnPlayerSpawned(playerId, controlledEntity);
				
		#ifdef GMSTATS
		if (IsGameModeStatisticsEnabled())
		{
			FactionAffiliationComponent fac = FactionAffiliationComponent.Cast(controlledEntity.FindComponent(FactionAffiliationComponent));
			SCR_GameModeStatistics.RecordSpawn(playerId, fac.GetAffiliatedFaction().GetFactionColor().PackToInt());
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player gets killed.
		\param playerId PlayerId of victim player.
		\param player Entity of victim player if any.
		\param killer Entity of killer instigator if any.
	*/
	protected bool IsPlayerKilled(int playerId)
	{
		return true;
	}
	protected override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		//--- Check if player was killed (could have been possessed entity, e.g., Game Master-controlled AI or unammed vehicle)
		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
		{
			if (comp.GetPlayerID(player, playerId) && playerId == 0)
				return;
		}

		super.OnPlayerKilled(playerId, player, killer);
		Event_OnPlayerKilled.Invoke(playerId, player, killer);

		// Dispatch events to children components
		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.OnPlayerKilled(playerId, player, killer);
		
		#ifdef GMSTATS
		if (IsGameModeStatisticsEnabled())
			SCR_GameModeStatistics.RecordDeath(player, killer);
		#endif
	}
	protected /*override*/ void OnPlayerDeleted(int playerId, IEntity player)
	{
		//--- Check if player was killed (could have been possessed entity, e.g., Game Master-controlled AI or unammed vehicle)
		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
		{
			if (comp.GetPlayerID(player, playerId) && playerId == 0)
				return;
		}

		//super.OnPlayerDeleted(playerId, player);
		Event_OnPlayerDeleted.Invoke(playerId, player);

		// Dispatch events to children components
		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.OnPlayerDeleted(playerId, player);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called when player role changes.
		\param playerId Player whose role is being changed.
		\param roleFlags Roles as a flags
	*/
	protected override void OnPlayerRoleChange(int playerId, EPlayerRole roleFlags)
	{
		super.OnPlayerRoleChange(playerId, roleFlags);
		Event_OnPlayerRoleChange.Invoke(playerId, roleFlags);

		// Dispatch events to children components
		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.OnPlayerRoleChange(playerId, roleFlags);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called once loading of all entities of the world have been finished (still within the loading)
		\param world Loaded world
	*/
	override event void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);
		Event_OnWorldPostProcess.Invoke(world);
		
		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.OnWorldPostProcess(world);
	};

	//------------------------------------------------------------------------------------------------
	/*
		When a controllable entity is spawned, this event is raised.
		\param entity Spawned entity that raised this event
	*/
	protected override void OnControllableSpawned(IEntity entity)
	{
		super.OnControllableSpawned(entity);
		Event_OnControllableSpawned.Invoke(entity);

		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.OnControllableSpawned(entity);

		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (character)
			HandleOnCharacterCreated(character);
	}

	//------------------------------------------------------------------------------------------------
	/*
		When a controllable entity is destroyed, this event is raised.
		\param entity Destroyed entity that raised this event
		\param instigator Instigator entity that destroyed our victim
	*/
	protected override void OnControllableDestroyed(IEntity entity, IEntity instigator)
	{
		super.OnControllableDestroyed(entity, instigator);
		Event_OnControllableDestroyed.Invoke(entity, instigator);

		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.OnControllableDestroyed(entity, instigator);

		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (character)
			HandleOnCharacterDeath(character.GetCharacterController(), instigator);
		
		//--- If the entity is player, call OnPlayerKilled (only when it wasn't player to begin with, in which case OnPlayerKilled is called by game code)
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity);
		if (playerID == 0)
		{
			foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			{
				bool isOverride = comp.GetPlayerID(entity, playerID);
				if (isOverride && playerID > 0)
				{
					OnPlayerKilled(playerID, entity, instigator);
					break;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*
		Prior to a controllable entity being DELETED, this event is raised.
		Controllable entity is such that has BaseControllerComponent and can be
		possessed either by a player, an AI or stay unpossessed.
		\param entity Entity about to be deleted
	*/
	protected override void OnControllableDeleted(IEntity entity)
	{
		super.OnControllableDeleted(entity);
		Event_OnControllableDeleted.Invoke(entity);
		
		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.OnControllableDeleted(entity);
		
		//--- If the entity is player, call OnPlayerDeleted
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity);
		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
		{
			if (comp.GetPlayerID(entity, playerID) && playerID > 0)
			{
				OnPlayerDeleted(playerID, entity);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What happens when a player is assigned a lodout
	//! Called from SCR_RespawnSystemComponent
	//! \param playerID the id of the target player
	//! \param assignedLoadout the faction that player was assigned or null if none
	void HandleOnLoadoutAssigned(int playerID, SCR_BasePlayerLoadout assignedLoadout)
	{
		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.HandleOnLoadoutAssigned(playerID, assignedLoadout);
	}

	//------------------------------------------------------------------------------------------------
	//! What happens when a player is assigned a faction
	//! Called from SCR_RespawnSystemComponent
	//! \param playerID the id of the target player
	//! \param assignedFaction the faction that player was assigned or null if none
	void HandleOnFactionAssigned(int playerID, Faction assignedFaction)
	{
		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.HandleOnFactionAssigned(playerID, assignedFaction);
	}

	//------------------------------------------------------------------------------------------------
	//! What happens when a player is assigned a spawn point
	//! Called from SCR_RespawnSystemComponent
	//! \param playerID the id of the target player
	//! \param assignedSpawnPoint the spawn point that player was assigned or null if none
	void HandleOnSpawnPointAssigned(int playerID, SCR_SpawnPoint spawnPoint)
	{
		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			comp.HandleOnSpawnPointAssigned(playerID, spawnPoint);
	}

	//------------------------------------------------------------------------------------------------
	//! Override and implement logic of when player can or can not respawn
	bool CanPlayerRespawn(int playerID)
	{
		// Handler has prio in spawn logic
		if (m_pRespawnHandlerComponent && !m_pRespawnHandlerComponent.CanPlayerSpawn(playerID))
				return false;

		// Respawn timers
		if (m_RespawnTimerComponent)
			return m_RespawnTimerComponent.GetCanPlayerSpawn(playerID);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns remaining respawn time in seconds for given player
	int GetPlayerRemainingRespawnTime(int playerID)
	{
		// Respawn timers
		if (m_RespawnTimerComponent)
			return m_RespawnTimerComponent.GetPlayerRemainingTime(playerID);

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Inform the gamemode that we want to request a respawn screen
	//! This request might be instantly processed or may want to be handled only if player is dead, etc.
	//! Override and implement based on needs
	void RequestPlayerRespawnSelection()
	{

	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		EGameFlags gameFlags = GetGame().GetGameFlags();
		writer.WriteIntRange(gameFlags, 0, (EGameFlags.Last<<1)-1);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		EGameFlags gameFlags;
		reader.ReadIntRange(gameFlags, 0, (EGameFlags.Last<<1)-1);

		GetGame().SetGameFlags(gameFlags, true);

		return true;
	}

	#ifdef GAME_MODE_DEBUG
	static bool s_bDrawPlayerInfo = false;
	static bool s_bDrawSelectionInfo = false;
	static int s_iLoadout;
	static int s_iFaction;
	//------------------------------------------------------------------------------------------------
	void DrawSelectionInfo()
	{
		DbgUI.Begin("RespawnSelection:");
		{
			SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(GetGame().GetPlayerController().GetRespawnComponent());
			DbgUI.InputInt("LoadoutIdx", s_iLoadout, 300);
			if (DbgUI.Button("Req. Loadout"))
				rc.RequestPlayerLoadout(m_pRespawnSystemComponent.GetLoadoutByIndex(s_iLoadout));

			DbgUI.InputInt("FactionIdx", s_iFaction, 300);
			if (DbgUI.Button("Req. Faction"))
				rc.RequestPlayerFaction(m_pRespawnSystemComponent.GetFactionByIndex(s_iFaction));

		}
		DbgUI.End();
	}
	//------------------------------------------------------------------------------------------------
	void DrawPlayerDebugInfo()
	{
		DbgUI.Begin("RespawnSystemComponent data:");

		array<int> players = {};
		PlayerManager pm = GetGame().GetPlayerManager();
		pm.GetAllPlayers(players);

		string currentPlayerStr = string.Empty;
		foreach (int playerId : players)
		{
			Faction faction = m_pRespawnSystemComponent.GetPlayerFaction(playerId);
			SCR_BasePlayerLoadout loadout = m_pRespawnSystemComponent.GetPlayerLoadout(playerId);
			SCR_SpawnPoint spawn = m_pRespawnSystemComponent.GetPlayerSpawnPoint(playerId);

			string factionStr = "None";
			if (faction)
				factionStr = faction.GetFactionKey();

			string loadoutStr = "None";
			if (loadout)
				loadoutStr = loadout.GetLoadoutName();

			string spawnPointStr = "None";
			if (spawn)
				spawnPointStr = string.Format("Spawn(rplId=%1)", SCR_SpawnPoint.GetSpawnPointRplId(spawn));

			currentPlayerStr = string.Format("Player: %1, Connected: %2, Faction: %3, Loadout: %4, SpawnPoint: %5",
				pm.GetPlayerName(playerId),
				pm.IsPlayerConnected(playerId),
				factionStr,
				loadoutStr,
				spawnPointStr
			);

			DbgUI.Text(currentPlayerStr);
		}

		DbgUI.Text(" "); // spacer doesnt work lol

		DbgUI.End();
	}
	//------------------------------------------------------------------------------------------------
	void DrawDebugInfo()
	{
		// Get lobby
		// Match lobby indexes to m_aPlayerControlledEntities
		// Display ID : Player Name : Entity Pointer

		ArmaReforgerScripted game = GetGame();
		PlayerManager playerManager = GetGame().GetPlayerManager();

		// Draw a list of players, their ID, their controlled entity (as in gamemode) and their controlled entity (as in lobby)
		const string DEBUG_ENTITIES_HEADER = "[playerId] [controlledEntity] [isConnected]";
		const string DEBUG_ENTITIES_FORMAT = "%1 | %2 | %3";

		DbgUI.Begin("Game Mode Diag Menu");
		DbgUI.Text(DEBUG_ENTITIES_HEADER);
		array<int> players = {};
		playerManager.GetAllPlayers(players);

		foreach (int playerId : players)
		{
			IEntity controlledEntity = playerManager.GetPlayerControlledEntity(playerId);
			bool isConnected = playerManager.IsPlayerConnected(playerId);
			string text = string.Format(DEBUG_ENTITIES_FORMAT, playerId, controlledEntity, isConnected);
			DbgUI.Text(text);
		}

		// Draw elapsed time
		const string ELAPSED_TIME_STR = "ElapsedTime: %1";
		DbgUI.Text(string.Format(ELAPSED_TIME_STR, GetElapsedTime()));

		const string IS_RUNNING_STR = "IsRunning: %1";
		DbgUI.Text(string.Format(IS_RUNNING_STR, IsRunning()));


		string gameState;
		switch (GetState())
		{
			case SCR_EGameModeState.PREGAME:
				gameState = "Pre-Game";
				break;

			case SCR_EGameModeState.GAME:
				gameState = "Game";
				break;

			case SCR_EGameModeState.POSTGAME:
				gameState = "Post-Game";
				break;

			default:
				gameState = "undefined";
				break;
		}

		const string GAME_STATE_STR = "GameState: %!";
		DbgUI.Text(string.Format(IS_RUNNING_STR, gameState));

		// Draw what flags are set for this game mode
		const string DEBUG_FLAGS_SET = "ETestFlags: %1";
		DbgUI.Text(string.Format(DEBUG_FLAGS_SET, m_eTestGameFlags.ToString()));

		// :)
		DbgUI.Check("Draw RespawnSystemComponent Info", s_bDrawPlayerInfo);
		DbgUI.Check("Draw RespawnSelection Info", s_bDrawSelectionInfo);

		// Draw info about additional components
		const string DEBUG_ADDITIONAL_GAMEMODE_COMPONENTS = "GameModeComponent Count: %1";
		int compCount = 0;
		if (m_aAdditionalGamemodeComponents)
			compCount = m_aAdditionalGamemodeComponents.Count();

		DbgUI.Text(string.Format(DEBUG_ADDITIONAL_GAMEMODE_COMPONENTS, compCount.ToString()));
		if (compCount != 0)
		{
			const string DEBUG_ADDITIONAL_COMPONENT = "GameModeComponent: %1";
			foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
			{
				DbgUI.Text(string.Format(DEBUG_ADDITIONAL_COMPONENT, comp));
			}
		}



		// End
		DbgUI.End();
	}
	#endif

	//------------------------------------------------------------------------------------------------
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
	}

	//------------------------------------------------------------------------------------------------
	//Called once tasks are initialized
	void HandleOnTasksInitialized()
	{
	}

	//------------------------------------------------------------------------------------------------
	void HandleOnCharacterCreated(ChimeraCharacter character)
	{
	}

	//------------------------------------------------------------------------------------------------
	void HandleOnCharacterDeath(notnull CharacterControllerComponent characterController, IEntity instigator)
	{
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Enables or disables controls for the local client.
		\param enabled True to enable controls, false to disable controls over controlled entity.
	*/
	protected void SetLocalControls(bool enabled)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (playerController)
		{
			IEntity controlledEntity = playerController.GetControlledEntity();
			if (controlledEntity)
			{
				ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
				if (character)
				{
					CharacterControllerComponent controller = character.GetCharacterController();
					if (controller)
					{
						bool doDisable = !enabled;
						controller.SetDisableWeaponControls(doDisable);
						controller.SetDisableMovementControls(doDisable);
					}
				}
			}
		}
	}
	
	#ifdef GMSTATS
	private float m_fLastRecordTime;
	
	private float m_fLastFlushTime;
	private float m_fFlushRecordInterval = 2500; // 2.5s
	private void UpdateStatistics(IEntity owner)
	{
		// Create new records only ever so often
		float timeStamp = owner.GetWorld().GetWorldTime();
		if (timeStamp > m_fLastRecordTime + SCR_GameModeStatistics.RECORD_INTERVAL_MS)
		{
			m_fLastRecordTime = timeStamp;
			
			PlayerManager pm = GetGame().GetPlayerManager();
			array<int> players = {};
			pm.GetPlayers(players);
			
			foreach (int pid : players)
			{
				IEntity ctrlEnt = pm.GetPlayerControlledEntity(pid);
				if (!ctrlEnt)
					continue;
				
				SCR_GameModeStatistics.RecordMovement(ctrlEnt, pid);
			}
		}

		// Flush data if recording in smaller intervals
		if (timeStamp > m_fLastFlushTime + m_fFlushRecordInterval)
		{
			m_fLastFlushTime = timeStamp;
			if (IsGameModeStatisticsEnabled())
			{
				if (SCR_GameModeStatistics.IsRecording())
					SCR_GameModeStatistics.Flush();
			}
		}
	}
	#endif

	//------------------------------------------------------------------------------------------------
	// TODO@AS: Small thing, but get rid of m_fTimeElapsed and use
	// some sort of m_fStartTime and Replication.Time() instead!
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		#ifdef GMSTATS
		if (IsGameModeStatisticsEnabled())
			UpdateStatistics(owner);
		#endif
		
		// Allow to accumulate time in pre-game too.
		bool isPregame = IsPregameSet() && GetState() == SCR_EGameModeState.PREGAME;
		// Increment elapsed time on every machine
		bool isRunning = IsRunning();
		if (isRunning || isPregame)
			m_fTimeElapsed += timeSlice;

		// Disable player controls if desired
		// TODO@AS: This should be handled server-side and replicated to clients
		// to prevent breaking this functionality, potential flaw
		if (m_bAutoDisabledControls)
		{
			SetLocalControls(isRunning);
		}

		// As the authority make corrections as needed
		if (IsMaster())
		{
			if (m_fTimeElapsed >= m_fLastTimeCorrection + m_fTimeCorrectionInterval)
			{
				Replication.BumpMe();
				m_fLastTimeCorrection = m_fTimeElapsed;
			}

			// Make sure to start the game when pre-game preiod is enabled
			if (isPregame && !IsManualPregameSet())
			{
				// Automatic handling checks condition
				if (CanStartGameMode())
					StartGameMode();
			}
			// Otherwise if game is bound by time limit, make sure to end on time limit
			else if (isRunning && m_fTimeLimit > 0 && m_fTimeElapsed >= m_fTimeLimit)
			{
				// Clamp elapsed time to time limit
				m_fTimeElapsed = Math.Clamp(m_fTimeElapsed, 0, m_fTimeLimit);

				SCR_GameModeEndData data = SCR_GameModeEndData.CreateSimple(EGameOverTypes.EDITOR_FACTION_DRAW); // TODO: Once FACTION_DRAW or TIME_LIMIT works..
				EndGameMode(data);
			}
		}
		#ifdef GAME_MODE_DEBUG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_GAME_MODE))
		{
			DrawDebugInfo();
			if (s_bDrawPlayerInfo)
				DrawPlayerDebugInfo();
			if (s_bDrawSelectionInfo)
				DrawSelectionInfo();
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// Set Test Game Flags
		#ifdef WORKBENCH
			if (GetGame().GetWorldEntity() && !GetGame().AreGameFlagsObtained())
			{
				GetGame().SetGameFlags(m_eTestGameFlags, false);
			}
		#endif

		// Find required components
        m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_pRespawnSystemComponent = SCR_RespawnSystemComponent.Cast(owner.FindComponent(SCR_RespawnSystemComponent));
		m_pRespawnHandlerComponent = SCR_RespawnHandlerComponent.Cast(owner.FindComponent(SCR_RespawnHandlerComponent));
		m_RespawnTimerComponent = SCR_RespawnTimerComponent.Cast(owner.FindComponent(SCR_RespawnTimerComponent));
		m_ScoringSystemComponent = SCR_BaseScoringSystemComponent.Cast(owner.FindComponent(SCR_BaseScoringSystemComponent));

		if (!m_RplComponent)
			Print("SCR_BaseGameMode is missing RplComponent! Game functionality might be broken!", LogLevel.ERROR);
		if (!m_pRespawnSystemComponent)
			Print("SCR_BaseGameMode is missing SCR_RespawnSystemComponent! Respawning functionality might be broken!", LogLevel.ERROR);
		if (!m_pRespawnHandlerComponent)
			Print("SCR_BaseGameMode is missing SCR_RespawnHandlerComponent! Respawning functionality might be broken!", LogLevel.ERROR);

		if (!m_aAdditionalGamemodeComponents)
			m_aAdditionalGamemodeComponents = new ref array<SCR_BaseGameModeComponent>();

		array<Managed> additionalComponents = new array<Managed>();
		int count = owner.FindComponents(SCR_BaseGameModeComponent, additionalComponents);

		m_aAdditionalGamemodeComponents.Clear();
		for (int i = 0; i < count; i++)
		{
			SCR_BaseGameModeComponent comp = SCR_BaseGameModeComponent.Cast(additionalComponents[i]);
			m_aAdditionalGamemodeComponents.Insert(comp);
		}
	}

	//------------------------------------------------------------------------------------------------
	float GetBleedingRate()
	{
		return m_fBleedingRateScale;
	}

	//------------------------------------------------------------------------------------------------
	void SetBleedingRate(float rate)
	{
		m_fBleedingRateScale = rate;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_BaseGameMode(IEntitySource src, IEntity parent)
	{
		#ifdef GAME_MODE_DEBUG
		if (!s_DebugRegistered)
		{
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_GAME_MODE, "", "Game Mode Diag", "Network");
			s_DebugRegistered = true;
		}
		#endif

		Activate();
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_BaseGameMode()
	{
		#ifdef GAME_MODE_DEBUG
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_GAME_MODE);
			s_DebugRegistered = false;
		#endif
		
		#ifdef GMSTATS
		if (SCR_GameModeStatistics.IsRecording())
			SCR_GameModeStatistics.StopRecording();
		#endif
	}
	
	#ifdef GMSTATS
	//! Should gamemode diagnostic statistics be enabled?
	private bool IsGameModeStatisticsEnabled()
	{
		// not authority
		if (m_RplComponent && !m_RplComponent.IsMaster())
			return false;
		
		return GetGame().InPlayMode();
	}
	#endif
	
};