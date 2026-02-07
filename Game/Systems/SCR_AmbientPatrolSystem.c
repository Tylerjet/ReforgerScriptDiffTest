//------------------------------------------------------------------------------------------------
class SCR_AmbientPatrolSystem : GameSystem
{
	protected static const int CHECK_INTERVAL = 3;					//s, how often should an individual patrol spawn be checked
	protected static const int DESPAWN_TIMEOUT = 10000;				//ms
	protected static const int SPAWN_RADIUS_MIN_SQ = 500 * 500;		// Square value for distance checks
	protected static const int SPAWN_RADIUS_MAX_SQ = 1000 * 1000;	// Square value for distance checks
	protected static const int SPAWN_RADIUS_BLOCK_SQ = 150 * 150;	// Square value for distance checks
	protected static const int DESPAWN_RADIUS_DIFF_SQ = 200 * 200;	// Square value for distance checks

	protected ref array<SCR_AmbientPatrolSpawnPointComponent> m_aPatrols = {};
	protected ref array<IEntity> m_aPlayers = {};

	protected int m_iLastAssignedIndex;
	protected int m_iIndexToCheck;
	protected int m_iSpawnDistanceSq;
	protected int m_iDespawnDistanceSq;

	protected float m_fTimer;
	protected float m_fCheckInterval;

	//------------------------------------------------------------------------------------------------
	override event protected void OnInit()
	{
		// No need to run updates unless some patrols are actually registered
		if (m_aPatrols.IsEmpty())
			Enable(false);

		RefreshPlayerList();

		// Calculate (de)spawn distance based on view distance, have it squared for faster distance calculation
		int fractionOfVD = GetGame().GetViewDistance() * 0.3;
		m_iSpawnDistanceSq = fractionOfVD * fractionOfVD;
		m_iSpawnDistanceSq = Math.Min(SPAWN_RADIUS_MAX_SQ, m_iSpawnDistanceSq);
		m_iSpawnDistanceSq = Math.Max(SPAWN_RADIUS_MIN_SQ, m_iSpawnDistanceSq);
		m_iDespawnDistanceSq = m_iSpawnDistanceSq + DESPAWN_RADIUS_DIFF_SQ;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

		if (!gameMode)
			return;

		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		gameMode.GetOnPlayerDeleted().Insert(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnected);
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnCleanup()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

		if (!gameMode)
			return;

		gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerKilled().Remove(OnPlayerKilled);
		gameMode.GetOnPlayerDeleted().Remove(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnected);
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnUpdate(ESystemPoint point)
	{
		if (!GetGame().AreGameFlagsSet(EGameFlags.SpawnAI))
		{
			Enable(false);
			return;
		}

		float timeSlice = GetWorld().GetFixedTimeSlice();

		m_fTimer += timeSlice;

		if (m_fTimer < m_fCheckInterval)
			return;

		m_fTimer = 0;

		// HOTFIX: Don't process spawning at the very start - wait for a save to be applied if it exists
		// Otherwise full-size groups get spawned even if they are marked as eliminated in the save file
		// TODO: Come up with a better solution
		if (GetGame().GetWorld().GetWorldTime() < SCR_GameModeCampaign.BACKEND_DELAY)
			return;

		ProcessSpawnpoint(m_iIndexToCheck);
		m_iIndexToCheck++;

		if (!m_aPatrols.IsIndexValid(m_iIndexToCheck))
			m_iIndexToCheck = 0;
	}

	//------------------------------------------------------------------------------------------------
	override event bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_AmbientPatrolSystem GetInstance()
	{
		World world = GetGame().GetWorld();

		if (!world)
			return null;

		return SCR_AmbientPatrolSystem.Cast(world.FindSystem(SCR_AmbientPatrolSystem));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCheckInterval()
	{
		m_fCheckInterval = CHECK_INTERVAL / m_aPatrols.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshPlayerList()
	{
		m_aPlayers.Clear();
		array<int> playerIds = {};
		PlayerManager pc = GetGame().GetPlayerManager();
		int playersCount = pc.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			IEntity player = pc.GetPlayerControlledEntity(playerId);

			if (!player)
				continue;

			CharacterControllerComponent comp = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));

			if (!comp || comp.IsDead())
				continue;

			m_aPlayers.Insert(player);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawnedOrDeleted(int playerId, IEntity player)
	{
		RefreshPlayerList();
	}

	//------------------------------------------------------------------------------------------------
	//! Composes an array of surviving ambient patrols which can be serialized
	int GetRemainingPatrolsInfo(out notnull array<int> remnantsInfo)
	{
		int size;
		ChimeraWorld world = GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();

		foreach (SCR_AmbientPatrolSpawnPointComponent presence : m_aPatrols)
		{
			if (!presence)
				continue;

			SCR_AIGroup grp = presence.GetSpawnedGroup();

			if (presence.GetMembersAlive() < 0 && !grp && !presence.GetIsSpawned())
				continue;

			remnantsInfo.Insert(presence.GetID());

			if (grp)
			{
				size = remnantsInfo.Insert(grp.GetAgentsCount());
			}
			else
			{
				if (presence.GetIsSpawned())
					size = remnantsInfo.Insert(0);
				else
					size = remnantsInfo.Insert(presence.GetMembersAlive());
			}

			// Subtract current time so when data is loaded at scenario start, the delay is correct
			remnantsInfo.Insert(presence.GetRespawnTimestamp().DiffMilliseconds(curTime));
		}

		return size;
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessSpawnpoint(int spawnpointIndex)
	{
		SCR_AmbientPatrolSpawnPointComponent spawnpoint = m_aPatrols[spawnpointIndex];

		if (!spawnpoint || (spawnpoint.GetMembersAlive() == 0 && !spawnpoint.GetIsSpawned()))
			return;

		ChimeraWorld world = GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		if (spawnpoint.GetRespawnTimestamp().Greater(currentTime))
			return;

		if (!spawnpoint.GetIsSpawned())
		{
			spawnpoint.SpawnPatrol();
			return;
		}

		bool playersNear;
		bool playersVeryNear;
		bool playersFar = true;
		vector location = spawnpoint.GetOwner().GetOrigin();
		int distance;

		// Define if any player is close enough to spawn or if all players are far enough to despawn
		foreach (IEntity player : m_aPlayers)
		{
			if (!player)
				continue;

			distance = vector.DistanceSq(player.GetOrigin(), location);

			if (distance > m_iDespawnDistanceSq)
				continue;

			playersFar = false;

			if (distance > m_iSpawnDistanceSq)
				continue;

			playersNear = true;

			if (distance > SPAWN_RADIUS_BLOCK_SQ)
				continue;

			playersVeryNear = true;
			break;
		}

		bool isAIOverLimit;
		AIWorld aiWorld = GetGame().GetAIWorld();

		if (aiWorld)
		{
			int maxChars = aiWorld.GetLimitOfActiveAIs();

			if (maxChars <= 0)
				isAIOverLimit = true;
			else
				isAIOverLimit = ((float)aiWorld.GetCurrentNumOfActiveAIs() / (float)maxChars) > spawnpoint.GetAILimitThreshold();
		}

		if (!isAIOverLimit && !playersVeryNear)
			spawnpoint.SetIsPaused(false);

		if (playersNear && !spawnpoint.GetIsPaused() && !spawnpoint.IsGroupActive())
		{
			// Do not spawn the patrol if the AI threshold setting has been reached
			if (isAIOverLimit)
			{
				spawnpoint.SetIsPaused(true);	// Make sure a patrol is not spawned too close to players when AI limit suddenly allows spawning of this group
				return;
			}

			spawnpoint.ActivateGroup();
			return;
		}

		// Delay is used so dying players don't see the despawn happen
		if (spawnpoint.GetIsSpawned() && playersFar && spawnpoint.IsGroupActive())
		{
			WorldTimestamp despawnT = spawnpoint.GetDespawnTimer();

			if (despawnT == 0)
				spawnpoint.SetDespawnTimer(currentTime.PlusMilliseconds(DESPAWN_TIMEOUT));
			else if (currentTime.Greater(despawnT))
				spawnpoint.DeactivateGroup();
		}
		else
		{
			spawnpoint.SetDespawnTimer(null);
		}
	}

	//------------------------------------------------------------------------------------------------
	void RegisterPatrol(notnull SCR_AmbientPatrolSpawnPointComponent patrol)
	{
		if (!IsEnabled())
			Enable(true);

		m_aPatrols.Insert(patrol);
		patrol.SetID(m_iLastAssignedIndex);
		m_iLastAssignedIndex++;
		UpdateCheckInterval();
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterPatrol(notnull SCR_AmbientPatrolSpawnPointComponent patrol)
	{
		m_aPatrols.RemoveItem(patrol);
		m_iIndexToCheck = 0;

		if (!m_aPatrols.IsEmpty())
		{
			UpdateCheckInterval();
			return;
		}

		Enable(false);
	}

	//------------------------------------------------------------------------------------------------
	int GetPatrols(out array<SCR_AmbientPatrolSpawnPointComponent> patrols)
	{
		if (patrols)
			return patrols.Copy(m_aPatrols);
		else
			return m_aPatrols.Count();
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected(int playerId, KickCauseCode cause = KickCauseCode.NONE, int timeout = -1)
	{
		RefreshPlayerList();
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		RefreshPlayerList();
	}
}
