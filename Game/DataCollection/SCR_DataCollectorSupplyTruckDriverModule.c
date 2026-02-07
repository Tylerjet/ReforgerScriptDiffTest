[BaseContainerProps()]
class SCR_DataCollectorSupplyTruckDriverModule : SCR_DataCollectorModule
{	
	protected ref map<int, IEntity> m_mTrackedDrivers = new map<int, IEntity>();
	
	protected float m_fTimeSinceUpdate = 0;
	protected float m_fTimeToUpdate = 1;
		
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
		
		array<EntitySlotInfo> infos = {};
				
		bool isSupplyTruck;
		for (int i = 0, count = slotManagerComponent.GetSlotInfos(infos); i < count; i++)
		{
			IEntity attachedEntity = infos[i].GetAttachedEntity();
			if (!attachedEntity)
				continue;
			
			SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(attachedEntity.FindComponent(SCR_CampaignSuppliesComponent));
			if (suppliesComponent)
			{
				//Player playerID is a driver of a vehicle with supplies
				m_mTrackedDrivers.Insert(playerID, playerEntity);
				return;
			}
		}	
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
		
		m_mTrackedDrivers.Remove(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void AddInvokers(IEntity player)
	{
		super.AddInvokers(player);
		if (!player)
			return;
		
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));
			
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
	override void OnPlayerDisconnected(int playerID, IEntity controlledEntity = null)
	{
		controlledEntity = m_mTrackedDrivers.Get(playerID);
		super.OnPlayerDisconnected(playerID, controlledEntity);
		
		m_mTrackedDrivers.Remove(playerID);
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
				
		IEntity playerEntity;
		IEntity vehicle;
		Physics physics;
		float distanceTraveled;
		int playerId;
		SCR_PlayerData playerData;
				
		for (int i = m_mTrackedDrivers.Count() - 1; i >= 0; i--)
		{
			playerEntity = m_mTrackedDrivers.GetElement(i);
			vehicle = CompartmentAccessComponent.GetVehicleIn(playerEntity);
			if (!vehicle)
				continue;
			
			physics = vehicle.GetPhysics();
			if (!physics)
				continue;
			
			distanceTraveled = physics.GetVelocity().Length() * timeTick;
			playerId = m_mTrackedDrivers.GetKey(i);
			
			playerData = GetGame().GetDataCollector().GetPlayerData(playerId);
			playerData.AddTraveledDistanceSupplyVehicle(distanceTraveled);
			playerData.AddTraveledTimeSupplyVehicle(timeTick);
			
			//Prototyping feature
			if (StatsVisualization)
			{
				StatsVisualization.Get(ESupplyTruckDriverModuleStats.MetersDrivenSupplyVehicle).SetText(playerData.GetTraveledDistanceSupplyVehicle().ToString());
				StatsVisualization.Get(ESupplyTruckDriverModuleStats.TimeDrivenSupplyVehicle).SetText(playerData.GetTraveledTimeSupplyVehicle().ToString());
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
		
		CreateEntry("Meters driven w Supply Vehicle: ", 0, ESupplyTruckDriverModuleStats.MetersDrivenSupplyVehicle);
		CreateEntry("Time driving Supply Vehicle: ", 0, ESupplyTruckDriverModuleStats.TimeDrivenSupplyVehicle);
	}
};

enum ESupplyTruckDriverModuleStats
{
	MetersDrivenSupplyVehicle,
	TimeDrivenSupplyVehicle
};