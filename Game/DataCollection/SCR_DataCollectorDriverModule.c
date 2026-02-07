[BaseContainerProps()]
class SCR_DataCollectorDriverModule : SCR_DataCollectorModule
{
	protected ref map<int, IEntity> m_mTrackedPlayersInVehicles = new map<int, IEntity>();
	
	protected float m_fTimeSinceUpdate = 0;
	protected float m_fTimeToUpdate = 1;
	
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
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		if (!compartment)
			return;
		
		IEntity playerEntity = compartment.GetOccupant();
		if (!playerEntity)
			return;
		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity);
		if (playerID == 0) // Non-player character
			return;
		
		SlotManagerComponent slotManagerComponent = SlotManagerComponent.Cast(targetEntity.FindComponent(SlotManagerComponent));
		if (!slotManagerComponent)
			return;
		
		m_mTrackedPlayersInVehicles.Insert(playerID, playerEntity);
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
		
		IEntity playerEntity = m_mTrackedPlayersInVehicles.Get(playerID);
		if (!playerEntity)
			return;
		
		ChimeraCharacter playerChimeraCharacter = ChimeraCharacter.Cast(playerEntity);
		
		//If the player died, blame the driver of that vehicle
		if (playerChimeraCharacter.GetDamageManager().GetState() == EDamageState.DESTROYED)
			PlayerDied(playerEntity, playerChimeraCharacter);
		
		m_mTrackedPlayersInVehicles.Remove(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PlayerDied(IEntity playerEntity, ChimeraCharacter playerChimeraCharacter)
	{
		IEntity vehicle = CompartmentAccessComponent.GetVehicleIn(playerEntity);
		if (!vehicle)
			return;
		
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;
		
		array<IEntity> occupants = {};
		compartmentManager.GetOccupants(occupants);
		
		if (occupants.Count() <= 1) //player was alone in vehicle. Do nothing
			return;
		
		BaseCompartmentSlot playerCompartmentSlot = playerChimeraCharacter.GetCompartmentAccessComponent().GetCompartment();
		if (!playerCompartmentSlot)
			return;
		
		int occupantsDead, pilotID;
		if (SCR_CompartmentAccessComponent.GetCompartmentType(playerCompartmentSlot) == ECompartmentType.Pilot)
		{
			//The driver was killed.
			occupantsDead = occupants.Count()-1; //All players from the vehicle are in danger now because of the pilot's death, so we act as if they died
			pilotID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity);
		}
		else
		{
			//A player died and it was not the pilot.
			occupantsDead = 1;
			
			//Find if there's a pilot and their ID
			array<IEntity> pilot = {};
			compartmentManager.GetOccupantsOfType(pilot, ECompartmentType.Pilot);
					
			//If there's no pilot it's none's fault.
			if (pilot.IsEmpty())
				return;
			
			pilotID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pilot.Get(0));
		}
		
		if (pilotID == 0)
			return; //Pilot is an AI
		
		GetGame().GetDataCollector().GetPlayerData(pilotID).AddPlayersDiedInVehicle(occupantsDead);
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
		
		Faction victimFaction = FactionAffiliationComponent.Cast(player.FindComponent(FactionAffiliationComponent)).GetAffiliatedFaction();
		Faction killerFaction = FactionAffiliationComponent.Cast(killer.FindComponent(FactionAffiliationComponent)).GetAffiliatedFaction();
		
		//Add a kill. Find if friendly or unfriendly and if opponent AI or opponent player
		//If factions exist, AddRoadKill(bool) and AddAIRoadKill(bool) will do the rest
		if (killerFaction && victimFaction)
		{
			if (playerID != 0)
				killerData.AddRoadKill(killerFaction.IsFactionFriendly(victimFaction));
			else
				killerData.AddAIRoadKill(killerFaction.IsFactionFriendly(victimFaction));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerID, IEntity controlledEntity = null)
	{
		controlledEntity = m_mTrackedPlayersInVehicles.Get(playerID);
		super.OnPlayerDisconnected(playerID, controlledEntity);
		
		m_mTrackedPlayersInVehicles.Remove(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Execute(IEntity owner, float timeTick)
	{
		//If there's no data collector or no players are tracked, do nothing
		if (!GetGame().GetDataCollector() || m_mTrackedPlayersInVehicles.IsEmpty())
			return;
		
		m_fTimeSinceUpdate += timeTick;
		
		if (m_fTimeSinceUpdate<m_fTimeToUpdate)
			return;
		
		IEntity vehicle;
		Physics physics;
		
		IEntity playerEntity;
		ChimeraCharacter playerChimeraCharacter;
		
		CompartmentAccessComponent playerCompartmentAccessComponent;
		
		float distanceTraveled;
		int playerId;
		
		SCR_BaseCompartmentManagerComponent compartmentManagerComponent;
		
		SCR_PlayerData playerData;
		
		for (int i = m_mTrackedPlayersInVehicles.Count() - 1; i >= 0; i--)
		{
			playerEntity = m_mTrackedPlayersInVehicles.GetElement(i);
			vehicle = CompartmentAccessComponent.GetVehicleIn(playerEntity);
			if (!vehicle)
				return;
			
			physics = vehicle.GetPhysics();
			if (!physics)
				return;
			
			//Zguba: Physics information could be unreliable because this is potentially only calculated on the driver's local
			distanceTraveled = physics.GetVelocity().Length() * m_fTimeSinceUpdate;
			if (distanceTraveled < 1)
				continue;
			playerChimeraCharacter = ChimeraCharacter.Cast(playerEntity);
			playerCompartmentAccessComponent = playerChimeraCharacter.GetCompartmentAccessComponent();
			
			if (!playerCompartmentAccessComponent)
				continue;
			
			playerId = m_mTrackedPlayersInVehicles.GetKey(i);
			playerData = GetGame().GetDataCollector().GetPlayerData(playerId);
		
			//If player is driver we give some points, if not we give others
			if (SCR_CompartmentAccessComponent.GetCompartmentType(playerCompartmentAccessComponent.GetCompartment()) == ECompartmentType.Pilot)
			{
				playerData.AddMetersDriven(distanceTraveled);
				
				//Need to find the number of players this pilot is driving around
				compartmentManagerComponent = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
				if (!compartmentManagerComponent)
					continue;
				array<IEntity> occupants = {};
				compartmentManagerComponent.GetOccupants(occupants);
				
				//Give points to driver for driving distanceTraveled meters. Not counting the driver as occupant for points calculation
				playerData.CalculatePointsAsDriver(distanceTraveled, occupants.Count()-1);
			}
			else
			{
				//Add distanceTraveled meters as occupant of a vehicle
				playerData.AddMetersAsOccupant(distanceTraveled);
			}
			
			//Prototyping feature
			if (StatsVisualization)
			{
				StatsVisualization.Get(EDriverModuleStats.MetersDriven).SetText(playerData.GetDistanceDriven().ToString());
				StatsVisualization.Get(EDriverModuleStats.MetersAsOccupant).SetText(playerData.GetDistanceAsOccupant().ToString());
				StatsVisualization.Get(EDriverModuleStats.PointsAsDriver).SetText(playerData.GetPointsAsDriverOfPlayers().ToString());
				StatsVisualization.Get(EDriverModuleStats.RoadKills).SetText(playerData.GetRoadKills().ToString());
				StatsVisualization.Get(EDriverModuleStats.AIRoadKills).SetText(playerData.GetAIRoadKills().ToString());
				StatsVisualization.Get(EDriverModuleStats.FriendlyRoadKills).SetText(playerData.GetFriendlyRoadKills().ToString());
				StatsVisualization.Get(EDriverModuleStats.FriendlyAIRoadKills).SetText(playerData.GetFriendlyAIRoadKills().ToString());
				StatsVisualization.Get(EDriverModuleStats.PlayersDiedInVehicle).SetText(playerData.GetPlayersDiedInVehicle().ToString());
			}				
		}
		m_fTimeSinceUpdate = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	override void CreateVisualization()
	{
		super.CreateVisualization();
		if (!StatsVisualization)
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
};
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