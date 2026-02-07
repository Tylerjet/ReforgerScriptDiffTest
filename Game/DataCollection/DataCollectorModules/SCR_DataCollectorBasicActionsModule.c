[BaseContainerProps()]
class SCR_DataCollectorBasicActionsModule : SCR_DataCollectorModule
{
	protected ref map<int, IEntity> m_mTrackedPlayers = new map<int, IEntity>();
	static const int LIMIT_WALKING_SPEED_PER_SECOND = 5;
	static const int MAX_WALKING_DISTANCE_PER_SECOND = LIMIT_WALKING_SPEED_PER_SECOND * TIME_TO_UPDATE;

	//------------------------------------------------------------------------------------------------
	protected override void AddInvokers(IEntity player)
	{
		super.AddInvokers(player);

		if (!player)
			return;

		SCR_ChimeraCharacter chimeraPlayer = SCR_ChimeraCharacter.Cast(player);
		if (!chimeraPlayer)
			return;

		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(chimeraPlayer.GetCompartmentAccessComponent());
		if (!compartmentAccessComponent)
			return;

		compartmentAccessComponent.GetOnCompartmentEntered().Insert(OnCompartmentEntered);
		compartmentAccessComponent.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
	}

	//------------------------------------------------------------------------------------------------
	protected override void RemoveInvokers(IEntity player)
	{
		super.RemoveInvokers(player);

		if (!player)
			return;

		SCR_ChimeraCharacter chimeraPlayer = SCR_ChimeraCharacter.Cast(player);
		if (!chimeraPlayer)
			return;

		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(chimeraPlayer.GetCompartmentAccessComponent());
		if (!compartmentAccessComponent)
			return;

		compartmentAccessComponent.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
		compartmentAccessComponent.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		if (!compartment)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return;

		int playerID = playerManager.GetPlayerIdFromControlledEntity(compartment.GetOccupant());
		if (playerID == 0) // Non-player character
			return;

		m_mTrackedPlayers.Remove(playerID);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		if (!compartment)
			return;

		IEntity playerEntity = compartment.GetOccupant();

		if (!playerEntity)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		int playerID = playerManager.GetPlayerIdFromControlledEntity(playerEntity);
		if (playerID == 0) // Non-player character or non-pilot
			return;

		m_mTrackedPlayers.Insert(playerID, playerEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerID, IEntity controlledEntity = null)
	{
		controlledEntity = m_mTrackedPlayers.Get(playerID);
		super.OnPlayerDisconnected(playerID, controlledEntity);

		m_mTrackedPlayers.Remove(playerID);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerID, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerID, controlledEntity);
		m_mTrackedPlayers.Insert(playerID, controlledEntity);
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);

		if (to)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(to);
			m_mTrackedPlayers.Insert(playerID, to);
		}
		else if (from)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(from);
			m_mTrackedPlayers.Remove(playerID);
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerID, IEntity player, IEntity killer)
	{
		super.OnPlayerKilled(playerID, player, killer);
		m_mTrackedPlayers.Remove(playerID);
	}

	//------------------------------------------------------------------------------------------------
	override void Update(IEntity owner, float timeTick)
	{
		m_fTimeSinceUpdate += timeTick;

		if (m_fTimeSinceUpdate < TIME_TO_UPDATE)
			return;

		Physics physics;
		float distanceTraveled;
		int playerId;
		SCR_PlayerData playerData;
		IEntity currentEntity;

		for (int i = m_mTrackedPlayers.Count() - 1; i >= 0; i--)
		{
			currentEntity = m_mTrackedPlayers.GetElement(i);
			if (!currentEntity)
				continue;

			physics = currentEntity.GetPhysics();
			if (!physics)
				continue;

			playerId = m_mTrackedPlayers.GetKey(i);
			playerData = GetGame().GetDataCollector().GetPlayerData(playerId);

			//We take current velocity and assume it was the same for the whole m_fTimeSinceUpdate timeframe
			distanceTraveled = physics.GetVelocity().Length() * m_fTimeSinceUpdate;

			//Safety measure
			if (distanceTraveled > MAX_WALKING_DISTANCE_PER_SECOND)
				distanceTraveled = MAX_WALKING_DISTANCE_PER_SECOND;

			playerData.AddStat(SCR_EDataStats.METERS_WALKED, distanceTraveled);

			//DEBUG display

#ifdef ENABLE_DIAG
			if (m_StatsVisualization)
				m_StatsVisualization.Get(EBasicActionsModuleStats.DistanceWalked).SetText(playerData.GetCurrentDistanceWalked().ToString());
#endif
		}
		m_fTimeSinceUpdate = 0;
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void CreateVisualization()
	{
		super.CreateVisualization();
		if (!m_StatsVisualization)
			return;

		CreateEntry("Distance Walked: ", 0, EBasicActionsModuleStats.DistanceWalked);
	}
#endif
};


#ifdef ENABLE_DIAG
enum EBasicActionsModuleStats
{
	DistanceWalked
};
#endif
