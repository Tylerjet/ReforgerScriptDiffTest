#include "scripts/Game/config.c"
//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialComponentClass : SCR_BaseGameModeComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialComponent : SCR_BaseGameModeComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "", "conf")]
	ResourceName m_sStagesConfig;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECampaignTutorialStage))]
	protected SCR_ECampaignTutorialStage m_eStartingStage;
	
	protected static const int TARGETS_SEARCH_DISTANCE = 150;
	protected static const int INVALID_STANCE = -1;
	protected static const vector BACK_DOOR_OFFSET = {0,-0.7,0};
	protected static float WAYPOINT_FADE_THRESHOLD = 20;
	protected static float WAYPOINT_MINIMUM_OPACITY = 0.2;
	protected static float WAYPOINT_DISTANCE_INDICATOR_FADE_START = 100;
	protected static float WAYPOINT_DISTANCE_INDICATOR_FADE_END = 50;
	
	protected ref array<ref SCR_CampaignTutorialStageInfo> m_aStageInfos = {};
	protected SCR_BaseCampaignTutorialStage m_Stage;
	protected ImageWidget m_wFadeOut;
	protected SCR_GameModeCampaign m_CampaignGamemode;
	protected bool m_bPlayerSpawned;
	protected SCR_ECampaignTutorialStage m_eStage = SCR_ECampaignTutorialStage.START;
	protected SCR_ECampaignTutorialStage m_eLastStage;
	protected ChimeraCharacter m_Player;
	protected int m_iStagesCnt;
	protected ImageWidget m_wWaypoint;
	protected RichTextWidget m_wWaypointDistance;
	protected bool m_bFirstStageSet;
	protected ref array<SCR_FiringRangeTarget> m_aFiringRangeTargets = {};
	protected SCR_CampaignSuppliesComponent m_SupplyTruckComponent;
	protected int m_iCountOfHits;
	protected SCR_CampaignMilitaryBaseComponent m_HQUS;
	protected BaseRadioComponent m_Radio;
	protected bool m_bIsMapOpen = false;
	protected bool m_bUsed3PV = false;
	protected ECharacterStance m_ePlayerStance = INVALID_STANCE;
	protected bool m_bCheckLeaning = false;
	protected SCR_CampaignMobileAssemblyComponent m_MobileAssemblyComponent;
	protected bool m_bMovedOutOfVehicleByScript = false;
	protected bool m_fDelayedControlSchemeChangeRunning = false;
	protected bool m_bIsUsingGamepad;
	protected SCR_ETutorialSupplyTruckWaypointMode m_eWaypointTruckPosition = SCR_ETutorialSupplyTruckWaypointMode.NONE;
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fStageTimestamp;
	#else
	protected WorldTimestamp m_fStageTimestamp;
	#endif
	protected static ref ScriptInvoker m_OnStructureBuilt = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnStructureBuilt()
	{
		return m_OnStructureBuilt;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		super.OnPlayerKilled(playerId, player, killer);
		
		delete(m_Stage);
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		if (!playerController)
			return;
		
		array<SCR_SpawnPoint> availableSpawnpoints = SCR_SpawnPoint.GetSpawnPointsForFaction(m_CampaignGamemode.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR));
		
		if (availableSpawnpoints.Count() < 2)
			return;
		
		TrySpawnPlayer(playerController, availableSpawnpoints[1]);
		//GetGame().GetCallqueue().CallLater(TryPlayerSpawn, 25, true, playerController);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		if (!SCR_HintManagerComponent.GetInstance().IsShown())
			return;
		
		m_bIsUsingGamepad = isGamepad;
		
		if (m_fDelayedControlSchemeChangeRunning)
			return;
		
		m_fDelayedControlSchemeChangeRunning = true;
		GetGame().GetCallqueue().CallLater(OnInputDeviceChangedDelayed, 250);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInputDeviceChangedDelayed()
	{
		m_fDelayedControlSchemeChangeRunning = false;
		bool switchedToKeyboard = !m_bIsUsingGamepad;
		m_Stage.OnInputDeviceChanged(switchedToKeyboard);
	}
	
	//------------------------------------------------------------------------------------------------
	ChimeraCharacter GetPlayer()
	{
		return m_Player;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnWorldPostProcess(World world)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void FiringRangeInit()
	{
		typename stages = SCR_ECampaignTutorialStage;
		IEntity shootingRangePos = GetGame().GetWorld().FindEntityByName(string.Format("PP_%1", stages.GetVariableName(SCR_ECampaignTutorialStage.WEAPON_PICK)));
		IEntity baseLaruns = GetGame().GetWorld().FindEntityByName("TownBaseLaruns");
		
		if (shootingRangePos)
			GetGame().GetWorld().QueryEntitiesBySphere(shootingRangePos.GetOrigin(), TARGETS_SEARCH_DISTANCE, CollectAllTargets, null, EQueryEntitiesFlags.ALL);
		
		if (baseLaruns)
			GetGame().GetWorld().QueryEntitiesBySphere(baseLaruns.GetOrigin(), 50, CollectAllTargets, null, EQueryEntitiesFlags.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	void MobileHQInit()
	{
		IEntity mHQ = GetGame().GetWorld().FindEntityByName("MobileHQ");
		
		if (!mHQ)
			return;
		
		SCR_CampaignFaction f = m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		
		if (!f)
			return;
		
		BaseRadioComponent radioComponent = BaseRadioComponent.Cast(mHQ.FindComponent(BaseRadioComponent));
		
		if (radioComponent && radioComponent.TransceiversCount() > 0)
		{
			radioComponent.SetPower(false);
			radioComponent.GetTransceiver(0).SetFrequency(f.GetFactionRadioFrequency());
			radioComponent.SetEncryptionKey(f.GetFactionRadioEncryptionKey());
		}
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(mHQ.FindComponent(SlotManagerComponent));
		
		if (!slotManager)
			return;
		
		array<EntitySlotInfo> slots = new array<EntitySlotInfo>;
		slotManager.GetSlotInfos(slots);
		
		foreach (EntitySlotInfo slot: slots)
		{
			if (!slot)
				continue;
			
			IEntity truckBed = slot.GetAttachedEntity();
			
			if (!truckBed)
				continue;
			
			m_MobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));
			
			if (m_MobileAssemblyComponent)
			{
				m_MobileAssemblyComponent.SetParentFactionID(GetGame().GetFactionManager().GetFactionIndex(f));
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SupplyTruckInit()
	{
		IEntity supplyTruck = GetGame().GetWorld().FindEntityByName("SupplyTruck");
		
		if (!supplyTruck)
			return;
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(supplyTruck.FindComponent(SlotManagerComponent));
		
		if (!slotManager)
			return;
		
		array<EntitySlotInfo> slots = new array<EntitySlotInfo>;
		slotManager.GetSlotInfos(slots);
		
		foreach (EntitySlotInfo slot: slots)
		{
			if (!slot)
				continue;
			
			IEntity truckBed = slot.GetAttachedEntity();
			
			if (!truckBed)
				continue;
			
			SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(truckBed.FindComponent(SCR_CampaignSuppliesComponent));
			
			if (suppliesComponent)
			{
				m_SupplyTruckComponent = suppliesComponent;
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_RifleRespawn()
	{
		SpawnAsset("M16", "{3E413771E1834D2F}Prefabs/Weapons/Rifles/M16/Rifle_M16A2.et");
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_MoveInJeep(IEntity position = null)
	{
		SpawnAsset("Jeep", "{5168FEA3054D6D15}Prefabs/Vehicles/Wheeled/M151A2/M151A2_M2HB_MERDC.et", position);
		MoveInVehicle("Jeep", ECompartmentType.Pilot);
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_RegisterJeepSkip()
	{
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
		if (accessComp)
		{
			accessComp.GetOnCompartmentLeft().Remove(OnJeepLeft);
			accessComp.GetOnCompartmentLeft().Insert(OnJeepLeft);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_SpawnTruck(IEntity position = null)
	{
		SpawnAsset("SupplyTruck", "{F37113A988304565}Prefabs/MP/Campaign/Assets/CampaignSupplyTruckWest.et", position);
		SupplyTruckInit();
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_ProcessTruck()
	{
		m_SupplyTruckComponent.AddSupplies(1000);
		MoveInVehicle("SupplyTruck", ECompartmentType.Pilot);
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
		
		if (accessComp)
		{
			accessComp.GetOnCompartmentLeft().Remove(OnSupplyTruckLeft);
			accessComp.GetOnCompartmentLeft().Insert(OnSupplyTruckLeft);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_PrepareChotain()
	{
		m_SupplyTruckComponent.AddSupplies(500);
		SCR_CampaignMilitaryBaseComponent baseChotain = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain").FindComponent(SCR_CampaignMilitaryBaseComponent));
		
		if (baseChotain && baseChotain.GetSupplies() < 950)
			baseChotain.AddSupplies(950 - baseChotain.GetSupplies());
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_CaptureLaruns()
	{
		SCR_CampaignMilitaryBaseComponent baseLaruns = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseLaruns").FindComponent(SCR_CampaignMilitaryBaseComponent));
				
		if (baseLaruns && baseLaruns.GetFaction() != m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
			baseLaruns.SetFaction(m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_HandleRespawnRadios()
	{
		while (m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetActiveRespawnRadios() > 0)
			m_CampaignGamemode.RemoveActiveRespawnRadio(m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_DeployMHQ()
	{
		if (m_MobileAssemblyComponent && !m_MobileAssemblyComponent.IsDeployed())
		{
			m_MobileAssemblyComponent.UpdateRadioCoverage();
			m_MobileAssemblyComponent.Deploy(SCR_EMobileAssemblyStatus.DEPLOYED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetStage(bool forcePlayerReset = false)
	{
		if (!m_Player.IsInVehicle() || forcePlayerReset)
			ResetPlayerPosition();
		
		if (m_Stage)
			m_Stage.Reset();
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity SpawnAsset(string name, ResourceName type, IEntity spawnpoint = null)
	{
		string posName;
		
		if (!spawnpoint)
			spawnpoint = GetGame().GetWorld().FindEntityByName(string.Format("SpawnPos_%1", name));
		
		if (!spawnpoint)
			return null;
		
		IEntity oldEntity = GetGame().GetWorld().FindEntityByName(name);
		Vehicle vehOld = Vehicle.Cast(oldEntity);
		
		if (vehOld)
		{
			SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
			if (compartmentAccessComponent)
			{
				m_bMovedOutOfVehicleByScript = true;
				compartmentAccessComponent.EjectOutOfVehicle();
			}
			
			vector pos = vehOld.GetOrigin();
			pos[1] = pos[1] + 5;
			
			vehOld.SetName("");
			vehOld.SetOrigin(pos);
			GetGame().GetCallqueue().CallLater(DeleteVehicleWhenEmpty, 10, true, vehOld);
		}
		else if (oldEntity)
			delete(oldEntity);
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		spawnpoint.GetWorldTransform(params.Transform);
		Resource res = Resource.Load(type);
		
		if (!res)
			return null;
		
		IEntity newEntity = GetGame().SpawnEntityPrefab(res, null, params);
		newEntity.SetName(name);
		
		Vehicle veh = Vehicle.Cast(newEntity);
		
		if (veh)
		{
			vector pos = veh.GetOrigin();
			pos[1] = pos[1] + 1;
			
			Physics physicsComponent = veh.GetPhysics();
			
			if (physicsComponent)
			{
				veh.SetOrigin(pos);
				physicsComponent.SetVelocity("0 -1 0");
			}
		}
		
		return newEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	void DeleteVehicleWhenEmpty(Vehicle veh)
	{
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
		if (!compartmentAccessComponent)
		{
			delete veh;
			GetGame().GetCallqueue().Remove(DeleteVehicleWhenEmpty);
		}
		
		if (compartmentAccessComponent.GetVehicle() != veh)
		{
			delete veh;
			GetGame().GetCallqueue().Remove(DeleteVehicleWhenEmpty);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CollectAllTargets(IEntity ent)
	{
		SCR_FiringRangeTarget target = SCR_FiringRangeTarget.Cast(ent);
		if (!target)
			return true;
		
		target.SetState(ETargetState.TARGET_DOWN);
		m_aFiringRangeTargets.Insert(target);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_FiringRangeTarget> GetAllTargets()
	{
		return m_aFiringRangeTargets;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTargetHits()
	{
		return m_iCountOfHits;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCheckLeaning(bool check)
	{
		m_bCheckLeaning = check;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerStanceToCheck(ECharacterStance stance)
	{
		m_ePlayerStance = stance;
	}
	
	//------------------------------------------------------------------------------------------------
	ECharacterStance GetPlayerStanceToCheck()
	{
		return m_ePlayerStance;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CountTargetHit(ETargetState state, SCR_FiringRangeTarget target)
	{
		if (state != ETargetState.TARGET_DOWN)
			return; 
		
		CharacterControllerComponent comp = CharacterControllerComponent.Cast(m_Player.FindComponent(CharacterControllerComponent));
		if (!comp)
			return;
		
		if (GetPlayerStanceToCheck() != INVALID_STANCE && comp.GetStance() != GetPlayerStanceToCheck())
			return;
		
		// Leaning doesn't matter or is wrong.
		if (m_bCheckLeaning && !IsCharacterLeaning())
			return;
		
		target.SetAutoResetTarget(false);
		target.Event_TargetChangeState.Remove(CountTargetHit);
		m_iCountOfHits ++;	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWaypointTruckPosition(SCR_ETutorialSupplyTruckWaypointMode mode)
	{
		m_eWaypointTruckPosition = mode;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);
		
		BaseWorld world = GetGame().GetWorld();
		SCR_CampaignMilitaryBaseComponent.Cast(world.FindEntityByName("MainBaseChotain").FindComponent(SCR_CampaignMilitaryBaseComponent)).SetCallsignIndex(0);
		SCR_CampaignMilitaryBaseComponent.Cast(world.FindEntityByName("TownBaseChotain").FindComponent(SCR_CampaignMilitaryBaseComponent)).SetCallsignIndex(1);
		SCR_CampaignMilitaryBaseComponent.Cast(world.FindEntityByName("TownBaseLaruns").FindComponent(SCR_CampaignMilitaryBaseComponent)).SetCallsignIndex(2);
		SCR_CampaignMilitaryBaseComponent.Cast(world.FindEntityByName("MajorBaseLevie").FindComponent(SCR_CampaignMilitaryBaseComponent)).SetCallsignIndex(4);
		
		// Attempt to spawn the player automatically, cease after spawn is successful in OnPlayerSpawned
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		TryPlayerSpawn(playerController);
		
		//if (!m_bPlayerSpawned)
		//	GetGame().GetCallqueue().CallLater(TryPlayerSpawn, 100, true, playerController);
	}
	
	//------------------------------------------------------------------------------------------------
	void TryPlayerSpawn(notnull PlayerController pc)
	{
		if (!m_CampaignGamemode)
			return;
		
		int playerId = pc.GetPlayerId();
		
		// Skip faction and spawnpoint selection
		SCR_RespawnSystemComponent respawnSystem = GetGameMode().GetRespawnSystemComponent();
		
		if (!respawnSystem)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		
		if (!factionManager)
			return;

		Faction factionUS = m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		
		if (!factionUS)
			return;
		
		SCR_BasePlayerLoadout loadout = GetGame().GetLoadoutManager().GetRandomFactionLoadout(factionUS);
		if (!loadout)
			return;
		
		array<SCR_SpawnPoint> availableSpawnpoints = SCR_SpawnPoint.GetSpawnPointsForFaction(m_CampaignGamemode.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR));
		
		if (availableSpawnpoints.Count() < 2)
			return;
		
		if (!pc)
			return;

		SCR_PlayerFactionAffiliationComponent playerFactionAffiliation = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		SCR_PlayerLoadoutComponent playerLoadout = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
		SCR_RespawnComponent respawnComp = SCR_RespawnComponent.Cast(pc.FindComponent(SCR_RespawnComponent));

		
		playerFactionAffiliation.GetOnPlayerFactionResponseInvoker_S().Insert(OnPlayerFactionResponse);
		playerLoadout.GetOnPlayerLoadoutResponseInvoker_S().Insert(OnPlayerLoadoutResponse);
		respawnComp.GetOnRespawnResponseInvoker_S().Insert(OnPlayerSpawnResponse);
		
		TrySetPlayerFaction(pc, factionUS);
		TrySetPlayerLoadout(pc, loadout);
		TrySpawnPlayer(pc, availableSpawnpoints[1]); // Select the second spawnpoint in array to spawn at the correct place.		
	}

	//------------------------------------------------------------------------------------------------	
	protected void OnPlayerFactionResponse(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		PlayerController controller = PlayerController.Cast(component.GetOwner());
		FactionManager factionManager = GetGame().GetFactionManager();
		
		if (!factionManager)
			return;

		Faction factionUS = factionManager.GetFactionByIndex(factionIndex);
		
		if (!factionUS)
			return;
		
		if(!response)
			TrySetPlayerFaction(controller, factionUS);
		else
			component.GetOnPlayerFactionResponseInvoker_S().Remove(OnPlayerFactionResponse);
	}

	//------------------------------------------------------------------------------------------------		
	protected void OnPlayerLoadoutResponse(SCR_PlayerLoadoutComponent component, int loadoutIndex, bool response)
	{
		PlayerController controller = PlayerController.Cast(component.GetOwner());
		SCR_BasePlayerLoadout loadout = GetGame().GetLoadoutManager().GetLoadoutByIndex(loadoutIndex);
		
		if(!response)
			TrySetPlayerLoadout(controller, loadout);
		else
			component.GetOnPlayerLoadoutResponseInvoker_S().Remove(OnPlayerLoadoutResponse);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawnResponse(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response)
	{
		PlayerController controller = PlayerController.Cast(requestComponent.GetOwner());
		array<SCR_SpawnPoint> availableSpawnpoints = SCR_SpawnPoint.GetSpawnPointsForFaction(m_CampaignGamemode.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR));
		SCR_RespawnComponent respawnComp = SCR_RespawnComponent.Cast(controller.FindComponent(SCR_RespawnComponent));
		
		if(response != SCR_ESpawnResult.OK)
			TrySpawnPlayer(controller, availableSpawnpoints[1]);
		else
			respawnComp.GetOnRespawnResponseInvoker_S().Remove(OnPlayerSpawnResponse);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void TrySetPlayerFaction(notnull PlayerController playerController, Faction faction)
	{
		// TODO@YURI:
		//	Make sure that a callback is hooked to SCR_PlayerFactionAffiliationComponent.GetOnPlayerFactionResponseInvoker_S() for authority callback
		//	or SCR_PlayerFactionAffiliationComponent.GetOnPlayerFactionResponseInvoker_O() for the owner client (the target/requesting player)
		//	prior to calling this method. In case the request fails, you will receive a reponse and you can handle it accordingly!
		//	no need for endless call later!
		
		SCR_PlayerFactionAffiliationComponent playerFactionAffiliation = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!playerFactionAffiliation.RequestFaction(faction))
		{
			// Request not sent, failed locally
			return;
		}
		
		// Request sent, await response.
		// Response delivered via SCR_PlayerFactionAffiliationComponent.GetOnCanPlayerFactionResponseInvoker..., explained above.
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TrySetPlayerLoadout(notnull PlayerController playerController, SCR_BasePlayerLoadout loadout)
	{
		// TODO@YURI:
		//	Make sure that a callback is hooked to SCR_PlayerLoadoutComponent.GetOnPlayerLoadoutResponseInvoker_S() for authority callback
		//	or SCR_PlayerLoadoutComponent.GetOnPlayerLoadoutResponseInvoker_O() for the owner client (the target/requesting player)
		//	prior to calling this method. In case the request fails, you will receive a reponse and you can handle it accordingly!
		//	no need for endless call later!
		
		SCR_PlayerLoadoutComponent playerLoadoutComponent = SCR_PlayerLoadoutComponent.Cast(playerController.FindComponent(SCR_PlayerLoadoutComponent));
		if (!playerLoadoutComponent.RequestLoadout(loadout))
		{
			// Request not sent, failed locally
			return;
		}
		
		// Request sent, await response.
		// Response delivered via SCR_PlayerLoadoutComponent.GetOnPlayerLoadoutResponseInvoker...(), explained above.
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TrySpawnPlayer(notnull PlayerController playerController, notnull SCR_SpawnPoint spawnPoint)
	{
		// TODO@YURI:
		//	Make sure that a callback is hooked to SCR_RespawnComponent.GetOnRespawnResponseInvoker_S() for authority callback
		//	or SCR_RespawnComponent.GetOnRespawnResponseInvoker_O() for the owner client (the target/requesting player)
		//	prior to calling this method. In case the request fails, you will receive a reponse and you can handle it accordingly!
		//	no need for endless call later!
		
		SCR_RespawnComponent spawnComponent = SCR_RespawnComponent.Cast(playerController.FindComponent(SCR_RespawnComponent));
		SCR_PlayerLoadoutComponent loadoutComponent = SCR_PlayerLoadoutComponent.Cast(playerController.FindComponent(SCR_PlayerLoadoutComponent));
		
		// Retrieve loadout the user had stored prior to this request; this is synchronized as an ask/response
		SCR_BasePlayerLoadout loadout = loadoutComponent.GetAssignedLoadout();
		ResourceName loadoutResource = loadout.GetLoadoutResource();
		SCR_SpawnData spawnData = new SCR_SpawnPointSpawnData(loadoutResource, spawnPoint.GetRplId());		
		if (!spawnComponent.RequestSpawn(spawnData))
		{
			// Request not sent, failed locally
			return;
		}
		
		// Request sent, await response.
		// Response delivered via SCR_RespawnComponent.GetOnRespawnResponseInvoker...(), explained above.
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		m_bIsMapOpen = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapClose(MapConfiguration config)
	{
		m_bIsMapOpen = false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsMapOpen()
	{
		return m_bIsMapOpen;
	}
	
	//------------------------------------------------------------------------------------------------
	BaseRadioComponent GetPlayerRadio()
	{
		return m_Radio;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateSupplyTruckWaypoint(IEntity waypoint, bool useTruck, string contextName, vector offset = vector.Zero)
	{
		if (contextName.IsEmpty() || !m_SupplyTruckComponent)
			return;
		
		IEntity ent = m_SupplyTruckComponent.GetOwner();
		
		if (useTruck)
			ent = ent.GetParent();
		
		ActionsManagerComponent amc = ActionsManagerComponent.Cast(ent.FindComponent(ActionsManagerComponent));
			
		if (!amc)
			return;
			
		UserActionContext context = amc.GetContext(contextName);
				
		if (!context)
			return;
				
		vector pos = context.GetOrigin();
		pos = pos + offset;
		waypoint.SetOrigin(pos);
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckRadioPickup(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		m_Radio = BaseRadioComponent.Cast(item.FindComponent(BaseRadioComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	bool CheckCharacterStance(ECharacterCommandIDs command)
	{
		CharacterAnimationComponent comp = CharacterAnimationComponent.Cast(m_Player.FindComponent(CharacterAnimationComponent));
		bool ret = false;
					
		if (!comp)
			return false;
		
		CharacterMovementState mState = new CharacterMovementState;
		comp.GetMovementState(mState);
		
		if (mState)
			ret = mState.m_CommandTypeId == command;
	
		delete mState;
		return ret;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsCharacterLeaning()
	{
		SCR_CharacterControllerComponent comp = SCR_CharacterControllerComponent.Cast(m_Player.FindComponent(SCR_CharacterControllerComponent));
		if (!comp)
			return false;
		
		return comp.IsLeaning();
	}
	
	//------------------------------------------------------------------------------------------------
	void Check3rdPersonViewUsed()
	{
		if (!m_Player)
			return;
		
		CameraHandlerComponent comp = CameraHandlerComponent.Cast(m_Player.FindComponent(CameraHandlerComponent));
		
		if (!comp)
			return;
		
		if (comp.IsInThirdPerson())
		{
			m_bUsed3PV = true;
			GetGame().GetCallqueue().Remove(Check3rdPersonViewUsed);
			
			if (m_eStage == SCR_ECampaignTutorialStage.WALL)
				GetGame().GetCallqueue().Remove(DelayedPopup);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetWas3rdPersonViewUsed()
	{
		return m_bUsed3PV;
	}
	
	//------------------------------------------------------------------------------------------------
	void MoveInVehicle(string vehName, ECompartmentType seat)
	{
		Vehicle veh = Vehicle.Cast(GetGame().GetWorld().FindEntityByName(vehName));
		
		if (!veh)
			return;
		
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
		if (!compartmentAccessComponent)
			return;
		
		if (compartmentAccessComponent.GetVehicle())
			GetGame().GetCallqueue().CallLater(MoveInVehicle, 100, false, vehName, seat);
		else
		{
			if (m_wFadeOut)
			{
				m_wFadeOut.SetOpacity(1);
				AnimateWidget.Opacity(m_wFadeOut, 0, 1.5);
			}
			
			m_bMovedOutOfVehicleByScript = false;
			compartmentAccessComponent.MoveInVehicle(veh, seat);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool CheckCharacterInVehicle(ECompartmentType seat)
	{
		CompartmentAccessComponent compartmentAccessComponent = CompartmentAccessComponent.Cast(m_Player.FindComponent(CompartmentAccessComponent));
				
		if (!compartmentAccessComponent)
			return false;
	
		BaseCompartmentSlot compartmentSlot = compartmentAccessComponent.GetCompartment();
	
		if (!compartmentSlot)
			return false;
		
		return SCR_CompartmentAccessComponent.GetCompartmentType(compartmentSlot) == seat;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSupplyTruckLeft(IEntity vehicle)
	{
		if (m_bMovedOutOfVehicleByScript)
			return;
		
		if (!m_Player)
			return;
		
		CharacterControllerComponent charController = CharacterControllerComponent.Cast(m_Player.FindComponent(CharacterControllerComponent));
				
		if (!charController)
			return;
				
		if (charController.IsDead())
			return;
		
		SCR_PopUpNotification.GetInstance().HideCurrentMsg();
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
		if (accessComp)
			accessComp.GetOnCompartmentLeft().Remove(OnSupplyTruckLeft);
		
		FinishStage(m_Stage, SCR_ECampaignTutorialStage.CONFLICT_UNLOADING_SUPPLIES);
		ResetStage(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnJeepLeft(IEntity vehicle)
	{
		if (m_bMovedOutOfVehicleByScript)
			return;
		
		if (!m_Player)
			return;
		
		CharacterControllerComponent charController = CharacterControllerComponent.Cast(m_Player.FindComponent(CharacterControllerComponent));
				
		if (!charController)
			return;
				
		if (charController.IsDead())
			return;
		
		SCR_PopUpNotification.GetInstance().HideCurrentMsg();
		GetGame().GetCallqueue().Remove(SCR_HintManagerComponent.ShowCustomHint);
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
		if (accessComp)
			accessComp.GetOnCompartmentLeft().Remove(OnJeepLeft);
		
		FinishStage(m_Stage, SCR_ECampaignTutorialStage.CONFLICT_TOUR_ARMORY);
		ResetPlayerPosition();
	}
	
	//------------------------------------------------------------------------------------------------
	void MovePlayer(notnull IEntity newPos)
	{
		if (m_Player.IsInVehicle())
		{
			GetGame().GetCallqueue().CallLater(MovePlayer, 250, false, newPos);
			return;
		}
		
		if (m_wFadeOut)
		{
			m_wFadeOut.SetOpacity(1);
			AnimateWidget.Opacity(m_wFadeOut, 0, 1.5);
		}
		
		vector mat[4];
		newPos.GetTransform(mat);
		m_Player.Teleport(mat);
	}
	
	//------------------------------------------------------------------------------------------------
	void DelayedPopup(string text = "", string subtitle = "", float duration = SCR_PopUpNotification.DEFAULT_DURATION, string param1 = "", string param2 = "", string subtitleParam1 = "", string subtitleParam2 = "")
	{
		SCR_PopUpNotification.GetInstance().PopupMsg(text, duration, text2: subtitle, param1: param1, param2: param2, text2param1: subtitleParam1, text2param2: subtitleParam2, category: SCR_EPopupMsgFilter.TUTORIAL);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnStructureBuilt(SCR_CampaignMilitaryBaseComponent base, IEntity structure)
	{
		if (m_Stage)
			m_Stage.OnStructureBuilt(base, structure);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		m_bPlayerSpawned = true;
		m_Player = ChimeraCharacter.Cast(entity);
		GetGame().GetCallqueue().Remove(TryPlayerSpawn);
		
		if (m_Stage)
			m_Stage.SetPlayer(m_Player);
		
		if (!m_bFirstStageSet)
		{
			if (m_CampaignGamemode.IsTutorial())
			{
				SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
				
				if (hudManager)
				{
					SCR_XPInfoDisplay display = SCR_XPInfoDisplay.Cast(hudManager.FindInfoDisplay(SCR_XPInfoDisplay));
					
					if (display)
						display.AllowShowingInfo(false);
				}
			}
			
			m_bFirstStageSet = true;
			m_HQUS = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("MainBaseChotain").FindComponent(SCR_CampaignMilitaryBaseComponent));
			m_HQUS.AlterSupplyIncomeTimer(9999999);
			FiringRangeInit();
			MobileHQInit();
			IEntity WP_cottage = GetGame().GetWorld().FindEntityByName("WP_CONFLICT_COMPAS_MOVE");
			IEntity WP_mobileHQ = GetGame().GetWorld().FindEntityByName("WP_CONFLICT_MAP_MOVE");
			SCR_MapDescriptorComponent cottageDescr = SCR_MapDescriptorComponent.Cast(WP_cottage.FindComponent(SCR_MapDescriptorComponent));
			SCR_MapDescriptorComponent mobileTruckDescr = SCR_MapDescriptorComponent.Cast(WP_mobileHQ.FindComponent(SCR_MapDescriptorComponent));
			cottageDescr.Item().SetVisible(false);
			mobileTruckDescr.Item().SetVisible(false);
			
			while (m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetActiveRespawnRadios() < m_CampaignGamemode.GetMaxRespawnRadios())
				m_CampaignGamemode.AddActiveRespawnRadio(m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
			
			m_wFadeOut = ImageWidget.Cast(GetGame().GetHUDManager().CreateLayout("{265245C299401BF6}UI/layouts/Menus/ContentBrowser/DownloadManager/ScrollBackground.layout", EHudLayers.OVERLAY));
			m_wFadeOut.SetOpacity(0);
			int stage = m_eStage;
#ifdef WORKBENCH
			stage = m_eStartingStage;
#endif
			SetStage(stage);
			GetGame().GetCallqueue().CallLater(Check3rdPersonViewUsed, 500, true);
			
#ifdef ENABLE_DIAG
			SetEventMask(m_CampaignGamemode, EntityEvent.FRAME);
#endif
		}
		else
		{
			SetStage(m_eStage);
		}
		
		ResetStage();
		
		// Load waypoint UI upon first spawn
		if (!m_wWaypoint)
		{	
			Widget waypointFrame = GetGame().GetHUDManager().CreateLayout("{4FEF75768BFDA310}UI/layouts/Tutorial/TutorialWaypoint.layout", EHudLayers.BACKGROUND);
			m_wWaypoint = ImageWidget.Cast(waypointFrame.FindAnyWidget("Icon"));
			m_wWaypointDistance = RichTextWidget.Cast(waypointFrame.FindAnyWidget("Distance"));
			
			if (m_wWaypoint)
			{
				m_wWaypoint.SetOpacity(0);
				m_wWaypoint.LoadImageFromSet(0, "{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset", "USSR_Base_Small_Select");
				m_wWaypoint.SetColor(Color.Yellow);
				FrameSlot.SetSize(m_wWaypoint, 64, 64);
			}
			
			if (m_wWaypointDistance)
			{
				m_wWaypointDistance.SetOpacity(0);
				m_wWaypointDistance.SetColor(Color.Yellow);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetResumeStage(SCR_ECampaignTutorialStage stage)
	{
		IEntity playerPos;
		typename stages = SCR_ECampaignTutorialStage;
		
		while (!playerPos && stage > 0)
		{
			playerPos = GetGame().GetWorld().FindEntityByName(string.Format("PP_%1", stages.GetVariableName(stage)));
			
			if (playerPos)
			{
				m_eStage = stage;
				return;
			}
			
			stage--;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ECampaignTutorialStage GetStage()
	{
		return m_eStage;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetStageDuration()
	{
		#ifndef AR_CAMPAIGN_TIMESTAMP
		return Replication.Time() - m_fStageTimestamp;
		#else
		ChimeraWorld world = GetOwner().GetWorld();
		return world.GetServerTimestamp().DiffMilliseconds(m_fStageTimestamp);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void FinishStage(SCR_BaseCampaignTutorialStage stage, SCR_ECampaignTutorialStage nextStage = -1)
	{
		if (m_wWaypoint)
		{
			m_wWaypoint.SetOpacity(0);
			m_wWaypointDistance.SetOpacity(0);
		}
		
		SetPlayerStanceToCheck(-1);
		m_iCountOfHits = 0;
		m_bCheckLeaning = false;
		m_eWaypointTruckPosition = SCR_ETutorialSupplyTruckWaypointMode.NONE;
		
		delete stage;
		
		if (nextStage == -1)
			SetStage(m_eStage + 1);
		else
			SetStage(nextStage);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetStage(SCR_ECampaignTutorialStage stage)
	{
		m_eStage = stage;
		
#ifdef ENABLE_DIAG
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, m_eStage);
#endif
		
		// Tutorial complete
		if (m_eStage >= m_iStagesCnt)
		{
			ClearEventMask(m_CampaignGamemode, EntityEvent.FRAME);
			
			if (m_wWaypoint)
			{
				m_wWaypoint.SetOpacity(0);
				m_wWaypointDistance.SetOpacity(0);
			}
			
			return;
		}
		
		if (!m_Player)
			return;
		
		// Run the next stage
		foreach (SCR_CampaignTutorialStageInfo stageInfo : m_aStageInfos)
		{
			if (stageInfo.GetIndex() == m_eStage)
			{
				#ifndef AR_CAMPAIGN_TIMESTAMP
				m_fStageTimestamp = Replication.Time();
				#else
				ChimeraWorld world = GetOwner().GetWorld();
				m_fStageTimestamp = world.GetServerTimestamp();
				#endif
				m_Stage = SCR_BaseCampaignTutorialStage.Cast(GetGame().SpawnEntity(stageInfo.GetClassName().ToType()));
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetPlayerPosition()
	{
		IEntity playerPos;
		typename stages = SCR_ECampaignTutorialStage;
		int stage = m_eStage;
		
		// If the current stage does not have its starting location set, use the previous one
		while (!playerPos && stage > 0)
		{
			playerPos = GetGame().GetWorld().FindEntityByName(string.Format("PP_%1", stages.GetVariableName(stage)));
			stage--;
		}
		
		if (playerPos)
			MovePlayer(playerPos);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateWaypoint(IEntity waypoint, float heightOffset)
	{
		switch (m_eWaypointTruckPosition)
		{
			case SCR_ETutorialSupplyTruckWaypointMode.DRIVER:
			{
				UpdateSupplyTruckWaypoint(waypoint, true, "door_l01");
				break;
			}
			
			case SCR_ETutorialSupplyTruckWaypointMode.BACK:
			{
				UpdateSupplyTruckWaypoint(waypoint, false, "door_rear", BACK_DOOR_OFFSET);
				break;
			}
		}
		
		vector WPPos = waypoint.GetOrigin();
		WPPos[1] = WPPos[1] + heightOffset;
		vector pos = GetGame().GetWorkspace().ProjWorldToScreen(WPPos, GetGame().GetWorld());
		int dist = vector.Distance(m_Player.GetOrigin(), WPPos);
		
		// Handle off-screen coords
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		int winX = workspace.GetWidth();
		int winY = workspace.GetHeight();
		int posX = workspace.DPIScale(pos[0]);
		int posY = workspace.DPIScale(pos[1]);
		
		if (posX < 0)
			pos[0] = 0;
		else if (posX > winX)
			pos[0] = workspace.DPIUnscale(winX);
		
		if (posY < 0)
			pos[1] = 0;
		else if (posY > winY || pos[2] < 0)
			pos[1] = workspace.DPIUnscale(winY);
		
		FrameSlot.SetPos(m_wWaypoint, pos[0], pos[1]);
		FrameSlot.SetPos(m_wWaypointDistance, pos[0], pos[1]);
		
		int distShown = dist;
		
		if (dist > 1000)
		{
			distShown = dist - (dist % 1000);
		}
		else if (dist > 100)
		{
			distShown = dist - (dist % 100);
		}
		else if (dist > 50)
		{
			distShown = dist - (dist % 50);
		}
		
		m_wWaypointDistance.SetTextFormat("#AR-Tutorial_WaypointUnits_meters", distShown);
		
		float opacity = 1;
		float distanceOpacity = 1;
		
		if (dist < WAYPOINT_FADE_THRESHOLD)
			opacity = Math.Lerp(WAYPOINT_MINIMUM_OPACITY, 1, dist / WAYPOINT_FADE_THRESHOLD);
		
		if (dist >= WAYPOINT_DISTANCE_INDICATOR_FADE_START)
		{
			distanceOpacity = 1;
		}
		else if (dist <= WAYPOINT_DISTANCE_INDICATOR_FADE_END)
		{
			distanceOpacity = 0;
		}
		else
		{
			distanceOpacity = Math.InverseLerp(WAYPOINT_DISTANCE_INDICATOR_FADE_END, WAYPOINT_DISTANCE_INDICATOR_FADE_START, dist);
		}
			
		m_wWaypoint.SetOpacity(opacity);
		m_wWaypointDistance.SetOpacity(distanceOpacity);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignSuppliesComponent GetSupplyTruckComponent()
	{
		return m_SupplyTruckComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool FindSupplyTruck(IEntity ent)
	{
		SCR_CampaignSuppliesComponent supplyTruckComponent = SCR_CampaignSuppliesComponent.Cast(ent.FindComponent(SCR_CampaignSuppliesComponent));
		
		if (!supplyTruckComponent)
			return true;
		
		Vehicle truck = Vehicle.Cast(ent.GetParent());
		
		if (!truck)
			return true;
		
		m_SupplyTruckComponent = supplyTruckComponent;
		truck.SetName("SupplyTruck");
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		m_CampaignGamemode = SCR_GameModeCampaign.GetInstance();
		
		if (!m_CampaignGamemode)
			return;
		
		typename stages = SCR_ECampaignTutorialStage;
		m_iStagesCnt = stages.GetVariableCount();
		m_CampaignGamemode.SetIsTutorial(true);
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		ChimeraWorld world = GetGame().GetWorld();
		
		if (world)
		{
			MusicManager musicMan = world.GetMusicManager();
			
			if (musicMan)
			{
				array<int> musicCategories = {};
				SCR_Enum.GetEnumValues(MusicCategory, musicCategories);
				
				foreach (int category : musicCategories)
				{
					musicMan.MuteCategory(category, true, true);
				}
			}
		}
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_TUTORIAL_MENU, "Tutorial", "");
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, "", "Stage", "Tutorial", string.Format("0 %1 0 1", m_iStagesCnt - 1));
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, m_eStage);
#endif
		
		Resource holder = BaseContainerTools.LoadContainer(m_sStagesConfig);
		
		if (!holder)
			return;
		
		BaseContainer container = holder.GetResource().ToBaseContainer();
		
		if (!container)
			return;
		
		SCR_CampaignTutorialStages stagesConf = SCR_CampaignTutorialStages.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		
		if (!stagesConf)
			return;
		
		stagesConf.GetStages(m_aStageInfos);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE) != m_eStage && m_Stage)
		{
			FinishStage(m_Stage, DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE));
			ResetStage();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTutorialComponent()
	{
		if (m_wWaypoint)
			m_wWaypoint.RemoveFromHierarchy();
		
		if (m_wWaypointDistance)
			m_wWaypointDistance.RemoveFromHierarchy();
		
		if (m_wFadeOut)
			m_wFadeOut.RemoveFromHierarchy();
		
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_CampaignTutorialStages
{
	[Attribute()]
	private ref array<ref SCR_CampaignTutorialStageInfo> m_TutorialStages;
	
	//------------------------------------------------------------------------------------------------
	void GetStages(out notnull array<ref SCR_CampaignTutorialStageInfo> tutorialStages)
	{
		tutorialStages = m_TutorialStages;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sStageClassName", true)]
class SCR_CampaignTutorialStageInfo
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECampaignTutorialStage))]
	protected SCR_ECampaignTutorialStage m_eStage;
	
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sStageClassName;
	
	//------------------------------------------------------------------------------------------------
	SCR_ECampaignTutorialStage GetIndex()
	{
		return m_eStage;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetClassName()
	{
		return m_sStageClassName;
	}
}

//------------------------------------------------------------------------------------------------
enum SCR_ETutorialSupplyTruckWaypointMode
{
	NONE,
	DRIVER,
	BACK
}

//------------------------------------------------------------------------------------------------
enum SCR_ECampaignTutorialStage
{
	START = 0,
	ZIGZAG_1,
	ZIGZAG_2,
	ZIGZAG_3,
	OVERPASS,
	WIREMESH,
	VAULT,
	VAULT2,
	WINDOW,
	FLAG,
	WALL,
	WALL2,
	LADDER_UP,
	LADDER_PLATFORM,
	LADDER_DOWN,
	LADDER_OFF,
	OBSTACLE_END,
	WEAPON_PICK,
	MAGAZINE_PICK,
	WEAPON_RELOAD,
	FIREPOZ_1,
	SHOOTING,
	SHOOTING_CROUCH,
	SHOOTING_PRONE,
	FIREPOZ_2,
	SHOOTING_LEANING,
	BOARDING,
	DRIVING_0,
	SWITCHING_SEATS,
	VEHICLE_SHOOTING,
	DRIVING_1,
	DRIVING_2,
	DRIVING_3,
	DRIVING_4,
	DRIVING_5,
	DRIVING_6,
	DRIVING_7,
	DRIVING_SERPENTINE,
	DRIVING_8,
	DRIVING_PRE_BASE,
	DRIVING_9,
	DISMOUNTING,
	CONFLICT_TOUR_ARMORY,
	CONFLICT_TOUR_BARRACKS,
	CONFLICT_TOUR_FIELDHOSPITAL,
	CONFLICT_TOUR_ANTENNA,
	CONFLICT_TOUR_HQ,
	CONFLICT_HQ_INFO,
	MAP_INFO,
	CONFLICT_TOUR_GARAGE,
	CONFLICT_TOUR_END,
	CONFLICT_REQUESTING_TRUCK,
	CONFLICT_SUPPLY_DEPOT,
	CONFLICT_LOADING_SUPPLIES,
	DRIVING_10,
	DRIVING_11,
	DRIVING_12,
	DRIVING_13,
	DRIVING_14,
	DRIVING_15,
	CONFLICT_UNLOADING_SUPPLIES,
	CONFLICT_BUILDING_START,
	CONFLICT_BUILD_SERVICE,
	CONFLICT_BUILD_BUNKER,
	CONFLICT_BUILDING_LOAD_SUPPLY,
	CONFLICT_BOARD_TRUCK,
	CONFLICT_BUILDING_QUIT,
	CONFLICT_BUILDING_TO_FAR,
	CONFLICT_BUILDING_CHECKPOINT,
	CONFLICT_BUILDING_QUIT2,
	CONFLICT_TASKS_MENU,
	CONFLICT_TASKS_INFO,
	CONFLICT_VOLUNTEERING,
	CONFLICT_VOLUNTEERING_INFO,
	DRIVING_16,
	DRIVING_17,
	DRIVING_18,
	CONFLICT_BASE_SEIZING,
	CONFLICT_BASE_NEARBY,
	CONFLICT_BASE_SEIZING_INFO,
	CONFLICT_SHOW_COMPAS,
	CONFLICT_COMPAS_DIRECTION_NORTH,
	CONFLICT_COMPAS_DIRECTION_2,
	CONFLICT_COMPAS_MOVE,
	CONFLICT_MAP_MOVE,
	CONFLICT_RADIO_PICKUP,
	CONFLICT_VOIP_SETUP,
	CONFLICT_VOIP_USE,
	CONFLICT_VOIP_INFO,
	CONFLICT_MHQ_DEPLOY,
	CONFLICT_MHQ_INFO,
	CONFLICT_SEIZE_ENEMY_HQ,
};