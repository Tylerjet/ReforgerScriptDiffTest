[BaseContainerProps()]
class SCR_DataCollectorShootingModule : SCR_DataCollectorModule
{
	protected ref map<int, IEntity> m_mTrackedPossibleShooters = new map<int, IEntity>();
	
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
		
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(player.FindComponent(EventHandlerManagerComponent));
		if (!eventHandlerManager)
			return;
		
		eventHandlerManager.RegisterScriptHandler("OnProjectileShot", this, OnWeaponFired);
		eventHandlerManager.RegisterScriptHandler("OnGrenadeThrown", this, OnGrenadeThrown);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnWeaponFired(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(playerID);
		
		//In the future we will use Weapon.GetWeaponType() and Weapon.GetWeaponSubtype() to determine the weapon shot and add it to the player's profile
		//For now, simply count a shot
		playerData.AddShot();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnGrenadeThrown(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		if (!weapon)
			return;
		
		SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(playerID);
		
		//Not counting smoke grenade 'cause it deals no damage. TO DO: Count it as a different specialization operation
		if(weapon.GetWeaponType()==EWeaponType.WT_SMOKEGRENADE)
			return;
		
		playerData.AddGrenadeThrown();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void RemoveInvokers(IEntity player)
	{
		super.RemoveInvokers(player);
		if(!player)
			return;
		
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));
		
		if(!compartmentAccessComponent)
			return;
		
		compartmentAccessComponent.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
		compartmentAccessComponent.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
		
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(player.FindComponent(EventHandlerManagerComponent));
		if (!eventHandlerManager)
			return;
		
		eventHandlerManager.RemoveScriptHandler("OnProjectileShot", this, OnWeaponFired);
		eventHandlerManager.RemoveScriptHandler("OnGrenadeThrown", this, OnGrenadeThrown);
	}
	
	//Players who enter a vehicle do not need to be tracked as possible shooters, unless using a turret
	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		
		//Turrets can shoot
		if (!compartment || !compartment.GetOccupant() || SCR_CompartmentAccessComponent.GetCompartmentType(compartment) == ECompartmentType.Turret)
			return;
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		
		int playerID = playerManager.GetPlayerIdFromControlledEntity(compartment.GetOccupant());
		
		m_mTrackedPossibleShooters.Remove(playerID);
	}
	
	//Players who leave a vehicle can be shooters
	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		if (!compartment)
			return;
		
		if (!compartment.GetOccupant())
			return;
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		
		int playerID = playerManager.GetPlayerIdFromControlledEntity(compartment.GetOccupant());
		if (playerID == 0) // Non-player character
			return;
		
		m_mTrackedPossibleShooters.Insert(playerID, compartment.GetOccupant());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerID, IEntity controlledEntity = null)
	{
		controlledEntity = m_mTrackedPossibleShooters.Get(playerID);
		super.OnPlayerDisconnected(playerID, controlledEntity);
		
		m_mTrackedPossibleShooters.Remove(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerID, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerID, controlledEntity);
		m_mTrackedPossibleShooters.Insert(playerID, controlledEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerID, IEntity player, IEntity killer)
	{
		super.OnPlayerKilled(playerID, player, killer);
		m_mTrackedPossibleShooters.Remove(playerID);
		
		//Adding death.
		if (playerID != 0)
		{
			SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(playerID);
			playerData.AddDeath();
		}
		
		int killerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(killer);
		if (killerID == 0) 
			return;
		
		// Suicide?
		if (playerID == killerID)
			return;
		
		SCR_PlayerData killerData = GetGame().GetDataCollector().GetPlayerData(killerID);
		
		FactionAffiliationComponent victimAffiliationComponent = FactionAffiliationComponent.Cast(player.FindComponent(FactionAffiliationComponent));
		FactionAffiliationComponent killerffiliationComponent = FactionAffiliationComponent.Cast(killer.FindComponent(FactionAffiliationComponent));
		if (!victimAffiliationComponent || !killerffiliationComponent)
			return;
		Faction victimFaction = victimAffiliationComponent.GetAffiliatedFaction();
		Faction killerFaction = killerffiliationComponent.GetAffiliatedFaction();
		
		//Add a kill. Find if friendly or unfriendly and if opponent AI or opponent player
		//If factions exist, AddKill(bool) and AddAIKill(bool) will do the rest
		if (killerFaction && victimFaction)
		{
			if (playerID != 0)
				killerData.AddKill(killerFaction.IsFactionFriendly(victimFaction));
			else
				killerData.AddAIKill(killerFaction.IsFactionFriendly(victimFaction));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Execute(IEntity owner, float timeTick)
	{
		//If there's no data collector, do nothing
		if (!GetGame().GetDataCollector())
			return;
		
		m_fTimeSinceUpdate += timeTick;
		
		if (m_fTimeSinceUpdate<m_fTimeToUpdate)
			return;
		
		SCR_PlayerData playerData;
		int playerId;
		
		for (int i = m_mTrackedPossibleShooters.Count() - 1; i >= 0; i--)
		{
		
			playerId = m_mTrackedPossibleShooters.GetKey(i);
			playerData = GetGame().GetDataCollector().GetPlayerData(playerId);
			
			//Prototyping feature
			if (StatsVisualization)
			{
				StatsVisualization.Get(EShootingModuleStats.Deaths).SetText(playerData.GetDeaths().ToString());
				StatsVisualization.Get(EShootingModuleStats.PlayerKills).SetText(playerData.GetPlayerKills().ToString());
				StatsVisualization.Get(EShootingModuleStats.AIKills).SetText(playerData.GetAIKills().ToString());
				StatsVisualization.Get(EShootingModuleStats.FriendlyPlayerKills).SetText(playerData.GetFriendlyPlayerKills().ToString());
				StatsVisualization.Get(EShootingModuleStats.FriendlyAIKills).SetText(playerData.GetFriendlyAIKills().ToString());
				StatsVisualization.Get(EShootingModuleStats.BulletsShot).SetText(playerData.GetBulletsShot().ToString());
				StatsVisualization.Get(EShootingModuleStats.GrenadesThrown).SetText(playerData.GetGrenadesThrown().ToString());
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
		
		CreateEntry("Deaths: ", 0, EShootingModuleStats.Deaths);
		CreateEntry("Player Kills: ", 0, EShootingModuleStats.PlayerKills);
		CreateEntry("AI Kills: ", 0, EShootingModuleStats.AIKills);
		CreateEntry("Friendly Player Kills: ", 0, EShootingModuleStats.FriendlyPlayerKills);
		CreateEntry("Friendly AI Kills: ", 0, EShootingModuleStats.FriendlyAIKills);
		CreateEntry("Bullets Shot: ", 0, EShootingModuleStats.BulletsShot);
		CreateEntry("Grenades Thrown: ", 0, EShootingModuleStats.GrenadesThrown);
	}
};

enum EShootingModuleStats
{
	Deaths,
	PlayerKills,
	AIKills,
	FriendlyPlayerKills,
	FriendlyAIKills,
	BulletsShot,
	GrenadesThrown
};