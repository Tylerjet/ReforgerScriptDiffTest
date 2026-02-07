//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialComponentClass : SCR_BaseGameModeComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialComponent : SCR_BaseGameModeComponent
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECampaignTutorialStage))]
	protected SCR_ECampaignTutorialStage m_eStartingStage;
	
	protected static const float STAGE_EVALUATION_PERIOD = 0.25;
	protected static const float DEFAULT_WAYPOINT_OFFSET = 1;
	protected static const float DEFAULT_AREA_RADIUS = 1;
	protected static const int TARGETS_SEARCH_DISTANCE = 150;
	protected static const int SLOTS_SEARCH_DISTANCE = 100;
	protected static const int DESIRED_FREQUENCY = 60000;
	protected static const int INVALID_STANCE = -1;
	
	protected static const vector BACK_DOOR_OFFSET = {0,-0.7,0};
	
	protected ImageWidget m_wFadeOut;
	protected SCR_GameModeCampaignMP m_CampaignGamemode;
	protected bool m_bPlayerSpawned;
	protected SCR_ECampaignTutorialStage m_eStage = SCR_ECampaignTutorialStage.START;
	protected SCR_ECampaignTutorialStage m_eLastStage;
	protected float m_fStageEvaluationTimer;
	protected float m_fAreaRadius;
	protected IEntity m_CurrentWaypoint;
	protected bool m_bIsPlayerInArea;
	protected ChimeraCharacter m_Player;
	protected bool m_bIsStageComplete;
	protected int m_iStagesCnt;
	protected float m_fEvaluationPeriod;
	protected float m_fWaypointHeight;
	protected ImageWidget m_wWaypoint;
	protected RichTextWidget m_wWaypointDistance;
	protected bool m_bFirstStageSet;
	protected bool m_bHasM16Mag;
	protected bool m_bShowWaypoint;
	protected bool m_bCheckWaypointReached;
	protected bool m_bShowOnetimeHintBunkerRotation;
	protected Vehicle m_Jeep;
	protected ref array<SCR_FiringRangeTarget> m_aFiringRangeTargets = {};
	protected float m_fStageTimer;
	protected float m_fStageDuration;
	protected SCR_CampaignSuppliesComponent m_SupplyTruckComponent;
	protected int m_iCountOfHits;
	protected SCR_CampaignBase m_HQUS;
	protected SCR_CampaignBuildingControllerComponent m_BuildingControllerComponent;
	protected bool m_bTaskMenuOpened = false;
	protected bool m_bRadioGrabbed = false;
	protected bool b_mapPositionSelected = false;
	protected BaseRadioComponent m_Radio;
	protected InputManager m_InputManager;		
	protected float m_fSavedTime;
	protected AudioHandle m_PlayedRadio;
	protected bool m_bIsVOIPDone;
	protected bool m_bIsMapOpen = false;
	protected bool m_bUsed3PV = false;
	protected ECharacterStance m_ePlayerStance = INVALID_STANCE;
	protected bool m_bCheckLeaning = false;
	protected SCR_CampaignMobileAssemblyComponent m_MobileAssemblyComponent;
	protected bool m_bMovedOutOfVehicleByScript = false;
	protected float m_fStageDelay;
	protected bool m_fDelayedControlSchemeChangeRunning = false;
	protected int m_iLastControlSchemeUsed;
	
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
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceChanged);
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_TUTORIAL_MENU, "Tutorial", "");
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, "", "Stage", "Tutorial", string.Format("0 %1 0 1", m_iStagesCnt - 1));
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, m_eStage);
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		super.OnPlayerKilled(playerId, player, killer);
		
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		if (!playerController)
			return;
		
		GetGame().GetCallqueue().CallLater(TryPlayerSpawn, 25, true, playerController);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		if (!SCR_HintManagerComponent.GetInstance().IsShown())
			return;
		
		if (newDevice != EInputDeviceType.KEYBOARD && newDevice != EInputDeviceType.GAMEPAD && newDevice != EInputDeviceType.MOUSE)
			return;
		
		m_iLastControlSchemeUsed = newDevice;
		
		if (m_fDelayedControlSchemeChangeRunning)
			return;
		
		m_fDelayedControlSchemeChangeRunning = true;
		GetGame().GetCallqueue().CallLater(OnInputDeviceChangedDelayed, 250);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInputDeviceChangedDelayed()
	{
		m_fDelayedControlSchemeChangeRunning = false;
		bool switchedToKeyboard = (m_iLastControlSchemeUsed == EInputDeviceType.KEYBOARD || m_iLastControlSchemeUsed == EInputDeviceType.MOUSE);
		
		switch (m_eStage)
		{
			case SCR_ECampaignTutorialStage.WIREMESH:
			{
				if (switchedToKeyboard)
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Crawling" + CreateString("#AR-Keybind_Prone", "CharacterProne", "CharacterStandCrouchToggle"), duration: -1);
				else
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Crawling" + CreateString("#AR-Keybind_Prone", "CharacterProne", "MenuBack"), duration: -1);
				break;
			}
			
			case SCR_ECampaignTutorialStage.LADDER_DOWN:
			{
				if (switchedToKeyboard)
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_LadderDown" + CreateString("#AR-Keybind_SpeedAnalog","CharacterSpeedAnalog"), duration: -1, isSilent: true);	
				else
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_LadderDown", duration: -1, isSilent: true);
				
				break;
			}
			
			case SCR_ECampaignTutorialStage.SHOOTING:
			{
				string custommsg;
				
				if (switchedToKeyboard)
					custommsg = CreateString("#AR-Keybind_Aim","CharacterWeaponADS");
				else
					custommsg = CreateString("#AR-Keybind_Aim","CharacterWeaponADSHold");
				
				custommsg = custommsg + CreateString("#AR-Keybind_Fire","CharacterFire");
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Firing" + custommsg, duration: -1, isSilent: true);
				break;
			}
			
			case SCR_ECampaignTutorialStage.SHOOTING_LEANING:
			{
				string custommsg;
				
				if (switchedToKeyboard)
					custommsg = CreateString("#AR-Keybind_Lean","CharacterLean");
				else
					custommsg = CreateString("#AR-Keybind_Lean","CharacterLeanAnalog");
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Leaning" + custommsg, duration: -1, isSilent: true);
				break;
			}
			
			case SCR_ECampaignTutorialStage.SHOOTING_PRONE:
			{
				if (switchedToKeyboard)
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringProne" + CreateString("#AR-Keybind_Prone", "CharacterProne", "CharacterStandCrouchToggle"), duration: -1);
				else
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringProne" + CreateString("#AR-Keybind_Prone", "CharacterProne", "MenuBack"), duration: -1);
				break;
			}
			
			case SCR_ECampaignTutorialStage.FIREPOZ_2:
			{
				if (switchedToKeyboard)
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringPosition2" + CreateString("#AR-Keybind_Prone", "CharacterProne", "CharacterStandCrouchToggle"), duration: -1);
				else
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringPosition2" + CreateString("#AR-Keybind_Prone", "CharacterProne", "MenuBack"), duration: -1);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_1:
			{
				string custommsg;
				if(GetGame().GetInputManager().IsUsingMouseAndKeyboard())
					custommsg = CreateString("#AR-Keybind_Freelook","Freelook");
				else
					custommsg = CreateString("#AR-Keybind_Freelook","MenuEnable");
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SwitchingSeats" + custommsg, duration: -1);
				break;
			}
			
			case SCR_ECampaignTutorialStage.MAP_INFO:
			{
				string custommsg;
				
				if (switchedToKeyboard)
					custommsg = CreateString("#AR-Keybind_Map","GadgetMap");
				else
					custommsg = CreateString("#AR-ControlsHint_OpenQuickSlot","Inventory_WeaponSwitching") + CreateString("#AR-Keybind_Map","SpawnPointPrev");
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MapControls" + custommsg, duration: -1, isSilent: true);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_TOUR_GARAGE:
			{
				string custommsg;
				
				if (switchedToKeyboard)
					custommsg = CreateString("#AR-Keybind_MapZoom","MapZoom","BuildingPreviewRotationUp") + CreateString("#AR-Keybind_MapPan", "MapPanDrag");
				else
					custommsg = CreateString("#AR-Keybind_MapZoom","MapZoom") + CreateString("#AR-Keybind_MapPan","MapPanVGamepad", "MapPanHGamepad");
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MapInfo" + custommsg, duration: -1, isSilent: true);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_SHOW_COMPAS:
			{	
				string custommsg;
				
				if (switchedToKeyboard)
					custommsg = CreateString("#AR-Keybind_Compass","GadgetCompass");
				else
					custommsg = CreateString("#AR-ControlsHint_OpenQuickSlot","Inventory_WeaponSwitching") + CreateString("#AR-Keybind_Compass","SpawnPointNext");
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Compass" + custommsg, duration: -1, isSilent: true);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_VOIP_USE:
			{
				string custommsg;
				
				if (switchedToKeyboard)
					custommsg = CreateString("#AR-Keybind_VONChannel","VONChannel");
				else
					custommsg = CreateString("#AR-Keybind_VONChannel","VONGamepad");
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Transmission" + custommsg, duration: -1, isSilent: true);
				break;
			}
		}
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
	void BuildingInit()
	{
		// Building part preparation Add aditional resources to a supply truck
		IEntity truck = IEntity.Cast(GetGame().GetWorld().FindEntityByName("EastSupplyTruck"));	
		if (truck)
		{
			array<EntitySlotInfo> slotInfos = {};
			EntitySlotInfo.GetSlotInfos(truck, slotInfos);
			foreach (EntitySlotInfo slotInfo: slotInfos)
			{
				IEntity slotEntity = slotInfo.GetAttachedEntity();
				if (!slotEntity)
					continue;
				
				SCR_CampaignSuppliesComponent supplyComp = SCR_CampaignSuppliesComponent.Cast(slotEntity.FindComponent(SCR_CampaignSuppliesComponent));
				if (supplyComp)
				{
					supplyComp.AddSupplies(500);
					continue;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetStage(bool forcePlayerReset = false)
	{
		if (!m_Player.IsInVehicle() || forcePlayerReset)
			ResetPlayerPosition();
		
		switch (m_eStage)
		{
			case SCR_ECampaignTutorialStage.MAGAZINE_PICK:;
			case SCR_ECampaignTutorialStage.WEAPON_RELOAD:;
			case SCR_ECampaignTutorialStage.FIREPOZ_1:;
			case SCR_ECampaignTutorialStage.SHOOTING:;
			case SCR_ECampaignTutorialStage.SHOOTING_CROUCH:;
			case SCR_ECampaignTutorialStage.SHOOTING_PRONE:;
			case SCR_ECampaignTutorialStage.FIREPOZ_2:;
			case SCR_ECampaignTutorialStage.SHOOTING_LEANING:
			{
				SpawnAsset("M16", "{3E413771E1834D2F}Prefabs/Weapons/Rifles/M16/Rifle_M16A2.et");
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_1:;
			case SCR_ECampaignTutorialStage.DRIVING_2:;
			case SCR_ECampaignTutorialStage.DRIVING_3:;
			case SCR_ECampaignTutorialStage.DRIVING_4:
			{
				SpawnAsset("Jeep", "{5168FEA3054D6D15}Prefabs/Vehicles/Wheeled/M151A2/M151A2_M2HB_MERDC.et", m_CurrentWaypoint.GetName());
				MoveInVehicle("Jeep", ECompartmentType.Pilot);
				break;
			}
			case SCR_ECampaignTutorialStage.DRIVING_5:;
			case SCR_ECampaignTutorialStage.DRIVING_6:;
			case SCR_ECampaignTutorialStage.DRIVING_7:;
			case SCR_ECampaignTutorialStage.DRIVING_8:;
			case SCR_ECampaignTutorialStage.DRIVING_9:
			{
				SpawnAsset("Jeep", "{5168FEA3054D6D15}Prefabs/Vehicles/Wheeled/M151A2/M151A2_M2HB_MERDC.et", m_CurrentWaypoint.GetName());
				MoveInVehicle("Jeep", ECompartmentType.Pilot);
				SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
				if (accessComp)
				{
					accessComp.GetOnCompartmentLeft().Remove(OnJeepLeft);
					accessComp.GetOnCompartmentLeft().Insert(OnJeepLeft);
				}
				
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_SUPPLY_DEPOT:
			{
				SpawnAsset("SupplyTruck", "{F37113A988304565}Prefabs/MP/Campaign/Assets/CampaignSupplyTruckWest.et");
				SupplyTruckInit();
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_LOADING_SUPPLIES: {
				SpawnAsset("SupplyTruck", "{F37113A988304565}Prefabs/MP/Campaign/Assets/CampaignSupplyTruckWest.et", "SupplyTruckDepotPos");
				SupplyTruckInit();
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_10:;
			case SCR_ECampaignTutorialStage.DRIVING_11:;
			case SCR_ECampaignTutorialStage.DRIVING_12:;
			case SCR_ECampaignTutorialStage.DRIVING_13:;
			case SCR_ECampaignTutorialStage.DRIVING_14:;
			case SCR_ECampaignTutorialStage.DRIVING_15:
			{
				SpawnAsset("SupplyTruck", "{F37113A988304565}Prefabs/MP/Campaign/Assets/CampaignSupplyTruckWest.et", m_CurrentWaypoint.GetName());
				SupplyTruckInit();
				m_SupplyTruckComponent.AddSupplies(1000);
				MoveInVehicle("SupplyTruck", ECompartmentType.Pilot);
				SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
				if (accessComp)
				{
					accessComp.GetOnCompartmentLeft().Remove(OnSupplyTruckLeft);
					accessComp.GetOnCompartmentLeft().Insert(OnSupplyTruckLeft);
				}
				
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_UNLOADING_SUPPLIES:
			{
				SpawnAsset("SupplyTruck", "{F37113A988304565}Prefabs/MP/Campaign/Assets/CampaignSupplyTruckWest.et", "WP_DRIVING_15");
				SupplyTruckInit();
				m_SupplyTruckComponent.AddSupplies(1000);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_START:;
			case SCR_ECampaignTutorialStage.CONFLICT_BUILD_SERVICE:;
			case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_LOAD_SUPPLY:;
			case SCR_ECampaignTutorialStage.CONFLICT_BOARD_TRUCK:
			{
				SpawnAsset("SupplyTruck", "{F37113A988304565}Prefabs/MP/Campaign/Assets/CampaignSupplyTruckWest.et", "WP_DRIVING_15");
				SupplyTruckInit();
				m_SupplyTruckComponent.AddSupplies(500);
				SCR_CampaignBase baseChotain = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain"));
				
				if (baseChotain && baseChotain.GetSupplies() < 800)
					baseChotain.AddSupplies(800 - baseChotain.GetSupplies());
				
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BUILD_BUNKER:;
			case SCR_ECampaignTutorialStage.CONFLICT_TURN_ON_BUILDING:;
			case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_QUIT:;
			case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_TO_FAR:
			{
				SpawnAsset("SupplyTruck", "{F37113A988304565}Prefabs/MP/Campaign/Assets/CampaignSupplyTruckWest.et", "SupplyTruckBunkerPos");
				SupplyTruckInit();
				m_SupplyTruckComponent.AddSupplies(500);
				SCR_CampaignBase baseChotain = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain"));
				
				if (baseChotain && baseChotain.GetSupplies() < 800)
					baseChotain.AddSupplies(800 - baseChotain.GetSupplies());
				
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_CHECKPOINT:;
			case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_QUIT2:;
			case SCR_ECampaignTutorialStage.CONFLICT_TASKS_MENU:;
			case SCR_ECampaignTutorialStage.CONFLICT_TASKS_INFO:;
			case SCR_ECampaignTutorialStage.CONFLICT_VOLUNTEERING:;
			case SCR_ECampaignTutorialStage.CONFLICT_VOLUNTEERING_INFO:
			{
				SpawnAsset("SupplyTruck", "{F37113A988304565}Prefabs/MP/Campaign/Assets/CampaignSupplyTruckWest.et", "SupplyTruckCheckpointPos");
				SupplyTruckInit();
				m_SupplyTruckComponent.AddSupplies(500);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_16:;
			case SCR_ECampaignTutorialStage.DRIVING_17:;
			case SCR_ECampaignTutorialStage.DRIVING_18:
			{
				SpawnAsset("SupplyTruck", "{F37113A988304565}Prefabs/MP/Campaign/Assets/CampaignSupplyTruckWest.et", m_CurrentWaypoint.GetName());
				MoveInVehicle("SupplyTruck", ECompartmentType.Pilot);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BASE_SEIZING_INFO:;
			case SCR_ECampaignTutorialStage.CONFLICT_SHOW_COMPAS:;
			case SCR_ECampaignTutorialStage.CONFLICT_COMPAS_DIRECTION_NORTH:;
			case SCR_ECampaignTutorialStage.CONFLICT_COMPAS_DIRECTION_2:;
			case SCR_ECampaignTutorialStage.CONFLICT_COMPAS_MOVE:;
			case SCR_ECampaignTutorialStage.CONFLICT_MAP_MOVE:;
			case SCR_ECampaignTutorialStage.CONFLICT_RADIO_PICKUP:;
			case SCR_ECampaignTutorialStage.CONFLICT_VOIP_SETUP:;
			case SCR_ECampaignTutorialStage.CONFLICT_VOIP_USE:;
			case SCR_ECampaignTutorialStage.CONFLICT_VOIP_INFO:;
			case SCR_ECampaignTutorialStage.CONFLICT_MHQ_DEPLOY:
			{
				SCR_CampaignBase baseLaruns = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseLaruns"));
				
				if (baseLaruns && !baseLaruns.GetOwningFaction())
				{
					if (baseLaruns.BeginCapture(SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(m_CampaignGamemode.FACTION_BLUFOR)), SCR_CampaignBase.INVALID_PLAYER_INDEX))
						baseLaruns.FinishCapture();
				}
				
				while (m_CampaignGamemode.GetActiveRespawnRadiosCount(m_CampaignGamemode.FACTION_BLUFOR) > 0)
					m_CampaignGamemode.RemoveActiveRespawnRadio(m_CampaignGamemode.FACTION_BLUFOR);
				
				break;
			}
			case SCR_ECampaignTutorialStage.CONFLICT_MHQ_INFO:;
			case SCR_ECampaignTutorialStage.CONFLICT_SEIZE_ENEMY_HQ:
			{
				SCR_CampaignBase baseLaruns = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseLaruns"));
				
				if (baseLaruns && !baseLaruns.GetOwningFaction())
				{
					if (baseLaruns.BeginCapture(SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(m_CampaignGamemode.FACTION_BLUFOR)), SCR_CampaignBase.INVALID_PLAYER_INDEX))
						baseLaruns.FinishCapture();
				}
				
				if (m_MobileAssemblyComponent)
				{
					if (!m_MobileAssemblyComponent.IsDeployed())
						m_MobileAssemblyComponent.Deploy(true);
				}
				
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity SpawnAsset(string name, ResourceName type, string teleportPos = string.Empty)
	{
		string posName;
		
		if (teleportPos != string.Empty)
			posName = teleportPos;
		else
			posName = string.Format("SpawnPos_%1", name);
		
		IEntity spawnpoint = GetGame().GetWorld().FindEntityByName(posName);
		
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
			//GetGame().GetCallqueue().CallLater(SCR_Global.DeleteEntityAndChildren, 250, false, vehOld);
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
	protected void SetPlayerStanceToCheck(ECharacterStance stance)
	{
		m_ePlayerStance = stance;
	}
	
	//------------------------------------------------------------------------------------------------
	ECharacterStance GetPlayerStanceToCheck()
	{
		return m_ePlayerStance;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool DeleteSlots(IEntity slot)
	{
		SCR_SiteSlotEntity slotToCheck = SCR_SiteSlotEntity.Cast(slot);
		if (!slotToCheck)
			return true;
			
		IEntity ent = GetGame().GetWorld().FindEntityByName("buildingSlot");
		if (!ent)
			return true;

		SCR_SiteSlotEntity buildingSlot = SCR_SiteSlotEntity.Cast(ent);
		if (buildingSlot == slotToCheck)
			return true;
		
		SCR_Global.DeleteEntityAndChildren(slotToCheck);
		return true;
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
	protected string CreateString(string description, string keybind, string keybind2 = "")
	{
		string returnString;
		string startColor = "<br/><color rgba='226,168,79,255'><shadow mode='image' color='0,0,0' size='1' offset='1,1' opacity = '0.5'>";
		string endAction = "\"/>";
		string startAction = "<action name=\"";
		string endColor = "</shadow></color>"; 
		
		if(m_eLastStage != m_eStage)
		{
			returnString = "<br/>";
			m_eLastStage = m_eStage;
		}
			
		returnString = returnString + startColor + startAction + keybind + endAction;
		if (keybind2 != "")
			returnString = returnString + " | " + startAction + keybind2 + endAction;
		
		returnString = returnString + endColor + "<b> " + description + " </b>";
		
		return returnString;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);
		
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		// Attempt to spawn the player automatically, cease after spawn is successful in OnPlayerSpawned
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
		
		if (availableSpawnpoints.IsEmpty())
			return;
		
		respawnSystem.DoSetPlayerSpawnPoint(playerId, SCR_SpawnPoint.GetSpawnPointRplId(availableSpawnpoints[0]));
		
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
	void RegisterTasksShown()
	{
		m_bTaskMenuOpened = true;
		GetGame().GetInputManager().RemoveActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateSupplyTruckWaypoint(bool useTruck, string contextName, vector offset = vector.Zero)
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
		m_CurrentWaypoint.SetOrigin(pos);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnVOIPPress()
	{
		IEntity unit = m_Radio.GetOwner().GetParent();
		
		if (unit == m_Player && m_Radio.GetFrequency() == DESIRED_FREQUENCY && m_Radio.IsPowered())
		{
			m_fSavedTime = GetGame().GetWorld().GetWorldTime();
			m_bIsVOIPDone = false;
			PlayRadioMsg();
		}
	}
	//------------------------------------------------------------------------------------------------
	void OnVOIPRelease()
	{
		PlayRadioMsg(true);
		
		if (m_Radio.GetFrequency() == DESIRED_FREQUENCY && m_Radio.IsPowered())
		{
			if (m_fSavedTime + 6000 >= GetGame().GetWorld().GetWorldTime())
				m_bIsVOIPDone = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckRadioPickup(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		BaseRadioComponent comp = BaseRadioComponent.Cast(item.FindComponent(BaseRadioComponent));
		m_Radio = comp;
		
		if (comp)
			m_bRadioGrabbed = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayRadioMsg(bool stop = false)
	{
		AudioSystem.TerminateSound(m_PlayedRadio);
		
		if (stop)
			return;
		
		SCR_CommunicationSoundComponent soundComp = SCR_CommunicationSoundComponent.Cast(m_Player.FindComponent(SCR_CommunicationSoundComponent));
		
		if (!soundComp)
			return;
		
		SignalsManagerComponent signalComp = SignalsManagerComponent.Cast(m_Player.FindComponent(SignalsManagerComponent));
		
		if (!signalComp)
			return;
		
		SCR_CallsignManagerComponent callsignManager = SCR_CallsignManagerComponent.Cast(m_CampaignGamemode.FindComponent(SCR_CallsignManagerComponent));
		
		if (!callsignManager)
			return;
		
		int baseCallsign = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("MainBaseLevie")).GetCallsign();
		
		int signalBase = signalComp.AddOrFindSignal("Base");
		int signalCompanyCaller = signalComp.AddOrFindSignal("CompanyCaller");
		int signalPlatoonCaller = signalComp.AddOrFindSignal("PlatoonCaller");
		int signalSquadCaller = signalComp.AddOrFindSignal("SquadCaller");
		
		int callerCallsignCompany, callerCallsignPlatoon, callerCallsignSquad, callerCallsignCharacter;
		callsignManager.GetEntityCallsignIndexes(m_Player, callerCallsignCompany, callerCallsignPlatoon, callerCallsignSquad, callerCallsignCharacter);
		
		signalComp.SetSignalValue(signalBase, baseCallsign);
		signalComp.SetSignalValue(signalCompanyCaller, callerCallsignCompany);
		signalComp.SetSignalValue(signalPlatoonCaller, callerCallsignPlatoon);
		signalComp.SetSignalValue(signalSquadCaller, callerCallsignSquad);
		
		m_PlayedRadio = soundComp.SoundEvent("SOUND_SL_SRT");
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
			GetGame().GetCallqueue().Remove(DelayedPopup);
		}
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
				WidgetAnimator.PlayAnimation(m_wFadeOut, WidgetAnimationType.Opacity, 0, 1.5);
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
		
		SCR_PopUpNotification.GetInstance().HideCurrentPopupMsg();
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
		if (accessComp)
			accessComp.GetOnCompartmentLeft().Remove(OnSupplyTruckLeft);
		
		SetStage(SCR_ECampaignTutorialStage.CONFLICT_UNLOADING_SUPPLIES);
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
		
		SCR_PopUpNotification.GetInstance().HideCurrentPopupMsg();
		GetGame().GetCallqueue().Remove(SCR_HintManagerComponent.ShowCustomHint);
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
		if (accessComp)
			accessComp.GetOnCompartmentLeft().Remove(OnJeepLeft);
		
		SetStage(SCR_ECampaignTutorialStage.CONFLICT_TOUR_ARMORY);
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
			WidgetAnimator.PlayAnimation(m_wFadeOut, WidgetAnimationType.Opacity, 0, 1.5);
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
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		m_bPlayerSpawned = true;
		m_Player = ChimeraCharacter.Cast(controlledEntity);
		GetGame().GetCallqueue().Remove(TryPlayerSpawn);
		
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
			
			m_InputManager = GetGame().GetInputManager();
			m_wFadeOut = ImageWidget.Cast(GetGame().GetHUDManager().CreateLayout("{265245C299401BF6}UI/layouts/Menus/ContentBrowser/DownloadManager/ScrollBackground.layout", EHudLayers.OVERLAY));
			m_wFadeOut.SetOpacity(0);
			int stage = m_eStage;
#ifdef WORKBENCH
			stage = m_eStartingStage;
#endif
			SetStage(stage);
			GetGame().GetCallqueue().CallLater(Check3rdPersonViewUsed, 500, true);
			SetEventMask(m_CampaignGamemode, EntityEvent.FRAME);
		}
		else
			SetStage(m_eStage);
		
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
				m_wWaypoint.LoadImageFromSet(0, m_CampaignGamemode.GetBuildingIconsImageset(), "USSR_Base_Main_Select");
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
	void SetStage(SCR_ECampaignTutorialStage stage)
	{
		m_eStage = stage;
		m_bIsStageComplete = false;
		m_bIsPlayerInArea = false;
		m_fStageEvaluationTimer = 0;
		m_fAreaRadius = 0;
		m_fStageTimer = 0;
		typename stages = SCR_ECampaignTutorialStage;
		m_CurrentWaypoint = GetGame().GetWorld().FindEntityByName(string.Format("WP_%1", stages.GetVariableName(m_eStage)));
		
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
		
		PrintFormat("Setting Stage %1 (%2/%3)", stages.GetVariableName(m_eStage), m_eStage, m_iStagesCnt - 1);
		
		if (!m_Player)
			return;
		
		// Custom stage setup, followed by common StartStageEvaluation method
		switch (m_eStage)
		{
			case SCR_ECampaignTutorialStage.START:
			{
				GetGame().GetCallqueue().CallLater(DelayedPopup, 2000, false, "#AR-Tutorial_ScenarioName-UC", "#AR-Tutorial_ScenarioDescription", 6, "", "", "", "");
				StartStageEvaluation(duration: 9); break;
			}
			
			case SCR_ECampaignTutorialStage.ZIGZAG_1:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_ObstacleCourseStart" + CreateString("#AR-Keybind_Move", "CharacterForward", "CharacterRight") + CreateString("#AR-KeybindEditor_LookRL","MouseX","MouseY"), duration: -1);
				
				StartStageEvaluation(); break;
				
			}
			
			case SCR_ECampaignTutorialStage.OVERPASS:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Scaffolding", duration: -1);
				StartStageEvaluation(0.5, 0.1);
				break;
			}
			
			case SCR_ECampaignTutorialStage.WIREMESH:
			{
				if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Crawling" + CreateString("#AR-Keybind_Prone", "CharacterProne", "CharacterStandCrouchToggle"), duration: -1);
				else
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Crawling" + CreateString("#AR-Keybind_Prone", "CharacterProne", "MenuBack"), duration: -1);
				StartStageEvaluation(waypointHeight: 0.2);
				break;
			}
			
			case SCR_ECampaignTutorialStage.VAULT:
			{
				GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowCustomHint, 3000, false, "#AR-Tutorial_Hint_Vaulting" + CreateString("#AR-Keybind_JumpVaultClimb","CharacterJump"), "", -1, false, EFieldManualEntryId.NONE);
				StartStageEvaluation(5, waypointHeight: 0.2);
				break;
			}
			
			case SCR_ECampaignTutorialStage.VAULT2:
			{
				StartStageEvaluation(5, waypointHeight: 0.2);
				break;
			}
			
			case SCR_ECampaignTutorialStage.WINDOW:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Window" + CreateString("#AR-Keybind_JumpVaultClimb","CharacterJump"), duration: -1);
				StartStageEvaluation(5, waypointHeight: 0.6);
				break;
			}
			
			case SCR_ECampaignTutorialStage.FLAG:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Pole", duration: -1);
				StartStageEvaluation(2);
				break;
			}
			
			case SCR_ECampaignTutorialStage.WALL:
			{
				if (!m_bUsed3PV)
					GetGame().GetCallqueue().CallLater(DelayedPopup, 2000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Camera", 7, "", "", "<color rgba='226,168,79,255'><shadow mode='image' color='0,0,0' size='1' offset='1,1' opacity = '0.5'><action name = 'SwitchCameraType'/></shadow></color>", "");
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Walls" + CreateString("#AR-Keybind_JumpVaultClimb","CharacterJump"), duration: -1);
				StartStageEvaluation(5, waypointHeight: 0.3);
				break;
			}
			
			case SCR_ECampaignTutorialStage.WALL2:
			{
				StartStageEvaluation(5, waypointHeight: 0.3);
				GetGame().GetCallqueue().Remove(Check3rdPersonViewUsed);
				break;
			}
			
			case SCR_ECampaignTutorialStage.LADDER_UP:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_LadderUp", duration: -1);
				StartStageEvaluation(3, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.LADDER_PLATFORM:
			{
				StartStageEvaluation(3);
				break;
			}
			
			case SCR_ECampaignTutorialStage.LADDER_DOWN:
			{
				if(GetGame().GetInputManager().IsUsingMouseAndKeyboard())
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_LadderDown" + CreateString("#AR-Keybind_SpeedAnalog","CharacterSpeedAnalog"), duration: -1);	
				else
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_LadderDown", duration: -1);	
				StartStageEvaluation(3, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.LADDER_OFF:
			{
				StartStageEvaluation(3, checkWaypointReached: false, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.OBSTACLE_END:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_ObstacleCourseFinish" + CreateString("#AR-Keybind_SpeedSprint","CharacterSprint") + CreateString("#AR-Keybind_SpeedSprintToggle","CharacterSprintToggle"), duration: -1);
				GetGame().GetCallqueue().CallLater(DelayedPopup, 2000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Sprint", 12, "", "", "", "");
				StartStageEvaluation(2);
				break;
			}
			
			case SCR_ECampaignTutorialStage.WEAPON_PICK:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_WeaponEquip" + CreateString("#AR-WeaponMenu_DescEquip","CharacterAction") + CreateString("#AR-KeybindEditor_MultiSelection","SelectAction"), duration: -1);
				IEntity gun = GetGame().GetWorld().FindEntityByName("M16");
				
				if (gun)
					m_CurrentWaypoint.SetOrigin(gun.GetOrigin());
				
				StartStageEvaluation(waypointHeight: 0, checkWaypointReached: false);
				SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
					if (storageManComp)	
						storageManComp.m_OnItemAddedInvoker.Insert(OnItemAdded);
				break;
			}
			
			case SCR_ECampaignTutorialStage.MAGAZINE_PICK:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MagazinePickup" + CreateString("#AR-Keybind_GadgetToggle","CharacterAction"), duration: -1);
				SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
				
				if (storageManComp)	
				{
					storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
					storageManComp.m_OnItemAddedInvoker.Insert(OnItemAdded);
				}
				
				IEntity ammobox = GetGame().GetWorld().FindEntityByName("Ammobox");
				
				if (ammobox)
					m_CurrentWaypoint.SetOrigin(ammobox.GetOrigin());
				
				StartStageEvaluation(waypointHeight: 0, checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.WEAPON_RELOAD:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MagazineLoading"+ CreateString("#AR-Keybind_Reload","CharacterReload"), duration: -1);
				StartStageEvaluation(20, waypointHeight: 0, checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.FIREPOZ_1:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringPosition1", duration: -1);
				StartStageEvaluation(2, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.SHOOTING:
			{
				foreach (SCR_FiringRangeTarget target : m_aFiringRangeTargets)
				{
					if (target.GetSetDistance() == 5)
					{
						target.SetState(ETargetState.TARGET_UP);
						target.Event_TargetChangeState.Remove(CountTargetHit);
						target.Event_TargetChangeState.Insert(CountTargetHit);
					}
				}
				
				string custommsg;
				
				if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
					custommsg = CreateString("#AR-Keybind_Aim","CharacterWeaponADS");
				else
					custommsg = CreateString("#AR-Keybind_Aim","CharacterWeaponADSHold");
				
				custommsg = custommsg + CreateString("#AR-Keybind_Fire","CharacterFire");
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Firing" + custommsg, duration: -1);
				StartStageEvaluation(20, waypointHeight: 0, checkWaypointReached: false, delay: 1);
				break;
			}
			
			case SCR_ECampaignTutorialStage.SHOOTING_CROUCH:
			{
				m_iCountOfHits = 0;
				SetPlayerStanceToCheck(ECharacterStance.CROUCH);
				foreach (SCR_FiringRangeTarget target : m_aFiringRangeTargets)
				{		
					if (target.GetSetDistance() == 25)
					{
						target.SetState(ETargetState.TARGET_UP);
						target.SetAutoResetTarget(true);
						target.Event_TargetChangeState.Remove(CountTargetHit);
						target.Event_TargetChangeState.Insert(CountTargetHit);
					}
				}
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringCrouch" + CreateString("#AR-Keybind_Crouch", "CharacterStandCrouchToggle"), duration: -1);
				StartStageEvaluation(20, waypointHeight: 0, checkWaypointReached: false, delay: 1);
				break;	
			}
			
			case SCR_ECampaignTutorialStage.SHOOTING_PRONE:
			{
				m_iCountOfHits = 0;
				SetPlayerStanceToCheck(ECharacterStance.PRONE);
				foreach (SCR_FiringRangeTarget target : m_aFiringRangeTargets)
				{		
					if (target.GetSetDistance() == 25)
					{
						target.SetState(ETargetState.TARGET_UP);
						target.SetAutoResetTarget(true);
						target.Event_TargetChangeState.Remove(CountTargetHit);
						target.Event_TargetChangeState.Insert(CountTargetHit);
					}
				}
				
				if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringProne" + CreateString("#AR-Keybind_Prone", "CharacterProne", "CharacterStandCrouchToggle"), duration: -1);
				else
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringProne" + CreateString("#AR-Keybind_Prone", "CharacterProne", "MenuBack"), duration: -1);
				StartStageEvaluation(20, waypointHeight: 0, checkWaypointReached: false, delay: 1);
				break;	
			}
			
			case SCR_ECampaignTutorialStage.FIREPOZ_2:
			{
				if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringPosition2" + CreateString("#AR-Keybind_Prone", "CharacterProne", "CharacterStandCrouchToggle"), duration: -1);
				else
					SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringPosition2" + CreateString("#AR-Keybind_Prone", "CharacterProne", "MenuBack"), duration: -1);
				StartStageEvaluation(2, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.SHOOTING_LEANING:
			{
				m_iCountOfHits = 0;
				SetPlayerStanceToCheck(-1);
				m_bCheckLeaning = true;
				
				foreach (SCR_FiringRangeTarget target : m_aFiringRangeTargets)
				{		
					if (target.GetSetDistance() == 10)
					{
						target.SetState(ETargetState.TARGET_UP);
						target.SetAutoResetTarget(true);
						target.Event_TargetChangeState.Remove(CountTargetHit);
						target.Event_TargetChangeState.Insert(CountTargetHit);
					}
				}
				
				string custommsg;
				if(GetGame().GetInputManager().IsUsingMouseAndKeyboard())
					custommsg = CreateString("#AR-Keybind_Lean","CharacterLean");
				else
					custommsg = CreateString("#AR-Keybind_Lean","CharacterLeanAnalog");
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Leaning" + custommsg, duration: -1);
				StartStageEvaluation(20, waypointHeight: 0, checkWaypointReached: false, delay: 1);
				break;	
			}
			
			case SCR_ECampaignTutorialStage.BOARDING:
			{
				m_Jeep = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("Jeep"));
				m_CurrentWaypoint.SetOrigin(m_Jeep.GetOrigin());
				GetGame().GetCallqueue().CallLater(DelayedPopup, 1000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_WeaponLowering", 12, "", "", "<color rgba='226,168,79,255'><shadow mode='image' color='0,0,0' size='1' offset='1,1' opacity = '0.5'><action name = 'CharacterRaiseWeapon'/></shadow></color>", "");
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Boarding" + CreateString("#AR-Editor_CommandAction_AIWaypoint_GetInNearest_Name","CharacterAction"), duration: -1);
				StartStageEvaluation(checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_0:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringPosition3" + CreateString("#AR-Keybind_Movement", "CarThrust", "CarBrake") + CreateString("#AR-Keybind_Steer", "CarSteering"), duration: -1);
				StartStageEvaluation(5, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.SWITCHING_SEATS:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Turrets" + CreateString("#AR-Editor_CommandAction_AIWaypoint_GetInNearest_Name","CharacterAction") , duration: -1);
				StartStageEvaluation();
				break;
			}
			
			case SCR_ECampaignTutorialStage.VEHICLE_SHOOTING:
			{
				m_iCountOfHits = 0;
				m_bCheckLeaning = false;
				
				foreach (SCR_FiringRangeTarget target : m_aFiringRangeTargets)
				{		
					if (target.GetSetDistance() == 100)
					{
						target.SetState(ETargetState.TARGET_UP);
						target.Event_TargetChangeState.Remove(CountTargetHit);
						target.Event_TargetChangeState.Insert(CountTargetHit);
					}
				}
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringTurret" + CreateString("#AR-Keybind_Fire","CharacterFire"), duration: -1);
				StartStageEvaluation(20, checkWaypointReached: false, showWaypoint: false, delay: 1);
				break;	
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_1:
			{
				string custommsg;
				if(GetGame().GetInputManager().IsUsingMouseAndKeyboard())
					custommsg = CreateString("#AR-Keybind_Freelook","Freelook");
				else
					custommsg = CreateString("#AR-Keybind_Freelook","MenuEnable");
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SwitchingSeats" + custommsg, duration: -1);
				StartStageEvaluation(10, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_2:
			{
				GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.HideHint, 10000, false);
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_3:
			{
				GetGame().GetCallqueue().CallLater(DelayedPopup, 1000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_HintToggle", 12, "", "", "<color rgba='226,168,79,255'><shadow mode='image' color='0,0,0' size='1' offset='1,1' opacity = '0.5'><action name = 'HintToggle'/></shadow></color>", "");
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_4:
			{
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_5:
			{
				SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Tutorial_Popup_Title-UC", 20, text2: "#AR-Tutorial_Popup_SkipStageJeep", category: SCR_EPopupMsgFilter.TUTORIAL);
				GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowCustomHint, 2000, false, "#AR-Tutorial_Hint_Stopping" + CreateString("#AR-Keybind_Exit","GetOut"), "", 20, false, EFieldManualEntryId.NONE);
				SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
				if (accessComp)
				{
					accessComp.GetOnCompartmentLeft().Remove(OnJeepLeft);
					accessComp.GetOnCompartmentLeft().Insert(OnJeepLeft);
				}
				
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_6:
			{
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_7:
			{
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_8:
			{
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_9:
			{
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DISMOUNTING:
			{
				SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
				if (accessComp)
					accessComp.GetOnCompartmentLeft().Remove(OnJeepLeft);
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Dismounting" + CreateString("#AR-Keybind_Exit","GetOut"), duration: -1);
				StartStageEvaluation();
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_TOUR_ARMORY:
			{
				GetGame().GetCallqueue().Remove(SCR_HintManagerComponent.ShowCustomHint);
				GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowCustomHint, 1500, false, "#AR-Tutorial_Hint_MainBase", "", -1, false, EFieldManualEntryId.NONE);
				StartStageEvaluation(5);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_TOUR_VEHICLE:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Armory", "", -1);
				StartStageEvaluation(5);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_TOUR_HQ1:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_VehicleDepot", "", -1);
				StartStageEvaluation(3);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_TOUR_HQ2:
			{
				StartStageEvaluation(2);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_TOUR_HQ3:
			{
				StartStageEvaluation(2);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_HQ_INFO:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_HQ", "", 13, isTimerVisible: true);
				SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
				SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);
				SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
				SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
				StartStageEvaluation(duration: 13);
				break;
			}
			
			case SCR_ECampaignTutorialStage.MAP_INFO:
			{
				string custommsg;
				
				if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
					custommsg = CreateString("#AR-Keybind_Map","GadgetMap");
				else
					custommsg = CreateString("#AR-ControlsHint_OpenQuickSlot","Inventory_WeaponSwitching") + CreateString("#AR-Keybind_Map","SpawnPointPrev");
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MapControls" + custommsg, "", -1);
				StartStageEvaluation();
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_TOUR_GARAGE:
			{
				string custommsg;
				
				if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
					custommsg = CreateString("#AR-Keybind_MapZoom","MapZoom","BuildingPreviewRotationUp") + CreateString("#AR-Keybind_MapPan", "MapPanDrag");
				else
					custommsg = CreateString("#AR-Keybind_MapZoom","MapZoom") + CreateString("#AR-Keybind_MapPan", "MapPanVGamepad", "MapPanHGamepad");
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MapInfo" + custommsg, "", -1);
				SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
				SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);
				StartStageEvaluation(5);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_TOUR_END:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MotorPool", "", 12, isTimerVisible: true);
				StartStageEvaluation(duration: 12);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_REQUESTING_TRUCK:
			{
				m_HQUS.AlterReinforcementsTimer(-float.MAX);
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_RequestingVehicle", duration: -1);
				StartStageEvaluation(checkPeriod: 1, checkWaypointReached: false, waypointHeight: 1.3);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_SUPPLY_DEPOT:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SupplyDepot", duration: -1);
				StartStageEvaluation(10, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_LOADING_SUPPLIES:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SuppliesLoad", duration: -1);
				GetGame().GetCallqueue().CallLater(UpdateSupplyTruckWaypoint, 250, true, false, "door_rear", BACK_DOOR_OFFSET);
				StartStageEvaluation(checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_10:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SuppliesDrive", duration: 10);
				GetGame().GetCallqueue().CallLater(DelayedPopup, 10500, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Reinforcements", 15, "", "", "", "");
				GetGame().GetCallqueue().Remove(UpdateSupplyTruckWaypoint);
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_11:
			{
				SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Tutorial_Popup_Title-UC", 20, text2: "#AR-Tutorial_Popup_SkipStageTruck", category: SCR_EPopupMsgFilter.TUTORIAL);
				SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
				if (accessComp)
				{
					accessComp.GetOnCompartmentLeft().Remove(OnSupplyTruckLeft);
					accessComp.GetOnCompartmentLeft().Insert(OnSupplyTruckLeft);
				}
				
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_12:
			{
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_13:
			{
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_14:
			{
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_15:
			{
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_15:
			{
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_UNLOADING_SUPPLIES:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SuppliesUnload", duration: -1);
				SCR_CampaignBase chotainBase = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain"));
				
				if (chotainBase && chotainBase.GetSupplies() < 350)
					chotainBase.AddSupplies(350 - chotainBase.GetSupplies());
				
				SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
				if (accessComp)
					accessComp.GetOnCompartmentLeft().Remove(OnSupplyTruckLeft);
				
				GetGame().GetCallqueue().CallLater(UpdateSupplyTruckWaypoint, 250, true, false, "door_rear", BACK_DOOR_OFFSET);
				StartStageEvaluation(checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_START:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildModeOpen", duration: -1);
				
				GetGame().GetCallqueue().Remove(UpdateSupplyTruckWaypoint);
				StartStageEvaluation(20, 0.25, waypointHeight: 1, checkWaypointReached: false);
				BuildingInit();
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BUILD_SERVICE:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingDepot", duration: -1);
				GetGame().GetCallqueue().CallLater(DelayedPopup, 4000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Tickets", 10, "", "", "", "");
				StartStageEvaluation(20, 0.25, waypointHeight: 0, checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_LOAD_SUPPLY:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SuppliesLoadTruck", duration: -1);
				GetGame().GetCallqueue().CallLater(UpdateSupplyTruckWaypoint, 250, true, false, "door_rear", BACK_DOOR_OFFSET);
				StartStageEvaluation(20, 0.25, waypointHeight: 0, checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BOARD_TRUCK:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BoardTruck", duration: -1);
				GetGame().GetCallqueue().Remove(UpdateSupplyTruckWaypoint);
				GetGame().GetCallqueue().CallLater(UpdateSupplyTruckWaypoint, 250, true, true, "door_l01");
				StartStageEvaluation(20, 0.25, waypointHeight: 0, checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BUILD_BUNKER:
			{	
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingBunker" + CreateString("#AR-ControlsHint_BuildingCompositionRotate","BuildingPreviewRotationUp","BuildingPreviewRotationDown"), duration: -1);
				GetGame().GetCallqueue().Remove(UpdateSupplyTruckWaypoint);
				GetGame().GetCallqueue().CallLater(DelayedPopup, 20000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_BlockedSlots", 12, "", "", "", "");
				StartStageEvaluation(20, 0.25, waypointHeight: 0, checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_TURN_ON_BUILDING:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingTurnOn", duration: -1);
				GetGame().GetCallqueue().Remove(UpdateSupplyTruckWaypoint);
				GetGame().GetCallqueue().CallLater(UpdateSupplyTruckWaypoint, 250, true, false, "door_rear", BACK_DOOR_OFFSET);
				StartStageEvaluation(20, 0.25, waypointHeight: 0, checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_QUIT:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingCheckpoint", duration: -1);
				StartStageEvaluation(20, 0.25, waypointHeight: 0, checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_TO_FAR:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildModeOpenTruck", duration: -1);
				GetGame().GetCallqueue().CallLater(UpdateSupplyTruckWaypoint, 250, true, false, "door_rear", BACK_DOOR_OFFSET);
				StartStageEvaluation(20, 0.25, waypointHeight: 0, checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_CHECKPOINT:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingCheckpointActual", duration: -1);
				GetGame().GetCallqueue().Remove(UpdateSupplyTruckWaypoint);
				StartStageEvaluation(20, 0.25, waypointHeight: 0, checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_QUIT2:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildModeClose", duration: -1);
				GetGame().GetCallqueue().CallLater(UpdateSupplyTruckWaypoint, 250, true, false, "door_rear", BACK_DOOR_OFFSET);
				StartStageEvaluation(20, 0.25, waypointHeight: 0, checkWaypointReached: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_TASKS_MENU:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_ObjectivesListOpen" + CreateString("#AR-Keybind_Tasks","TasksOpen"), duration: -1);
				GetGame().GetCallqueue().Remove(UpdateSupplyTruckWaypoint);
				GetGame().GetInputManager().AddActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
				StartStageEvaluation();
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_TASKS_INFO:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_ObjectivesListInfo", duration: 12, isTimerVisible: true);
				StartStageEvaluation(duration: 12);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_VOLUNTEERING:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Volunteer", duration: -1);
				StartStageEvaluation();
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_VOLUNTEERING_INFO:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_VolunteerInfo", duration: 15, isTimerVisible: true);
				StartStageEvaluation(duration: 15);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_16:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SeizeBase", duration: -1);
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_17:
			{
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.DRIVING_18:
			{
				StartStageEvaluation(20, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BASE_SEIZING:
			{
				SCR_CampaignBase baseLaruns = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseLaruns"));
				
				if (baseLaruns && m_CurrentWaypoint)
					m_CurrentWaypoint.SetOrigin(baseLaruns.GetHQRadio().GetOrigin());
				
				
				StartStageEvaluation(35, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BASE_NEARBY:
			{
				SCR_CampaignBase baseLaruns = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseLaruns"));
				
				if (baseLaruns && m_CurrentWaypoint)
					m_CurrentWaypoint.SetOrigin(baseLaruns.GetHQRadio().GetOrigin());
				
				foreach (SCR_FiringRangeTarget target : m_aFiringRangeTargets)
				{		
					if (target.GetSetDistance() == 200)
						target.SetState(ETargetState.TARGET_UP);
				}
				
				GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.HideHint, 15000, false);
				GetGame().GetCallqueue().CallLater(DelayedPopup, 15000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Enemies", 12, "", "", "", "");
				StartStageEvaluation(checkWaypointReached: false, waypointHeight: 0);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_BASE_SEIZING_INFO:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_ConflictGoal", duration: 15, isTimerVisible: true);
				GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowCustomHint, 15500, false, "#AR-Tutorial_Hint_NavigationIntro", "", 10, false, EFieldManualEntryId.NONE);
				StartStageEvaluation(duration: 26);
				break;
			}
							
			case SCR_ECampaignTutorialStage.CONFLICT_SHOW_COMPAS:
			{
				string custommsg;
				
				if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
					custommsg = CreateString("#AR-Keybind_Compass","GadgetCompass");
				else
					custommsg = CreateString("#AR-ControlsHint_OpenQuickSlot","Inventory_WeaponSwitching") + CreateString("#AR-Keybind_Compass","SpawnPointNext");
				
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Compass" + custommsg, duration: -1);
				StartStageEvaluation();
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_COMPAS_DIRECTION_NORTH:
			{			
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_CompassNorth", duration: -1);
				StartStageEvaluation(checkPeriod: 1);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_COMPAS_DIRECTION_2:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_CompassAzimuth" + CreateString("#AR-Keybind_CompassADS","GadgetADS"), duration: -1);
				StartStageEvaluation(checkPeriod: 1);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_COMPAS_MOVE:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Farmhouse", duration: -1);
				GetGame().GetCallqueue().CallLater(DelayedPopup, 21000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_NoWaypoints", 15, "", "", "", "");
				StartStageEvaluation(20, 0.25, showWaypoint: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_MAP_MOVE:
			{	
				GenericEntity WP_cottage = GenericEntity.Cast(GetGame().GetWorld().FindEntityByName("WP_CONFLICT_COMPAS_MOVE"));
				GenericEntity WP_mobileHQ = GenericEntity.Cast(GetGame().GetWorld().FindEntityByName("WP_CONFLICT_MAP_MOVE"));
				SCR_MapDescriptorComponent cottageDescr = SCR_MapDescriptorComponent.Cast(WP_cottage.FindComponent(SCR_MapDescriptorComponent));
				SCR_MapDescriptorComponent mobileTruckDescr = SCR_MapDescriptorComponent.Cast(WP_mobileHQ.FindComponent(SCR_MapDescriptorComponent));
				cottageDescr.Item().SetVisible(true);
				mobileTruckDescr.Item().SetVisible(true);
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MHQLocation", duration: -1);
				StartStageEvaluation(20, 0.25, showWaypoint: false);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_RADIO_PICKUP:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_RadioPickup", duration: -1);
				GetGame().GetCallqueue().CallLater(DelayedPopup, 10000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Radios", 10, "", "", "", "");
				SCR_InventoryStorageManagerComponent comp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
				
				if (comp)
				{
					comp.m_OnItemAddedInvoker.Remove(CheckRadioPickup);
					comp.m_OnItemAddedInvoker.Insert(CheckRadioPickup);
				}
				
				while (m_CampaignGamemode.GetActiveRespawnRadiosCount(m_CampaignGamemode.FACTION_BLUFOR) > 0)
					m_CampaignGamemode.RemoveActiveRespawnRadio(m_CampaignGamemode.FACTION_BLUFOR);
				
				StartStageEvaluation();
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_VOIP_SETUP:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_RadioInfo" + CreateString("#AR-Keybind_VONMenu", "VONMenu"), duration: -1);
				StartStageEvaluation();
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_VOIP_USE:
			{
				
				string custommsg;
				if(GetGame().GetInputManager().IsUsingMouseAndKeyboard())
					custommsg = CreateString("#AR-Keybind_VONChannel","VONChannel");
				else
					custommsg = CreateString("#AR-Keybind_VONChannel","VONGamepad");
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Transmission" + custommsg, duration: -1);
				m_InputManager.AddActionListener("VONChannel", EActionTrigger.DOWN, OnVOIPPress);
				m_InputManager.AddActionListener("VONChannel", EActionTrigger.UP, OnVOIPRelease);
				m_InputManager.AddActionListener("VONGamepad", EActionTrigger.DOWN, OnVOIPPress);
				m_InputManager.AddActionListener("VONGamepad", EActionTrigger.UP, OnVOIPRelease);
				StartStageEvaluation();
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_VOIP_INFO:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_TransmissionInfo", duration: 15, isTimerVisible: true);
				m_InputManager.RemoveActionListener("VONChannel", EActionTrigger.DOWN, OnVOIPPress);
				m_InputManager.RemoveActionListener("VONChannel", EActionTrigger.UP, OnVOIPRelease);
				m_InputManager.RemoveActionListener("VONGamepad", EActionTrigger.DOWN, OnVOIPPress);
				m_InputManager.RemoveActionListener("VONGamepad", EActionTrigger.UP, OnVOIPRelease);
				StartStageEvaluation(duration: 15);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_MHQ_DEPLOY:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MHQDeploy", duration: -1);
				StartStageEvaluation();
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_MHQ_INFO:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MHQInfo", duration: 20, isTimerVisible: true);
				StartStageEvaluation(duration: 20);
				break;
			}
			
			case SCR_ECampaignTutorialStage.CONFLICT_SEIZE_ENEMY_HQ:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SeizeHQ", duration: 60);
				m_CampaignGamemode.SetIsTutorial(false);
				StartStageEvaluation();
				break;
			}
			
			default: {StartStageEvaluation(); break;};
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
	void StartStageEvaluation(float areaRadius = DEFAULT_AREA_RADIUS, float checkPeriod = STAGE_EVALUATION_PERIOD, bool showWaypoint = true, bool checkWaypointReached = true, float waypointHeight = DEFAULT_WAYPOINT_OFFSET, float duration = 0, float delay = 0)
	{
		m_fAreaRadius = Math.Pow(areaRadius, 2);
		m_fEvaluationPeriod = checkPeriod;
		m_fWaypointHeight = waypointHeight;
		m_bShowWaypoint = showWaypoint;
		m_bCheckWaypointReached = checkWaypointReached;
		m_fStageDuration = duration;
		m_fStageDelay = delay;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE) != m_eStage)
		{
			SetStage(DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE));
			ResetStage();
		}
#endif
		m_fStageEvaluationTimer += timeSlice;
		m_fStageTimer += timeSlice;
		
		// Render waypoint if allowed
		if (m_wWaypoint)
		{
			if (m_CurrentWaypoint && m_bShowWaypoint)
			{
				vector WPPos = m_CurrentWaypoint.GetOrigin();
				WPPos[1] = WPPos[1] + m_fWaypointHeight;
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
				m_wWaypoint.SetOpacity(1);
				m_wWaypointDistance.SetTextFormat("#AR-Tutorial_WaypointUnits_meters", dist);
				m_wWaypointDistance.SetOpacity(1);
			}
			else
			{
				m_wWaypoint.SetOpacity(0);
				m_wWaypointDistance.SetOpacity(0);
			}
		}
		
		// Stage evaluation
		if (m_fStageEvaluationTimer < m_fEvaluationPeriod)
			return;
		
		m_fStageEvaluationTimer = 0;
		bool waypointReached = true;
		bool stageComplete = false;
		
		if (!m_Player)
			return;
		
		if (m_bIsStageComplete)
			return;
		
		if (m_CurrentWaypoint && m_fAreaRadius != 0 && m_bCheckWaypointReached)
			waypointReached = vector.DistanceSq(m_Player.GetOrigin(), m_CurrentWaypoint.GetOrigin()) <= m_fAreaRadius;
		
		// If stage duration is defined, check the time elapsed
		if (m_fStageDuration != 0)
		{
			m_fStageTimer += timeSlice;
			stageComplete = m_fStageTimer >= m_fStageDuration;
		}
		else
			switch (m_eStage)	// Otherwise, additional custom condition can be defined here
			{
				case SCR_ECampaignTutorialStage.WIREMESH:
				{
					CharacterControllerComponent comp = CharacterControllerComponent.Cast(m_Player.FindComponent(CharacterControllerComponent));
					
					if (comp)
						stageComplete = comp.GetStance() == ECharacterStance.PRONE;
					
					break;
				}
			
				case SCR_ECampaignTutorialStage.VAULT:
				{
					stageComplete = CheckCharacterStance(ECharacterCommandIDs.CLIMB);
					break;
				}
			
				case SCR_ECampaignTutorialStage.VAULT2:
				{
					stageComplete = CheckCharacterStance(ECharacterCommandIDs.CLIMB);
					break;
				}
			
				case SCR_ECampaignTutorialStage.WINDOW:
				{
					stageComplete = CheckCharacterStance(ECharacterCommandIDs.CLIMB);
					break;
				}
			
				case SCR_ECampaignTutorialStage.WALL:
				{
					stageComplete = CheckCharacterStance(ECharacterCommandIDs.CLIMB);
					break;
				}
			
				case SCR_ECampaignTutorialStage.WALL2:
				{
					stageComplete = CheckCharacterStance(ECharacterCommandIDs.CLIMB);
					break;
				}
			
				case SCR_ECampaignTutorialStage.LADDER_UP:
				{
					stageComplete = CheckCharacterStance(ECharacterCommandIDs.LADDER);
					break;
				}
			
				case SCR_ECampaignTutorialStage.LADDER_DOWN:
				{
					stageComplete = CheckCharacterStance(ECharacterCommandIDs.LADDER);
					break;
				}
			
				case SCR_ECampaignTutorialStage.LADDER_OFF:
				{
					stageComplete = !CheckCharacterStance(ECharacterCommandIDs.LADDER) && vector.DistanceSq(m_Player.GetOrigin(), m_CurrentWaypoint.GetOrigin()) < 2;
					break;
				}
			
				case SCR_ECampaignTutorialStage.WEAPON_PICK:
				{
					BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Player.FindComponent(BaseWeaponManagerComponent));
					
					if (weaponManager.GetCurrent() && m_bHasM16Mag)
					{
						SetStage(SCR_ECampaignTutorialStage.WEAPON_RELOAD);
						break;
					}
				
					if (weaponManager)	
						stageComplete = weaponManager.GetCurrent();
								
					break;
				}
			
				case SCR_ECampaignTutorialStage.MAGAZINE_PICK:
				{
					stageComplete = m_bHasM16Mag;
					break;
				}
			
				case SCR_ECampaignTutorialStage.WEAPON_RELOAD:
				{
					BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Player.FindComponent(BaseWeaponManagerComponent));
					if (!weaponManager)
						break;
	
					BaseWeaponComponent weaponComponent = weaponManager.GetCurrent();
					if (!weaponComponent)
						break;
					
					BaseMagazineComponent mag = weaponComponent.GetCurrentMagazine();
					if (mag)
						stageComplete = (mag.GetAmmoCount() != 0);
				
					break;
				}
			
				case SCR_ECampaignTutorialStage.FIREPOZ_1:
				{
					stageComplete = true;
					break;
				}
			
				case SCR_ECampaignTutorialStage.SHOOTING:
				{
					stageComplete = m_iCountOfHits > 4;
					break;
				}
				
				case SCR_ECampaignTutorialStage.SHOOTING_CROUCH:
				{				
					stageComplete = m_iCountOfHits > 4;
					break;
				}
			
				case SCR_ECampaignTutorialStage.SHOOTING_PRONE:
				{				
					stageComplete = m_iCountOfHits > 4;
					break;
				}
			
				case SCR_ECampaignTutorialStage.FIREPOZ_2:
				{
					stageComplete = true;
					break;
				}
			
				case SCR_ECampaignTutorialStage.SHOOTING_LEANING:
				{				
					stageComplete = m_iCountOfHits > 4;
					break;
				}
			
				case SCR_ECampaignTutorialStage.BOARDING:
				{
					stageComplete = CheckCharacterInVehicle(ECompartmentType.Pilot);
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_0:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.SWITCHING_SEATS:
				{
					stageComplete = CheckCharacterInVehicle(ECompartmentType.Turret);
					break;
				}
				
				case SCR_ECampaignTutorialStage.VEHICLE_SHOOTING:
				{
					stageComplete = m_iCountOfHits > 7;
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_1:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_2:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_3:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_4:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_5:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_6:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_7:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_8:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_9:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
				
				case SCR_ECampaignTutorialStage.DISMOUNTING:
				{
					stageComplete = !m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.MAP_INFO:
				{
					stageComplete = m_bIsMapOpen;
					break;
				}
				
				case SCR_ECampaignTutorialStage.CONFLICT_REQUESTING_TRUCK:
				{
					IEntity nearbyEntities;
					GetGame().GetWorld().QueryEntitiesBySphere(m_Player.GetOrigin(), 30, FindSupplyTruck, null, EQueryEntitiesFlags.DYNAMIC);
					stageComplete = m_SupplyTruckComponent != null;
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_LOADING_SUPPLIES:
				{
					stageComplete = m_SupplyTruckComponent.GetSupplies() == m_SupplyTruckComponent.GetSuppliesMax();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_10:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_11:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_12:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_13:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_14:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.DRIVING_15:
				{
					stageComplete = m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_UNLOADING_SUPPLIES:
				{
					SCR_CampaignBase chotainBase = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain"));
					stageComplete = chotainBase.GetSupplies() >= 850;
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_START:
				{
					SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(m_Player.FindComponent(SCR_CampaignBuildingComponent));		
					stageComplete = !(!buildComp || !buildComp.IsBuilding());
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_BUILD_SERVICE:
				{
					SCR_CampaignBase chotainBase = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain"));				
					SCR_CampaignDeliveryPoint service = chotainBase.GetBaseService(ECampaignServicePointType.REPAIR_DEPOT);
				
					if (service.IsBuilt() && m_SupplyTruckComponent && m_SupplyTruckComponent.GetSupplies() >= 200)
					{
						SetStage(SCR_ECampaignTutorialStage.CONFLICT_BOARD_TRUCK);
						break;
					}
				
					stageComplete = service.IsBuilt();				
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_LOAD_SUPPLY:
				{
					if (!m_SupplyTruckComponent)
					{
						stageComplete = true;
						break;
					}
				
					GenericEntity WP = GenericEntity.Cast(GetGame().GetWorld().FindEntityByName("WP_CONFLICT_BUILDING_LOAD_SUPPLY"));
					WP.SetOrigin(m_SupplyTruckComponent.GetOwner().GetOrigin());
					stageComplete = m_SupplyTruckComponent.GetSupplies() >= 200;
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_BOARD_TRUCK:
				{
					stageComplete = CheckCharacterInVehicle(ECompartmentType.Pilot);
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_BUILD_BUNKER:
				{
					SCR_SiteSlotEntity bunker_slot = SCR_SiteSlotEntity.Cast(GetGame().GetWorld().FindEntityByName("buildingSlot"));					
					if (bunker_slot && bunker_slot.IsOccupied())
					{
						SetStage(SCR_ECampaignTutorialStage.CONFLICT_BUILDING_QUIT);
						break;
					}

					SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(m_Player.FindComponent(SCR_CampaignBuildingComponent));
					if (buildComp && !buildComp.IsBuilding())
						SetStage(SCR_ECampaignTutorialStage.CONFLICT_TURN_ON_BUILDING);
				
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_TURN_ON_BUILDING:
				{				
					SCR_SiteSlotEntity bunker_slot = SCR_SiteSlotEntity.Cast(GetGame().GetWorld().FindEntityByName("buildingSlot"));
													
					if (bunker_slot && bunker_slot.IsOccupied())
					{
						SetStage(SCR_ECampaignTutorialStage.CONFLICT_BUILDING_QUIT);
						break;
					}
				
					SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(m_Player.FindComponent(SCR_CampaignBuildingComponent));
					
					if (buildComp && buildComp.IsBuilding())
					{
						SetStage(SCR_ECampaignTutorialStage.CONFLICT_BUILD_BUNKER);
						break;
					}
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_QUIT:
				{				
					SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(m_Player.FindComponent(SCR_CampaignBuildingComponent));
					SCR_SiteSlotEntity checkpoint_slot = SCR_SiteSlotEntity.Cast(GetGame().GetWorld().FindEntityByName("buildingSlotCheckpoint"));	
					if (checkpoint_slot.IsOccupied())
					{
						SetStage(SCR_ECampaignTutorialStage.CONFLICT_BUILDING_QUIT2);	
						break;
					}
						
					stageComplete = (!buildComp || !buildComp.IsBuilding());
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_TO_FAR:
				{
					SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(m_Player.FindComponent(SCR_CampaignBuildingComponent));
					stageComplete = buildComp.IsBuilding();
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_CHECKPOINT:
				{
					SCR_SiteSlotEntity checkpoint_slot = SCR_SiteSlotEntity.Cast(GetGame().GetWorld().FindEntityByName("buildingSlotCheckpoint"));	
					stageComplete = checkpoint_slot.IsOccupied();
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_BUILDING_QUIT2:
				{				
					SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(m_Player.FindComponent(SCR_CampaignBuildingComponent));		
					stageComplete = (!buildComp || !buildComp.IsBuilding());
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_TASKS_MENU:
				{
					stageComplete = m_bTaskMenuOpened;
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_VOLUNTEERING:
				{
					SCR_CampaignTaskManager tManager = SCR_CampaignTaskManager.GetCampaignTaskManagerInstance();
					
					if (!tManager)
						break;
				
					SCR_CampaignBase baseLaruns = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseLaruns"));
				
					if (!baseLaruns)
						break;
				
					SCR_CampaignTask task = tManager.GetTask(baseLaruns, GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_BLUFOR), SCR_CampaignTaskType.CAPTURE);
				
					if (!task)
						break;
					
					stageComplete = task.GetAssignee() == SCR_BaseTaskExecutor.GetLocalExecutor();
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_BASE_SEIZING:
				{
					stageComplete = !m_Player.IsInVehicle();
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_BASE_NEARBY:
				{
					SCR_CampaignBase baseLaruns = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseLaruns"));
					
					if (baseLaruns)
						stageComplete = baseLaruns.GetOwningFaction() == GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_BLUFOR);
					
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_SHOW_COMPAS:
				{
					SCR_GadgetManagerComponent gadgetComponent = SCR_GadgetManagerComponent.Cast(m_Player.FindComponent(SCR_GadgetManagerComponent));
					IEntity compas = gadgetComponent.GetHeldGadget();
					IEntity compasGadget = gadgetComponent.GetGadgetByType(EGadgetType.COMPASS);
					stageComplete = compas == compasGadget;
					break;
				} 
			
				case SCR_ECampaignTutorialStage.CONFLICT_COMPAS_DIRECTION_NORTH:
				{
					float angle = m_Player.GetCharacterController().GetAimingAngles()[0];
					stageComplete = Math.AbsFloat(angle) < 3;
					break;
				}
				
				case SCR_ECampaignTutorialStage.CONFLICT_COMPAS_DIRECTION_2:
				{
					float angle = m_Player.GetCharacterController().GetAimingAngles()[0];
					stageComplete = (angle > 137) && (angle < 143);
					break;
				}
				
				case SCR_ECampaignTutorialStage.CONFLICT_RADIO_PICKUP:
				{
					stageComplete = m_bRadioGrabbed;
					break;
				}
				
				case SCR_ECampaignTutorialStage.CONFLICT_VOIP_SETUP:
				{	
					stageComplete = (m_Radio.GetFrequency() == DESIRED_FREQUENCY);
					break;
				}
				
				case SCR_ECampaignTutorialStage.CONFLICT_VOIP_USE:
				{
					if(m_Radio.GetFrequency() != DESIRED_FREQUENCY)
						m_fSavedTime = 0;
					stageComplete = m_Radio.GetFrequency() == DESIRED_FREQUENCY && m_fSavedTime + 6000 <= GetGame().GetWorld().GetWorldTime() && m_fSavedTime != 0 && m_bIsVOIPDone == false;
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_MHQ_DEPLOY:
				{
					stageComplete = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_BLUFOR)).GetDeployedMobileAssembly() != null;
					break;
				}
			
				case SCR_ECampaignTutorialStage.CONFLICT_SEIZE_ENEMY_HQ:
				{
					SCR_CampaignBase HQ = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("MainBaseLevie"));
					
					if (HQ)
						stageComplete = HQ.GetOwningFaction() == GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_BLUFOR);
					
					break;
				}
			
				default:
				{
					stageComplete = true;
					break;
				}
			}
		
		if (waypointReached && stageComplete)
		{
			m_bIsStageComplete = true;
			Print("Stage completed");
			
			if (m_fStageDelay == 0)
				SetStage(m_eStage + 1);
			else
				GetGame().GetCallqueue().CallLater(SetStage, m_fStageDelay * 1000, false, m_eStage + 1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemAdded(IEntity item)
	{
		MagazineComponent magComp = MagazineComponent.Cast(item.FindComponent(MagazineComponent));
		if (!magComp)
			return false;
		 
		BaseMagazineWell baseMagwell;
		baseMagwell = magComp.GetMagazineWell();
		if (!baseMagwell)
			return false;
		
		MagazineWellStanag556 m16mag = MagazineWellStanag556.Cast(baseMagwell);
		if (!m16mag)	
			return false;
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManComp)	
			storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
		
		m_bHasM16Mag = true;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool FindBuildingController(IEntity ent)
	{
		m_BuildingControllerComponent = SCR_CampaignBuildingControllerComponent.Cast(ent.FindComponent(SCR_CampaignBuildingControllerComponent));
		
		if (m_BuildingControllerComponent)
		{
			m_bShowOnetimeHintBunkerRotation = true;
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool FindSupplyTruck(IEntity ent)
	{
		m_SupplyTruckComponent = SCR_CampaignSuppliesComponent.Cast(ent.FindComponent(SCR_CampaignSuppliesComponent));
		
		if (m_SupplyTruckComponent)
		{
			ent.GetParent().SetName("SupplyTruck");
			return false;
		}
		
		return true;
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
		
		GetGame().OnInputDeviceUserChangedInvoker().Remove(OnInputDeviceChanged);
	}
};

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
	DRIVING_8,
	DRIVING_9,
	DISMOUNTING,
	CONFLICT_TOUR_ARMORY,
	CONFLICT_TOUR_VEHICLE,
	CONFLICT_TOUR_HQ1,
	CONFLICT_TOUR_HQ2,
	CONFLICT_TOUR_HQ3,
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
	CONFLICT_BUILDING_LOAD_SUPPLY,
	CONFLICT_BOARD_TRUCK,
	CONFLICT_BUILD_BUNKER,
	CONFLICT_TURN_ON_BUILDING,
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