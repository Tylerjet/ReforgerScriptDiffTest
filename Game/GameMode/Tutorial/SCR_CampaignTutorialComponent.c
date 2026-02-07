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
	protected SCR_GameModeCampaignMP m_CampaignGamemode;
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
	protected SCR_CampaignBase m_HQUS;
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
	protected float m_fStageTimestamp;
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		super.OnPlayerKilled(playerId, player, killer);
		
		delete(m_Stage);
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		if (!playerController)
			return;
		
		GetGame().GetCallqueue().CallLater(TryPlayerSpawn, 25, true, playerController);
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
		SCR_CampaignBase chotain = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain"));
		chotain.SpawnComposition(ECampaignCompositionType.RADIO_ANTENNA);
		chotain.SpawnComposition(ECampaignCompositionType.HOSPITAL);
		chotain.SpawnComposition(ECampaignCompositionType.ARMORY);
		chotain.SpawnComposition(ECampaignCompositionType.SUPPLIES);
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
		
		SCR_CampaignFaction f = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_BLUFOR));
		
		if (!f)
			return;
		
		BaseRadioComponent radioComponent = BaseRadioComponent.Cast(mHQ.FindComponent(BaseRadioComponent));
		
		if (radioComponent)
		{
			radioComponent.TogglePower(false);
			radioComponent.SetFrequency(f.GetFactionRadioFrequency());
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
		SCR_CampaignBase baseChotain = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain"));
		
		if (baseChotain && baseChotain.GetSupplies() < 950)
			baseChotain.AddSupplies(950 - baseChotain.GetSupplies());
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_CaptureLaruns()
	{
		SCR_CampaignBase baseLaruns = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseLaruns"));
				
		if (baseLaruns && !baseLaruns.GetOwningFaction())
		{
			if (baseLaruns.BeginCapture(SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(m_CampaignGamemode.FACTION_BLUFOR)), SCR_CampaignBase.INVALID_PLAYER_INDEX))
				baseLaruns.FinishCapture();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_HandleRespawnRadios()
	{
		while (m_CampaignGamemode.GetActiveRespawnRadiosCount(m_CampaignGamemode.FACTION_BLUFOR) > 0)
			m_CampaignGamemode.RemoveActiveRespawnRadio(m_CampaignGamemode.FACTION_BLUFOR);
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_DeployMHQ()
	{
		if (m_MobileAssemblyComponent && !m_MobileAssemblyComponent.IsDeployed())
			m_MobileAssemblyComponent.Deploy(true);
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
		SCR_CampaignBase.Cast(world.FindEntityByName("MainBaseChotain")).SetCallsignIndex(0);
		SCR_CampaignBase.Cast(world.FindEntityByName("TownBaseChotain")).SetCallsignIndex(1);
		SCR_CampaignBase.Cast(world.FindEntityByName("TownBaseLaruns")).SetCallsignIndex(2);
		SCR_CampaignBase.Cast(world.FindEntityByName("MajorBaseLevie")).SetCallsignIndex(4);
		
		// Attempt to spawn the player automatically, cease after spawn is successful in OnPlayerSpawned
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		TryPlayerSpawn(playerController);
		
		if (!m_bPlayerSpawned)
			GetGame().GetCallqueue().CallLater(TryPlayerSpawn, 100, true, playerController);
	}
	
	//------------------------------------------------------------------------------------------------
	void TryPlayerSpawn(notnull PlayerController pc)
	{
		if (!m_CampaignGamemode)
			return;
		
		int playerId = pc.GetPlayerId();
		
		// Skip faction and spawnpoint selection
		SCR_RespawnSystemComponent respawnSystem = m_CampaignGamemode.GetRespawnSystemComponent();
		
		if (!respawnSystem)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		
		if (!factionManager)
			return;

		Faction factionUS = factionManager.GetFactionByKey(m_CampaignGamemode.FACTION_BLUFOR);
		
		if (!factionUS)
			return;
		
		respawnSystem.DoSetPlayerFaction(playerId, respawnSystem.GetFactionIndex(factionUS));
		array<SCR_SpawnPoint> availableSpawnpoints = SCR_SpawnPoint.GetSpawnPointsForFaction(m_CampaignGamemode.FACTION_BLUFOR);
		
		if (availableSpawnpoints.Count() < 2)
			return;
		
		respawnSystem.DoSetPlayerSpawnPoint(playerId, SCR_SpawnPoint.GetSpawnPointRplId(availableSpawnpoints[1]));
		
		if (!pc)
			return;
		
		if (m_CampaignGamemode.CanPlayerRespawn(pc.GetPlayerId()))
			pc.RequestRespawn();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapOpen()
	{
		m_bIsMapOpen = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapClose()
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
	void OnStructureBuilt(SCR_CampaignBase base, IEntity structure)
	{
		if (m_Stage)
			m_Stage.OnStructureBuilt(base, structure);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		m_bPlayerSpawned = true;
		m_Player = ChimeraCharacter.Cast(controlledEntity);
		GetGame().GetCallqueue().Remove(TryPlayerSpawn);
		
		if (m_Stage)
			m_Stage.SetPlayer(m_Player);
		
		if (!m_bFirstStageSet)
		{
			m_bFirstStageSet = true;
			m_HQUS = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("MainBaseChotain"));
			m_HQUS.AlterReinforcementsTimer(9999999);
			FiringRangeInit();
			MobileHQInit();
			IEntity WP_cottage = GetGame().GetWorld().FindEntityByName("WP_CONFLICT_COMPAS_MOVE");
			IEntity WP_mobileHQ = GetGame().GetWorld().FindEntityByName("WP_CONFLICT_MAP_MOVE");
			SCR_MapDescriptorComponent cottageDescr = SCR_MapDescriptorComponent.Cast(WP_cottage.FindComponent(SCR_MapDescriptorComponent));
			SCR_MapDescriptorComponent mobileTruckDescr = SCR_MapDescriptorComponent.Cast(WP_mobileHQ.FindComponent(SCR_MapDescriptorComponent));
			cottageDescr.Item().SetVisible(false);
			mobileTruckDescr.Item().SetVisible(false);
			
			while (m_CampaignGamemode.GetActiveRespawnRadiosCount(m_CampaignGamemode.FACTION_BLUFOR) < m_CampaignGamemode.GetMaxRespawnRadios())
				m_CampaignGamemode.AddActiveRespawnRadio(m_CampaignGamemode.FACTION_BLUFOR);
			
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
				m_wWaypoint.LoadImageFromSet(0, m_CampaignGamemode.GetBuildingIconsImageset(), "USSR_Base_Small_Select");
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
		return Replication.Time() - m_fStageTimestamp;
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
				m_fStageTimestamp = Replication.Time();
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
		
		m_CampaignGamemode = SCR_GameModeCampaignMP.Cast(GetGameMode());
		
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