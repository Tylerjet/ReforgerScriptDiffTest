class SCR_DataCollectorDriverModuleContext : Managed
{
	IEntity m_Player;
	IEntity m_Vehicle;
	bool m_bPilot;
	
	void SCR_DataCollectorDriverModuleContext(notnull IEntity player, notnull IEntity vehicle, bool pilot)
	{
		m_Player = player;
		m_Vehicle = vehicle;
		m_bPilot = pilot;
	}
}

[BaseContainerProps()]
class SCR_DataCollectorDriverModule : SCR_DataCollectorModule
{
	protected ref map<int, ref SCR_DataCollectorDriverModuleContext> m_mTrackedPlayersInVehicles = new map<int, ref SCR_DataCollectorDriverModuleContext>();

	//------------------------------------------------------------------------------------------------
	protected override void AddInvokers(IEntity player)
	{
		super.AddInvokers(player);
		if (!player)
			return;

		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));

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

		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));

		if (!compartmentAccessComponent)
			return;

		compartmentAccessComponent.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
		compartmentAccessComponent.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (!targetEntity || !manager)
		{
			Print("ERROR IN DATACOLLECTOR DRIVER MODULE: TARGETENTITY OR MANAGER ARE EMPTY.", LogLevel.ERROR);
			return;
		}
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		if (!compartment)
			return;

		IEntity playerEntity = compartment.GetOccupant();
		if (!playerEntity)
			return;

		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity);
		if (playerID == 0) // Non-player character
			return;
		
		m_mTrackedPlayersInVehicles.Insert(playerID, new SCR_DataCollectorDriverModuleContext(playerEntity, targetEntity, SCR_CompartmentAccessComponent.GetCompartmentType(compartment) == ECompartmentType.Pilot));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		if (!compartment)
			return;

		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(compartment.GetOccupant());
		if (playerID == 0) // Non-player character
			return;

		//Is the player inside a vehicle?
		SCR_DataCollectorDriverModuleContext playerContext = m_mTrackedPlayersInVehicles.Get(playerID);
		if (!playerContext)
			return;
		
		//If the player died, blame the driver of that vehicle
		ChimeraCharacter playerChimeraCharacter = ChimeraCharacter.Cast(playerContext.m_Player);
		if (playerChimeraCharacter && playerChimeraCharacter.GetDamageManager().GetState() == EDamageState.DESTROYED)
			PlayerDied(playerID, playerContext);

		m_mTrackedPlayersInVehicles.Remove(playerID);
	}

/*
TODO: REMOVE THIS, REPLACE WITH SENDING THROUGH RPL THE STATS FROM THE SERVER RECURRENTLY, AS IT'S ONLY FOR DEBUGGING PURPOSES
#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);

		if (to)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(to);
			m_mTrackedPlayersInVehicles.Insert(playerID, to);
		}
		else if (from)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(from);
			m_mTrackedPlayersInVehicles.Remove(playerID);
		}
		
	}
#endif
*/
	
	//------------------------------------------------------------------------------------------------
	protected void PlayerDied(int PlayerID, notnull SCR_DataCollectorDriverModuleContext playerContext)
	{
		//TODO: Replace this using a C++ implemented method to get the count of occupants of the vehicle
		/**********************************************************************************************/
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(playerContext.m_Vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;
		
		array<IEntity> occupants = {};
		compartmentManager.GetOccupants(occupants);
		
		if (occupants.Count() <= 1) //player was alone in vehicle. Do nothing
			return;
		/**********************************************************************************************/

		if (playerContext.m_bPilot)
		{
			Print("OK PLAYER KILLED WAS A PILOT. ADDING "+(occupants.Count()-1), LogLevel.ERROR);
			//The driver was killed.
			//All players from the vehicle are in danger now because of the pilot's death, so we act as if they died
			GetGame().GetDataCollector().GetPlayerData(PlayerID).AddStat(SCR_EDataStats.PLAYERS_DIED_IN_VEHICLE, occupants.Count()-1);
			return;
		}
		
		//Find if there's a pilot and their ID
		array<IEntity> pilot = {};
		compartmentManager.GetOccupantsOfType(pilot, ECompartmentType.Pilot);

		//If there's no pilot it's none's fault.
		if (pilot.IsEmpty())
			return;

		//Let's assume there's only one pilot, or if there are multiple, let's assume the main one is in the position 0
		int pilotID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pilot.Get(0));

		if (pilotID == 0)
			return; //Pilot is an AI
Print("OK PLAYER DIED. ADDING ONLY 1", LogLevel.ERROR);
		//A player died and it was not the pilot. So the pilot has someone dying on their vehicle
		GetGame().GetDataCollector().GetPlayerData(PlayerID).AddStat(SCR_EDataStats.PLAYERS_DIED_IN_VEHICLE, 1);
		//TODO: Make sure it is not possible to add this kill multiple times to the pilot if all players die simultaneously
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerID, IEntity player, IEntity killer)
	{
		//We are only looking for roadkills here. That's why we don't call super.OnPlayerKilled. This behaviour is very specific
		
		if (!player || !killer)
			return;

		int killerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(killer);

		//If the killer is AI count no roadKill
		if (killerID == 0)
			return;

		//Find compartment of killer to see if they are a driver
		ChimeraCharacter killerChimeraCharacter = ChimeraCharacter.Cast(killer);
		if (!killerChimeraCharacter)
			return;

		CompartmentAccessComponent killerCompartmentAccessComponent = killerChimeraCharacter.GetCompartmentAccessComponent();
		if (!killerCompartmentAccessComponent)
			return;

		BaseCompartmentSlot killerCompartmentSlot = killerCompartmentAccessComponent.GetCompartment();

		if (!killerCompartmentSlot)
			return;

		//Check if killer is a driver
		if (SCR_CompartmentAccessComponent.GetCompartmentType(killerCompartmentSlot) != ECompartmentType.Pilot)
			return;

		//Now we know the killer is not an AI and they are a driver. Add roadkill!

		SCR_PlayerData killerData = GetGame().GetDataCollector().GetPlayerData(killerID);

		FactionAffiliationComponent victimAffiliation = FactionAffiliationComponent.Cast(player.FindComponent(FactionAffiliationComponent));
		if (!victimAffiliation)
			return;
		
		FactionAffiliationComponent killerAffiliation = FactionAffiliationComponent.Cast(killer.FindComponent(FactionAffiliationComponent));
		if (!killerAffiliation)
			return;
		
		Faction victimFaction = victimAffiliation.GetAffiliatedFaction();
		Faction killerFaction = killerAffiliation.GetAffiliatedFaction();

		//Add a kill. Find if friendly or unfriendly and if opponent AI or opponent player
		//If factions exist, AddRoadKill(bool) and AddAIRoadKill(bool) will do the rest
		if (killerFaction && victimFaction)
		{
			if (playerID != 0)
			{
				if (killerFaction.IsFactionFriendly(victimFaction))
					killerData.AddStat(SCR_EDataStats.FRIENDLY_ROADKILLS);
				else
					killerData.AddStat(SCR_EDataStats.ROADKILLS);
			}	
			else
			{
				if (killerFaction.IsFactionFriendly(victimFaction))
					killerData.AddStat(SCR_EDataStats.FRIENDLY_AI_ROADKILLS);
				else
					killerData.AddStat(SCR_EDataStats.AI_ROADKILLS);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerID, IEntity controlledEntity = null)
	{
		if (!controlledEntity)
			controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		super.OnPlayerDisconnected(playerID, controlledEntity);

		m_mTrackedPlayersInVehicles.Remove(playerID);
	}

	//------------------------------------------------------------------------------------------------
	override void Update(IEntity owner, float timeTick)
	{
		//If there are no players tracked, do nothing
		if (m_mTrackedPlayersInVehicles.IsEmpty())
			return;

		m_fTimeSinceUpdate += timeTick;

		if (m_fTimeSinceUpdate < TIME_TO_UPDATE)
			return;

		SCR_BaseCompartmentManagerComponent compartmentManagerComponent;
		SCR_DataCollectorDriverModuleContext playerContext;
		SCR_PlayerData playerData;
		
		Physics physics;
		float distanceTraveled;
		
		//for (MapIterator iterator = m_mTrackedPlayersInVehicles.Begin(); iterator != m_mTrackedPlayersInVehicles.End(); iterator++)
		for (int i = 0; i < m_mTrackedPlayersInVehicles.Count(); ++i)
		{
			//playerContext = m_mTrackedPlayersInVehicles.GetIteratorElement(iterator);
			playerContext = m_mTrackedPlayersInVehicles.GetElement(i);
			
			physics = playerContext.m_Vehicle.GetPhysics();
			if (!physics)
				continue;

			distanceTraveled = physics.GetVelocity().Length() * m_fTimeSinceUpdate;
			if (distanceTraveled < 1)
				continue;
			
			//playerData = GetGame().GetDataCollector().GetPlayerData(m_mTrackedPlayersInVehicles.GetIteratorKey(iterator));
			playerData = GetGame().GetDataCollector().GetPlayerData(m_mTrackedPlayersInVehicles.GetKey(i));

			//If player is driver we give some points, if not we give others
			if (playerContext.m_bPilot)
			{
				playerData.AddStat(SCR_EDataStats.METERS_DRIVEN, distanceTraveled);

				//Need to find the number of players this pilot is driving around
				//TODO: Replace this using a C++ implemented method to get the count of occupants of the vehicle
				compartmentManagerComponent = SCR_BaseCompartmentManagerComponent.Cast(playerContext.m_Vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
				if (!compartmentManagerComponent)
					continue;
				array<IEntity> occupants = {};
				compartmentManagerComponent.GetOccupants(occupants);

				//Give points to driver for driving distanceTraveled meters. Not counting the driver as occupant for points calculation
				playerData.AddPointsAsDriverOfPlayers(distanceTraveled, occupants.Count()-1);
			}
			else
			{
				//Add distanceTraveled meters as occupant of a vehicle
				playerData.AddStat(SCR_EDataStats.METERS_AS_OCCUPANT, distanceTraveled);
			}

			//DEBUG display
#ifdef ENABLE_DIAG
			if (m_StatsVisualization)
			{
				m_StatsVisualization.Get(EDriverModuleStats.MetersDriven).SetText(playerData.GetCurrentDistanceDriven().ToString());
				m_StatsVisualization.Get(EDriverModuleStats.MetersAsOccupant).SetText(playerData.GetCurrentDistanceAsOccupant().ToString());
				m_StatsVisualization.Get(EDriverModuleStats.PointsAsDriver).SetText(playerData.GetCurrentPointsAsDriverOfPlayers().ToString());
				m_StatsVisualization.Get(EDriverModuleStats.RoadKills).SetText(playerData.GetCurrentRoadKills().ToString());
				m_StatsVisualization.Get(EDriverModuleStats.AIRoadKills).SetText(playerData.GetCurrentAIRoadKills().ToString());
				m_StatsVisualization.Get(EDriverModuleStats.FriendlyRoadKills).SetText(playerData.GetCurrentFriendlyRoadKills().ToString());
				m_StatsVisualization.Get(EDriverModuleStats.FriendlyAIRoadKills).SetText(playerData.GetCurrentFriendlyAIRoadKills().ToString());
				m_StatsVisualization.Get(EDriverModuleStats.PlayersDiedInVehicle).SetText(playerData.GetCurrentPlayersDiedInVehicle().ToString());
			}
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

		CreateEntry("Meters driven: ", 0, EDriverModuleStats.MetersDriven);
		CreateEntry("Meters as Occupant: ", 0, EDriverModuleStats.MetersAsOccupant);
		CreateEntry("Points as Driver of ppl: ", 0, EDriverModuleStats.PointsAsDriver);
		CreateEntry("RoadKills: ", 0, EDriverModuleStats.RoadKills);
		CreateEntry("AI RoadKills: ", 0, EDriverModuleStats.AIRoadKills);
		CreateEntry("Friendly RoadKills: ", 0, EDriverModuleStats.FriendlyRoadKills);
		CreateEntry("Friendly AI RoadKills: ", 0, EDriverModuleStats.FriendlyAIRoadKills);
		CreateEntry("DeadPlayers at ur Vehicle: ", 0, EDriverModuleStats.PlayersDiedInVehicle);
	}
#endif
};

#ifdef ENABLE_DIAG
enum EDriverModuleStats
{
	MetersDriven,
	MetersAsOccupant,
	PointsAsDriver,
	RoadKills,
	AIRoadKills,
	FriendlyRoadKills,
	FriendlyAIRoadKills,
	PlayersDiedInVehicle
};
#endif
