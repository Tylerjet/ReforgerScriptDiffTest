class SCR_CampaignTutorialArlandComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_CampaignTutorialArlandComponent : SCR_BaseGameModeComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "", "conf")]
	ResourceName m_sStagesConfigAdvanced;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETutorialArlandStageMasters))]
	protected SCR_ETutorialArlandStageMasters m_eStartingStage;
	
	protected static const int TARGETS_SEARCH_DISTANCE = 1500;
	protected static const int INVALID_STANCE = -1;
	protected static const vector BACK_DOOR_OFFSET = { 0, -0.7, 0 };
	protected static float WAYPOINT_FADE_THRESHOLD = 20;
	protected static float WAYPOINT_MINIMUM_OPACITY = 0.2;
	protected static float WAYPOINT_DISTANCE_INDICATOR_FADE_START = 100;
	protected static float WAYPOINT_DISTANCE_INDICATOR_FADE_END = 50;
	protected static const float SPAWN_STAGE_DISTANCE = 10;
	protected static const float FADE_DURATION = 1;
	protected static const float FADE_SPEED = 0.4;
	protected static const int NUMBER_OF_TUTORIALS = 10;
	static string VEHICLE_MAINTENANCE_PREFAB = "{543C31BC05F032B6}PrefabsEditable/Auto/Compositions/Slotted/SlotFlatSmall/E_VehicleMaintenance_S_US_01.et";
	static string HELICOPTER_PREFAB = "{70BAEEFC2D3FEE64}Prefabs/Vehicles/Helicopters/UH1H/UH1H.et";
	static string HUMMER_PREFAB = "{751AFEEA19DDFB04}Prefabs/Vehicles/Wheeled/M998/M998_covered_long_MERDC.et";
	static string JEEP_PREFAB = "{94DE32169691AC34}Prefabs/Vehicles/Wheeled/M151A2/M151A2_transport_MERDC.et";
	static string TRUCK_PREFAB = "{C8656AECF5DF41D9}Prefabs/Vehicles/Wheeled/M923A1/M923A1_repair_MERDC.et";
	static string DRIVING_RANGE_PREFAB = "{B6F5006CBD24E9A1}Prefabs/TutorialDrivingRange.et";
	
	protected ref array<ref SCR_CampaignTutorialArlandStageInfo> m_aStageInfos = {};
	protected ref array<ref SCR_CampaignTutorialArlandStages> m_aStageConfigs = {};
	protected int m_iActiveStage;
	protected SCR_BaseCampaignTutorialArlandStage m_Stage;
	protected ImageWidget m_wFadeOut;
	protected SCR_GameModeCampaign m_CampaignGamemode;
	protected bool m_bPlayerSpawned;
	protected SCR_ECampaignTutorialArlandStage m_eStage = SCR_ECampaignTutorialArlandStage.START;
	protected SCR_ECampaignTutorialArlandStage m_eLastStage;
	protected ChimeraCharacter m_Player;
	protected int m_iStagesCnt;
	protected ref array<Widget> m_aWaypoints = {};
	protected bool m_bFirstStageSet;
	protected ref array<SCR_FiringRangeTarget> m_aFiringRangeTargets = {};
	protected SCR_CampaignSuppliesComponent m_SupplyTruckComponent;
	protected int m_iCountOfHits;
	protected SCR_CampaignMilitaryBaseComponent m_HQUS;
	protected BaseRadioComponent m_Radio;
	protected bool m_bIsMapOpen = false;
	protected bool m_bUsed3PV = false;
	protected ECharacterStance m_ePlayerStance = INVALID_STANCE;
	protected bool m_bCheckIsDeployed = false;
	protected bool m_bCheckIsDeployedBipod = false;
	protected bool m_bCheckLeaning = false;
	protected SCR_CampaignMobileAssemblyComponent m_MobileAssemblyComponent;
	protected bool m_bMovedOutOfVehicleByScript = false;
	protected bool m_fDelayedControlSchemeChangeRunning = false;
	protected bool m_bIsUsingGamepad;
	protected SCR_ETutorialArlandSupplyTruckWaypointMode m_eWaypointTruckPosition = SCR_ETutorialArlandSupplyTruckWaypointMode.NONE;
	protected float m_fStageTimestamp;
	protected SCR_CampaignTutorialArlandStages m_ActiveConfig;
	protected ResourceName m_WaypointWidget = "{B80E30D68492B2AE}UI/Imagesets/TutorialHud/TutorialHUD.imageset";
	protected static ref ScriptInvoker m_OnStructureBuilt = new ScriptInvoker();
	protected ref array<IEntity> m_aPlacedCompositions = {};
	protected IEntity m_PreviewBunker;
	protected SCR_PlacingEditorComponent m_PlacingComponent;
	protected bool m_bIsFirstRun = false;
	protected Vehicle m_RepairTruck;
	protected Vehicle m_Helicopter;
	protected Vehicle m_Hummer;
	protected Vehicle m_Jeep;
	protected Vehicle m_HummerRepairable;
	protected IEntity m_DrivingRange;
	protected bool m_bStagesComplete = false;
	protected ref array<bool> m_aTutorialBool = {};
	protected SCR_VoiceoverSystem m_VoiceoverSystem;
	
	//------------------------------------------------------------------------------------------------
	void HideSubtitles(Widget widgeter)
	{
		widgeter.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupVoiceSystem()
	{
		if (!m_VoiceoverSystem)
			m_VoiceoverSystem = SCR_VoiceoverSystem.GetInstance();
		
		if (!m_ActiveConfig)
			return;
		
		ResourceName voiceDataConfig = m_ActiveConfig.GetVoiceOverDataConfig();
		if (voiceDataConfig.IsEmpty())
			return;
		
		m_VoiceoverSystem.SetData(voiceDataConfig);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_VoiceoverSystem GetVoiceSystem()
	{
		return m_VoiceoverSystem;
	}

	//------------------------------------------------------------------------------------------------
	protected void RemovePlayerMapMarkers()
	{
		SCR_MapMarkerManagerComponent mapMarkerManager = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		if (!mapMarkerManager)
			return;
		
		array <ref SCR_MapMarkerBase> markerArray = {};		
		markerArray = mapMarkerManager.GetStaticMarkers();
		for (int index = markerArray.Count()-1; index >= 0; index--)
		{
			mapMarkerManager.RemoveStaticMarker(markerArray[index]);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//!
	void HandleAchievement()
	{
		/* Achievement SWEAT_SAVES_BLOOD */
		if (m_bStagesComplete)
		{	
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (!playerController)
				return;
			
			SCR_AchievementsHandler handler = SCR_AchievementsHandler.Cast(playerController.FindComponent(SCR_AchievementsHandler));
			if (!handler)
				return;
		
			handler.UnlockAchievement(AchievementId.SWEAT_SAVES_BLOOD);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] tutorialNumber
	//! \param[in] completed
	void SetStagesComplete(int tutorialNumber, bool completed)
	{
		if (m_aTutorialBool.IsIndexValid(tutorialNumber))
			m_aTutorialBool[tutorialNumber] = completed;
		else
			return;
		
		m_bStagesComplete = true;
		foreach (bool value : m_aTutorialBool)
		{
			if (!value)
			{
				m_bStagesComplete = false;
				break;
			}

		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void StageReset_Helicopter()
	{
		m_Helicopter = Vehicle.Cast(SpawnAsset("UH1COURSE", HELICOPTER_PREFAB));
		if (!m_Helicopter)
			return;
		
		//general helicopter damage check
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_Helicopter.GetDamageManager());
		if (damageManager)
			damageManager.GetOnDamageStateChanged().Insert(OnHelicopterDamaged);
		
		//Drowned periodical check
		GetGame().GetCallqueue().CallLater(HelicopterDrownedCheck, 1000, true);
		
		//Rotor damage check
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_Helicopter.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;
		
		IEntity rotor;
		EntitySlotInfo slot = slotManager.GetSlotByName("RotorMain");
		if (slot)
		{
			rotor = slot.GetAttachedEntity();
			SCR_RotorDamageManagerComponent rotorDmgComp = SCR_RotorDamageManagerComponent.Cast(rotor.FindComponent(SCR_RotorDamageManagerComponent));
			rotorDmgComp.GetOnDamageStateChanged().Insert(OnHelicopterDamaged);
	}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HelicopterDrownedCheck()
	{
		if (!m_Helicopter)
			return;
		
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Helicopter.FindComponent(VehicleControllerComponent_SA));
		if (!vehicleController)
			return;
		
		if (!vehicleController.GetEngineDrowned())
			return;
		
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void RemoveHelicopterInvokers()
	{	
		if (!m_Helicopter)
			return;
		
		//General damage invoker
		SCR_VehicleDamageManagerComponent heliDamageManager = SCR_VehicleDamageManagerComponent.Cast(m_Helicopter.GetDamageManager());
			if (heliDamageManager)
				heliDamageManager.GetOnDamageStateChanged().Remove(OnHelicopterDamaged);
		
		//Drowned periodical check
		GetGame().GetCallqueue().Remove(HelicopterDrownedCheck);
		
		//Rotor damage invoker
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_Helicopter.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;
		
		EntitySlotInfo slot = slotManager.GetSlotByName("RotorMain");
		if (slot)
		{
			IEntity rotor = slot.GetAttachedEntity();
			if (!rotor)
				return;
			
			SCR_RotorDamageManagerComponent rotorDmgComp = SCR_RotorDamageManagerComponent.Cast(rotor.FindComponent(SCR_RotorDamageManagerComponent));
			if (!rotorDmgComp)
				return;
			
			rotorDmgComp.GetOnDamageStateChanged().Remove(OnHelicopterDamaged);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHelicopterDamaged(EDamageState state)
	{	
		if (state == EDamageState.UNDAMAGED)
			return;
		
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVehicleDamaged(EDamageState state)
	{	
		if (state == EDamageState.UNDAMAGED)
			return;
		
		if (m_Jeep)
		{
			SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_Jeep.GetDamageManager());
			if (damageManager)
			{
				damageManager.GetOnDamageStateChanged().Remove(OnVehicleDamaged);
			
				SCR_HitZone engine = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
				if (engine)
					engine.GetOnDamageStateChanged().Remove(OnVehicleDamaged);
		
				SCR_HitZone gearbox = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
				if (gearbox)
					gearbox.GetOnDamageStateChanged().Remove(OnVehicleDamaged);
			}
		}
		
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEngineStoppedJeep()
	{
		if (!m_Jeep)
			return;
		
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Jeep.FindComponent(VehicleControllerComponent_SA));
		if (!vehicleController || !vehicleController.GetEngineDrowned())
			return;
		
		vehicleController.GetOnEngineStop().Remove(OnEngineStoppedJeep);
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEngineStoppedHmw()
	{
		if (!m_Hummer)
			return;
		
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Hummer.FindComponent(VehicleControllerComponent_SA));
		if (!vehicleController || !vehicleController.GetEngineDrowned())
			return;
		
		vehicleController.GetOnEngineStop().Remove(OnEngineStoppedHmw);
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEngineStoppedTruck()
	{
		if (!m_RepairTruck)
			return;
		
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_RepairTruck.FindComponent(VehicleControllerComponent_SA));
		if (!vehicleController || !vehicleController.GetEngineDrowned())
			return;
		
		vehicleController.GetOnEngineStop().Remove(OnEngineStoppedTruck);
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTruckDamaged(EDamageState state)
	{	
		if (state == EDamageState.UNDAMAGED)
			return;
		
		if (m_RepairTruck)
		{
			SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_RepairTruck.GetDamageManager());
			if (damageManager)
			{
				damageManager.GetOnDamageStateChanged().Remove(OnTruckDamaged);
			
				SCR_HitZone engine = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
				if (engine)
					engine.GetOnDamageStateChanged().Remove(OnTruckDamaged);
		
				SCR_HitZone gearbox = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
				if (gearbox)
					gearbox.GetOnDamageStateChanged().Remove(OnTruckDamaged);
		
			}
		}
		
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
		//------------------------------------------------------------------------------------------------
	protected void OnHmwDamaged(EDamageState state)
	{	
		if (state == EDamageState.UNDAMAGED)
			return;
		
		if (m_Hummer)
		{
			SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_Hummer.GetDamageManager());
			if (damageManager)
			{
				damageManager.GetOnDamageStateChanged().Remove(OnHmwDamaged);
			
				SCR_HitZone engine = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
				if (engine)
					engine.GetOnDamageStateChanged().Remove(OnHmwDamaged);
		
				SCR_HitZone gearbox = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
				if (gearbox)
					gearbox.GetOnDamageStateChanged().Remove(OnHmwDamaged);
			}
		}
		
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] prefabName
	//! \return
	IEntity FindBuiltComposition(string prefabName)
	{
		if (!m_aPlacedCompositions)
			return null;
		
		foreach (IEntity composition : m_aPlacedCompositions)
		{
			if (composition && composition.GetPrefabData().GetPrefabName() == prefabName)
				return composition; 
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetActiveStage()
	{
		return m_iActiveStage;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] i
	void SetActiveStage(int i)
	{
		m_iActiveStage = i;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] descriptorOwnerName
	//! \param[in] enable
	void ShowMapDescriptor(string descriptorOwnerName, bool enable)
	{
		IEntity ent = IEntity.Cast(GetGame().GetWorld().FindEntityByName(descriptorOwnerName));
		if (!ent)
			return;

		SCR_MapDescriptorComponent descr = SCR_MapDescriptorComponent.Cast(ent.FindComponent(SCR_MapDescriptorComponent));
		if (!descr)
			return;

		descr.Item().SetVisible(enable);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] angle
	//! \param[in] range
	//! \return
	// ToDo - replace with proper Math.MapAngle usage
	bool IsMyAngleInRange(float angle, float range)
	{
		float cameraAngle = Math.RAD2DEG * m_Player.GetCharacterController().GetInputContext().GetAimingAngles()[0];
		return (cameraAngle > angle - range) && (cameraAngle < angle+range);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] ent
	//! \return angle to entity from players current position.
	float GetEntityCompassAngle(notnull IEntity ent)
	{
		if (!m_Player)
			return 0;
		
		vector direction = ent.GetOrigin() - m_Player.GetOrigin();
		direction.Normalize();
		
		float angle = Math.Atan2(direction[0], direction[2]) * Math.RAD2DEG;
		
		return Math.Round(angle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] ent
	void SetPreviewBunker(IEntity ent)
	{
		m_PreviewBunker = ent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetPreviewBunker()
	{
		return m_PreviewBunker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] modeEntity
	void OnModeAdded(SCR_EditorModeEntity modeEntity)
	{
		if (modeEntity.GetModeType() != EEditorMode.BUILDING)
			return;
		
		m_PlacingComponent = SCR_PlacingEditorComponent.Cast(modeEntity.FindComponent(SCR_PlacingEditorComponent));
		if (!m_PlacingComponent)
			return;
		
		m_PlacingComponent.GetOnPlaceEntityServer().Insert(OnPlacedPreview);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetupEditorModeListener()
	{	
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		if (!editorManager)
			return;
		
		editorManager.GetOnModeAdd().Insert(OnModeAdded);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_PlacingEditorComponent GetPlacingComponent()
	{
		return m_PlacingComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlacedPreview(int prefabID, SCR_EditableEntityComponent ent)
	{
		if (!ent)
			return;
		
		m_aPlacedCompositions.Insert(ent.GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnObjectDestroyed(EDamageState state)
	{
		if (state == EDamageState.DESTROYED)
			SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvoker GetOnStructureBuilt()
	{
		return m_OnStructureBuilt;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity);
		
		delete(m_Stage);
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		if (!playerController)
			return;
		
		array<SCR_SpawnPoint> availableSpawnpoints = SCR_SpawnPoint.GetSpawnPointsForFaction(m_CampaignGamemode.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR));
		
		if (availableSpawnpoints.Count() < 2)
			return;
		
		TrySpawnPlayer(playerController, availableSpawnpoints[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		if (!SCR_HintManagerComponent.GetInstance())
			return;
		
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
	//! \return
	ChimeraCharacter GetPlayer()
	{
		return m_Player;
	}	
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] fadeOut
	void FadeToBlack(bool fadeOut)
	{
		SCR_FadeInOutEffect fade = SCR_FadeInOutEffect.Cast(GetGame().GetHUDManager().FindInfoDisplay(SCR_FadeInOutEffect));
		fade.FadeOutEffect(fadeOut, FADE_DURATION);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetConfigEnvironmentOverride()
	{
		if (!m_ActiveConfig)
			return;
		
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;
				
		TimeAndWeatherManagerEntity tmwManager = world.GetTimeAndWeatherManager();
		if (!tmwManager)
			return;
		
		string weatherName;
		switch (m_ActiveConfig.GetWeatherOverride())
		{
			case SCR_EWeatherStates.CLEAR:
				weatherName = "Clear";
				break;
			
			case SCR_EWeatherStates.RAINY:
				weatherName = "Rainy";
				break;
			
			case SCR_EWeatherStates.CLOUDY:
				weatherName = "Cloudy";
				break;
			
			case SCR_EWeatherStates.OVERCAST:
				weatherName = "Overcast";
				break;
		}
		
		tmwManager.ForceWeatherTo(m_ActiveConfig.ShouldWeatherLoop(), weatherName);
		tmwManager.SetTimeOfTheDay(m_ActiveConfig.GetTime());
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void SetActiveConfig(SCR_ETutorialArlandStageMasters config)
	{
		RemovePlayerMapMarkers();
		
		if (m_ActiveConfig && m_ActiveConfig.GetConfigClassName() == SCR_ETutorialArlandStageMasters.HELICOPTERS)
			RemoveHelicopterInvokers();
				
		foreach(SCR_CampaignTutorialArlandStages configs:m_aStageConfigs)
		{
			if(configs.GetStagesFromConfig() == config)
			{
				m_ActiveConfig = configs;
				m_ActiveConfig.GetStages(m_aStageInfos);
				if (m_Stage)
					m_Stage.FlushWaypoints();
				FlushWaypoints();
				delete m_Stage;
				m_iActiveStage = 0;
				SetStage(m_aStageInfos[0].GetIndex(), m_aStageInfos[m_aStageInfos.Count()-1].GetIndex());
				ResetPlayerPosition();
				
				SetConfigEnvironmentOverride();
				SetupVoiceSystem();
			}
			
		}

		SetupStageCounts();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void FiringRangeInit()
	{
		typename stages = SCR_ECampaignTutorialArlandStage;
		IEntity shootingRangePos = GetGame().GetWorld().FindEntityByName(string.Format("PP_%1", stages.GetVariableName(SCR_ECampaignTutorialArlandStage.WEAPON_PICK)));
		IEntity baseBeauregard = GetGame().GetWorld().FindEntityByName("TownBaseBeauregard");
		
		if (shootingRangePos)
			GetGame().GetWorld().QueryEntitiesBySphere(shootingRangePos.GetOrigin(), TARGETS_SEARCH_DISTANCE, CollectAllTargets, null, EQueryEntitiesFlags.ALL);
		
		if (baseBeauregard)
			GetGame().GetWorld().QueryEntitiesBySphere(baseBeauregard.GetOrigin(), 50, CollectAllTargets, null, EQueryEntitiesFlags.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
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
		
		array<EntitySlotInfo> slots = {};
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
	//!
	void SupplyTruckInit()
	{
		IEntity supplyTruck = GetGame().GetWorld().FindEntityByName("BuildingSupplyTruck");
		
		if (!supplyTruck)
			return;
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(supplyTruck.FindComponent(SlotManagerComponent));
		
		if (!slotManager)
			return;
		
		array<EntitySlotInfo> slots = {};
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
	//!
	void StageReset_RifleRespawn()
	{
		SpawnAsset("M16", "{3E413771E1834D2F}Prefabs/Weapons/Rifles/M16/Rifle_M16A2.et");
		SpawnAsset("M21", "{81EB948E6414BD6F}Prefabs/Weapons/Rifles/M14/Rifle_M21_ARTII.et");
		SpawnAsset("M249", "{D2B48DEBEF38D7D7}Prefabs/Weapons/MachineGuns/M249/MG_M249.et");
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] position
	void StageReset_MoveInJeep(IEntity position = null)
	{
		SpawnAsset("Jeep", "{5168FEA3054D6D15}Prefabs/Vehicles/Wheeled/M151A2/M151A2_M2HB_MERDC.et", position);
		MoveInVehicle("Jeep", ECompartmentType.Pilot);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] position
	void StageReset_SpawnTruck(IEntity position = null)
	{
		SpawnAsset("SupplyTruck", "{F37113A988304565}Prefabs/MP/Campaign/Assets/CampaignSupplyTruckWest.et", position);
		SupplyTruckInit();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void StageReset_ProcessTruck()
	{
		//m_SupplyTruckComponent.AddSupplies(1000);
		MoveInVehicle("SupplyTruck", ECompartmentType.Pilot);
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
		
		if (accessComp)
		{
			accessComp.GetOnCompartmentLeft().Remove(OnSupplyTruckLeft);
			accessComp.GetOnCompartmentLeft().Insert(OnSupplyTruckLeft);
		}
	}
	
//	//------------------------------------------------------------------------------------------------
//	//!
//	void StageReset_PrepareFarm()
//	{
//		m_SupplyTruckComponent.AddSupplies(500);
//		SCR_CampaignMilitaryBaseComponent baseFarm = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseFarm").FindComponent(SCR_CampaignMilitaryBaseComponent));
//
//		if (baseFarm && baseFarm.GetSupplies() < 950)
//			baseFarm.AddSupplies(950 - baseFarm.GetSupplies());
//	}
		
	//------------------------------------------------------------------------------------------------
	//!
	void StageReset_ResetSeizing()
	{
		if (m_DrivingRange)
			SCR_EntityHelper.DeleteEntityAndChildren(m_DrivingRange);
		
		SCR_CampaignMilitaryBaseComponent baseBeauregard = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseBeauregard").FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (baseBeauregard)
		{
			baseBeauregard.SetFaction(m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
			baseBeauregard.RefreshTasks();
		}
		
		SCR_CampaignMilitaryBaseComponent baseMossHill = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("MainBaseMossHill").FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (baseMossHill)
		{
			baseMossHill.SetFaction(m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
			baseMossHill.RefreshTasks();
		}
		
		SpawnAsset("Seizing_Car", "{6B24D5AFD884D64C}Prefabs/Vehicles/Wheeled/M998/M998_MERDC.et");
		SpawnAsset("Seizing_Seizing_Car_Pub", "{6B24D5AFD884D64C}Prefabs/Vehicles/Wheeled/M998/M998_MERDC.et");
		IEntity mhq = SpawnAsset("MobileHQ", "{9CB496688A3BCC3E}Prefabs/MP/Campaign/Assets/CampaignMobileAssemblyWest.et");
		if (!mhq)
			return;
		
		SCR_ArsenalComponent arsenalComp = SCR_ArsenalComponent.FindArsenalComponent(mhq);
		if (!arsenalComp)
			return;
		
		SCR_ResourceComponent resourceComp = SCR_ResourceComponent.FindResourceComponent(arsenalComp.GetOwner());
		if (resourceComp)
			resourceComp.SetResourceTypeEnabled(false);
		
		SCR_CampaignFaction bluforFaction = m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		if (!bluforFaction)
			return;
		
		BaseRadioComponent radioComponent = BaseRadioComponent.Cast(mhq.FindComponent(BaseRadioComponent));
		if (radioComponent && radioComponent.TransceiversCount() > 0)
		{
			radioComponent.SetPower(false);
			radioComponent.GetTransceiver(0).SetFrequency(bluforFaction.GetFactionRadioFrequency());
			radioComponent.SetEncryptionKey(bluforFaction.GetFactionRadioEncryptionKey());
		}
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(mhq.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;
		
		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);
		
		foreach (EntitySlotInfo slot : slots)
		{
			IEntity truckBed = slot.GetAttachedEntity();

			if (!truckBed)
				continue;
			
			SCR_CampaignMobileAssemblyComponent mobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));
			if (mobileAssemblyComponent)
			{	
				mobileAssemblyComponent.SetParentFactionID(GetGame().GetFactionManager().GetFactionIndex(bluforFaction));
				break;
			}
		}
		
	}

	//------------------------------------------------------------------------------------------------
	//!
	void StageReset_DeployMHQ()
	{
		if (m_MobileAssemblyComponent && !m_MobileAssemblyComponent.IsDeployed())
		{
			m_MobileAssemblyComponent.UpdateRadioCoverage();
			m_MobileAssemblyComponent.Deploy(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool PrepareMedicalAmbulance()
	{
		IEntity ambulance = SpawnAsset("Ambulance", "{3B1EB924602C7A07}Prefabs/Vehicles/Wheeled/M998/M997_maxi_ambulance_MERDC.et");
		if (!ambulance)
			return false;
		
		SCR_VehicleDamageManagerComponent vehicleDamageManager = SCR_VehicleDamageManagerComponent.Cast(ambulance.FindComponent(SCR_VehicleDamageManagerComponent));
		if (!vehicleDamageManager)
			return false;
		
		vehicleDamageManager.GetOnDamageStateChanged().Insert(OnObjectDestroyed);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool PrepareAccidentArea()
	{
		IEntity victim = SpawnAsset("Victim", "{2F912ED6E399FF47}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Unarmed.et");
		if (!victim)
			return false;
		
		SCR_CharacterDamageManagerComponent damageComp = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageComp)
			return false;
		
		IEntity vehicle = SpawnAsset("AccidentJeep", "{86D830868F026D54}Prefabs/Vehicles/Wheeled/M151A2/M151A2_MERDC.et");
		if (!vehicle)
			return false;
		
		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(victim.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccess)
			return false;
		
		if (!compartmentAccess.MoveInVehicle(vehicle, ECompartmentType.Pilot))
			return false;
		
		//prepare injuries on victim
		damageComp.AddParticularBleeding("LThigh");
		damageComp.AddParticularBleeding("Chest");
		damageComp.GetHitZoneByName("LThigh").SetHealth(0);
		damageComp.GetHitZoneByName("Chest").SetHealth(0);
		damageComp.ForceUnconsciousness();
		damageComp.UpdateBloodClothes();
		damageComp.GetOnDamageStateChanged().Insert(OnObjectDestroyed);
		
		GetGame().GetCallqueue().Remove(RefreshVictimBloodLevel);
		GetGame().GetCallqueue().Remove(RefreshVictimResilience);
		GetGame().GetCallqueue().CallLater(RefreshVictimBloodLevel, 1000, true);
		GetGame().GetCallqueue().CallLater(RefreshVictimResilience, 1000, true);
		
		//Lock jeep to prevent players from driving away with it. However, this behavior cannot break any stages.
		SCR_VehicleSpawnProtectionComponent spawnProtectionComponent = SCR_VehicleSpawnProtectionComponent.Cast(vehicle.FindComponent(SCR_VehicleSpawnProtectionComponent));
		if (spawnProtectionComponent)
		{
			spawnProtectionComponent.SetProtectOnlyDriverSeat(false);
			spawnProtectionComponent.SetReasonText("#AR-Campaign_Action_BuildBlocked-UC");
			spawnProtectionComponent.SetVehicleOwner(-2);
		}
		
		//Disable damage handling on jeep to prevent players or uncanny physics from accidentally destroying it
		SCR_VehicleDamageManagerComponent vehicleDamageManager = SCR_VehicleDamageManagerComponent.Cast(vehicle.FindComponent(SCR_VehicleDamageManagerComponent));
		if (vehicleDamageManager)
			vehicleDamageManager.EnableDamageHandling(false);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ResetStage_Medical()
	{
		//Return back to HUB should stage preparation fail
		if (!PrepareMedicalAmbulance())
			SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
		
		if (!PrepareAccidentArea())
			SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
		
		SpawnAsset("Backpack", "{4805E67E2AE30F8D}Prefabs/Items/Equipment/Backpacks/Backpack_Medical_M5.et");
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ResetStage_VehiclesSimple()
	{
		m_Jeep = Vehicle.Cast(SpawnAsset("SmallJeep", JEEP_PREFAB));
		if (!m_Jeep)
			return;
		
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_Jeep.GetDamageManager());
		if (damageManager)
		{
			damageManager.GetOnDamageStateChanged().Insert(OnVehicleDamaged);
		
			SCR_HitZone engine = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
			if (engine)
				engine.GetOnDamageStateChanged().Insert(OnVehicleDamaged);
		
			SCR_HitZone gearbox = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
			if (gearbox)
				gearbox.GetOnDamageStateChanged().Insert(OnVehicleDamaged);
		}
			
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Jeep.FindComponent(VehicleControllerComponent_SA));
		if (vehicleController)
			vehicleController.GetOnEngineStop().Insert(OnEngineStoppedJeep);
		
		m_DrivingRange = SpawnAsset("TutorialDrivingRange", DRIVING_RANGE_PREFAB);
		if (!m_DrivingRange)
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ResetStage_VehiclesHeavy()
	{
		m_RepairTruck = Vehicle.Cast(SpawnAsset("Truck",TRUCK_PREFAB ));
		if (!m_RepairTruck)
			return;
		
		m_Hummer = Vehicle.Cast(SpawnAsset("Hummer", HUMMER_PREFAB));
		if (!m_Hummer)
			return;
		
		m_HummerRepairable = Vehicle.Cast(SpawnAsset("Hummer2", HUMMER_PREFAB));
		if (!m_HummerRepairable)
			return;
		
		SCR_DamageManagerComponent damageManagerBroken = SCR_DamageManagerComponent.GetDamageManager(m_HummerRepairable);
		
		if (damageManagerBroken)
		{
			HitZone engineHitZone = damageManagerBroken.GetHitZoneByName("Engine_01");
			
			if (engineHitZone)
			{
				// This will work only if hitzone belongs to this damage manager
				vector transform[3];
				transform[0] = m_HummerRepairable.GetOrigin();
				transform[1] = vector.Forward;
				transform[2] = vector.Up;

				DamageManagerComponent engineDamageManager = DamageManagerComponent.Cast(engineHitZone.GetHitZoneContainer());
				if (engineDamageManager)
				{
					SCR_DamageContext damageContext1 = new SCR_DamageContext(EDamageType.TRUE, engineHitZone.GetMaxHealth(), transform, engineDamageManager.GetOwner(), engineHitZone, Instigator.CreateInstigator(null), null, -1, -1);
					SCR_DamageContext damageContext2 = new SCR_DamageContext(EDamageType.INCENDIARY, 50, transform, engineDamageManager.GetOwner(), engineHitZone, Instigator.CreateInstigator(null), null, -1, -1);

					engineDamageManager.HandleDamage(damageContext1);
					engineDamageManager.HandleDamage(damageContext2);
				}
			}
		}
		
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_RepairTruck.GetDamageManager());
		if (damageManager)
		{
			damageManager.GetOnDamageStateChanged().Insert(OnTruckDamaged);
		
			SCR_HitZone engine = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
			if (engine)
				engine.GetOnDamageStateChanged().Insert(OnTruckDamaged);
		
			SCR_HitZone gearbox = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
			if (gearbox)
				gearbox.GetOnDamageStateChanged().Insert(OnTruckDamaged);
		}
		
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_RepairTruck.FindComponent(VehicleControllerComponent_SA));
		if (vehicleController)
			vehicleController.GetOnEngineStop().Insert(OnEngineStoppedTruck);
		
		damageManager = SCR_VehicleDamageManagerComponent.Cast(m_Hummer.GetDamageManager());
		if (damageManager)
		{
			damageManager.GetOnDamageStateChanged().Insert(OnHmwDamaged);
		
			SCR_HitZone engine = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
			if (engine)
				engine.GetOnDamageStateChanged().Insert(OnHmwDamaged);
		
			SCR_HitZone gearbox = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
			if (gearbox)
				gearbox.GetOnDamageStateChanged().Insert(OnHmwDamaged);
		}
		
		vehicleController = VehicleControllerComponent_SA.Cast(m_Hummer.FindComponent(VehicleControllerComponent_SA));
		if (vehicleController)
			vehicleController.GetOnEngineStop().Insert(OnEngineStoppedHmw);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ResetStage_ShootingRange()
	{
		IEntity m16 = SpawnAsset("M16", "{3E413771E1834D2F}Prefabs/Weapons/Rifles/M16/Rifle_M16A2.et");
		if (!m16)
			return;	
			
		IEntity m249 = SpawnAsset("M249", "{D2B48DEBEF38D7D7}Prefabs/Weapons/MachineGuns/M249/MG_M249.et");
		if (!m249)
			return;
		
		IEntity m21 = SpawnAsset("M21", "{81EB948E6414BD6F}Prefabs/Weapons/Rifles/M14/Rifle_M21_ARTII.et");
		if (!m21)
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ResetStage_CampaignBuilding()
	{
		//Delete built compositions.
		foreach (IEntity ent : m_aPlacedCompositions)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(ent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void RefreshVictimResilience()
	{
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (!victim)
			return;
		
		SCR_CharacterDamageManagerComponent damageComp = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageComp)
			return;
		
		damageComp.GetResilienceHitZone().SetHealth(0);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void RefreshVictimBloodLevel()
	{
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (!victim)
			return;
		
		SCR_CharacterDamageManagerComponent damageComp = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageComp)
			return;
		
		damageComp.GetBloodHitZone().SetHealth(2000);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_CharacterDamageManagerComponent GetVictimDamageManager()
	{
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (!victim)
			return null;
		
		return SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] forcePlayerReset
	void ResetStage(bool forcePlayerReset = false)
	{
		if (!m_Player.IsInVehicle() || forcePlayerReset)
			ResetPlayerPosition();
		
		if (m_Stage)
			m_Stage.Reset();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Vehicle GetHummer()
	{
		return m_HummerRepairable;
	}
		
	//------------------------------------------------------------------------------------------------
	//! \return
	Vehicle GetRepairTruck()
	{
		return m_RepairTruck;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] name
	//! \param[in] type
	//! \param[in] spawnpoint
	//! \return
	IEntity SpawnAsset(string name, ResourceName type, IEntity spawnpoint = null)
	{
		string posName;
		
		if (!spawnpoint)
			spawnpoint = GetGame().GetWorld().FindEntityByName(string.Format("SpawnPos_%1", name));
		
		if (!spawnpoint)
			return null;
		
		IEntity oldEntity = GetGame().GetWorld().FindEntityByName(name);

		if (oldEntity)
			SCR_EntityHelper.DeleteEntityAndChildren(oldEntity);
		
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
	//!
	//! \param[in] veh
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
	//! \return
	array<SCR_FiringRangeTarget> GetAllTargets()
	{
		return m_aFiringRangeTargets;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetTargetHits()
	{
		return m_iCountOfHits;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] check
	void SetCheckLeaning(bool check)
	{
		m_bCheckLeaning = check;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] stance
	void SetPlayerStanceToCheck(ECharacterStance stance)
	{
		m_ePlayerStance = stance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] check
	void SetPlayerDeployedCheck(bool check)
	{
		m_bCheckIsDeployed = check;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] check
	void SetPlayerDeployedBipodCheck(bool check)
	{
		m_bCheckIsDeployedBipod = check;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ECharacterStance GetPlayerStanceToCheck()
	{
		return m_ePlayerStance;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] state
	//! \param[in] target
	void CountTargetHit(ETargetState state, SCR_FiringRangeTarget target)
	{
		if (state != ETargetState.TARGET_DOWN)
			return; 
		
		CharacterControllerComponent comp = CharacterControllerComponent.Cast(m_Player.FindComponent(CharacterControllerComponent));
		if (!comp)
			return;
		
		if (GetPlayerStanceToCheck() != INVALID_STANCE && comp.GetStance() != GetPlayerStanceToCheck())
			return;
		
		if (m_bCheckIsDeployed && !comp.GetIsWeaponDeployed())
			return;
		
		if (m_bCheckIsDeployedBipod && !comp.GetIsWeaponDeployedBipod())
			return;
		
		// Leaning doesn't matter or is wrong.
		if (m_bCheckLeaning && !IsCharacterLeaning())
			return;
		
		target.SetAutoResetTarget(false);
		target.Event_TargetChangeState.Remove(CountTargetHit);
		m_iCountOfHits ++;	
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] mode
	void SetWaypointTruckPosition(SCR_ETutorialArlandSupplyTruckWaypointMode mode)
	{
		m_eWaypointTruckPosition = mode;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);
		
		BaseWorld world = GetGame().GetWorld();
		SCR_CampaignMilitaryBaseComponent.Cast(world.FindEntityByName("MainBaseHQ").FindComponent(SCR_CampaignMilitaryBaseComponent)).SetCallsignIndex(0);
		SCR_CampaignMilitaryBaseComponent.Cast(world.FindEntityByName("TownBaseFarm").FindComponent(SCR_CampaignMilitaryBaseComponent)).SetCallsignIndex(1);
		SCR_CampaignMilitaryBaseComponent.Cast(world.FindEntityByName("TownBaseBeauregard").FindComponent(SCR_CampaignMilitaryBaseComponent)).SetCallsignIndex(2);
		SCR_CampaignMilitaryBaseComponent.Cast(world.FindEntityByName("MainBaseMossHill").FindComponent(SCR_CampaignMilitaryBaseComponent)).SetCallsignIndex(4);
		
		// Attempt to spawn the player automatically, cease after spawn is successful in OnPlayerSpawned
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		ForcePlayerFaction(playerController);
		
		while (m_aTutorialBool.Count() < NUMBER_OF_TUTORIALS)
		{
			m_aTutorialBool.Insert(false);
	}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pc
	void ForcePlayerFaction(notnull PlayerController pc)
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

		Faction factionUS =  m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		if (!factionUS)
			return;
		
		SCR_BasePlayerLoadout loadout = GetGame().GetLoadoutManager().GetRandomFactionLoadout(factionUS);
		if (!loadout)
			return;
		
		array<SCR_SpawnPoint> availableSpawnpoints = SCR_SpawnPoint.GetSpawnPointsForFaction(m_CampaignGamemode.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR));
		
		if (availableSpawnpoints.Count() < 1)
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
		TrySpawnPlayer(pc, availableSpawnpoints[0]);		
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
	//! \param[in] config
	void OnMapOpen(MapConfiguration config)
	{
		m_bIsMapOpen = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void OnMapClose(MapConfiguration config)
	{
		m_bIsMapOpen = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsMapOpen()
	{
		return m_bIsMapOpen;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	BaseRadioComponent GetPlayerRadio()
	{
		return m_Radio;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] waypoint
	//! \param[in] useTruck
	//! \param[in] contextName
	//! \param[in] offset
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
	//!
	//! \param[in] item
	//! \param[in] storageOwner
	void CheckRadioPickup(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		m_Radio = BaseRadioComponent.Cast(item.FindComponent(BaseRadioComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] command
	//! \return
	bool CheckCharacterStance(ECharacterCommandIDs command)
	{
		CharacterAnimationComponent comp = CharacterAnimationComponent.Cast(m_Player.FindComponent(CharacterAnimationComponent));
		bool ret = false;
					
		if (!comp)
			return false;
		
		CharacterMovementState mState = new CharacterMovementState();
		comp.GetMovementState(mState);
		
		if (mState)
			ret = mState.m_CommandTypeId == command;
	
		delete mState;
		return ret;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetFirstRun()
	{
		return m_bIsFirstRun;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] firstRun
	void SetFirstRun(bool firstRun)
	{
		m_bIsFirstRun = firstRun;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsCharacterLeaning()
	{
		SCR_CharacterControllerComponent comp = SCR_CharacterControllerComponent.Cast(m_Player.FindComponent(SCR_CharacterControllerComponent));
		if (!comp)
			return false;
		
		return comp.IsLeaning();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
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
			
			if (m_eStage == SCR_ECampaignTutorialArlandStage.WALL)
				GetGame().GetCallqueue().Remove(DelayedPopup);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetWas3rdPersonViewUsed()
	{
		return m_bUsed3PV;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] vehName
	//! \param[in] seat
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
				AnimateWidget.Opacity(m_wFadeOut, 0, 0.8);
			}
			
			m_bMovedOutOfVehicleByScript = false;
			compartmentAccessComponent.MoveInVehicle(veh, seat);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] seat
	//! \return
	bool CheckCharacterInVehicle(ECompartmentType seat)
	{
		CompartmentAccessComponent compartmentAccessComponent = CompartmentAccessComponent.Cast(m_Player.FindComponent(CompartmentAccessComponent));
				
		if (!compartmentAccessComponent)
			return false;
	
		BaseCompartmentSlot compartmentSlot = compartmentAccessComponent.GetCompartment();
	
		if (!compartmentSlot)
			return false;
		
		return compartmentSlot.GetType() == seat;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicle
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
		
		FinishStage(m_Stage, SCR_ECampaignTutorialArlandStage.CONFLICT_UNLOADING_SUPPLIES);
		m_iActiveStage = 17;
		ResetStage(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] newPos
	//! \param[in] loadoutResourceName
	void MovePlayer(notnull IEntity newPos, ResourceName loadoutResourceName)
	{		
		GetGame().GetCallqueue().CallLater(ResetPlayerCharacter, FADE_SPEED*100, false, newPos, loadoutResourceName);
		
		if (m_wFadeOut)
			AnimateWidget.Opacity(m_wFadeOut, 0, FADE_SPEED);
		}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetPlayerCharacter(IEntity newPos, ResourceName loadoutResourceName)
	{
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		newPos.GetWorldTransform(params.Transform);
		
		Resource res = Resource.Load(loadoutResourceName);
		if (!res.IsValid())
			return;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		IEntity oldCharacter = playerController.GetControlledEntity();
		if (!oldCharacter)
		{
			Print("SCR_CampaignTutorialArlandComponent:: No playercontrolled entity found!", LogLevel.WARNING);
			return;
		}
		
		//Save old identity to keep same face
		Identity oldIdentity;
		SCR_CharacterIdentityComponent identityComp = SCR_CharacterIdentityComponent.Cast(oldCharacter.FindComponent(SCR_CharacterIdentityComponent));
		if (identityComp)
			oldIdentity = identityComp.GetIdentity();
		
		IEntity newCharacter = GetGame().SpawnEntityPrefab(res, null, params);
		if (!newCharacter)
			return;
		
		identityComp = SCR_CharacterIdentityComponent.Cast(newCharacter.FindComponent(SCR_CharacterIdentityComponent));
		if (identityComp && oldIdentity)
			identityComp.SetIdentity(oldIdentity);
		
		m_Player = ChimeraCharacter.Cast(newCharacter);
		playerController.SetControlledEntity(newCharacter);
		
		SCR_EntityHelper.DeleteEntityAndChildren(oldCharacter);
		
		if (!m_Stage)
			return;
		
		m_Stage.SetPlayer(m_Player);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] text
	//! \param[in] subtitle
	//! \param[in] duration
	//! \param[in] param1
	//! \param[in] param2
	//! \param[in] subtitleParam1
	//! \param[in] subtitleParam2
	void DelayedPopup(string text = "", string subtitle = "", float duration = SCR_PopUpNotification.DEFAULT_DURATION, string param1 = "", string param2 = "", string subtitleParam1 = "", string subtitleParam2 = "")
	{
		SCR_PopUpNotification.GetInstance().PopupMsg(text, duration, text2: subtitle, param1: param1, param2: param2, text2param1: subtitleParam1, text2param2: subtitleParam2, category: SCR_EPopupMsgFilter.TUTORIAL);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	//! \param[in] structure
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
			m_HQUS = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("MainBaseHQ").FindComponent(SCR_CampaignMilitaryBaseComponent));
			m_HQUS.AlterSupplyIncomeTimer(9999999);
			FiringRangeInit();
			MobileHQInit();
			IEntity WP_cottage = GetGame().GetWorld().FindEntityByName("WP_CONFLICT_COMPAS_MOVE");
			IEntity WP_mobileHQ = GetGame().GetWorld().FindEntityByName("WP_CONFLICT_MAP_MOVE");
			SCR_MapDescriptorComponent cottageDescr = SCR_MapDescriptorComponent.Cast(WP_cottage.FindComponent(SCR_MapDescriptorComponent));
			SCR_MapDescriptorComponent mobileTruckDescr = SCR_MapDescriptorComponent.Cast(WP_mobileHQ.FindComponent(SCR_MapDescriptorComponent));
			cottageDescr.Item().SetVisible(false);
			mobileTruckDescr.Item().SetVisible(false);

			m_wFadeOut = ImageWidget.Cast(GetGame().GetHUDManager().CreateLayout("{265245C299401BF6}UI/layouts/Menus/ContentBrowser/DownloadManager/ScrollBackground.layout", EHudLayers.OVERLAY));
			m_wFadeOut.SetOpacity(1);
			int stage = m_eStage;
			SetActiveConfig(0);
#ifdef WORKBENCH
			stage = m_eStartingStage;
#endif
			//TODO: @yuri pls fix for the future, very dirty indeed.
			GetGame().GetCallqueue().CallLater(Check3rdPersonViewUsed, 500, true);
			
#ifdef ENABLE_DIAG
			SetEventMask(m_CampaignGamemode, EntityEvent.FRAME);
#endif
		}
		else
			SetStage(m_eStage);
		
		ResetStage();
	}

	//------------------------------------------------------------------------------------------------	
	//!
	void CreateWaypoint()
	{
		Widget waypointFrame = GetGame().GetHUDManager().CreateLayout("{825C6D728AC3E029}UI/layouts/Tutorial/TutorialWaypointEdited.layout", EHudLayers.BACKGROUND);
		m_aWaypoints.Insert(waypointFrame.FindAnyWidget("RootTutorialHud"));
		
		//setup base image
		ImageWidget image = ImageWidget.Cast(waypointFrame.FindAnyWidget("Icon"));
		image.SetOpacity(0);
		image.LoadImageFromSet(0, m_WaypointWidget, "MISC");
		image.SetColor(Color.FromInt(Color.YELLOW));
		
		//setup distance
		RichTextWidget text = RichTextWidget.Cast(waypointFrame.FindAnyWidget("Distance"));
		text.SetOpacity(0);
		text.SetColor(Color.FromInt(Color.YELLOW));
		
		//setup misc
		image = ImageWidget.Cast(waypointFrame.FindAnyWidget("IconExtra"));
		image.SetOpacity(0);
		image.SetVisible(false);
		image.LoadImageFromSet(0, m_WaypointWidget, "CUSTOM");
		image.SetColor(Color.FromInt(Color.YELLOW));
	}
	
	//------------------------------------------------------------------------------------------------	
	//!
	void FlushWaypoints()
	{
		foreach (Widget waypoints : m_aWaypoints)
		{
			waypoints.RemoveFromHierarchy();
		}

		m_aWaypoints.Clear();
	}	
	
	//------------------------------------------------------------------------------------------------	
	//! \param[in] imageName
	//! \param[in] visible
	//! \param[in] index
	void SetWaypointMiscImage(string imageName, bool visible, int index = 0)
	{
		ImageWidget waypointMisc;
		
		if (m_aWaypoints.IsIndexValid(index))
		{
			waypointMisc = ImageWidget.Cast(m_aWaypoints[index].FindAnyWidget("IconExtra"));
			waypointMisc.LoadImageFromSet(0, m_WaypointWidget, imageName);
			waypointMisc.SetVisible(visible);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] stage
	void SetResumeStage(SCR_ECampaignTutorialArlandStage stage)
	{
		IEntity playerPos;
		typename stages = SCR_ECampaignTutorialArlandStage;
		
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
	//! \return
	SCR_ECampaignTutorialArlandStage GetStage()
	{
		return m_eStage;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetStageDuration()
	{
		return GetGame().GetWorld().GetWorldTime() - m_fStageTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] stage
	//! \param[in] nextStage
	void FinishStage(SCR_BaseCampaignTutorialArlandStage stage, SCR_ECampaignTutorialArlandStage nextStage = -1)
	{	
		SetPlayerStanceToCheck(-1);
		m_iCountOfHits = 0;
		m_bCheckLeaning = false;
		m_eWaypointTruckPosition = SCR_ETutorialArlandSupplyTruckWaypointMode.NONE;

		if (GetStage() == SCR_ECampaignTutorialArlandStage.END_STAGE)
		{
			SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(m_pGameMode);
			SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, winnerFactionId: 1);
			gamemode.EndGameMode(endData)
		}
		
		FlushWaypoints();
		stage.FlushWaypoints();
		
		delete stage;
		if (nextStage == -1)
		{
			m_iActiveStage++;
			if(m_aStageInfos.IsIndexValid(m_iActiveStage))
				SetStage(m_aStageInfos[m_iActiveStage].GetIndex());
			else
				SetActiveConfig(0);
		}
		else
		{
			SetStage(nextStage);
	}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] stage
	//! \param[in] lastStage
	void SetStage(SCR_ECampaignTutorialArlandStage stage, SCR_ECampaignTutorialArlandStage lastStage = 0)
	{	
		m_eStage = stage;
		if(lastStage != 0)
			m_eLastStage = lastStage;
		
#ifdef ENABLE_DIAG
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, m_eStage);
#endif
		
		if (!m_Player)
			return;
		
		// Run the next stage
		if(!m_aStageInfos)
		{
			SetActiveConfig(0);
			return;
		}
		
		foreach (SCR_CampaignTutorialArlandStageInfo stageInfo : m_aStageInfos)
		{
			if (stageInfo.GetIndex() == m_eStage)
			{
				m_fStageTimestamp = GetGame().GetWorld().GetWorldTime();
				m_Stage = SCR_BaseCampaignTutorialArlandStage.Cast(GetGame().SpawnEntity(stageInfo.GetClassName().ToType()));
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ResetPlayerPosition()
	{
		IEntity playerPos;
		typename stages = SCR_ECampaignTutorialArlandStage;
		int stage = m_eStage;
		
		// If the current stage does not have its starting location set, use the previous one
		while (!playerPos && stage >= 0)
		{
			playerPos = GetGame().GetWorld().FindEntityByName(string.Format("PP_%1", stages.GetVariableName(stage)));
			stage--;
		}
		
		if (playerPos)
		{
			if (m_wFadeOut)
			{
				//m_wFadeOut.SetOpacity(1);
				AnimateWidget.Opacity(m_wFadeOut, 1, 4);
			}
			
			GetGame().GetCallqueue().CallLater(MovePlayer, 500, false, playerPos, m_ActiveConfig.GetPlayerLoadout());
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hide
	void ToggleWaypoints(bool hide)
	{
		foreach(Widget waypoint: m_aWaypoints)
		{
			waypoint.SetVisible(hide);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] waypoints
	//! \param[in] heightOffset
	void UpdateWaypoints(array<IEntity> waypoints, float heightOffset)
	{
		int x = 0;
		if (waypoints.IsEmpty())
			return;

		foreach (IEntity waypoint : waypoints)
		{
			if (!waypoint)
				continue;

			vector WPPos = waypoint.GetOrigin();
			ImageWidget markerPos;
			ImageWidget markerExtra;
			RichTextWidget markerDistance;
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
			
			FrameSlot.SetPos(m_aWaypoints[x], pos[0], pos[1]);
			
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
			
			markerPos = ImageWidget.Cast(m_aWaypoints[x].FindAnyWidget("Icon"));
			markerExtra = ImageWidget.Cast(m_aWaypoints[x].FindAnyWidget("IconExtra"));
			markerDistance = RichTextWidget.Cast(m_aWaypoints[x].FindAnyWidget("Distance"));
			
			markerPos.SetOpacity(opacity);
			markerExtra.SetOpacity(opacity);
			markerDistance.SetOpacity(distanceOpacity);
			markerDistance.SetTextFormat("#AR-Tutorial_WaypointUnits_meters", distShown);
			x++;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_CampaignSuppliesComponent GetSupplyTruckComponent()
	{
		return m_SupplyTruckComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] ent
	//! \return
	bool FindSupplyTruck(IEntity ent)
	{
		SCR_CampaignSuppliesComponent supplyTruckComponent = SCR_CampaignSuppliesComponent.Cast(ent.FindComponent(SCR_CampaignSuppliesComponent));
		
		if (!supplyTruckComponent)
			return true;
		
		Vehicle truck = Vehicle.Cast(ent.GetParent());
		
		if (!truck)
			return true;
		
		m_SupplyTruckComponent = supplyTruckComponent;
		truck.SetName("BuildingSupplyTruck");
		return false;
	}
		
	//------------------------------------------------------------------------------------------------
	protected void SetupStageCounts(int stage = 0)
	{
		array<ref SCR_CampaignTutorialArlandStageInfo> stagelist = {};
				
		m_ActiveConfig.GetStages(stagelist);
		m_iStagesCnt = stagelist.Count();
			
#ifdef ENABLE_DIAG
		if (m_eStage > m_iStagesCnt - 1)
		{
			int diff = m_eStage - m_iStagesCnt - 1;
			int numStage = m_eStage - diff;		
		}		
		
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, "", "Stage", "Tutorial", string.Format("0 %1 0 1", m_iStagesCnt - 1));
#endif			
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		m_CampaignGamemode = SCR_GameModeCampaign.GetInstance();
		
		if (!m_CampaignGamemode)
			return;
		
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
		DiagMenu.RegisterItem(SCR_DebugMenuID.DEBUGUI_TUTORIAL_CONFIG, "", "Config", "Tutorial", "Hub,Movement,Weapons,ConflictSimple,ConflictSige,Driving");
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, "", "Stage", "Tutorial", string.Format("0 %1 0 1", m_iStagesCnt - 1));
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, m_eStage);
#endif
		
		Resource holder = BaseContainerTools.LoadContainer(m_sStagesConfigAdvanced);
		if (!holder)
			return;
		
		BaseContainer container = holder.GetResource().ToBaseContainer();
		if (!container)
			return;
		
		SCR_CampaignTutorialArlandStagesConfig configsConf = SCR_CampaignTutorialArlandStagesConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!configsConf)
			return;
		
		configsConf.GetConfigs(m_aStageConfigs);
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		core.Event_OnEditorManagerCreatedServer.Insert(SetupEditorModeListener);
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
	//!
	void CleanUpWaypoints()
	{
		if (m_aWaypoints.IsEmpty())
			return;

		foreach (Widget waypoint : m_aWaypoints)
		{
			if (!waypoint)
				continue;

			waypoint.RemoveFromHierarchy();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CampaignTutorialArlandComponent()
	{	
		if (m_wFadeOut)
			m_wFadeOut.RemoveFromHierarchy();
		
		CleanUpWaypoints();
		
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
}