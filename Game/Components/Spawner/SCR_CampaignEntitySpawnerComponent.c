class SCR_CampaignEntitySpawnerComponentClass : SCR_EntitySpawnerComponentClass
{
}

/*!
Component that has a list of entities which it can spawn in the world
*/
class SCR_CampaignEntitySpawnerComponent : SCR_EntitySpawnerComponent
{	
	[Attribute("150", desc: "Range in which should component search for base.", category: "Entity Spawner")]
	protected float m_fBaseSearchDistance;
	
	protected SCR_CampaignBase m_Base;
	
	//------------------------------------------------------------------------------------------------
	override void PerformSpawn(int index = -1, IEntity user = null)
	{
		super.PerformSpawn(index, user);
		
		if (!m_SpawnedEntity)
			return;
		
		BaseRadioComponent radioComponent = BaseRadioComponent.Cast(m_SpawnedEntity.FindComponent(BaseRadioComponent));
		SCR_ECampaignHints hintID = SCR_ECampaignHints.NONE;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!playerController)
			return;
		
		SCR_CampaignNetworkComponent networkComp = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!networkComp)
			return;
		
		// If radio truck was requested, set its radio frequency etc.
		if (radioComponent)
		{
			SCR_CampaignFaction f = m_Base.GetOwningFaction();
		
			if (f)
			{
				radioComponent.TogglePower(false);
				radioComponent.SetFrequency(f.GetFactionRadioFrequency());
				radioComponent.SetEncryptionKey(f.GetFactionRadioEncryptionKey());
			}
		}
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_SpawnedEntity.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;
			
		array<EntitySlotInfo> slots = new array<EntitySlotInfo>;
		slotManager.GetSlotInfos(slots);
		
		IEntity truckBed;
		SCR_CampaignSuppliesComponent suppliesComponent;
		SCR_CampaignMobileAssemblyComponent mobileAssemblyComponent;
		
		foreach (EntitySlotInfo slot: slots)
		{
			if (!slot)
				continue;

			truckBed = slot.GetAttachedEntity();	
			if (!truckBed)
				continue;
				
			suppliesComponent = SCR_CampaignSuppliesComponent.Cast(truckBed.FindComponent(SCR_CampaignSuppliesComponent));
			mobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));
				
			// If supply truck was requested, show hint and handle garbage collector
			if (suppliesComponent)
			{
				hintID = SCR_ECampaignHints.SUPPLY_RUNS;
				EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(m_SpawnedEntity.FindComponent(EventHandlerManagerComponent));
				SCR_CampaignGarbageManager gManager = SCR_CampaignGarbageManager.Cast(GetGame().GetGarbageManager());
					
				if (eventHandlerManager && gManager)
					eventHandlerManager.RegisterScriptHandler("OnCompartmentLeft", m_SpawnedEntity, OnSupplyTruckLeft);
				
				networkComp.SendVehicleSpawnHint(hintID);
			}
				
			// If mobile assembly was requested, set its parent faction
			if (mobileAssemblyComponent)
			{
				SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();
					
				if (fManager)
					mobileAssemblyComponent.SetParentFactionID(fManager.GetFactionIndex(m_Base.GetOwningFaction()));
					
				hintID = SCR_ECampaignHints.MOBILE_ASSEMBLY;
				networkComp.SendVehicleSpawnHint(hintID);
			}
		}
		
		networkComp.SetLastRequestTimestamp(Replication.Time());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns false if player spawned too recently
	protected bool CooldownCheck(notnull IEntity user)
	{
		int userId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(userId));
		if (!playerController)
			return false;
		
		SCR_CampaignNetworkComponent networkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!networkComponent)
			return false;
		
		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.GetInstance();
		if (!factionManager)
			return false;
		
		ECharacterRank rank = SCR_CharacterRankComponent.GetCharacterRank(user);
		float timeout = networkComponent.GetLastRequestTimestamp() + (float)factionManager.GetRankRequestCooldown(rank);
		
		if (timeout < Replication.Time())
			return true;
		
		return false;
	} 
	
	//------------------------------------------------------------------------------------------------
	//! Override of original CanSpawn to determine whether vehicle can be spawned
	override int CanSpawn(int entityIndex = -1, IEntity user = null)
	{
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(user);
		if (!player)
			return SCR_EntityRequestDeniedReason.NOT_AVAILABLE;
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(player.GetFaction());
		if (!faction)
			return SCR_EntityRequestDeniedReason.NOT_AVAILABLE;
		
		if (m_Base.GetOwningFaction() != faction)
			return SCR_EntityRequestDeniedReason.NOT_AVAILABLE; 
		
		int status = super.CanSpawn(entityIndex, user);
		if (status == SCR_EntityRequestDeniedReason.CAN_SPAWN || status == SCR_EntityRequestDeniedReason.CAN_SPAWN_TRIGGER)
		{
			if (!RankCheck(entityIndex, user))
				status = SCR_EntityRequestDeniedReason.RANK_LOW;
		}

		if (!CooldownCheck(user))
			status = SCR_EntityRequestDeniedReason.COOLDOWN;
		
		return status;
	}
	//------------------------------------------------------------------------------------------------
	//! Override of original GetSpawnerSupplies to use Campaign Supply Component supplies of assigned base
	override float GetSpawnerSupplies()
	{
		if (!m_Base)
			return 0;
		
		return m_Base.GetSupplies();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Override of original GetSpawnerSupplies to use Campaign Supply Component supplies of assigned base
	override void AddSpawnerSupplies(float newValue)
	{
		if (m_Base)
			m_Base.AddSupplies(newValue);
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeAssetList(SCR_EntityAssetList assetList)
	{
		if (!assetList)
			return;
		
		m_EntityAssetList = assetList;
	}
	
	//------------------------------------------------------------------------------------------------
	bool RankCheck(int index, IEntity user)
	{
		SCR_EntityInfo spawnInfo = GetEntryAtIndex(index);
		if (!spawnInfo)
			return false;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (!campaign)
			return false;
		
		if (campaign.CanRequestWithoutRank())
			return true;
		
		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.GetInstance();
		if (!factionManager)
			return false;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		IEntity playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return false;
		
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!campaignNetworkComponent)
			return false;
		
		ECharacterRank rank;
		
		if (IsProxy())
			rank = SCR_CharacterRankComponent.GetCharacterRank(user);
		else
			rank = factionManager.GetRankByXP(campaignNetworkComponent.GetPlayerXP());

		//Check if the player has high enough rank
		if (spawnInfo.GetMinimumRankID() > rank)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Bump supply truck lifetime in garbage manager if it's parked near a base
	protected void OnSupplyTruckLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		SCR_CampaignGarbageManager gManager = SCR_CampaignGarbageManager.Cast(GetGame().GetGarbageManager());
		
		if (!gManager)
			return;
		
		if (!gManager.IsInserted(vehicle))
			return;
		
		array<SCR_CampaignBase> bases = SCR_CampaignBaseManager.GetBases();
		int baseDistanceSq = Math.Pow(SCR_CampaignGarbageManager.MAX_BASE_DISTANCE, 2);
		vector vehPos = vehicle.GetOrigin();
		bool baseNearby;
		
		foreach (SCR_CampaignBase base: bases)
		{
			if (!base)
				continue;
			
			baseNearby = vector.DistanceSqXZ(vehPos, base.GetOrigin()) <= baseDistanceSq;
			
			if (baseNearby)
				break;
		}
		
		if (!baseNearby)
			return;
		
		float curLifetime = gManager.GetRemainingLifetime(vehicle);
		
		if (curLifetime >= SCR_CampaignGarbageManager.PARKED_SUPPLY_TRUCK_LIFETIME)
			return;
		
		gManager.Bump(vehicle, SCR_CampaignGarbageManager.PARKED_SUPPLY_TRUCK_LIFETIME - curLifetime);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool BaseSearchCB(IEntity ent)
	{
		SCR_CampaignBase base = SCR_CampaignBase.Cast(ent);
		if (base && base.GetType() != CampaignBaseType.RELAY)
		{
			m_Base = base;
			m_Base.RegisterVehicleSpawner(this);
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		// returns if query was finished without finding base
		if (GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), m_fBaseSearchDistance, BaseSearchCB, null, EQueryEntitiesFlags.ALL))
			return;
		
	}
}

