
[EntityEditorProps(category: "GameScripted/CampaignGameMode", description: "Campaign game mode script.", color: "0 0 255 255")]
class SCR_GameModeCampaignMPClass: SCR_BaseGameModeClass
{
};

/// @ingroup GameMode

//------------------------------------------------------------------------------------------------
class SCR_GameModeCampaignMP : SCR_BaseGameMode
{
	[Attribute("US", UIWidgets.EditBox, "", "")]
	protected FactionKey m_sBLUFORFactionKey;
	
	[Attribute("USSR", UIWidgets.EditBox, "", "")]
	protected FactionKey m_sOPFORFactionKey;
	
	[Attribute("FIA", UIWidgets.EditBox, "", "")]
	protected FactionKey m_sINDFORFactionKey;
	
	[Attribute("8", UIWidgets.Slider, "Starting time of day (hours)", "0 23 1")]
	protected int m_iStartingHours;
	
	[Attribute("0", UIWidgets.Slider, "Starting time of day (minutes)", "0 59 1")]
	protected int m_iStartingMinutes;
	
	[Attribute("1", UIWidgets.Slider, "Time acceleration during the day (1 = 100%, 2 = 200% etc)", "0.1 12 0.1")]
	protected float m_fDaytimeAcceleration;
	
	[Attribute("1", UIWidgets.Slider, "Time acceleration during the night (1 = 100%, 2 = 200% etc)", "0.1 12 0.1")]
	protected float m_fNighttimeAcceleration;
	
	[Attribute("40", UIWidgets.EditBox, "Total maximum of all player and allied AI units.")]
	int m_iTotalPlayersLimit;
	
	[Attribute("1", UIWidgets.CheckBox, "Randomized Main bases", "")]
	bool m_bRandomizeBases;
	
	[Attribute("1", UIWidgets.CheckBox, "Randomized starting supplies in small bases", "")]
	bool m_bRandomizeSupplies;
	
	[Attribute("1", UIWidgets.CheckBox, "Apply starting owner of bases only if Advanced stage param is enabled in mission header", "")]
	bool m_bCheckAdvancedStageConfig;
	
	[Attribute("30", UIWidgets.EditBox, "Time in seconds for how long the vehicle is reserved for a player who request it.")] 
	int m_iSpawnedVehicleTimeProtection;
	
	[Attribute("{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset")]
	protected ResourceName m_sBuildingIconImageset;
	
	[Attribute("{2294DC90D8E050D2}UI/layouts/Campaign/BuildingHUDIcon.layout")]
	protected ResourceName m_sHUDIcon;
	
	[Attribute("{F1AC26310BAE3788}Prefabs/MP/Campaign/CampaignFactionManager.et")]
	ResourceName m_FactionManagerPrefab;
	
	[Attribute("{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et")]
	ResourceName m_CycleWaypointPrefab;
	
	[Attribute("{22A875E30470BD4F}Prefabs/AI/Waypoints/AIWaypoint_Patrol.et")]
	ResourceName m_PatrolWaypointPrefab;
	
	[Attribute("{EAAE93F98ED5D218}Prefabs/AI/Waypoints/AIWaypoint_CaptureRelay.et")]
	ResourceName m_RetakeWaypointPrefab;
	
	[Attribute("{B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et")]
	ResourceName m_SeekDestroyWaypointPrefab;
	
	[Attribute("{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et")]
	ResourceName m_DefendWaypointPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Remnants group prefab", "et")]
	private ResourceName m_RemnantsGroupPrefab;
	
	//*****************************//
	//PUBLIC STATIC SCRIPT INVOKERS//
	//*****************************//
	
	static ref ScriptInvoker s_OnBaseCaptured = new ref ScriptInvoker();
	static ref ScriptInvoker s_OnFactionAssigned = new ref ScriptInvoker();
	static ref ScriptInvoker s_OnFactionAssignedLocalPlayer = new ref ScriptInvoker();
	static ref ScriptInvoker s_OnMobileAssemblyDeployChanged = new ScriptInvoker();
	static ref ScriptInvoker s_OnServiceBuild = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnPlayerEnterBase = new ref ScriptInvoker(); //Gives SCR_CampaignBase
	protected ref ScriptInvoker Event_OnPlayerLeftBase = new ref ScriptInvoker(); //Gives SCR_CampaignBase
		
	//************************//
	//RUNTIME STATIC VARIABLES//
	//************************//
	protected static SCR_RespawnSystemComponent s_RespawnSystemComponent = null;
	protected static SCR_GameModeCampaignMP s_Instance = null;
	protected static string s_sBackendFilename;
	protected static const int REINFORCEMENTS_PERIOD_MAIN = 900;	//s: how ofter reinforcements arrive at Main bases
	protected static const int REINFORCEMENTS_PERIOD_MAJOR = 1200;	//s: how ofter reinforcements arrive at Major bases
	protected static const int XP_INFO_DURATION = 10000; 
	protected static const int BACKEND_WRITE_PERIOD = 60000;
	protected static const int INVALID_BACKEND_ID = 0;
	protected static const int REMNANTS_SPAWN_RADIUS_MIN = Math.Pow(500, 2);
	protected static const int REMNANTS_SPAWN_RADIUS_MAX = Math.Pow(1000, 2);
	protected static const int REMNANTS_DESPAWN_RADIUS_DIFF = Math.Pow(200, 2);
	protected static int STARTING_SUPPLIES_MIN = 750;
	protected static int STARTING_SUPPLIES_MAX = 1250;
	protected static const int STARTING_SUPPLIES_INTERVAL = 25;
	protected static bool m_bBackendStateLoaded = false;
	static const int GARAGE_VEHICLE_SPAWN_INTERVAL = 60000;
	static const int DAY_IN_SECONDS = 24 * 60 * 60;
	static const float DAYTIME_START = 5.0;
	static const float DAYTIME_END = 20.0;
	static const int RESPAWN_TICKETS_MAX_DEFAULT = 0;				// TODO: Move this and the following to attributes(?)
	static const int RESPAWN_TICKETS_MAX_BONUS_FUEL = 6;
	static const int RESPAWN_TICKETS_MAX_BONUS_REPAIR = 6;
	static const int RESPAWN_TICKETS_MAX_BONUS_ARMORY = 8;
	static const int RESPAWN_TICKETS_MAX_BONUS_HOSPITAL = 10;
	static const int RESPAWN_TICKETS_MAX_BONUS_BARRACKS = 15;
	static const int REINFORCEMENTS_SUPPLIES = 10000;
	static const int REINFORCEMENTS_CHECK_PERIOD = 1000;			//ms: how often we should check for reinforcements arrival
	static const float SUPPLY_TRUCK_UNLOAD_RADIUS = 25;				//m: maximum distance from a supply depot a player can still (un)load their truck
	static FactionKey FACTION_BLUFOR;
	static FactionKey FACTION_OPFOR;
	static FactionKey FACTION_INDFOR;
	static const int AI_GROUPS_PER_FACTION = 3;
	static const int REMNANTS_CNT_MIN = 9;
	static const int REMNANTS_CNT_MAX = 9;
	static const int RESPAWN_TICKETS_REPLENISH_PERIOD = 60000;
	static const int RESPAWN_TICKETS_REPLENISH_AMOUNT = 2;
	static const int BASE_CALLSIGNS_COUNT = 40;
	static const int ENDING_TIMEOUT = 15000;
	static const int BUILDING_CONTROLLER_SEARCH_DISTANCE = 5;
	static const int SKILL_LEVEL_MAX = 10;
	static const float SKILL_LEVEL_XP_BONUS = 0.1;
	static const int SKILL_LEVEL_XP_COST = 1000;		// how much XP is needed for new level
	
	//************************//
	//RUNTIME MEMBER VARIABLES//
	//************************//
	protected ref array<SCR_AIGroup> m_aAIGroups = new ref array<SCR_AIGroup>();
	protected ref array<ref SCR_CampaignClientData> m_aRegisteredClients = new array<ref SCR_CampaignClientData>;
	protected ref array<ref SCR_CampaignRemnantsPresence> m_aRemnantsPresence = new array<ref SCR_CampaignRemnantsPresence>;
	protected ref array<ref SCR_CampaignXPRewardInfo> m_aXPRewardList;
	protected ref array<Vehicle> m_aRequestedVehicles = new array<Vehicle>;
	protected ref map<SCR_SiteSlotEntity, ref SCR_BuildingDedicatedSlotData> m_aDedicatedSlots = new map<SCR_SiteSlotEntity, ref SCR_BuildingDedicatedSlotData>();
	protected ref array<SCR_CampaignBase> m_aBasesWithPlayer = new array<SCR_CampaignBase>;
	protected float m_fRemnantForcesTimer = 0;
	protected int m_iLocationCheckedForPlayerProximity = -1;
	protected bool m_bAllRemnantsSpawned = false;
	protected bool m_bFlipBases = false;
	protected ref Resource m_rRemnants;
	protected ref Resource m_rAIVehicleWest;
	protected ref Resource m_rAIVehicleEast;
	protected int m_iAIGroupSizeWest;
	protected int m_iAIGroupSizeEast;
	protected bool m_bAllowRemnantsDespawn = true;
	protected bool m_bAdvancedStage = false;
	protected bool m_bIsPlayerInRadioRange = true;
	protected Faction m_LastPlayerFaction;
	protected SCR_CampaignBase m_LastVisitedBase;
	protected int m_iBaseSeizingHintsShown;
	protected int m_iSuppliesHintsShown;
	protected SCR_CampaignBase m_FirstBaseWithPlayer;
	protected bool m_bDaytimeAcceleration = true;
	protected bool m_bStartupHintsShown = false;
	protected bool m_bRespawnHintShown = false;
	protected bool m_bReinforcementsHintShown = false;
	protected bool m_bTicketsHintShown = false;
	protected bool m_bBaseLostHintShown = false;
	protected AudioHandle m_PlayedRadio = AudioHandle.Invalid;
	protected RplId m_LastDeployedHQIDWest = RplId.Invalid();
	protected RplId m_LastDeployedHQIDEast = RplId.Invalid();
	protected IEntity m_BuildingController;
	protected ref SimplePreload m_Preload;
	protected int m_iRemnantLocationsCnt;
	protected float m_fXpMultiplier = 1;
	protected bool m_bIgnoreMinimumVehicleRank;
	protected ref map<int, Faction> m_aUnprocessedFactionAssignments = new ref map<int, Faction>();
	protected bool m_bCanShowSpawnPosition; // check for showing player spawn location on map
	protected vector m_vFirstSpawnPosition = vector.Zero;
	protected SCR_MapEntity m_MapEntity;
	protected SCR_MapCampaignUI m_MapCampaignUI;
	protected bool m_bWasMapOpened;
	protected ref TimeContainer m_SpawnTime;
	protected bool m_bIsTutorial;
	protected bool m_bIsShowingXPBar;
	protected int m_iRegisteredPlayerFaction = -1;
	protected int m_iRegisteredPlayerXP;
	protected bool m_bMatchOver;
	
	// Interactions component related
	protected const float INTERACTION_COMPONENTS_UPDATE_TIME = 2;
	protected ref array<SCR_CampaignInteractionsComponent> m_aInteractionComponents = new ref array<SCR_CampaignInteractionsComponent>();
	protected int m_iInteractionComponentsIndex = 0;
	protected float m_fInteractionComponentsTimer = INTERACTION_COMPONENTS_UPDATE_TIME;
	
	// UI-related
	protected float m_fHideXPInfo = 0;
	protected Widget m_wGOScreen;
	protected Widget m_wXPInfo;
	protected Widget m_wPlayersList;
	protected Widget m_wPlayersListSlot;
	protected bool m_bNegativeXP = false;
	ref array<Widget> m_aPlayerList = new ref array<Widget>();
	protected eCampaignStatusMessage m_iLastStatusMsgShown = -1;
	
	//********************************//
	//RUNTIME SYNCHED MEMBER VARIABLES//
	//********************************//
	[RplProp()]
	protected float m_fGameEnd = -1;
	[RplProp(onRplName: "OnMobileAssemblyDeployChangedWest")]
	protected RplId m_DeployedMobileAssemblyIDWest = RplId.Invalid();
	[RplProp(onRplName: "OnMobileAssemblyDeployChangedEast")]
	protected RplId m_DeployedMobileAssemblyIDEast = RplId.Invalid();
	[RplProp()]
	protected int m_iCustomRadioRange = -1;
	[RplProp()]
	protected int m_iMaxRespawnRadios = 5;
	[RplProp()]
	protected int m_iActiveRespawnRadiosCntWest;
	[RplProp()]
	protected int m_iActiveRespawnRadiosCntEast;
	
	//************************//
	//PROTECTED STATIC METHODS//
	//************************//
	
	//*********************//
	//PUBLIC STATIC METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	static Resource GetFactionManagerResource()
	{
		SCR_GameModeCampaignMP campaign = GetCampaign();
		if (!campaign)
			return null;
		
		return Resource.Load(campaign.m_FactionManagerPrefab);
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetBackendFilename()
	{
		return s_sBackendFilename;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_GameModeCampaignMP GetCampaign()
	{
		return SCR_GameModeCampaignMP.Cast(GetGame().GetGameMode());
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsBackendStateLoaded()
	{
		return m_bBackendStateLoaded;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the period of reinforcements for given base
	static float GetReinforcementsPeriod(SCR_CampaignBase base)
	{
		if (!base)
			return -1;
		
		switch (base.GetType())
		{
			case CampaignBaseType.MAIN: {return REINFORCEMENTS_PERIOD_MAIN;};
			case CampaignBaseType.MAJOR: {return REINFORCEMENTS_PERIOD_MAJOR;};
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if we're not actually playing (and halt the code in proper places)
	static bool NotPlaying()
	{
		auto game = GetGame();
		
		if (!game || !game.InPlayMode())
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns an instance of this game mode.
	static SCR_GameModeCampaignMP GetInstance()
	{
		return s_Instance;
	}
	
	//*********************//
	//PUBLIC MEMBER METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	bool CanShowPlayerSpawn()
	{
		return m_bCanShowSpawnPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetCycleWaypointPrefab()
	{
		return m_CycleWaypointPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetPatrolWaypointPrefab()
	{
		return m_PatrolWaypointPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRetakeWaypointPrefab()
	{
		return m_RetakeWaypointPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetSeekDestroyWaypointPrefab()
	{
		return m_SeekDestroyWaypointPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDefendWaypointPrefab()
	{
		return m_DefendWaypointPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	Faction GetPlayerForcedFaction(int playerID)
	{
		Faction faction = null;
		SCR_CampaignClientData clientData;
		clientData = GetClientData(playerID);
		
		if (clientData)
			faction = clientData.GetFaction();
		
		return faction;
	}

	//------------------------------------------------------------------------------------------------
	bool IsMobileAssemblyDeployed(notnull SCR_CampaignFaction faction)
	{
		switch (faction.GetFactionKey())
		{
			case FACTION_BLUFOR: {return m_DeployedMobileAssemblyIDWest.IsValid();};
			case FACTION_OPFOR: {return m_DeployedMobileAssemblyIDEast.IsValid();};
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsPlayerInRadioRange(bool status)
	{
		m_bIsPlayerInRadioRange = status;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add XP to given entity
	void AwardXP(IEntity player, CampaignXPRewards rewardID, float multiplier = 1.0, bool volunteer = false)
	{
		if (IsProxy() || !player)
			return;
		
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(player));
		AwardXP(playerController, rewardID, multiplier, volunteer);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add XP to given controller
	void AwardXP(PlayerController controller, CampaignXPRewards rewardID, float multiplier = 1.0, bool volunteer = false)
	{
		if (!controller || IsProxy())
			return;

		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(controller.FindComponent(SCR_CampaignNetworkComponent));
		
		if (campaignNetworkComponent)
			campaignNetworkComponent.AddPlayerXP(rewardID, multiplier, volunteer)
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMaxRespawnRadios()
	{
		return m_iMaxRespawnRadios;
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayRadioMsg(SCR_ERadioMsg msg, int baseCallsign, int callerCallsignCompany, int callerCallsignPlatoon, int callerCallsignSquad, int calledCallsignCompany, int calledCallsignPlatoon, int calledCallsignSquad, int param, float seed, float quality)
	{
		if (m_bIsTutorial)
			return;
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		if (!pc)
			return;
		
		IEntity player = pc.GetMainEntity();
		
		if (!player)
			return;
		
		SCR_CommunicationSoundComponent soundComp = SCR_CommunicationSoundComponent.Cast(player.FindComponent(SCR_CommunicationSoundComponent));
		
		if (!soundComp)
			return;
		
		SignalsManagerComponent signalComp = SignalsManagerComponent.Cast(player.FindComponent(SignalsManagerComponent));
		
		if (!signalComp)
			return;
		
		int signalBase = signalComp.AddOrFindSignal("Base");
		int signalCompanyCaller = signalComp.AddOrFindSignal("CompanyCaller");
		int signalCompanyCalled = signalComp.AddOrFindSignal("CompanyCalled");
		int signalPlatoonCaller = signalComp.AddOrFindSignal("PlatoonCaller");
		int signalPlatoonCalled = signalComp.AddOrFindSignal("PlatoonCalled");
		int signalSquadCaller = signalComp.AddOrFindSignal("SquadCaller");
		int signalSquadCalled = signalComp.AddOrFindSignal("SquadCalled");
		float signalSeed = signalComp.AddOrFindSignal("Seed");
		float signalQuality = signalComp.AddOrFindSignal("TransmissionQuality");
		
		if (baseCallsign != SCR_CampaignBase.INVALID_BASE_INDEX)
			signalComp.SetSignalValue(signalBase, baseCallsign);
		
		if (callerCallsignCompany != SCR_CampaignBase.INVALID_PLAYER_INDEX)
		{
			signalComp.SetSignalValue(signalCompanyCaller, callerCallsignCompany);
			signalComp.SetSignalValue(signalPlatoonCaller, callerCallsignPlatoon);
			signalComp.SetSignalValue(signalSquadCaller, callerCallsignSquad);
		}
		
		if (calledCallsignCompany != SCR_CampaignBase.INVALID_PLAYER_INDEX)
		{
			signalComp.SetSignalValue(signalCompanyCalled, calledCallsignCompany);
			signalComp.SetSignalValue(signalPlatoonCalled, calledCallsignPlatoon);
			signalComp.SetSignalValue(signalSquadCalled, calledCallsignSquad);
		}
		
		signalComp.SetSignalValue(signalSeed, seed);
		signalComp.SetSignalValue(signalQuality, quality);
		
		string msgName;
		LocalizedString text;
		LocalizedString text2;
		string param1;
		string text2param1;
		string text2param2;
		int duration = SCR_PopUpNotification.DEFAULT_DURATION;
		SCR_ECampaignPopupPriority prio = SCR_ECampaignPopupPriority.DEFAULT;
		SCR_CampaignBase base = SCR_CampaignBaseManager.FindBaseByID(param);
		string sound;
		
		switch (msg)
		{
			case SCR_ERadioMsg.SEIZED_MAIN:
			{
				msgName = "SOUND_HQ_MOB";
				break;
			}
			
			case SCR_ERadioMsg.SEIZED_MAJOR:
			{
				msgName = "SOUND_HQ_FOB";
				break;
			}
			
			case SCR_ERadioMsg.SEIZED_SMALL:
			{
				msgName = "SOUND_HQ_COP";
				break;
			}
			
			case SCR_ERadioMsg.DEMOTION_RENEGADE:
			{
				msgName = "SOUND_HQ_REN";
				text = "#AR-Campaign_Demotion-UC";
				text2 = "#AR-Rank_Renegade";
				break;
			}
			
			case SCR_ERadioMsg.DEMOTION:
			{
				msgName = "SOUND_HQ_DEM";
				text = "#AR-Campaign_Demotion-UC";
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(m_LastPlayerFaction);
				
				if (f)
					text2 = f.GetRankNameUpperCase(param);
				
				break;
			}
			
			case SCR_ERadioMsg.PROMOTION_PRIVATE:
			{
				msgName = "SOUND_HQ_POP";
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestPrivate";
				break;
			}
			
			case SCR_ERadioMsg.PROMOTION_CORPORAL:
			{
				msgName = "SOUND_HQ_POC";
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestCorporal";
				break;
			}
			
			case SCR_ERadioMsg.PROMOTION_SERGEANT:
			{
				msgName = "SOUND_HQ_POS";
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestSergeant";
				break;
			}
			
			case SCR_ERadioMsg.PROMOTION_LIEUTENANT:
			{
				msgName = "SOUND_HQ_POL";
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestLieutenant";
				break;
			}
			
			case SCR_ERadioMsg.PROMOTION_CAPTAIN:
			{
				msgName = "SOUND_HQ_PON";
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestCaptain";
				break;
			}
			
			case SCR_ERadioMsg.PROMOTION_MAJOR:
			{
				msgName = "SOUND_HQ_POM";
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestMajor";
				break;
			}
			
			case SCR_ERadioMsg.REINFORCEMENTS:
			{
				if (!base)
					return;
				
				msgName = "SOUND_HQ_RIF";
				text = "#AR-Campaign_ReinforcementsArrival-UC";
				text2 = "#AR-Campaign_ReinforcementsArrivalDetails";
				param1 = base.GetBaseNameUpperCase();
				GetGame().GetCallqueue().CallLater(ShowHint, 8500, false, SCR_ECampaignHints.REINFORCEMENTS);
				break;
			}
			
			case SCR_ERadioMsg.VICTORY:
			{
				SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();

				if (!fManager)
					return;
				
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(param));
				
				if (!f || f != SCR_RespawnSystemComponent.GetLocalPlayerFaction())
					return;
				
				msgName = "SOUND_HQ_VIC";
				break;
			}
			
			case SCR_ERadioMsg.DEFEAT:
			{
				SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();
				
				if (!fManager)
					return;
				
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(param));
				
				if (!f || f == SCR_RespawnSystemComponent.GetLocalPlayerFaction())
					return;
				
				msgName = "SOUND_HQ_DEF";			
				break;
			}
			
			case SCR_ERadioMsg.RELAY:
			{
				msgName = "SOUND_SL_RRD";
				break;
			}
			
			case SCR_ERadioMsg.REQUEST_EVAC:
			{
				msgName = "SOUND_SL_ERT";
				break;
			}
			
			case SCR_ERadioMsg.REQUEST_FUEL:
			{
				msgName = "SOUND_SL_RRT";
				break;
			}
			
			case SCR_ERadioMsg.SUPPLIES:
			{
				msgName = "SOUND_SL_SDD";
				break;
			}
			
			case SCR_ERadioMsg.REQUEST_REINFORCEMENTS:
			{
				msgName = "SOUND_SL_REI";
				break;
			}
			
			case SCR_ERadioMsg.REQUEST_TRANSPORT:
			{
				msgName = "SOUND_SL_TRT";
				break;
			}
			
			case SCR_ERadioMsg.CONFIRM:
			{
				msgName = "SOUND_SL_CSR";
				break;
			}
			
			case SCR_ERadioMsg.TASK_ASSIGN_SEIZE:
			{
				msgName = "SOUND_SL_SRT";
				break;
			}
			
			case SCR_ERadioMsg.TASK_UNASSIGN_REFUEL:
			{
				msgName = "SOUND_HQ_RRU";
				break;
			}
			
			case SCR_ERadioMsg.TASK_UNASSIGN_TRANSPORT:
			{
				msgName = "SOUND_HQ_TRU";
				break;
			}
			
			case SCR_ERadioMsg.TASK_UNASSIGN_EVAC:
			{
				msgName = "SOUND_HQ_ETU";
				break;
			}
			
			case SCR_ERadioMsg.TASK_CANCEL_REQUEST:
			{
				msgName = "SOUND_HQ_RCR";
				break;
			}
			
			case SCR_ERadioMsg.TASK_ASSIST:
			{
				msgName = "SOUND_SL_CHR";
				break;
			}
			
			case SCR_ERadioMsg.BASE_LOST:
			{
				msgName = "SOUND_HQ_BAL";
				break;
			}
			
			case SCR_ERadioMsg.BASE_UNDER_ATTACK:
			{
				if (!base)
					return;
				
				msgName = "SOUND_HQ_BUA";
				text = "#AR-Campaign_BaseUnderAttack-UC";
				text2 = "%1 (%2)";
				text2param1 = base.GetBaseName();
				text2param2 = base.GetCallsignDisplayName();
				
				if (m_FirstBaseWithPlayer == base)
					sound = "SOUND_SIREN";
				
				prio = SCR_ECampaignPopupPriority.BASE_UNDER_ATTACK;
				duration = 11;
				break;
			}
			
			case SCR_ERadioMsg.BUILT_ARMORY:
			{
				if (!base)
					return;
				
				msgName = "SOUND_HQ_BAA";
				text = "#AR-Campaign_Building_Available-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_Armory-UC";
				duration = 5;
				break;
			}
			
			case SCR_ERadioMsg.BUILT_FUEL:
			{
				if (!base)
					return;
				
				msgName = "SOUND_HQ_BFA";
				text = "#AR-Campaign_Building_Available-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_FuelDepot-UC";
				duration = 5;
				break;
			}
			
			case SCR_ERadioMsg.BUILT_REPAIR:
			{
				if (!base)
					return;
				
				msgName = "SOUND_HQ_BRA";
				text = "#AR-Campaign_Building_Available-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_RepairDepot-UC";
				duration = 5;
				break;
			}
			
			case SCR_ERadioMsg.DESTROYED_ARMORY:
			{
				if (!base)
					return;
				
				msgName = "SOUND_HQ_BAD";
				text = "#AR-Campaign_Building_Destroyed-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_Armory-UC";
				duration = 5;
				break;
			}
			
			case SCR_ERadioMsg.DESTROYED_FUEL:
			{
				if (!base)
					return;
				
				msgName = "SOUND_HQ_BFD";
				text = "#AR-Campaign_Building_Destroyed-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_FuelDepot-UC";
				duration = 5;
				break;
			}
			
			case SCR_ERadioMsg.DESTROYED_REPAIR:
			{
				if (!base)
					return;
				
				msgName = "SOUND_HQ_BRD";
				text = "#AR-Campaign_Building_Destroyed-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_RepairDepot-UC";
				duration = 5;
				break;
			}
			
			case SCR_ERadioMsg.REPAIRED_ARMORY:
			{
				if (!base)
					return;
				
				msgName = "SOUND_HQ_BAR";
				text = "#AR-Campaign_Building_Available-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_Armory-UC";
				duration = 5;
				break;
			}
			
			case SCR_ERadioMsg.REPAIRED_FUEL:
			{
				if (!base)
					return;
				
				msgName = "SOUND_HQ_BFR";
				text = "#AR-Campaign_Building_Available-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_FuelDepot-UC";
				duration = 5;
				break;
			}
			
			case SCR_ERadioMsg.REPAIRED_REPAIR:
			{
				if (!base)
					return;
				
				msgName = "SOUND_HQ_BRR";
				text = "#AR-Campaign_Building_Available-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_RepairDepot-UC";
				duration = 5;
				break;
			}
		}
				
		if (!msgName.IsEmpty())
		{
			AudioSystem.TerminateSound(m_PlayedRadio);
			m_PlayedRadio = soundComp.SoundEvent(msgName);
		}
		
		if (!text.IsEmpty() || !text2.IsEmpty())
			SCR_PopUpNotification.GetInstance().PopupMsg(text, duration, text2: text2, prio: prio, param1: param1, sound: sound, text2param1: text2param1, text2param2: text2param2);
	}
	
	//------------------------------------------------------------------------------------------------
	Resource GetRemnantsGroupPrefab()
	{
		return m_rRemnants;
	}
	
	//------------------------------------------------------------------------------------------------
	//An event that is typically called from a SCR_CampaignBase that has been captured
	void OnBaseCaptured(SCR_CampaignBase capturedBase)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_GAMEMODE))
			return;
#endif
		if (!m_bMatchOver)
			EvaluateGame();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if the session is run as client
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	//------------------------------------------------------------------------------------------------
	//! Getter for "Rank required" parameter for spawning vehicles.
	// TRUE, if rank requirement is disabled
	bool CanRequestWithoutRank()
	{
		return m_bIgnoreMinimumVehicleRank;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for bases flip
	bool AreBasesFlipped()
	{
		return m_bFlipBases;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds the spawned vehicle to an array handled by garbage collector etc.
	//! \param veh The spawned vehicle
	void RegisterSpawnedVehicle(notnull Vehicle veh)
	{
		m_aRequestedVehicles.Insert(veh);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddBaseWithPlayer(notnull SCR_CampaignBase base, notnull ChimeraCharacter player)
	{
		if (m_aBasesWithPlayer.Find(base) == -1)
		{
			Event_OnPlayerEnterBase.Invoke(base);
			m_aBasesWithPlayer.Insert(base);
			base.ToggleRadioChatter(true);
		}
		
		// Make sure the base closest to player is the first element
		int index;
		int minCheckedDistance = int.MAX;
		int basesCnt = m_aBasesWithPlayer.Count();
		vector playerPos = player.GetOrigin();
		
		for (int i = 0; i < basesCnt; i++)
		{
			int distance = vector.DistanceSq(m_aBasesWithPlayer[i].GetOrigin(), playerPos);
			
			if (distance < minCheckedDistance)
			{
				minCheckedDistance = distance;
				index = i;
			}
		}	
			
		if (basesCnt > 1 && index != 0)
			m_aBasesWithPlayer.SwapItems(0, index);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveBaseWithPlayer(notnull SCR_CampaignBase base)
	{
		m_aBasesWithPlayer.RemoveItem(base);
		base.ToggleRadioChatter(false);
		Event_OnPlayerLeftBase.Invoke(base);
	}
	
	ScriptInvoker GetOnPlayerEnterBase()
	{
		return Event_OnPlayerEnterBase;
	}
	
	ScriptInvoker GetOnPlayerLeftBase()
	{
		return Event_OnPlayerLeftBase;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBase GetBasePlayerPresence(out array<SCR_CampaignBase> basesWithPlayer = null)
	{
		if (basesWithPlayer)
			basesWithPlayer = m_aBasesWithPlayer;
		
		if (m_aBasesWithPlayer.Count() == 0)
			return null;
		else
			return m_aBasesWithPlayer[0];
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPlayerInBase(notnull SCR_CampaignBase base)
	{
		return m_aBasesWithPlayer.Find(base) != -1;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetBuildingIconsImageset()
	{
		return m_sBuildingIconImageset;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetHUDIconLayout()
	{
		return m_sHUDIcon;
	}	
	
	//------------------------------------------------------------------------------------------------
	bool AllowStartingBaseOwners()
	{
		return ((m_bCheckAdvancedStageConfig && m_bAdvancedStage) || !m_bCheckAdvancedStageConfig);
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckForBasesWithPlayer()
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId());
		
		if (!player)
		{
			m_FirstBaseWithPlayer = null;
			return;
		}
		
		CharacterControllerComponent comp = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));
		
		if (!comp || comp.IsDead())
		{
			m_FirstBaseWithPlayer = null;
			return;
		}
		
		array<SCR_CampaignBase> basesWithPlayer = new array<SCR_CampaignBase>();
		SCR_CampaignBase closestBase = GetBasePlayerPresence(basesWithPlayer);
		
		if (closestBase)
		{
			foreach (SCR_CampaignBase base: basesWithPlayer)
				if (!m_FirstBaseWithPlayer || base.GetCapturingFaction())
					m_FirstBaseWithPlayer = base;
		}
		else
			m_FirstBaseWithPlayer = null;
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_CampaignBase> GetBasesInRangeOfMobileHQ(notnull IEntity mobileHQ)
	{
		array<SCR_CampaignBase> bases = {};
		vector mobileHQPos = mobileHQ.GetOrigin();
		float mobileHQRadioRangeSq;
		IEntity truck = mobileHQ.GetParent();
		
		BaseRadioComponent comp;
		
		if (truck)
			comp = BaseRadioComponent.Cast(truck.FindComponent(BaseRadioComponent));
		
		if (!comp)
			return bases;
		
		mobileHQRadioRangeSq = Math.Pow(comp.GetRange(), 2);
		
		foreach (SCR_CampaignBase base: SCR_CampaignBaseManager.GetBases())
		{
			if (!base)
				continue;
			
			float dist = vector.DistanceSq(mobileHQPos, base.GetOrigin());
			
			if (dist < mobileHQRadioRangeSq && dist < Math.Pow(base.GetSignalRange(), 2))
				bases.Insert(base);
		}
		
		return bases;
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckMobileAssemblies()
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();
		
		if (!fManager)
			return;
		
		array<Faction> factions = new array<Faction>();
		fManager.GetFactionsList(factions);
		
		foreach (Faction f: factions)
		{
			if (!f)
				continue;
			
			SCR_CampaignFaction factionC = SCR_CampaignFaction.Cast(f);
			
			if (!factionC)
				continue;
			
			SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
			if (!campaign)
				return;
			
			if (!campaign.IsMobileAssemblyDeployed(factionC))
				continue;
			
			IEntity assembly = factionC.GetDeployedMobileAssembly();
			FactionKey factionCKey = factionC.GetFactionKey();
			
			if (!assembly)
			{
				campaign.SetDeployedMobileAssemblyID(factionCKey, RplId.Invalid());
				continue;
			}
			
			DamageManagerComponent damageComponent = DamageManagerComponent.Cast(assembly.FindComponent(DamageManagerComponent));
			
			if (damageComponent && damageComponent.GetState() == EDamageState.DESTROYED)
			{
				SCR_CampaignMobileAssemblyComponent comp = SCR_CampaignMobileAssemblyComponent.Cast(assembly.FindComponent(SCR_CampaignMobileAssemblyComponent));
				
				if (comp)
					comp.Deploy(false);
				
				Rpc(RpcDo_MobileAssemblyFeedback, ECampaignClientNotificationID.ASSEMBLY_DESTROYED, -1, comp.GetParentFactionID());
				
				if (RplSession.Mode() != RplMode.Dedicated)
					RpcDo_MobileAssemblyFeedback(ECampaignClientNotificationID.ASSEMBLY_DESTROYED, -1, comp.GetParentFactionID());
				
				continue;
			}
			
			IEntity truck = assembly.GetParent();
			Physics physicsComponent;
			vector vel;
			
			if (truck)
			{
				physicsComponent = truck.GetPhysics();
				vel = physicsComponent.GetVelocity();
				vel[1] = 0;
			}
			
			if (physicsComponent && vel.LengthSq() > 0.01)
			{
				SCR_CampaignMobileAssemblyComponent comp = SCR_CampaignMobileAssemblyComponent.Cast(assembly.FindComponent(SCR_CampaignMobileAssemblyComponent));
				
				if (comp)
					comp.Deploy(false);
				
				Rpc(RpcDo_MobileAssemblyFeedback, ECampaignClientNotificationID.ASSEMBLY_DISMANTLED, -1, comp.GetParentFactionID());
				
				if (RplSession.Mode() != RplMode.Dedicated)
					RpcDo_MobileAssemblyFeedback(ECampaignClientNotificationID.ASSEMBLY_DISMANTLED, -1, comp.GetParentFactionID());
				
				continue;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDeployedMobileAssemblyID(FactionKey faction, RplId ID)
	{
		switch (faction)
		{
			case FACTION_BLUFOR: {m_DeployedMobileAssemblyIDWest = ID; OnMobileAssemblyDeployChangedWest(); break;};
			case FACTION_OPFOR: {m_DeployedMobileAssemblyIDEast = ID; OnMobileAssemblyDeployChangedEast(); break;};
		}
		
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	RplId GetDeployedMobileAssemblyID(FactionKey faction)
	{
		switch (faction)
		{
			case FACTION_BLUFOR: {return m_DeployedMobileAssemblyIDWest;};
			case FACTION_OPFOR: {return m_DeployedMobileAssemblyIDEast;};
		}
		
		return RplId.Invalid();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show info on current XP progress
	void ShowXPInfo(int totalXP, CampaignXPRewards rewardID, int XP, bool volunteer, bool profileUsed, int skillLevel)
	{
		if (rewardID == CampaignXPRewards.UNDEFINED)
			return;
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_RespawnSystemComponent.GetLocalPlayerFaction());
			
		if (!faction)
			return;
		
		SCR_CampaignFactionManager campaignFactionManager = SCR_CampaignFactionManager.GetInstance();
		ECharacterRank curRank = campaignFactionManager.GetRankByXP(totalXP);
		ECharacterRank prevRank = campaignFactionManager.GetRankByXP(totalXP - XP);
		string rankText = faction.GetRankName(curRank);
		ResourceName rankIconName = faction.GetRankInsignia(curRank);
		
		if (!m_wXPInfo && GetGame().GetHUDManager())
		{
			m_wXPInfo = GetGame().GetHUDManager().CreateLayout("{E0B82B4FCC95EE05}UI/layouts/HUD/CampaignMP/RankProgress.layout", EHudLayers.MEDIUM, 0);
			m_wXPInfo.SetVisible(false);
		}
		
		if (!m_wXPInfo)
			return;
		
		TextWidget title = TextWidget.Cast(m_wXPInfo.FindWidget("Title"));
		TextWidget rank = TextWidget.Cast(m_wXPInfo.FindWidget("Rank"));
		TextWidget rankNoIcon = TextWidget.Cast(m_wXPInfo.FindWidget("RankNoIcon"));
		ImageWidget rankIcon = ImageWidget.Cast(m_wXPInfo.FindWidget("RankIcon"));
		TextWidget skill = TextWidget.Cast(m_wXPInfo.FindWidget("Skill"));
		
		if (rankIconName.IsEmpty())
		{
			rankNoIcon.SetTextFormat(rankText);
			rankIcon.SetVisible(false);
			rank.SetTextFormat("");
		}
		else
		{
			rank.SetTextFormat(rankText);
			rankIcon.LoadImageTexture(0, rankIconName);
			rankIcon.SetColor(Color.FromRGBA(226,168,79,255));
			rankIcon.SetVisible(true);
			rankNoIcon.SetTextFormat("");
		}
		
		int showXP = XP;
		
		// Show XP progress bar
		ProgressBarWidget progress = ProgressBarWidget.Cast(m_wXPInfo.FindWidget("Progress"));
		ProgressBarWidget progressDiff = ProgressBarWidget.Cast(m_wXPInfo.FindWidget("ProgressDiff"));
		
		if (XP > 0)
		{
			title.SetColor(Color.FromRGBA(27, 194, 98, 255));
			progressDiff.SetColor(Color.FromRGBA(27, 194, 98, 255));
			m_bNegativeXP = false;
		}
		else
		{
			title.SetColor(Color.FromRGBA(255, 72, 74, 255));
			progressDiff.SetColor(Color.FromRGBA(255, 72, 74, 255));
			m_bNegativeXP = true;
		}
		
		if (campaignFactionManager.GetRankNext(curRank) == ECharacterRank.INVALID)	// Player at max level, no gain to show
		{
			progress.SetMin(0);
			progress.SetMax(1);
			progress.SetCurrent(1);
			progressDiff.SetMin(0);
			progressDiff.SetMax(1);
			progressDiff.SetCurrent(0);
		}
		else
		{
			if (campaignFactionManager.GetRankPrev(curRank) == ECharacterRank.INVALID && XP < 0)	// Player is renegade and losing XP, just show red bar
			{
				progress.SetMin(0);
				progress.SetMax(1);
				progress.SetCurrent(0);
				progressDiff.SetMin(0);
				progressDiff.SetMax(1);
				progressDiff.SetCurrent(1);
			}
			else
			{
				int XPCurRank = campaignFactionManager.GetRankXP(curRank);
				int XPNextRank = campaignFactionManager.GetRankXP(campaignFactionManager.GetRankNext(curRank));
					
				if (curRank == prevRank)
				{
					if (campaignFactionManager.GetRankPrev(curRank) != ECharacterRank.INVALID)	// Standard XP change
					{
						progress.SetMin(XPCurRank);
						progress.SetMax(XPNextRank);
						progressDiff.SetMin(XPCurRank);
						progressDiff.SetMax(XPNextRank);
					}
					else
					{
						progress.SetMin(totalXP - XP);	// XP change as renegade, show progress towards a normal rank from current XP
						progress.SetMax(totalXP - XP + 1);
						progressDiff.SetMin(totalXP - XP);
						progressDiff.SetMax(XPNextRank);
					}
					
					if (XP > 0)	// XP gain
					{
						progress.SetCurrent(totalXP - XP);
						progressDiff.SetCurrent(totalXP);
					}
					else	// XP loss
					{
						progress.SetCurrent(totalXP);
						progressDiff.SetCurrent(totalXP - XP);
					}
				}
				else
				{
					if (curRank > prevRank)	// Promotion
					{
						progress.SetMin(0);
						progress.SetMax(1);
						progress.SetCurrent(0);
						progressDiff.SetMin(XPCurRank);
						progressDiff.SetMax(XPNextRank);
						progressDiff.SetCurrent(totalXP);
					}
					else	// Demotion
					{
						progress.SetMin(XPCurRank);
						progress.SetMax(XPNextRank);
						progress.SetCurrent(totalXP);
						progressDiff.SetMin(0);
						progressDiff.SetMax(1);
						progressDiff.SetCurrent(1);
					}
				}
			}
		}
		
		// Show skill info
		if (XP > 0 && profileUsed)
		{
			LocalizedString skillName;
			
			switch (GetXPRewardSkill(rewardID))
			{
				case EProfileSkillID.WEAPON_HANDLER: {skillName = "#AR-Campaign_SkillWeaponSpecialist"; break;};
				case EProfileSkillID.DRIVER: {skillName = "#AR-Campaign_SkillDriver"; break;};
				case EProfileSkillID.SCOUT: {skillName = "#AR-Campaign_SkillScout"; break;};
				case EProfileSkillID.OPERATOR: {skillName = "#AR-Campaign_SkillOperator"; break;};
			}
			
			if (!skillName.IsEmpty())
				skill.SetTextFormat("#AR-Campaign_LevelInfo", skillName, skillLevel);
		}
		
		if (volunteer)
			title.SetTextFormat("#AR-Campaign_RewardBonus_Volunteer", GetXPRewardName(rewardID));
		else
			title.SetTextFormat(GetXPRewardName(rewardID));

		m_wXPInfo.SetVisible(true);
		m_fHideXPInfo = GetWorld().GetWorldTime() + XP_INFO_DURATION;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetSlotPresetResourceName(SCR_SiteSlotEntity slot, SCR_CampaignFaction givenFaction = null)
	{
		SCR_BuildingDedicatedSlotData slotData;		
		bool found = m_aDedicatedSlots.Find(slot, slotData);
		SCR_CampaignFaction faction;
		
		if (givenFaction)
			faction = givenFaction;
		else
			faction = SCR_CampaignFaction.Cast(SCR_RespawnSystemComponent.GetLocalPlayerFaction());
		
		if (!faction || !found || !slotData)
			return ResourceName.Empty;
		
		return faction.GetBuildingPrefab(slotData.GetCompositionType());
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBase GetSlotPresetBase(SCR_SiteSlotEntity slot)
	{
		SCR_BuildingDedicatedSlotData slotData;
		bool found = m_aDedicatedSlots.Find(slot, slotData);
		if (!found || !slotData)
			return null;
		
		return slotData.GetBase();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetupDaytime(int hours, int minutes, int seconds = 0)
	{
		TimeAndWeatherManagerEntity manager = GetGame().GetTimeAndWeatherManager();
		
		if (!manager)
			return;
			
		manager.SetHoursMinutesSeconds(hours, minutes, seconds);
		manager.SetDayDuration(DAY_IN_SECONDS / m_fDaytimeAcceleration);
		GetGame().GetCallqueue().Remove(HandleDaytimeAcceleration);
		GetGame().GetCallqueue().CallLater(HandleDaytimeAcceleration, 10000, true, manager);
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsMatchOver()
	{
		return m_bMatchOver;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HandleDaytimeAcceleration(notnull TimeAndWeatherManagerEntity manager)
	{
		float timeOfDay = manager.GetTimeOfTheDay();
		
		if (timeOfDay > DAYTIME_END || timeOfDay < DAYTIME_START)
		{
			if (m_bDaytimeAcceleration)
			{
				m_bDaytimeAcceleration = false;
				manager.SetDayDuration(DAY_IN_SECONDS / m_fNighttimeAcceleration);
			}
		}
		else
		{
			if (!m_bDaytimeAcceleration)
			{
				m_bDaytimeAcceleration = true;
				manager.SetDayDuration(DAY_IN_SECONDS / m_fDaytimeAcceleration);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterRemnantsPresence(notnull SCR_CampaignRemnantsSpawnPoint spawnpoint, bool defenders = false)
	{
		SCR_CampaignFaction remnantsFaction = SCR_CampaignFaction.Cast(SCR_CampaignFactionManager.GetInstance().GetFactionByKey(FACTION_INDFOR));
		
		if (!remnantsFaction)
			return;
		
		SCR_CampaignRemnantsPresence presence;
		
		if (defenders)
		{
			presence = new SCR_CampaignRemnantsPresence;
			presence.SetIsDefendersSpawn(true);
		}
		else if (spawnpoint.GetGroupType() == SCR_CampaignRemnantsGroupType.RANDOM)
		{
			// Randomize group type
			Math.Randomize(-1);
			float seed = Math.RandomFloat01();
			array<ResourceName> groups = {};
			
			remnantsFaction.GetRemnantsByProbability(seed, groups);
			
			if (groups.IsEmpty())
				return;
			
			presence = new SCR_CampaignRemnantsPresence;
			presence.SetGroupPrefab(groups.GetRandomElement());
		}
		else
		{
			ResourceName prefab = remnantsFaction.GetRemnantsByType(spawnpoint.GetGroupType());
			
			if (prefab.IsEmpty())
				return;
			
			presence = new SCR_CampaignRemnantsPresence;
			presence.SetGroupPrefab(prefab);
		}
		
		vector locationCenter = spawnpoint.GetOrigin();
		presence.SetSpawnpoint(locationCenter);
		
		// Spawn waypoints
		SCR_CampaignRemnantsSpawnPoint child = SCR_CampaignRemnantsSpawnPoint.Cast(spawnpoint.GetChildren());
		array<AIWaypoint> patrolWaypoints = {};
		array<int> waypointIndexes = {};
		int pointsCnt = 1;
		int lowestIndex = int.MAX;
		
		// First waypoint is spawned on the group spawnpoint
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = presence.GetSpawnpoint();
		AIWaypoint firstWP = AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(GetPatrolWaypointPrefab()), null, params));
		
		while (child)
		{
			vector pos = child.GetOrigin();
			locationCenter += pos;
			params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = pos;
			AIWaypoint wp = AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(GetPatrolWaypointPrefab()), null, params));
			
			// Sort waypoints by indexes
			if (wp)
			{
				int indexNew = child.GetWaypointIndex();
				int index = -1;
				
				for (int i = 0, cnt = waypointIndexes.Count(); i < cnt; i++)
				{
					if (indexNew <= waypointIndexes[i] && indexNew < lowestIndex)
					{
						index = i;
						lowestIndex = indexNew;
					}
				}
				
				if (index == -1)
				{
					patrolWaypoints.Insert(wp);
					waypointIndexes.Insert(indexNew);
					lowestIndex = indexNew;
				}
				else
				{
					patrolWaypoints.InsertAt(wp, index);
					waypointIndexes.InsertAt(indexNew, index);
				}
			}
			
			child = SCR_CampaignRemnantsSpawnPoint.Cast(child.GetSibling());
			pointsCnt++;
		}
		
		params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = presence.GetSpawnpoint();
		
		if (patrolWaypoints.IsEmpty())
		{
			SCR_DefendWaypoint defendWaypoint = SCR_DefendWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(GetDefendWaypointPrefab()), null, params));
			
			if (defendWaypoint)
				presence.SetWaypoint(defendWaypoint);
		}
		else
		{
			if (!patrolWaypoints.IsEmpty())
				patrolWaypoints.InsertAt(firstWP, 0);
			
			AIWaypointCycle cycleWaypoint = AIWaypointCycle.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(GetCycleWaypointPrefab()), null, params));
			
			if (cycleWaypoint)
			{
				cycleWaypoint.SetWaypoints(patrolWaypoints);
				cycleWaypoint.SetRerunCounter(-1);
				presence.SetWaypoint(cycleWaypoint);
			}
		}
		
		// Calculate the central location from which player presence will be checked
		locationCenter[0] = locationCenter[0] / pointsCnt;
		locationCenter[1] = locationCenter[1] / pointsCnt;
		locationCenter[2] = locationCenter[2] / pointsCnt;
		
		presence.SetCenter(locationCenter);
		
		// Locate parent base
		int distLimit = Math.Pow(SCR_CampaignRemnantsPresence.PARENT_BASE_DISTANCE_THRESHOLD, 2);
		float minDistance = float.MAX;
		SCR_CampaignBase nearestBase;
		bool register = true;
		
		foreach (SCR_CampaignBase base: SCR_CampaignBaseManager.GetBases())
		{
			if (base.GetType() == CampaignBaseType.RELAY)
				continue;
			
			float dist = vector.DistanceSqXZ(locationCenter, base.GetOrigin());
			
			if (dist > distLimit || dist > minDistance)
				continue;
			
			// Do not register remnants group if its parent base is pre-owned
			if (base.GetType() != CampaignBaseType.MAIN && (!AllowStartingBaseOwners() || base.GetStartingBaseOwner().IsEmpty() || base.GetStartingBaseOwner() == FACTION_INDFOR))
			{
				nearestBase = base;
				minDistance = dist;
				continue;
			}
			else
			{
				register = false;
				break;
			}
		}
		
		if (register)
		{
			presence.SetID(m_iRemnantLocationsCnt + 1);
			m_aRemnantsPresence.Insert(presence);
			m_iRemnantLocationsCnt++;
			
			if (nearestBase)
			{
				nearestBase.RegisterRemnants(presence);
				
				if (defenders)
				{
					presence.SetParentBase(nearestBase);
					nearestBase.UpdateBaseDefenders(presence);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetRemainingRemnantsInfo(out notnull array<int> remnantsInfo)
	{
		int size;
		
		foreach (SCR_CampaignRemnantsPresence presence: m_aRemnantsPresence)
		{
			if (!presence)
				continue;
			
			SCR_AIGroup grp = presence.GetSpawnedGroup();
			
			if (presence.GetMembersAlive() < 0 && !grp && !presence.GetIsSpawned())
				continue;
			
			remnantsInfo.Insert(presence.GetID());
			
			if (grp)
				size = remnantsInfo.Insert(grp.GetAgentsCount());
			else
			{
				if (presence.GetIsSpawned())
					size = remnantsInfo.Insert(0);
				else
					size = remnantsInfo.Insert(presence.GetMembersAlive());
			}
		}
		
		return size;
	}
	
	//------------------------------------------------------------------------------------------------
	void StoreRemnantsStates(out notnull array<ref SCR_CampaignRemnantInfoStruct> outEntries)
	{
		array<int> remnantsInfo = {};
		int cnt = GetRemainingRemnantsInfo(remnantsInfo);
		
		for (int i = 0; i < cnt; i++)
		{
			SCR_CampaignRemnantInfoStruct struct = new SCR_CampaignRemnantInfoStruct();
			struct.SetID(remnantsInfo[i]);
			struct.SetMembersAlive(remnantsInfo[i + 1]);
			outEntries.Insert(struct);
			i++;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadRemnantsStates(notnull array<ref SCR_CampaignRemnantInfoStruct> entries)
	{
		foreach (SCR_CampaignRemnantsPresence presence: m_aRemnantsPresence)
		{
			if (!presence)
				continue;
			
			foreach (SCR_CampaignRemnantInfoStruct info: entries)
			{
				if (info.GetID() == presence.GetID())
					presence.SetMembersAlive(info.GetMembersAlive());
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add a new waypoint picked from current task list to a specific group
	//! \param group Group to assign a new waypoint to
	protected void AddAIWaypoint(SCR_AIGroup group)
	{
		if (!group)
			return;

		AIWaypoint waypoint = group.GetCurrentWaypoint();

		// If the group already has a waypoint assigned, stop - unless it is the depot helper one
		if (waypoint && waypoint.GetParent() && waypoint.GetParent().Type() != SCR_CampaignDeliveryPoint)
			return;

		string factionKey = group.m_faction;
		
		Faction faction = SCR_CampaignFactionManager.GetInstance().GetFactionByKey(factionKey);
		
		SCR_CampaignTaskManager taskManager = SCR_CampaignTaskManager.GetCampaignTaskManagerInstance();
		if (!taskManager)
			return;

		array<SCR_BaseTask> availableTasks = new array<SCR_BaseTask>();
		array<SCR_CampaignTask> usableTasks = new array<SCR_CampaignTask>();	// only tasks that have a waypoint assigned
		taskManager.GetFilteredTasks(availableTasks, faction);
		
		foreach (int currentIndex, SCR_BaseTask taskBase: availableTasks)
		{
			SCR_CampaignTask thisTask = SCR_CampaignTask.Cast(taskBase);

			if (thisTask.GetAIWaypoint() && thisTask.GetTaskState() == SCR_TaskState.OPENED)
			{
				usableTasks.Insert(thisTask);
			}
		};

		if (usableTasks.Count() != 0)
		{
			SCR_CampaignTask newTask = usableTasks.GetRandomElement();
			waypoint = newTask.GetAIWaypoint();

			if (waypoint)
				group.AddWaypoint(waypoint);
		};
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update waypoints for the AIs
	//! \param baseTask The task whose state just changed
	//! \param newTaskState New state of the task
	void RefreshAIWaypoints(SCR_BaseTask baseTask, SCR_TaskState newTaskState)
	{
		//Complete waypoints for groups that were doing the completed task, assign a new waypoint for all idle groups
		foreach (int currentIndex, SCR_AIGroup group: m_aAIGroups)
		{
			if (!group)
				continue;
			
			if (baseTask && newTaskState)
			{
				SCR_CampaignTask task = SCR_CampaignTask.Cast(baseTask);
				if (!task)
					continue;

				string factionKey = task.GetTargetFaction().GetFactionKey();
				if (newTaskState != 0 && group.m_faction == factionKey)
				{
					AIWaypoint waypoint = group.GetCurrentWaypoint();
					if (waypoint)
					{
						if (waypoint == task.GetAIWaypoint())
						{
							group.CompleteWaypoint(waypoint);
						}
					};
				};
			}

			AddAIWaypoint(group);
		};
	}
	
	//------------------------------------------------------------------------------------------------
	void GetRequestedVehicles(out notnull array<Vehicle> vehicleList)
	{
		vehicleList = m_aRequestedVehicles;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBuildingInterfaceClosed()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	// Triggered each time player build a composition. Slot is the slot he used.
	void OnStructureBuilt(SCR_SiteSlotEntity slot, SCR_CampaignNetworkComponent networkComponent)
	{
		SCR_CampaignDeliveryPoint service;
		SCR_CampaignBase foundBase;
		ECampaignServicePointType serviceType;
		int ticketsBonus;
		
		// Find out if the slot is used for a service structure in some base
		// If found, raise the cap on respawn tickets
		foreach (SCR_CampaignBase base: SCR_CampaignBaseManager.GetBases())
		{
			if (!base)
				continue;
			
			if (base.GetAssignedSlot(ECampaignCompositionType.FUEL) == slot) {foundBase = base; serviceType = ECampaignServicePointType.FUEL_DEPOT; service = base.GetBaseService(serviceType); ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_FUEL; break;};
			if (base.GetAssignedSlot(ECampaignCompositionType.REPAIR) == slot) {foundBase = base; serviceType = ECampaignServicePointType.REPAIR_DEPOT; service = base.GetBaseService(serviceType); ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_REPAIR; break;};
			if (base.GetAssignedSlot(ECampaignCompositionType.ARMORY) == slot) {foundBase = base; serviceType = ECampaignServicePointType.ARMORY; service = base.GetBaseService(serviceType); ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_ARMORY; break;};
			if (base.GetAssignedSlot(ECampaignCompositionType.HOSPITAL) == slot) {foundBase = base; serviceType = ECampaignServicePointType.FIELD_HOSPITAL; service = base.GetBaseService(serviceType); ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_HOSPITAL; break;};
			if (base.GetAssignedSlot(ECampaignCompositionType.BARRACKS) == slot) {foundBase = base; serviceType = ECampaignServicePointType.BARRACKS; service = base.GetBaseService(serviceType); ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_BARRACKS; break;};
		}
		
		if (foundBase)
		{
			foundBase.AddRespawnTicketsMax(ticketsBonus);
			s_OnServiceBuild.Invoke(foundBase, networkComponent);
		}
						
		if (service)
		{
			service.SetIsBuilt(true);
			
			if (foundBase)
			{
				SCR_CampaignFaction owner = foundBase.GetOwningFaction();
				
				if (owner && foundBase.IsBaseInFactionRadioSignal(owner))
					switch (serviceType)
					{
						case ECampaignServicePointType.FUEL_DEPOT: {owner.SendHQMessage(SCR_ERadioMsg.BUILT_FUEL, param: foundBase.GetBaseID()); break;};
						case ECampaignServicePointType.ARMORY: {owner.SendHQMessage(SCR_ERadioMsg.BUILT_ARMORY, param: foundBase.GetBaseID()); break;};
						case ECampaignServicePointType.REPAIR_DEPOT: {owner.SendHQMessage(SCR_ERadioMsg.BUILT_REPAIR, param: foundBase.GetBaseID()); break;};
					}
			}
		}
		
		// Repir depot was built, handle periodic vehicle spawning
		if (foundBase && service == foundBase.GetBaseService(ECampaignServicePointType.REPAIR_DEPOT))
		{
			GetGame().GetCallqueue().CallLater(service.HandleVehicleSpawn, 1000); // Spawn the first vehicle right away
			GetGame().GetCallqueue().CallLater(service.HandleVehicleSpawn, GARAGE_VEHICLE_SPAWN_INTERVAL, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Triggered each time when the strcture status (damaged / repaired) is changed. Base is the base to which the composition belongs to.
	void OnStructureChanged(notnull SCR_CampaignBase base, SCR_SiteSlotEntity slot, SCR_CampaignDeliveryPoint deliveryPoint, bool add)
	{
		int ticketsBonus;
		bool found;
		SCR_BuildingDedicatedSlotData slotData;	
		ECampaignServicePointType serviceType;	
		
		if (slot)
		{
			found = m_aDedicatedSlots.Find(slot, slotData);
		
			if (!found)
				return;
			
			switch (slotData.GetCompositionType())
	        {
				case ECampaignCompositionType.FUEL: {serviceType = ECampaignServicePointType.FUEL_DEPOT; ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_FUEL; break;}
				case ECampaignCompositionType.REPAIR: {serviceType = ECampaignServicePointType.REPAIR_DEPOT; ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_REPAIR; break;}
				case ECampaignCompositionType.ARMORY: {serviceType = ECampaignServicePointType.ARMORY; ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_ARMORY; break;}
				case ECampaignCompositionType.HOSPITAL: {serviceType = ECampaignServicePointType.FIELD_HOSPITAL; ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_HOSPITAL; break;}
				case ECampaignCompositionType.BARRACKS: {serviceType = ECampaignServicePointType.BARRACKS; ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_BARRACKS; break;}
			};
		}
		else if (deliveryPoint)
		{
			serviceType = deliveryPoint.GetServiceType();
			
			switch (serviceType)
			{
				case ECampaignServicePointType.FUEL_DEPOT: {ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_FUEL; break;}
				case ECampaignServicePointType.REPAIR_DEPOT: {ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_REPAIR; break;}
				case ECampaignServicePointType.ARMORY: {ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_ARMORY; break;}
				case ECampaignServicePointType.FIELD_HOSPITAL: {ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_HOSPITAL; break;}
				case ECampaignServicePointType.BARRACKS: {ticketsBonus = RESPAWN_TICKETS_MAX_BONUS_BARRACKS; break;}
			}
		}
		
		SCR_CampaignFaction owner = base.GetOwningFaction();
				
		if (add)
		{
			base.AddRespawnTicketsMax(ticketsBonus);
			
			if (owner && base.IsBaseInFactionRadioSignal(owner))
				switch (serviceType)
				{
					case ECampaignServicePointType.FUEL_DEPOT: {owner.SendHQMessage(SCR_ERadioMsg.REPAIRED_FUEL, param: base.GetBaseID()); break;};
					case ECampaignServicePointType.ARMORY: {owner.SendHQMessage(SCR_ERadioMsg.REPAIRED_ARMORY, param: base.GetBaseID()); break;};
					case ECampaignServicePointType.REPAIR_DEPOT: {owner.SendHQMessage(SCR_ERadioMsg.REPAIRED_REPAIR, param: base.GetBaseID()); break;};
				}
		}
		else
		{
			base.AddRespawnTicketsMax(-ticketsBonus);	
			
			if (owner && base.IsBaseInFactionRadioSignal(owner))
				switch (serviceType)
				{
					case ECampaignServicePointType.FUEL_DEPOT: {owner.SendHQMessage(SCR_ERadioMsg.DESTROYED_FUEL, param: base.GetBaseID()); break;};
					case ECampaignServicePointType.ARMORY: {owner.SendHQMessage(SCR_ERadioMsg.DESTROYED_ARMORY, param: base.GetBaseID()); break;};
					case ECampaignServicePointType.REPAIR_DEPOT: {owner.SendHQMessage(SCR_ERadioMsg.DESTROYED_REPAIR, param: base.GetBaseID()); break;};
				}
		}		
	}
		
	//------------------------------------------------------------------------------------------------
	// Time in seconds for how long only the player who requested the vehicle can get in as driver.
	int GetVehicleProtectionTime()
	{
		return m_iSpawnedVehicleTimeProtection;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsTutorial(bool isTutorial)
	{
		m_bIsTutorial = isTutorial;
		
		if (m_bIsTutorial)
			SCR_PopUpNotification.SetFilter(SCR_EPopupMsgFilter.TUTORIAL);
		else
			SCR_PopUpNotification.SetFilter(SCR_EPopupMsgFilter.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsTutorial()
	{
		return m_bIsTutorial;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddActiveRespawnRadio(FactionKey faction)
	{
		switch (faction)
		{
			case FACTION_BLUFOR: {m_iActiveRespawnRadiosCntWest++; break;};
			case FACTION_OPFOR: {m_iActiveRespawnRadiosCntEast++; break;};
		}
		
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveActiveRespawnRadio(FactionKey faction)
	{
		bool reactivate = false;
		
		switch (faction)
		{
			case FACTION_BLUFOR: {if (m_iActiveRespawnRadiosCntWest == GetMaxRespawnRadios()) reactivate = true; m_iActiveRespawnRadiosCntWest--; break;};
			case FACTION_OPFOR: {if (m_iActiveRespawnRadiosCntEast == GetMaxRespawnRadios()) reactivate = true; m_iActiveRespawnRadiosCntEast--; break;};
		}
		
		Replication.BumpMe();
		
		// Check all players for radios if limit is no longer maxed, activate a dormant one
		if (reactivate)
		{
			array<SCR_SpawnPoint> allSpawnpoints = SCR_SpawnPoint.GetSpawnPoints();
			PlayerManager pMan = GetGame().GetPlayerManager();
			
			if (!pMan)
				return;
			
			foreach (SCR_SpawnPoint spawnpoint: allSpawnpoints)
			{
				SCR_PlayerRadioSpawnPointCampaign campaignSpawnpoint = SCR_PlayerRadioSpawnPointCampaign.Cast(spawnpoint);
				
				if (!campaignSpawnpoint)
					continue;
				
				if (campaignSpawnpoint.GetFactionKey() == faction)
					continue;
				
				SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(pMan.GetPlayerControlledEntity(campaignSpawnpoint.GetPlayerID()));
				
				if (!player)
					continue;
				
				CharacterControllerComponent charController = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));
				
				if (!charController)
					continue;
				
				if (charController.IsDead())
					continue;
				
				Faction playerFaction = player.GetFaction();
				
				if (!playerFaction)
					continue;
				
				if (playerFaction.GetFactionKey() != faction)
					continue;
				
				BaseLoadoutManagerComponent loadoutManager = BaseLoadoutManagerComponent.Cast(player.FindComponent(BaseLoadoutManagerComponent));
		
				if (!loadoutManager)
					continue;
				
				IEntity backpack = loadoutManager.GetClothByArea(ELoadoutArea.ELA_Backpack);
				
				if (!backpack)
					continue;
				
				BaseLoadoutClothComponent loadoutCloth = BaseLoadoutClothComponent.Cast(backpack.FindComponent(BaseLoadoutClothComponent));
				
				if (loadoutCloth && loadoutCloth.GetArea() == ELoadoutArea.ELA_Backpack && backpack.FindComponent(SCR_RadioComponent))
				{
					campaignSpawnpoint.DeactivateSpawnPointPublic();
					campaignSpawnpoint.ActivateSpawnPointPublic();
					CheckRadioSpawnpointsSignalCoverage();
					
					if (GetActiveRespawnRadiosCount(faction) == GetMaxRespawnRadios())
						return;
					else
						continue;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMapCampaignUI(SCR_MapCampaignUI mapUi)
	{
		m_MapCampaignUI = mapUi;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpawnTime()
	{
		TimeAndWeatherManagerEntity manager = GetGame().GetTimeAndWeatherManager();
		
		if(manager)
				m_SpawnTime = manager.GetTime();
		else
			Print("Time And Weather manager not found", LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	TimeContainer GetSpawnTime()
	{
		return m_SpawnTime;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetActiveRespawnRadiosCount(FactionKey faction)
	{
		switch (faction)
		{
			case FACTION_BLUFOR: {return m_iActiveRespawnRadiosCntWest;};
			case FACTION_OPFOR: {return m_iActiveRespawnRadiosCntEast;};
		}
		
		return 0;
	}
	
	//************************//
	//PROTECTED MEMBER METHODS//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	//! Returns faction of the player with the given ID
	Faction GetPlayerFaction(int playerID)
	{		
		if (s_RespawnSystemComponent)
			return s_RespawnSystemComponent.GetPlayerFaction(playerID);
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemAdded(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		BaseLoadoutClothComponent loadoutCloth = BaseLoadoutClothComponent.Cast(item.FindComponent(BaseLoadoutClothComponent));
		
		if (loadoutCloth && loadoutCloth.GetArea() == ELoadoutArea.ELA_Backpack && item.FindComponent(SCR_RadioComponent))
		{
			SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(item.GetParent());
			
			if (!player)
				return;
			
			SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(player.GetFaction());
		
			if (!faction)
				return;
			
			// Radio was picked up when the spawnpoint limit has been reached, disable its spawnpoit
			if (GetActiveRespawnRadiosCount(faction.GetFactionKey()) >= GetMaxRespawnRadios())
			{
				PlayerManager pMan = GetGame().GetPlayerManager();
			
				if (!pMan)
					return;
				
				if (!storageOwner)
					return;
				
				int playerId = pMan.GetPlayerIdFromControlledEntity(storageOwner.GetOwner());
				array<SCR_SpawnPoint> allSpawnpoints = SCR_SpawnPoint.GetSpawnPoints();
				SCR_PlayerRadioSpawnPointCampaign campaignSpawnpoint;
				
				foreach (SCR_SpawnPoint spawnpoint: allSpawnpoints)
				{
					campaignSpawnpoint = SCR_PlayerRadioSpawnPointCampaign.Cast(spawnpoint);
					
					if (!campaignSpawnpoint)
						continue;
					
					if (campaignSpawnpoint.GetPlayerID() != playerId)
						continue;
					
					campaignSpawnpoint.DeactivateSpawnPointPublic();
					return;
				}
			}
			
			AddActiveRespawnRadio(faction.GetFactionKey());
			CheckRadioSpawnpointsSignalCoverage();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemRemoved(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(storageOwner.GetOwner());
		
		if (!player)
			return;
		
		CharacterControllerComponent controller = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));
		
		if (!controller)
			return;
		
		if (controller.IsDead())
			return;
		
		BaseLoadoutClothComponent loadoutCloth = BaseLoadoutClothComponent.Cast(item.FindComponent(BaseLoadoutClothComponent));
		
		if (loadoutCloth && loadoutCloth.GetArea() == ELoadoutArea.ELA_Backpack && item.FindComponent(SCR_RadioComponent))
		{
			SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(player.GetFaction());
		
			if (!faction)
				return;
			
			RemoveActiveRespawnRadio(faction.GetFactionKey());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Outputs count of owned bases for each faction into the factionsOwnedBasesCount array
	protected void CountFactionsOwnedBases(array<Faction> factions, out array<int> factionsOwnedBasesCount)
	{
		// The array doesn't exist, do nothing
		if (!factionsOwnedBasesCount)
			return;
		
		// Make sure the array is cleared
		factionsOwnedBasesCount.Clear();
		
		// For each faction
		for (int i = 0, count = factions.Count(); i < count; i++)
		{
			// Add new entry into the factionsOwnedBasesCount array
			Faction faction = factions[i];
			factionsOwnedBasesCount.Insert(0);
			
			// Check each registered base and increment entry for the current faction in factionsOwnedBasesCount...
			// ...If the base is owned by this faction
			foreach (SCR_CampaignBase base : SCR_CampaignBaseManager.GetBases())
			{
				switch (base.GetType())
				{
					case CampaignBaseType.RELAY:
					{
						break;
					}
					case CampaignBaseType.SMALL:
					{
						break;
					}
					case CampaignBaseType.MAJOR:
					{
						/*if (faction == base.GetOwningFaction())
							factionsOwnedBasesCount[i] = factionsOwnedBasesCount[i] + 1;*/
						break;
					}
					case CampaignBaseType.MAIN:
					{
						if (faction == base.GetOwningFaction())
							factionsOwnedBasesCount[i] = factionsOwnedBasesCount[i] + 1;
						break;
					}
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckPlayerInsideRadioRange()
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();
		if (!fManager) return;
		
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player) return;
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_RespawnSystemComponent.GetLocalPlayerFaction());
		if (!faction) return;
		
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		if (!baseManager) return;
		
		bool isInRangeNow = baseManager.IsEntityInFactionRadioSignal(player, faction);
		
		if (isInRangeNow != m_bIsPlayerInRadioRange)
		{
			m_bIsPlayerInRadioRange = isInRangeNow;
			
			if (isInRangeNow)
				SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_RadioRangeEntered-UC", duration: 3);
			else
				SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_RadioRangeLeft-UC", duration: 3);
			
			SCR_UISoundEntity.SoundEvent("SOUND_RADIO_ROGERBEEP");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckRadioSpawnpointsSignalCoverage()
	{
		array<SCR_SpawnPoint> spawnpoints = SCR_SpawnPoint.GetSpawnPoints();
		SCR_CampaignFactionManager factionM = SCR_CampaignFactionManager.GetInstance();
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		
		if (!baseManager || !factionM)
			return;
		
		foreach (SCR_SpawnPoint spawnpoint: spawnpoints)
		{	
			SCR_PlayerRadioSpawnPointCampaign spawnpointC = SCR_PlayerRadioSpawnPointCampaign.Cast(spawnpoint);
			
			if (!spawnpointC)
				continue;
			
			if (spawnpointC.GetFlags() & EntityFlags.STATIC)
				continue;
			
			Faction faction = spawnpointC.GetCachedFaction();
			
			if (!faction)
				continue;
			
			bool isInRange = baseManager.IsEntityInFactionRadioSignal(spawnpointC, faction);
			
			if (isInRange)
				spawnpointC.SetFaction(faction);
			else
				spawnpointC.SetFaction(null);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowHint(SCR_ECampaignHints hintID)
	{
		if (m_bIsTutorial)
			return;
		
		switch (hintID)
		{
			case SCR_ECampaignHints.SIGNAL:
			{	
				if (!m_bStartupHintsShown)
				{
					m_bStartupHintsShown = true;
					SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_Signal_Text", "#AR-Campaign_GamemodeName", 30, fieldManualEntry: EFieldManualEntryId.CONFLICT_OVERVIEW);
					GetGame().GetCallqueue().CallLater(ShowHint, 32000, false, SCR_ECampaignHints.SERVICES);
				}
				else
				{
					if (!m_bRespawnHintShown)
					{
						m_bRespawnHintShown = true;
						ShowHint(SCR_ECampaignHints.RESPAWN);
					}
				}
					
				return;
			}
			
			case SCR_ECampaignHints.SERVICES:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_ServicesText", "#AR-Campaign_Hint_Services_Title", 30, fieldManualEntry: EFieldManualEntryId.CONFLICT_SERVICES);
				return;
			}
			
			case SCR_ECampaignHints.RESPAWN:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_Respawn_Text", "#AR-Campaign_Hint_Respawn_Title", 20, fieldManualEntry: EFieldManualEntryId.CONFLICT_RESPAWN);
				return;
			}
			
			case SCR_ECampaignHints.SEIZING:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_EnteringEnemyBase_Text", "#AR-Campaign_EnteringEnemyBase", 15, fieldManualEntry: EFieldManualEntryId.CONFLICT_SEIZING_BASES);
				m_iBaseSeizingHintsShown++;
				return;
			}
			
			case SCR_ECampaignHints.SUPPLY_RUNS:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_SupplyRun_Text", "#AR-Campaign_Hint_SupplyRun_Title", 30, fieldManualEntry: EFieldManualEntryId.CONFLICT_SUPPLIES);
				return;
			}
			
			case SCR_ECampaignHints.SUPPLIES_UNLOADING:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_SupplyRunUnload_Text", "#AR-Campaign_Hint_SupplyRun_Title", 10, fieldManualEntry: EFieldManualEntryId.CONFLICT_SUPPLIES);
				m_iSuppliesHintsShown++;
				return;
			}
			
			case SCR_ECampaignHints.KILL_XP:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_KillXP_Text", "#AR-Campaign_Hint_KillXP_Title", 10, fieldManualEntry: EFieldManualEntryId.CONFLICT_RANKS);
				return;
			}
			
			case SCR_ECampaignHints.MOBILE_ASSEMBLY:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_MobileAssembly_Text", "#AR-Vehicle_MobileAssembly_Name", 30, fieldManualEntry: EFieldManualEntryId.CONFLICT_MHQ);
				return;
			}
			
			case SCR_ECampaignHints.REINFORCEMENTS:
			{
				if (!m_bReinforcementsHintShown)
				{
					m_bReinforcementsHintShown = true;
					SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_Reinforcements_Text", "#AR-Campaign_Hint_Reinforcements_Title", 20, fieldManualEntry: EFieldManualEntryId.CONFLICT_REINFORCEMENTS);
				}
				
				return;
			}
			
			case SCR_ECampaignHints.TICKETS:
			{
				if (!m_bTicketsHintShown)
				{
					m_bTicketsHintShown = true;
					SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_Supplies_Text", "#AR-Campaign_Hint_Services_Title", 30, fieldManualEntry: EFieldManualEntryId.CONFLICT_RESPAWN);
				}
				
				return;
			}
			
			case SCR_ECampaignHints.BASE_LOST:
			{
				if (!m_bBaseLostHintShown)
				{
					m_bBaseLostHintShown = true;
					SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_BaseLost_Text", "#AR-Campaign_Hint_BaseLost_Title", 15, fieldManualEntry: EFieldManualEntryId.CONFLICT_SEIZING_BASES);
				}
				
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EnteringNewBase()
	{
		SCR_CampaignFaction f = SCR_CampaignFaction.Cast(SCR_RespawnSystemComponent.GetLocalPlayerFaction());
		if (!f) return;
		
		if (m_FirstBaseWithPlayer && m_FirstBaseWithPlayer != m_LastVisitedBase)
		{
			m_LastVisitedBase = m_FirstBaseWithPlayer;
			
			// Entering enemy base
			if (m_iBaseSeizingHintsShown < 5 && m_FirstBaseWithPlayer.GetOwningFaction() != f && m_FirstBaseWithPlayer.IsBaseInFactionRadioSignal(f))
			{
				ShowHint(SCR_ECampaignHints.SEIZING);
				
				//Play music theme
				SCR_MusicManager musicManager = SCR_MusicManager.GetInstance();				
				if (musicManager)
				{
					musicManager.PlayPriorityAmbientOneShot("SOUND_ONENTERINGENEMYBASE", true, false);
				}
			}
			
			// Entering a friendly base in a supply truck
			if (m_iSuppliesHintsShown < 5 && m_FirstBaseWithPlayer.GetOwningFaction() == f)
			{
				ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
				if (!player) return;
				
				if (!player.IsInVehicle()) return;
				
				CompartmentAccessComponent compartmentAccessComponent = CompartmentAccessComponent.Cast(player.FindComponent(CompartmentAccessComponent));
				if (!compartmentAccessComponent) return;
				
				BaseCompartmentSlot compartmentSlot = compartmentAccessComponent.GetCompartment();
				if (!compartmentSlot) return;
				
				Vehicle vehicle = Vehicle.Cast(compartmentSlot.GetOwner());
				if (!vehicle) return;
				
				SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(vehicle.FindComponent(SCR_CampaignSuppliesComponent));
				if (!suppliesComponent) return;
				
				if (m_FirstBaseWithPlayer.GetType() == CampaignBaseType.SMALL)
				{
					if (suppliesComponent.GetSupplies() != 0)
						ShowHint(SCR_ECampaignHints.SUPPLIES_UNLOADING);
				}
				else
				{
					if (m_FirstBaseWithPlayer.GetType() != CampaignBaseType.RELAY)
					{
						if (suppliesComponent.GetSupplies() != suppliesComponent.GetSuppliesMax())
						{
							m_iSuppliesHintsShown++;
							SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_SupplyRunLoad_Text", "#AR-Campaign_Hint_SupplyRun_Title", 10, fieldManualEntry: EFieldManualEntryId.CONFLICT_SUPPLIES);
						}
					}
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Evaluates every factions owned bases count and decides whether there is any winning faction
	protected Faction GetWinner(array<Faction> factions, array<int> factionsOwnedBasesCount)
	{
		// Assume anyone is a winner
		array<Faction> winners = new array<Faction>();
		winners.Copy(factions);
		
		int losersCount = 0;
		
		// Check for loosers
		for (int i = factions.Count() - 1; i >= 0; i--)
		{
			SCR_CampaignFaction fCast = SCR_CampaignFaction.Cast(factions[i]);
			
			if (!fCast)
				continue;
			
			if (!fCast.IsPlayable())
				continue;
			
			// Any faction with 0 bases is considered a looser
			if (factionsOwnedBasesCount[i] <= 0)
			{
				losersCount++;
				
				// Therefore remove it from the possible winners array
				winners.Remove(i)
			}
		}
		
		// Nobody lost, therefore none is winner
		if (losersCount == 0)
			return null;
		else
		{
			// There have been some loosers, everyone else is a winner, random pick one
			// This will most likely never be used, since we assume only 2 factions fighting.
			int winnersCount = winners.Count();
			
			// At least one faction can win
			if (winnersCount > 0)
			{
				int index = Math.RandomInt(0, winnersCount);
				return winners[index];
			}
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ProcessFactionAssignment(Faction assignedFaction)
	{
		m_LastPlayerFaction = assignedFaction;
		s_OnFactionAssignedLocalPlayer.Invoke(assignedFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	protected Widget CreateMuteWidget()
	{
		return GetGame().GetWorkspace().CreateWidgets("{F8FA8AA1F524640D}UI/layouts/HUD/CampaignMP/CampaignMutePlayerLine.layout", m_wPlayersListSlot);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RegisterTasksShown()
	{
		SCR_PopUpNotification.GetInstance().HideCurrentPopupMsg();
		GetGame().GetInputManager().RemoveActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
		GetGame().GetInputManager().RemoveActionListener("GadgetMap", EActionTrigger.DOWN, RegisterTasksShown);
	}
	
	//------------------------------------------------------------------------------------------------
	//Checks whether some faction won the game
	protected void EvaluateGame()
	{
		array<Faction> factions = new array<Faction>();
		array<int> factionsOwnedBasesCount = new array<int>();
		
		// Get all factions
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();
		
		if (!fManager)
			return;
		
		fManager.GetFactionsList(factions);
		
		for (int i = 0, cnt = factions.Count(); i < cnt; i++)
		{
			SCR_Faction playable = SCR_Faction.Cast(factions[i]);
			
			if(playable && !playable.IsPlayable())
			{
				factions.Remove(i);
				cnt--;
				i--;
			}
		}
		
		// Count their bases
		CountFactionsOwnedBases(factions, factionsOwnedBasesCount);
		
		// Check if any faction won based on their owned bases
		Faction winner = GetWinner(factions, factionsOwnedBasesCount);
		
		// Some faction won, show it to everyone
		if (winner)
		{
			m_bMatchOver = true;
			
			// Match is over, save it so if "Continue" is selected following this the game is not loaded at an end screen
			if (!IsProxy())
			{
				SCR_SaveLoadComponent saveLoadComponent = SCR_SaveLoadComponent.GetInstance();
				saveLoadComponent.Save();
			}
			
			m_fGameEnd = Replication.Time() + ENDING_TIMEOUT;
			int factionID = fManager.GetFactionIndex(winner);
			
			foreach (Faction faction: factions)
			{
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(faction);
			
				if (f)
				{
					if (winner == f)
						f.SendHQMessage(SCR_ERadioMsg.VICTORY, param: factionID);
					else
						f.SendHQMessage(SCR_ERadioMsg.DEFEAT, param: factionID);
				}
			}
			
			// For the server end the game, replicate to all clients.
			// listening components can react to this by e.g. showing end screen
			if (IsMaster())
			{
				SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(SCR_GameModeEndData.ENDREASON_SCORELIMIT, winnerFactionId: factionID);
				super.EndGameMode(endData);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Process UI
	//! \param timeSlice Time since last update
	protected void ProcessUI(float timeSlice)
	{
		if (!m_FirstBaseWithPlayer)
		{
			if (m_iLastStatusMsgShown == eCampaignStatusMessage.SEIZING_YOU)
				SCR_PopUpNotification.GetInstance().ToggleStatusMsg(eCampaignStatusMessage.SEIZING_YOU, false);
			
			if (m_iLastStatusMsgShown == eCampaignStatusMessage.SEIZING_ENEMY)
				SCR_PopUpNotification.GetInstance().ToggleStatusMsg(eCampaignStatusMessage.SEIZING_ENEMY, false);
			
			m_iLastStatusMsgShown = -1;
		}
		else
		{
			SCR_CampaignFaction baseWithPlayerCapturingFaction = SCR_CampaignFaction.Cast(m_FirstBaseWithPlayer.GetCapturingFaction());
			
			if (!baseWithPlayerCapturingFaction)
			{
				if (m_iLastStatusMsgShown == eCampaignStatusMessage.SEIZING_YOU)
					SCR_PopUpNotification.GetInstance().ToggleStatusMsg(eCampaignStatusMessage.SEIZING_YOU, false);
			
				if (m_iLastStatusMsgShown == eCampaignStatusMessage.SEIZING_ENEMY)
					SCR_PopUpNotification.GetInstance().ToggleStatusMsg(eCampaignStatusMessage.SEIZING_ENEMY, false);
				
				m_iLastStatusMsgShown = -1;
			}
			else
			{
				GenericEntity player = GenericEntity.Cast(SCR_PlayerController.GetLocalControlledEntity());
				
				if (!player)
					return;
				
				FactionAffiliationComponent comp = FactionAffiliationComponent.Cast(player.FindComponent(FactionAffiliationComponent));
				float captureStart = m_FirstBaseWithPlayer.GetCaptureStartTimestamp();
				
				if (baseWithPlayerCapturingFaction == comp.GetAffiliatedFaction())
				{
					if (m_iLastStatusMsgShown != eCampaignStatusMessage.SEIZING_YOU && m_FirstBaseWithPlayer.GetReconfiguredByID() != SCR_PlayerController.GetLocalPlayerId())
						SCR_PopUpNotification.GetInstance().ToggleStatusMsg(eCampaignStatusMessage.SEIZING_YOU, true, captureStart, captureStart + (SCR_CampaignBase.RADIO_RECONFIGURATION_DURATION * 1000));
					
					if (m_iLastStatusMsgShown == eCampaignStatusMessage.SEIZING_ENEMY)
						SCR_PopUpNotification.GetInstance().ToggleStatusMsg(eCampaignStatusMessage.SEIZING_ENEMY, false);
					
					m_iLastStatusMsgShown = eCampaignStatusMessage.SEIZING_YOU;
				}
				else
				{
					if (m_iLastStatusMsgShown != eCampaignStatusMessage.SEIZING_ENEMY)
						SCR_PopUpNotification.GetInstance().ToggleStatusMsg(eCampaignStatusMessage.SEIZING_ENEMY, true, captureStart, captureStart + (SCR_CampaignBase.RADIO_RECONFIGURATION_DURATION * 1000));
					
					if (m_iLastStatusMsgShown == eCampaignStatusMessage.SEIZING_YOU)
						SCR_PopUpNotification.GetInstance().ToggleStatusMsg(eCampaignStatusMessage.SEIZING_YOU, false);
					
					m_iLastStatusMsgShown = eCampaignStatusMessage.SEIZING_ENEMY;
				}
			}
		};
		
		float curTime = GetWorld().GetWorldTime();
		
		// Hide XP info if timer runs out
		if (m_wXPInfo && m_wXPInfo.IsVisible())
		{
			if(curTime > m_fHideXPInfo)
			{
				TextWidget title = TextWidget.Cast(m_wXPInfo.FindWidget("Title"));
				title.SetTextFormat("");
			}
			if (curTime > m_fHideXPInfo && !m_bIsShowingXPBar)
				m_wXPInfo.SetVisible(false);
			else
			{
				if (curTime > (m_fHideXPInfo - (XP_INFO_DURATION * 0.7)) && curTime <= m_fHideXPInfo)
				{
					ProgressBarWidget progressDiff = ProgressBarWidget.Cast(m_wXPInfo.FindWidget("ProgressDiff"));
					ProgressBarWidget progress = ProgressBarWidget.Cast(m_wXPInfo.FindWidget("Progress"));
					Color color1 = progressDiff.GetColor();
					Color color2;
					
					if (m_bNegativeXP)
						color2 = Color.FromRGBA(159, 40, 40, 0);
					else
						color2 = progress.GetColor();
					
					color1.Lerp(color2, Math.InverseLerp(m_fHideXPInfo - (XP_INFO_DURATION * 0.7), m_fHideXPInfo, curTime));
					progressDiff.SetColor(color1);
				}	
			}	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn new group for given faction 
	//! \param factionKey Valid faction key
	protected SCR_AIGroup SpawnNewGroup(string factionKey, SCR_AIGroup oldGrp = null)
	{
		return null; // disabed for now
		
		SCR_CampaignFaction f = SCR_CampaignFaction.Cast(SCR_CampaignFactionManager.GetInstance().GetFactionByKey(factionKey));
		
		if (!f)
			return null;
		
		SCR_CampaignBase HQ = f.GetMainBase();
		
		if (!HQ)
			return null;
		
		SCR_SpawnPoint baseSpawnPoint = HQ.GetBaseSpawnPoint();
		
		if (!baseSpawnPoint)
			return null;
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		vector position = baseSpawnPoint.GetOrigin();
		params.Transform[3] = position;
		Resource res = Resource.Load(f.GetAIGroupPrefab());
		
		if (!res)
			return null;
		
		SCR_AIGroup newGrp = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(res, null, params));
		
		if (!newGrp)
			return null;

		if (factionKey == FACTION_OPFOR)
			newGrp.SetMaxUnitsToSpawn(m_iAIGroupSizeEast);
		else
			newGrp.SetMaxUnitsToSpawn(m_iAIGroupSizeWest);

		newGrp.SpawnUnits();
		m_aAIGroups.Insert(newGrp);
		
		if (GetGame().AreGameFlagsSet( EGameFlags.SpawnVehicles ))
			SpawnAIGroupVehicle(newGrp, baseSpawnPoint, oldGrp);
	
		return newGrp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn new vehicle for given AI group
	//! \param group AI group to occupy the vehicle
	//! \param spawnPoint Spawn point used to spawn the group
	protected void SpawnAIGroupVehicle(SCR_AIGroup group, IEntity spawnPoint, SCR_AIGroup oldGrp = null)
	{
		// Get the base the AI group spawned at
		IEntity base = spawnPoint;
		
		while (base && base.Type() != SCR_CampaignBase)
		{
			base = base.GetParent();
		}
		
		if (!base || SCR_CampaignBase.Cast(base).GetType() != CampaignBaseType.MAIN)
			return;

		// Check if the base has got any AI vehicle spawnpoints
		SCR_CampaignDeliveryPoint depot = SCR_CampaignBase.Cast(base).GetBaseService(ECampaignServicePointType.VEHICLE_DEPOT);
		
		if (!depot)
			return;
		
		IEntity child = depot.GetChildren();
		BaseGameTriggerEntity trg;
		SCR_TimedWaypoint wp;
		
		while (child)
		{
			// If a loiter waypoint is present, save it for later
			if (child.Type() == SCR_TimedWaypoint)
				wp = SCR_TimedWaypoint.Cast(child);
			
			BaseGameTriggerEntity thisTrg = BaseGameTriggerEntity.Cast(child);
			
			if (thisTrg)
			{
				auto identifier = thisTrg.FindComponent(SCR_CampaignAIVehicleSpawnComponent);
				
				if (identifier)
				{
					thisTrg.QueryEntitiesInside();
					array<IEntity> inside = new array<IEntity>();
					thisTrg.GetEntitiesInside(inside);
					
					if (inside.Count() == 0)
						trg = thisTrg;
				}
			}
			
			child = child.GetSibling();
		}
		
		if (!trg)
			return;
		
		// Spawn the vehicle
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		trg.GetWorldTransform(params.Transform);
		Resource res;
		
		if (group.GetFactionName() == FACTION_OPFOR)
		{
			res = m_rAIVehicleEast;
		} else {
			res = m_rAIVehicleWest;
		};
		
		if (!res)
			return;
		
		// Get rid of previously used vehicles if too far
		bool spawnNewVeh = true;
		
		if (oldGrp)
		{
			array<IEntity> vehsInUse = new array<IEntity>();
			oldGrp.GetUsableVehicles(vehsInUse);
			IEntity leader = group.GetLeaderEntity();
			
			if (!leader)
				return;
			
			foreach (IEntity vehOld: vehsInUse)
			{
				if (vehOld)
				{
					if (vector.DistanceSq(leader.GetOrigin(), vehOld.GetOrigin()) > 250000)		// 500m
					{
						oldGrp.RemoveUsableVehicle(vehOld);
					} else {
						group.AddUsableVehicle(vehOld);
						spawnNewVeh = false;
					}
				}
			}
		}
		
		if (spawnNewVeh)
		{
			Vehicle veh = Vehicle.Cast(GetGame().SpawnEntityPrefab(res, null, params));
			veh.GetPhysics().SetVelocity("0 -0.1 0"); // Make the vehicle copy the terrain properly
			GetInstance().RegisterSpawnedVehicle(veh); // Register for garbage collector etc.
			
			// Add loiter waypoint
			if (wp)
				group.AddWaypoint(wp);
			
			// Assign the vehicle to the group
			group.AddUsableVehicle(veh);
		}
	}
	
	//***********//
	//RPC METHODS//
	//***********//
	
	//------------------------------------------------------------------------------------------------
	void MobileAssemblyFeedback(ECampaignClientNotificationID msgID, int playerID, int factionID)
	{
		Rpc(RpcDo_MobileAssemblyFeedback, msgID, playerID, factionID);
				
		if (RplSession.Mode() != RplMode.Dedicated)
			RpcDo_MobileAssemblyFeedback(msgID, playerID, factionID);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMobileAssemblyDeployChangedWest()
	{
		if (m_DeployedMobileAssemblyIDWest != RplId.Invalid())
			m_LastDeployedHQIDWest = m_DeployedMobileAssemblyIDWest;
		
		SCR_CampaignMobileAssemblyComponent comp = SCR_CampaignMobileAssemblyComponent.Cast(Replication.FindItem(m_LastDeployedHQIDWest));
		
		if (comp)
			comp.OnDeployChanged();
		
		s_OnMobileAssemblyDeployChanged.Invoke(FACTION_BLUFOR, m_DeployedMobileAssemblyIDWest.IsValid());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMobileAssemblyDeployChangedEast()
	{
		if (m_DeployedMobileAssemblyIDEast != RplId.Invalid())
			m_LastDeployedHQIDEast = m_DeployedMobileAssemblyIDEast;
		
		SCR_CampaignMobileAssemblyComponent comp = SCR_CampaignMobileAssemblyComponent.Cast(Replication.FindItem(m_LastDeployedHQIDEast));
		
		if (comp)
			comp.OnDeployChanged();
		
		s_OnMobileAssemblyDeployChanged.Invoke(FACTION_OPFOR, m_DeployedMobileAssemblyIDEast.IsValid());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_MobileAssemblyFeedback(ECampaignClientNotificationID msgID, int playerID, int factionID)
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();
		
		if (!fManager)
			return;
		
		if (fManager.GetFactionIndex(SCR_RespawnSystemComponent.GetLocalPlayerFaction()) != factionID)
			return;
		
		int duration = 6;
		LocalizedString text;
		LocalizedString text2;
		string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);
		
		switch (msgID)
		{
			case ECampaignClientNotificationID.ASSEMBLY_DEPLOYED:
			{
				text = "#AR-Campaign_MobileAssemblyDeployed-UC";
				text2 = "#AR-Campaign_MobileAssemblyPlayerName";
				break;
			}
			case ECampaignClientNotificationID.ASSEMBLY_DISMANTLED:
			{
				text = "#AR-Campaign_MobileAssemblyDismantled-UC";
				text2 = "#AR-Campaign_MobileAssemblyPlayerName";
				break;
			}
			case ECampaignClientNotificationID.ASSEMBLY_DESTROYED:
			{
				text = "#AR-Campaign_MobileAssemblyDestroyed-UC";
				break;
			}
		}
		
		if (text != string.Empty)
		{
			if (text2 != string.Empty && playerName != string.Empty)
				SCR_PopUpNotification.GetInstance().PopupMsg(text, duration, text2: text2, text2param1: playerName, prio: SCR_ECampaignPopupPriority.MHQ);
			else
				SCR_PopUpNotification.GetInstance().PopupMsg(text, duration);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns XP reward amount
	int GetXPRewardAmount(CampaignXPRewards reward)
	{
		int rewardsCnt = m_aXPRewardList.Count();
		
		for (int i = 0; i < rewardsCnt; i++)
		{			
			if (m_aXPRewardList[i].GetRewardID() == reward)
				return m_aXPRewardList[i].GetRewardXP();
		}
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns XP reward amount
	float GetXPMultiplier()
	{
		return m_fXpMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	Faction GetLastPlayerFaction()
	{
		return m_LastPlayerFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns XP reward name
	string GetXPRewardName(CampaignXPRewards reward)
	{
		int rewardsCnt = m_aXPRewardList.Count();
		
		for (int i = 0; i < rewardsCnt; i++)
		{			
			if (m_aXPRewardList[i].GetRewardID() == reward)
				return m_aXPRewardList[i].GetRewardName();
		}
		
		return "";
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns XP reward skill
	EProfileSkillID GetXPRewardSkill(CampaignXPRewards reward)
	{
		int rewardsCnt = m_aXPRewardList.Count();
		
		for (int i = 0; i < rewardsCnt; i++)
		{			
			if (m_aXPRewardList[i].GetRewardID() == reward)
				return m_aXPRewardList[i].GetRewardSkill();
		}
		
		return EProfileSkillID.GLOBAL;
	}
	
	//------------------------------------------------------------------------------------------------
	protected PlayerController GetPlayerControllerFromUID(string ID)
	{
		if (ID == string.Empty)
			return null;
		
		BackendApi bApi = GetGame().GetBackendApi();
		
		if (!bApi)
			return null;
		
		array<int> pcList = {};
		int playersCnt = GetGame().GetPlayerManager().GetPlayers(pcList);
		
		for (int i = 0; i < playersCnt; i++)
			if (bApi.GetPlayerUID(pcList[i]) == ID)
				return GetGame().GetPlayerManager().GetPlayerController(pcList[i]);
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ApplyClientData(int ID)
	{		
		ChimeraGame g = GetGame();
		
		if (!g)
			return;
		
		SCR_CampaignClientData clientData = GetClientData(ID);
		
		if (!clientData)
			return;
		
		if (clientData.GetApplied())
			return;
		
		clientData.SetApplied(true);	// Data was applied, don't do it again until another reconnection
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(ID);
		
		if (!pc)
			return;
		
		/*vector pos = clientData.GetStartingPosition();
		
		if (pos.LengthSq() != 0)
			controlledEntity.SetOrigin(pos);	// Place on last position*/
		
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
	
		if (!campaignNetworkComponent)
			return;
		
		campaignNetworkComponent.AddPlayerXP(CampaignXPRewards.UNDEFINED, addDirectly: clientData.GetXP() - campaignNetworkComponent.GetPlayerXP());	// Reapply XP
	}
	
	//------------------------------------------------------------------------------------------------
	protected void WriteAllClientsData()
	{
		array<int> pcList = {};
		int playersCnt = GetGame().GetPlayerManager().GetPlayers(pcList);
		
		for (int i = 0; i < playersCnt; i++)
		{
			PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(pcList[i]);
			
			if (!pc)
				continue;
			
			int ID = pc.GetPlayerId();
			WriteClientData(ID, pc: pc);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// !Save object with player's current data
 	protected void WriteClientData(int playerID, bool disconnecting = false, PlayerController pc = null)
	{
		SCR_CampaignClientData clientData = GetClientData(playerID);

		if (!clientData)
			return;

		if (!pc)
			pc = GetGame().GetPlayerManager().GetPlayerController(playerID);
		
		if (!pc)
			return;

		if (disconnecting)
			clientData.SetApplied(false);	// Player is disconnecting, flag their data for re-application
		
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
		
		if (!campaignNetworkComponent)
			return;
		
		// Set data readable from PlayerController
		clientData.SetXP(campaignNetworkComponent.GetPlayerXP());
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		
		// Set data from player's entity
		if (player)
			clientData.SetStartingPosition(player.GetOrigin());
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterSavedPlayerFaction(int factionIndex)
	{
		m_iRegisteredPlayerFaction = factionIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterSavedPlayerXP(int XP)
	{
		m_iRegisteredPlayerXP = XP;
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnMobileHQ(notnull SCR_CampaignFaction faction, vector pos, vector rot)
	{
		if (faction.GetDeployedMobileAssembly() != null)
			return;
		
		EntitySpawnParams params = EntitySpawnParams();
		GetWorldTransform(params.Transform);
		params.TransformMode = ETransformMode.WORLD;
		Math3D.AnglesToMatrix(rot, params.Transform);
		params.Transform[3] = pos;
		
		IEntity MHQ = GetGame().SpawnEntityPrefab(Resource.Load(faction.GetMobileHQPrefab()), null, params);
		
		if (!MHQ)
			return;
		
		BaseRadioComponent radioComponent = BaseRadioComponent.Cast(MHQ.FindComponent(BaseRadioComponent));
		
		if (radioComponent)
		{
			radioComponent.TogglePower(false);
			radioComponent.SetFrequency(faction.GetFactionRadioFrequency());
			radioComponent.SetEncryptionKey(faction.GetFactionRadioEncryptionKey());
		}
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(MHQ.FindComponent(SlotManagerComponent));
		
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
			
			SCR_CampaignMobileAssemblyComponent mobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));
			
			if (mobileAssemblyComponent)
			{
				mobileAssemblyComponent.SetParentFactionID(GetGame().GetFactionManager().GetFactionIndex(faction));
				mobileAssemblyComponent.UpdateRadioCoverage();
				mobileAssemblyComponent.Deploy(true);
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update amount of AIs to be spawned in groups based on current player count
	protected void UpdateAIGroupSizes()
	{
		return; // disabed for now
		
		array<int> players = {};
		int cnt = GetGame().GetPlayerManager().GetPlayers(players);
		int playersWest = 0;
		int playersEast = 0;
		
		for (int i = 0; i < cnt; i++)
		{
			Faction f = GetPlayerFaction(players[i]);
			
			if (!f)
				continue;
			
			switch (f.GetFactionKey())
			{
				case FACTION_BLUFOR: {playersWest++; break;};
				case FACTION_OPFOR: {playersEast++; break;};
			}
		}
		
		int limitPerFaction = m_iTotalPlayersLimit / 2;
		int sizeOldWest = m_iAIGroupSizeWest;
		int sizeOldEast = m_iAIGroupSizeEast;
		
		// Update limits for future groups
		m_iAIGroupSizeWest = Math.Round((limitPerFaction - playersWest) / AI_GROUPS_PER_FACTION);
		m_iAIGroupSizeEast = Math.Round((limitPerFaction - playersEast) / AI_GROUPS_PER_FACTION);
		
		// No change, terminate
		if (sizeOldWest == m_iAIGroupSizeWest && sizeOldEast == m_iAIGroupSizeEast)
			return;

		// Get rid of current AIs which exceed the limit
		foreach (SCR_AIGroup group: m_aAIGroups)
		{
			if (!group)
				continue;
			
			array<AIAgent> members = new array<AIAgent>();
			int memberCnt = group.GetAgents(members);
			int diff;
			
			if (group.GetFactionName() == FACTION_BLUFOR)
				diff = memberCnt - m_iAIGroupSizeWest;
			else
				diff = memberCnt - m_iAIGroupSizeEast;

			for (int i = memberCnt - 1; i >= 0 && i < diff; i--)
			{
				AIAgent member = members[i];
				
				if (!member)
					continue;
				
				IEntity ent = member.GetControlledEntity();
				
				if (!ent)
					continue;
				
				group.RemoveAIEntityFromGroup(ent);
				SCR_Global.DeleteEntityAndChildren(ent);
			}
		}
		
		// Spawn new groups if they were disabled completely before
		if ((sizeOldWest == 0 && m_iAIGroupSizeWest != 0) || (sizeOldEast == 0 && m_iAIGroupSizeEast != 0))
		{
			foreach (SCR_AIGroup group: m_aAIGroups)
			{
				if (group.GetAgentsCount() == 0)
				{
					m_aAIGroups.RemoveItem(group);
					AddAIWaypoint(SpawnNewGroup(group.m_faction, group));
					delete group;
				};
			};
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Manager for spawning enemies inside bases
	//! \param timeSlice timeSlice from OnFrame event
	protected void HandleRemnantForces(float timeSlice)
	{
		if (m_bAllRemnantsSpawned)
			return;
		
		m_fRemnantForcesTimer += timeSlice;
		int checkID = m_iLocationCheckedForPlayerProximity;
		
		if (m_iRemnantLocationsCnt == 0 || m_fRemnantForcesTimer < (2 / m_iRemnantLocationsCnt))
			return;
		
		m_fRemnantForcesTimer = 0;
		SCR_CampaignRemnantsPresence location = null;
		
		while (!location || (location.GetMembersAlive() == 0 && !location.GetIsSpawned()))
		{
			m_iLocationCheckedForPlayerProximity++;
			
			if (m_iLocationCheckedForPlayerProximity == m_iRemnantLocationsCnt)
			{
				m_iLocationCheckedForPlayerProximity = -1;
				
				if (checkID == -1)
					m_bAllRemnantsSpawned = true;

				return;
			}
			
			location = m_aRemnantsPresence[m_iLocationCheckedForPlayerProximity];
			
			if (location.GetIsDefendersSpawn() && location.GetSpawnTime() != 0 && location.GetSpawnTime() > Replication.Time())
				location = null;
		};
		
		array<int> players = {};
		int playersCount = GetGame().GetPlayerManager().GetPlayers(players);
		bool playersNear = false;
		bool playersFar = true;
		
		// Calculate (de)spawn distance based on view distance, have it squared for faster distance calculation
		float spawnDistanceSq = Math.Pow(GetGame().GetViewDistance() / 2, 2);
		spawnDistanceSq = Math.Min(REMNANTS_SPAWN_RADIUS_MAX, spawnDistanceSq);
		spawnDistanceSq = Math.Max(REMNANTS_SPAWN_RADIUS_MIN, spawnDistanceSq);
		float despawnDistanceSq = spawnDistanceSq + REMNANTS_DESPAWN_RADIUS_DIFF;
		
		for (int i = 0; i < playersCount; i++)
		{
			IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(players[i]);
			
			if (!playerEntity)
				continue;

			float dist = vector.DistanceSq(playerEntity.GetOrigin(), location.GetCenter());

			if (dist < despawnDistanceSq)
			{
				playersFar = false;

				if (dist < spawnDistanceSq)
				{
					playersNear = true;
					break;
				}
			}
		}

		if (!location.GetIsSpawned() && playersNear)
		{
			SpawnRemnants(location);
			return;
		}
		
		// Delay is used so dying players don't see the despawn happen
		if (m_bAllowRemnantsDespawn && location.GetIsSpawned() && playersFar)
		{
			float despawnT = location.GetDespawnTimer();
			
			if (despawnT == -1)
				location.SetDespawnTimer(Replication.Time() + 10000);
			else
				if (Replication.Time() > despawnT)
				{
					DespawnRemnants(location);
				}
		}
		else
			location.SetDespawnTimer(-1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnRemnants(notnull SCR_CampaignRemnantsPresence location)
	{
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = location.GetSpawnpoint();
		Resource res = Resource.Load(location.GetGroupPrefab());
		
		if (!res)
			return;
		
		SCR_AIGroup grp = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(res, null, params));
		
		if (!grp)
			return;
		
		location.SetDespawnTimer(-1);
		location.SetIsSpawned(true);
		
		if (location.GetMembersAlive() > 0)
			grp.SetMaxUnitsToSpawn(location.GetMembersAlive());
		
		grp.SpawnUnits();
		AIWaypoint wp = location.GetWaypoint();
		
		if (wp)
			grp.AddWaypoint(location.GetWaypoint());
		
		location.SetSpawnedGroup(grp);
		
		if (location.GetIsDefendersSpawn())
		{
			SCR_AIGroupUtilityComponent comp = SCR_AIGroupUtilityComponent.Cast(grp.FindComponent(SCR_AIGroupUtilityComponent));
			
			if (comp)
			{
				ScriptInvoker onEnemyDetected = comp.GetOnEnemyDetected();
				
				if (onEnemyDetected)
					onEnemyDetected.Insert(OnEnemyDetectedByDefenders);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DespawnRemnants(notnull SCR_CampaignRemnantsPresence location)
	{
		location.SetDespawnTimer(-1);
		location.SetIsSpawned(false);
		
		SCR_AIGroup grp = location.GetSpawnedGroup();
		bool updateDefenders = location.GetIsDefendersSpawn() && location.GetParentBase();
		
		if (!grp)
		{
			location.SetMembersAlive(0);
			
			if (updateDefenders)
				location.GetParentBase().UpdateBaseDefenders(location);
			
			return;
		}
		
		array<AIAgent> units = new array<AIAgent>();
		grp.GetAgents(units);
		int cnt = grp.GetAgentsCount();
		location.SetMembersAlive(cnt);
		
		if (cnt == 0 && updateDefenders)
			location.GetParentBase().UpdateBaseDefenders(location);
		
		for (int i = 0; i < cnt; i++)
		{
			if (units[i])
			{
				grp.RemoveAIEntityFromGroup(units[i].GetControlledEntity());
				delete units[i].GetControlledEntity();
				delete units[i];
			}
		}

		delete grp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEnemyDetectedByDefenders(SCR_AIGroup grp, Faction detectedFaction)
	{
		if (!detectedFaction)
			return;
		
		foreach (SCR_CampaignRemnantsPresence presence: m_aRemnantsPresence)
		{
			if (!presence || presence.GetSpawnedGroup() != grp)
				continue;
			
			SCR_CampaignBase base = presence.GetParentBase();
			
			if (!base)
				return;
			
			SCR_CampaignFaction baseOwner = base.GetOwningFaction();
			
			if (!baseOwner)
				return;
			
			if (!base.IsBaseInFactionRadioSignal(baseOwner))
				return;
			
			if (base.GetLastEnemyContactTimestamp() != 0 && base.GetLastEnemyContactTimestamp() > (Replication.Time() - (SCR_CampaignBase.UNDER_ATTACK_WARNING_PERIOD * 1000)))
				return;
			
			baseOwner.SendHQMessage(SCR_ERadioMsg.BASE_UNDER_ATTACK, base.GetCallsign(), param: base.GetBaseID());
			base.SetLastEnemyContactTimestamp(Replication.Time());
			base.SetAttackingFaction(GetGame().GetFactionManager().GetFactionIndex(detectedFaction));
			
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get corresponding client data, create new object if not found
	protected SCR_CampaignClientData GetClientData(int ID)
	{
		if (ID == 0)
			return null;
		
		SCR_CampaignClientData clientData;
		int clientsCnt = m_aRegisteredClients.Count();

		// Check if the client is reconnecting
		for (int i = 0; i < clientsCnt; i++)
		{
			if (m_aRegisteredClients[i].GetID() == ID)
			{
				clientData = m_aRegisteredClients[i];
				break;
			}
		}
		
		if (!clientData)
		{
			clientData = new SCR_CampaignClientData;
			clientData.SetID(ID);
			m_aRegisteredClients.Insert(clientData);
		}

		return clientData;
	}
	
	//------------------------------------------------------------------------------------------------
	// Refresh building preview when composition was disassambled.
	void RefreshBuildingPreview(EntityID slotID)
	{
		Rpc(Rpc_RefreshBuildingPreview, slotID);
		Rpc_RefreshBuildingPreview(slotID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Refresh building preview
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Rpc_RefreshBuildingPreview(EntityID slotID)
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player) 
			return;
				
		SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(player.FindComponent(SCR_CampaignBuildingComponent));		
		if (!buildComp || !buildComp.IsBuilding())
			return;
		
		SCR_SiteSlotEntity slotEnt = SCR_SiteSlotEntity.Cast(GetGame().GetWorld().FindEntityByID(slotID));
		if (!slotEnt)
			return;
		
		//-- Get the controller
		GetGame().GetWorld().QueryEntitiesBySphere(slotEnt.GetOrigin(), BUILDING_CONTROLLER_SEARCH_DISTANCE, FindController, null, EQueryEntitiesFlags.ALL);
		if (!m_BuildingController)
			return;
		
		SCR_CampaignBuildingControllerComponent buildContComp = SCR_CampaignBuildingControllerComponent.Cast(m_BuildingController.FindComponent(SCR_CampaignBuildingControllerComponent));
		if (!buildContComp)
			return;
		
		
		
		IEntity suppliesProvider =  buildContComp.GetSuppliesProvider();
		if (!suppliesProvider)
			return;
		
		SCR_CampaignFaction faction;
		ref array<ref SCR_CampaignSlotComposition> SlotData = new array<ref SCR_CampaignSlotComposition>();
		
		if (suppliesProvider.Type() == SCR_CampaignBase)
		{
			SCR_CampaignBase base = SCR_CampaignBase.Cast(suppliesProvider);
			faction = SCR_CampaignFaction.Cast(base.GetOwningFaction());
		}
		else
		{
			FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(suppliesProvider.FindComponent(FactionAffiliationComponent));
			if (!factionAffiliationComponent)
			    return;
				
			faction = SCR_CampaignFaction.Cast(factionAffiliationComponent.GetDefaultAffiliatedFaction());
		}
		
		ResourceName slotResName = slotEnt.GetPrefabData().GetPrefabName();
		SlotData = faction.GetSlotResource(buildComp.GetCompositionType(slotResName));
		
		Color iconColor;
		if (faction)
			iconColor = faction.GetFactionColor();
	
		buildComp.SpawnNewPreview(SlotData[0], slotEnt, suppliesProvider, buildContComp, iconColor);
	}
	
	//------------------------------------------------------------------------------------------------
	// Search for a controller dedicated to the slot
	protected bool FindController(IEntity ent)
	{		
		SCR_CampaignBuildingControllerComponent contComp = SCR_CampaignBuildingControllerComponent.Cast(ent.FindComponent(SCR_CampaignBuildingControllerComponent));
		if (!contComp)
		    return true;
						
		m_BuildingController = ent;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void AfterAllBasesInitialized()
	{
#ifdef ENABLE_BUILDING_DEBUG
		Print("Method AfterAllBasesInitialized() has been executed.");
#endif	
		
		SCR_CampaignFactionManager campaignFactionManager = SCR_CampaignFactionManager.GetInstance();
		SCR_CampaignFaction west = SCR_CampaignFaction.Cast(campaignFactionManager.GetFactionByKey(FACTION_BLUFOR));
		SCR_CampaignFaction east = SCR_CampaignFaction.Cast(campaignFactionManager.GetFactionByKey(FACTION_OPFOR));
		
		if (!west || !east)
			return;
		
		SCR_CampaignBase HQWest = west.GetMainBase();
		SCR_CampaignBase HQEast = east.GetMainBase();
		
		int baseIndex = 0;
		
		array<int> suppliesBufferWest = new array<int>();
		array<int> suppliesBufferEast = new array<int>();
		int intervalMultiplier = Math.Floor((STARTING_SUPPLIES_MAX - STARTING_SUPPLIES_MIN) / STARTING_SUPPLIES_INTERVAL);
		array<SCR_CampaignBase> bases = SCR_CampaignBaseManager.GetBases();
		array<SCR_CampaignBase> basesSorted = {};
		
		// Sort indexes by distance from BLUFOR HQ
		if (HQWest)
		{
			foreach (SCR_CampaignBase base: bases)
			{
				bool done = false;
				float distanceToHQ = vector.DistanceSqXZ(HQWest.GetOrigin(), base.GetOrigin());
				
				for (int i = 0, cnt = basesSorted.Count(); i < cnt; i++)
				{
					SCR_CampaignBase baseCheckedAgainst = basesSorted[i];
					
					if (distanceToHQ < vector.DistanceSqXZ(HQWest.GetOrigin(), baseCheckedAgainst.GetOrigin()))
					{
						basesSorted.InsertAt(base, i);
						done = true;
						break;
					}
				}
				
				if (!done)
					basesSorted.Insert(base);
			}
		}
		else
			basesSorted.Copy(bases);
		
#ifdef ENABLE_BUILDING_DEBUG
		PrintFormat("Number of all sorted bases is: %1",basesSorted.Count());
#endif	
		
		foreach (SCR_CampaignBase base: basesSorted)
		{
			if (!base)
				continue;
			
			if (base.GetType() != CampaignBaseType.RELAY && baseIndex < BASE_CALLSIGNS_COUNT)
			{
				base.SetCallsignIndex(baseIndex);
				baseIndex++;
			}
			
			if (RplSession.Mode() != RplMode.Dedicated)
				base.HideMapLocationLabel();

			if (!IsProxy())
			{
				base.SpawnAIWaypoints();
			}

			if (IsProxy() || (HQWest && HQEast))
			{
				CampaignBaseType type = base.GetType();
				
				if (type != CampaignBaseType.MAIN && !IsProxy())
				{
					SCR_CampaignFaction setFaction = base.GetOwningFaction();
					
					if (!setFaction)
					{
						vector pos = base.GetOrigin();
						float distHQWest = vector.DistanceSq(pos, HQWest.GetOrigin());
						float distHQEast = vector.DistanceSq(pos, HQEast.GetOrigin());
						float distMajorWest = float.MAX;
						float distMajorEast = float.MAX;
						
						foreach (SCR_CampaignBase checkedBase: SCR_CampaignBaseManager.GetBases())
						{
							SCR_CampaignFaction f = checkedBase.GetOwningFaction();
							
							if (checkedBase == base) continue;
							if (checkedBase.GetType() != CampaignBaseType.MAJOR) continue;
							if (!f) continue;
							
							float dist = vector.DistanceSq(pos, checkedBase.GetOrigin());
							
							if (f.GetFactionKey() == FACTION_BLUFOR)
							{
								if (dist < distMajorWest)
									distMajorWest = dist;
							}
							else
							{
								if (dist < distMajorEast)
									distMajorEast = dist;
							}
						}
						
						setFaction = HQWest.GetOwningFaction();
						
						if (distMajorWest > distMajorEast)
							setFaction = HQEast.GetOwningFaction();
						else
						{
							if (distMajorEast == distMajorWest && distHQWest > distHQEast)
								setFaction = HQEast.GetOwningFaction();
						}
						
						// Add random starting supplies to small bases
						if (!IsProxy() && m_bRandomizeSupplies && setFaction && base.GetType() == CampaignBaseType.SMALL)
						{
							int amount;
							FactionKey fKey = setFaction.GetFactionKey();
							
							// Check if we have preset supplies stored in buffer
							if (fKey == FACTION_BLUFOR && !suppliesBufferWest.IsEmpty())
							{
								amount = suppliesBufferWest[0];
								suppliesBufferWest.RemoveOrdered(0);
							}
							else if (fKey == FACTION_OPFOR && !suppliesBufferEast.IsEmpty())
							{
								amount = suppliesBufferEast[0];
								suppliesBufferEast.RemoveOrdered(0);
							}
							else
							{
								// Supplies from buffer not applied, add random amount, store to opposite faction's buffer
								amount = STARTING_SUPPLIES_MIN + (STARTING_SUPPLIES_INTERVAL * Math.RandomIntInclusive(0, intervalMultiplier));
								
								if (fKey == FACTION_BLUFOR)
									suppliesBufferEast.Insert(amount);
								else
									suppliesBufferWest.Insert(amount);
							}
							
							base.AddSupplies(amount - base.GetSupplies());
						}
					}
					
					base.SetBuildingsFaction(setFaction);
				}
				
				// Spawn pre-built structures only on server
				if (!IsProxy() && base.GetBuildingsFaction())
					GetGame().GetCallqueue().CallLater(base.SpawnBuildings, 500);
			}
			
			// Prepare a list of dedicated Slots
			SCR_SiteSlotEntity slot;
			
			for (int i = ECampaignCompositionType.LAST; i >= 0; i--)
			{
				slot = base.GetAssignedSlot(i); 
				if (slot) 
				{
					SCR_BuildingDedicatedSlotData slotData = new SCR_BuildingDedicatedSlotData();	
					slotData.SetData(i, base);
					m_aDedicatedSlots.Insert(slot, slotData);
					
#ifdef ENABLE_BUILDING_DEBUG
					PrintFormat("Slot: %1 was added to m_aDedicatedSlots",slot);
#endif	
					
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OverrideBaseRadioRanges()
	{
		array<SCR_CampaignBase> bases = SCR_CampaignBaseManager.GetBases();
		
		foreach (SCR_CampaignBase base: bases)
		{
			BaseRadioComponent comp = BaseRadioComponent.Cast(base.FindComponent(BaseRadioComponent));
			
			if (!comp)
				continue;
			
			comp.SetRange(m_iCustomRadioRange);
			base.LinkBases();
			base.HandleMapInfo();
		}
		
		SCR_CampaignBaseManager bm = SCR_CampaignBaseManager.GetInstance();
		
		if (bm)
			bm.UpdateBasesSignalCoverage()
	}
	
	//****************//
	//OVERRIDE METHODS//
	//****************//
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if player with provided playerID is able to respawn, false otherwise
	override bool CanPlayerRespawn(int playerID)
	{
		return super.CanPlayerRespawn(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;
		
		writer.WriteInt(m_iCustomRadioRange);
		writer.WriteInt(m_iMaxRespawnRadios);
		writer.WriteBool(m_bIgnoreMinimumVehicleRank);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!super.RplLoad(reader))
			return false;
		
		reader.ReadInt(m_iCustomRadioRange);
		reader.ReadInt(m_iMaxRespawnRadios);
		reader.ReadBool(m_bIgnoreMinimumVehicleRank);
		
		if (m_iCustomRadioRange != -1)
			GetGame().GetCallqueue().CallLater(OverrideBaseRadioRanges, 250);
		
		SCR_PlayerSpawnPointManagerComponent comp = SCR_PlayerSpawnPointManagerComponent.Cast(FindComponent(SCR_PlayerSpawnPointManagerComponent));
		
		if (comp)
			comp.EnablePlayerSpawnPoints(m_iMaxRespawnRadios >= 0);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);
		
		if (NotPlaying())
			return;
		
		if (!IsProxy())
		{	
			if (GetGame().AreGameFlagsSet( EGameFlags.SpawnAI ))
			{		
				SCR_CampaignFactionManager campaignFactionManager = SCR_CampaignFactionManager.GetInstance();
				
				if (!campaignFactionManager)
					return;
						
				//! First time spawn of all AI groups and their waypoints (after tasks were initialized)
				for (int i = 0; i < AI_GROUPS_PER_FACTION * 2; i++)
				{
					string factionKey = FACTION_BLUFOR;
				
					if (i >= AI_GROUPS_PER_FACTION)
						factionKey = FACTION_OPFOR;
					
					SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(campaignFactionManager.GetFactionByKey(factionKey));
					
					if (faction && faction.GetAIsAllowed())
						GetGame().GetCallqueue().CallLater(SpawnNewGroup, 250 * (i + 1), false, factionKey);
				}
	
				GetGame().GetCallqueue().CallLater(RefreshAIWaypoints, 2000, false, null, -1);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Called once tasks are initialized
	override void HandleOnTasksInitialized()
	{
		super.HandleOnTasksInitialized();
	}

	//------------------------------------------------------------------------------------------------
	//! What happens when a player connects.
	//! Method is called from SCR_DeathmatchLobbyEntity
	//! \param playerID is a unique player identifier that defines which player has disconnected.
	override void OnPlayerConnected(int playerId)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_GAMEMODE))
			return;
#endif
		ApplyClientData(playerId);
		super.OnPlayerConnected(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);
		
		if (!IsProxy())
		{
			// Apply forced faction (saved in previous connection to this match)
			SCR_RespawnSystemComponent rsc = SCR_RespawnSystemComponent.Cast(GetRespawnSystemComponent());
			
			if (!rsc)
				return;
			
			Faction forcedFaction = GetPlayerForcedFaction(playerId);
			
			if (forcedFaction)
				rsc.DoSetPlayerFaction(playerId, rsc.GetFactionIndex(forcedFaction));
		}
		
		// See HandleOnFactionAssigned()
		if (SCR_PlayerController.GetLocalPlayerId() != 0 && !GetLastPlayerFaction())
		{
			for (int i = 0, cnt = m_aUnprocessedFactionAssignments.Count(); i < cnt; i++)
			{
				if (m_aUnprocessedFactionAssignments.GetKey(i) == SCR_PlayerController.GetLocalPlayerId())
					ProcessFactionAssignment(m_aUnprocessedFactionAssignments.GetElement(i));
			}			
		}
		
		// TODO: Manage properly in MP environment
		if (!Replication.IsRunning())
		{
			if (m_iRegisteredPlayerFaction != -1)
			{
				SCR_RespawnSystemComponent respawnSystem = GetRespawnSystemComponent();
				
				if (respawnSystem)
					respawnSystem.DoSetPlayerFaction(playerId, m_iRegisteredPlayerFaction);
			}
			
			if (m_iRegisteredPlayerXP != 0)
			{
				PlayerController pc = GetGame().GetPlayerController();
				
				if (pc)
				{
					SCR_CampaignNetworkComponent comp = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));

					if (comp)
						comp.AddPlayerXP(CampaignXPRewards.UNDEFINED, addDirectly: m_iRegisteredPlayerXP - comp.GetPlayerXP());
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess(int iPlayerID)
	{
		super.OnPlayerAuditSuccess(iPlayerID);
		
		SCR_PlayerProfileManagerComponent comp = SCR_PlayerProfileManagerComponent.Cast(FindComponent(SCR_PlayerProfileManagerComponent));
		
		if (comp)
			comp.LoadConnectingPlayerProfile(iPlayerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditFail(int iPlayerID)
	{
		super.OnPlayerAuditFail(iPlayerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! What happens when a player disconnects.
	override void OnPlayerDisconnected(int playerId)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_GAMEMODE))
			return;
#endif
		super.OnPlayerDisconnected(playerId);
		
		GetTaskManager().OnPlayerDisconnected(playerId);
		WriteClientData(playerId, true);
		UpdateAIGroupSizes();
		
		// Disconnecting player is currently capturing a base; handle it
		foreach (SCR_CampaignBase base: SCR_CampaignBaseManager.GetBases())
		{
			if (!base)
				continue;
			
			if (base.GetCapturingFaction() && base.GetReconfiguredByID() == playerId)
			{
				base.EndCapture();
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Serves for enabling spawn hint on map. Also saves player spawn position
	void EnablePlayerSpawnHint(bool enable)
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		
		if (!player && enable)
			return;
		
		m_bCanShowSpawnPosition = enable;
		
		if (enable)
		{
			m_vFirstSpawnPosition = player.GetOrigin();
			SetSpawnTime();
		}
		else
		{
			m_vFirstSpawnPosition = vector.Zero;
			SetMapOpened(false);
			
			if (m_MapCampaignUI)
				m_MapCampaignUI.RemoveSpawnPositionHint();
		}

		GetGame().GetCallqueue().Remove(EnablePlayerSpawnHint);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns players position after spawning
	vector GetPlayerSpawnPos()
	{
		return m_vFirstSpawnPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if map was already opened 
	bool WasMapOpened()
	{
		return m_bWasMapOpened;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets that map was already opened by player
	void SetMapOpened(bool wasOpened)
	{
		m_bWasMapOpened = wasOpened;
	}
	
	//------------------------------------------------------------------------------------------------
	//! What happens when a player spawns.
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_GAMEMODE))
			return;
#endif
		super.OnPlayerSpawned(playerId, controlledEntity);
		
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
	
		if (pc)
		{
			SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
			
			if (campaignNetworkComponent)
			{
				campaignNetworkComponent.UpdatePlayerRank(false);
				campaignNetworkComponent.EnableShowingSpawnPosition(true)
			}
		}
		
		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(controlledEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (inventoryManager)
		{
			inventoryManager.m_OnItemAddedInvoker.Insert(OnItemAdded);
			inventoryManager.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSpawnPointUsed(SCR_SpawnPoint spawnPoint, int playerId)
	{
		IEntity parent = spawnPoint.GetParent();
		SCR_CampaignBase base = null;
		
		while (parent)
		{
			if (parent.Type() == SCR_CampaignBase)
			{
				base = SCR_CampaignBase.Cast(parent);
				break;
			}
			
			parent = parent.GetParent();
		}
		
		if (!base)
			return;
		
		if (base.GetType() == CampaignBaseType.SMALL)
			base.AddRespawnTickets(-1);
		
		// Location popup for player
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
	
		if (playerController)
		{
			SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
			
			if (campaignNetworkComponent)
				campaignNetworkComponent.RespawnLocationPopup(base.GetBaseID());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandleOnCharacterDeath(notnull CharacterControllerComponent characterController, IEntity instigator)
	{
		super.HandleOnCharacterDeath(characterController, instigator);
		
		// Game ended
		if (m_fGameEnd != -1)
			return;
		
		if (!characterController)
			return;
		
		// Get rid of player map icon
		SCR_ChimeraCharacter character = characterController.GetCharacter();
		if (!character)
			return;
		
		MapDescriptorComponent comp = MapDescriptorComponent.Cast(character.FindComponent(MapDescriptorComponent));
		
		if (comp)
		{
			MapItem item = comp.Item();
			
			if (item)
				item.SetVisible(false);
		}
		
		if (character == ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()))
		{
			m_aBasesWithPlayer.Clear();
			m_FirstBaseWithPlayer = null;
			SCR_PopUpNotification.GetInstance().HideCurrentPopupMsg();
		}
		
		if (!IsProxy())
		{
			// Handle AI respawn;
			foreach (SCR_AIGroup group: m_aAIGroups)
			{
				if (group.GetAgentsCount() == 0)
				{
					m_aAIGroups.RemoveItem(group);
					AddAIWaypoint(SpawnNewGroup(group.m_faction, group));
					delete group;
				}
			}
			
			SCR_ChimeraCharacter instigatorChar;
			
			// Handle XP for kill
			if (instigator)
			{
				// Instigator is a vehicle, find the driver
				if (instigator.IsInherited(Vehicle))
				{
					GenericEntity instigatorGeneric = GenericEntity.Cast(instigator);
					
					if (!instigatorGeneric)
						return;
					
					auto compartmentManager = instigatorGeneric.FindComponent(BaseCompartmentManagerComponent);
					
					if (!compartmentManager)
						return;
					
					BaseCompartmentManagerComponent compartmentManagerCast = BaseCompartmentManagerComponent.Cast(compartmentManager);
					array<BaseCompartmentSlot> compartments = new array <BaseCompartmentSlot>();
					int cnt = compartmentManagerCast.GetCompartments(compartments);
					BaseCompartmentSlot slot;
					
					for (int i = 0; i < cnt; i++)
					{
						slot = compartments[i];
						
						if (slot.Type() == PilotCompartmentSlot)
						{
							instigatorChar = SCR_ChimeraCharacter.Cast(slot.GetOccupant());
							break;
						}
					}
					
					if (!instigatorChar)
						return;
				}
				else
				{
					instigatorChar = SCR_ChimeraCharacter.Cast(instigator);
				}
				
				if (!instigatorChar)
					return;
				
				auto foundComponentVictim = character.FindComponent(FactionAffiliationComponent);
				auto foundComponentKiller = instigatorChar.FindComponent(FactionAffiliationComponent);
	
				if (!foundComponentKiller || !foundComponentVictim)
					return;
	
				auto castedComponent = FactionAffiliationComponent.Cast(foundComponentKiller);
				Faction killerFaction = castedComponent.GetAffiliatedFaction();
				castedComponent = FactionAffiliationComponent.Cast(foundComponentVictim);
				Faction victimFaction = castedComponent.GetAffiliatedFaction();
				
				if (killerFaction && victimFaction)
				{
					if (killerFaction == victimFaction)
					{
						if (instigatorChar != character)
							AwardXP(instigatorChar, CampaignXPRewards.FRIENDLY_KILL);
					}
					else
					{
						float multiplier = 1;
						
						if (SCR_CampaignDefendTask.IsCharacterInAnyDefendTaskRange(instigatorChar))
							multiplier = 1.25;
						
						if (instigatorChar.IsInVehicle())
							AwardXP(instigatorChar, CampaignXPRewards.ENEMY_KILL_VEH);
						else
							AwardXP(instigatorChar, CampaignXPRewards.ENEMY_KILL);
					}
				}
			}
			
			BaseLoadoutManagerComponent loadoutManager = BaseLoadoutManagerComponent.Cast(character.FindComponent(BaseLoadoutManagerComponent));
		
			if (loadoutManager)
			{
				IEntity backpack = loadoutManager.GetClothByArea(ELoadoutArea.ELA_Backpack);
			
				if (backpack)
				{
					BaseLoadoutClothComponent loadoutCloth = BaseLoadoutClothComponent.Cast(backpack.FindComponent(BaseLoadoutClothComponent));
					
					if (loadoutCloth && loadoutCloth.GetArea() == ELoadoutArea.ELA_Backpack && backpack.FindComponent(SCR_RadioComponent))
					{
						Faction faction = character.GetFaction();
						
						if (faction)
							RemoveActiveRespawnRadio(faction.GetFactionKey());
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void ShowXP(bool visible)
	{
		m_bIsShowingXPBar = visible;
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		
		if(m_wXPInfo && hudManager)
		{
			m_wXPInfo.SetVisible(visible);
			return;
		}
		int totalXP = 0;
			
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_RespawnSystemComponent.GetLocalPlayerFaction());		
		if (!faction)
			return;
			
		SCR_CampaignFactionManager campaignFactionManager = SCR_CampaignFactionManager.GetInstance();
		ECharacterRank curRank = campaignFactionManager.GetRankByXP(totalXP);
		string rankText = faction.GetRankName(curRank);
		ResourceName rankIconName = faction.GetRankInsignia(curRank);
			
		m_wXPInfo = hudManager.CreateLayout("{E0B82B4FCC95EE05}UI/layouts/HUD/CampaignMP/RankProgress.layout", EHudLayers.MEDIUM, 0);

		if (!m_wXPInfo)
			return;
			
		TextWidget title = TextWidget.Cast(m_wXPInfo.FindWidget("Title"));
		TextWidget rank = TextWidget.Cast(m_wXPInfo.FindWidget("Rank"));
		TextWidget rankNoIcon = TextWidget.Cast(m_wXPInfo.FindWidget("RankNoIcon"));
		ImageWidget rankIcon = ImageWidget.Cast(m_wXPInfo.FindWidget("RankIcon"));
		TextWidget skill = TextWidget.Cast(m_wXPInfo.FindWidget("Skill"));
			
		rank.SetTextFormat(rankText);
		rankIcon.LoadImageTexture(0, rankIconName);
		rankIcon.SetColor(Color.FromRGBA(226,168,79,255));
		rankIcon.SetVisible(true);
		rankNoIcon.SetTextFormat("");
			
		ProgressBarWidget progress = ProgressBarWidget.Cast(m_wXPInfo.FindWidget("Progress"));
		ProgressBarWidget progressDiff = ProgressBarWidget.Cast(m_wXPInfo.FindWidget("ProgressDiff"));
			
		progress.SetMin(0);
		progress.SetMax(1);
		progress.SetCurrent(0);
		progressDiff.SetMin(0);
		progressDiff.SetMax(1);
		progressDiff.SetCurrent(0);
		title.SetTextFormat("");
		m_wXPInfo.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------	
	override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_GAMEMODE))
			return;
#endif
		super.OnPlayerKilled(playerId, player, killer);
		
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
	
		if (pc)
		{
			SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
			
			if (campaignNetworkComponent)
				campaignNetworkComponent.EnableShowingSpawnPosition(false)
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! What happens when a player is assigned a faction
	//! Called from SCR_RespawnSystemComponent
	//! \param playerID the id of the target player
	//! \param assignedFaction the faction that player was assigned or null if none
	override void HandleOnFactionAssigned(int playerID, Faction assignedFaction)
	{
		super.HandleOnFactionAssigned(playerID, assignedFaction);
		
		// Save faction selected in client's data
		if (!IsProxy())
		{
			SCR_CampaignClientData clientData;
			clientData = GetClientData(playerID);
			
			if (clientData && assignedFaction)
				clientData.SetFaction(SCR_CampaignFaction.Cast(assignedFaction));
			
			UpdateAIGroupSizes();
		}
		
		// When a faction is being assigned to the client automatically by server, playerId might not yet be registered
		// In that case, this saves the connecting player's data and processes them later in local OnPlayerRegistered()
		if (RplSession.Mode() != RplMode.Dedicated && !GetLastPlayerFaction())
		{
			if (SCR_PlayerController.GetLocalPlayerId() == playerID)
				ProcessFactionAssignment(assignedFaction);
			else
				m_aUnprocessedFactionAssignments.Insert(playerID, assignedFaction);
		}
		
		s_OnFactionAssigned.Invoke(playerID, assignedFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{		
		if (NotPlaying())
			return;
		
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_GAMEMODE))
			return;
#endif
		super.EOnInit(owner);
		
		// Initialize players list UI
		m_wPlayersList = GetGame().GetWorkspace().CreateWidgets("{3370BBD6566BB16C}UI/layouts/HUD/CampaignMP/CampaignMutePlayersUI.layout");
		m_wPlayersList.SetVisible(false);
		
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.Cast(FindComponent(SCR_RespawnSystemComponent));		

		if (!respawnSystem)
			Print("There is no RespawnSystemComponent attached to the GameMode entity. Faction scoring will not work.");
		else 
			s_RespawnSystemComponent = respawnSystem;
		
		if (!IsProxy())
		{
			GetGame().GetCallqueue().CallLater(WriteAllClientsData, BACKEND_WRITE_PERIOD, true);
			
			if (m_iCustomRadioRange != -1)
				OverrideBaseRadioRanges();
			
			SCR_CampaignFactionManager campaignFactionManager = SCR_CampaignFactionManager.GetInstance();
			
			if (campaignFactionManager)
			{
				SCR_CampaignFaction fWest = SCR_CampaignFaction.Cast(campaignFactionManager.GetFactionByKey(FACTION_BLUFOR));
				SCR_CampaignFaction fEast = SCR_CampaignFaction.Cast(campaignFactionManager.GetFactionByKey(FACTION_OPFOR));
				
				if (fWest && fEast)
				{
					m_rAIVehicleWest = Resource.Load(fWest.GetAIGroupVehiclePrefab());
					m_rAIVehicleEast = Resource.Load(fEast.GetAIGroupVehiclePrefab());
				}
			}
			
			m_rRemnants = Resource.Load(m_RemnantsGroupPrefab);
			
			GetGame().GetCallqueue().CallLater(SetupDaytime, 500, false, m_iStartingHours, m_iStartingMinutes, 0);		// TODO: Move to init when possible
			GetGame().GetCallqueue().CallLater(CheckMobileAssemblies, 500, true);
			
			SCR_PlayerSpawnPointManagerComponent comp = SCR_PlayerSpawnPointManagerComponent.Cast(FindComponent(SCR_PlayerSpawnPointManagerComponent));
			
			if (comp)
			{
				if (GetMaxRespawnRadios() >= 0)
				{
					comp.EnablePlayerSpawnPoints(true);
					GetGame().GetCallqueue().CallLater(CheckRadioSpawnpointsSignalCoverage, 500, true);
				}
				else
					comp.EnablePlayerSpawnPoints(false);
			}
		}
		
		SCR_UITaskManagerComponent.s_OnTaskListVisible.Insert(ShowXP);
		
		if (RplSession.Mode() != RplMode.Dedicated)
		{
			GetGame().GetCallqueue().CallLater(CheckPlayerInsideRadioRange, 3000, true);
			GetGame().GetCallqueue().CallLater(EnteringNewBase, 3000, true);
			GetGame().GetCallqueue().CallLater(CheckForBasesWithPlayer, 500, true);
			
			GetGame().GetInputManager().AddActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
			GetGame().GetInputManager().AddActionListener("GadgetMap", EActionTrigger.DOWN, RegisterTasksShown);
		}
		
		m_MapEntity = SCR_MapEntity.Cast(GetGame().GetMapManager());
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_UP))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_UP, 0);
			PlayerController pc = GetGame().GetPlayerController();
			
			if (pc)
			{
				SCR_CampaignNetworkComponent comp = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
			
				if (comp)
					comp.CheatRank();
			}
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_DOWN))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_DOWN, 0);
			PlayerController pc = GetGame().GetPlayerController();
			
			if (pc)
			{
				SCR_CampaignNetworkComponent comp = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
			
				if (comp)
					comp.CheatRank(true);
			}
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_GAMEMODE))
			return;
#endif
		super.EOnFrame(owner, timeSlice);

		if (RplSession.Mode() != RplMode.Dedicated)
			ProcessUI(timeSlice);
		
		if (!IsProxy())
		{
			if (GetGame().AreGameFlagsSet( EGameFlags.SpawnAI ))
				HandleRemnantForces(timeSlice);
		}
		
		// End session
		if (m_fGameEnd != -1 && Replication.Time() >= m_fGameEnd)
		{
			m_fGameEnd = -1;
			
			if (RplSession.Mode() == RplMode.Dedicated)
			{
				/*BackendApi bApi = GetGame().GetBackendApi();
				
				if (bApi)
				{
					DSSession session = bApi.GetDSSession();
					
					if (session)
						session.RequestRestart();
				}*/
				GetGame().RequestClose();
			}
			else
				GameStateTransitions.RequestGameplayEndTransition();
		}
		
		if (m_Preload)
		{
			bool finished = m_Preload.Update(timeSlice);
			
			if (finished)
				m_Preload = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	void SCR_GameModeCampaignMP(IEntitySource src, IEntity parent)
	{
		FACTION_BLUFOR = m_sBLUFORFactionKey;
		FACTION_OPFOR = m_sOPFORFactionKey;
		FACTION_INDFOR = m_sINDFORFactionKey;
		
		if (NotPlaying())
			return;
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_GAMEMODE, "", "Execute gamemode", "Conflict", true);
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_UP, "", "Promotion", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_DOWN, "", "Demotion", "Conflict");
#endif
		if (!s_Instance)
			s_Instance = this;
		
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
		
		if (!IsProxy())
		{
			SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
			
			if (header)
			{
				// Set maximum amount of players and AI, header value cannot be higher than entity preset
				int playersLimitHeader = header.m_iPlayerCount;
			
				if (playersLimitHeader >= 0)
					m_iTotalPlayersLimit = Math.Min(m_iTotalPlayersLimit, playersLimitHeader);
				
				if (playersLimitHeader > m_iTotalPlayersLimit)
					Print("Header value of max players is higher than gamemode entity setting. Using entity setting...", LogLevel.WARNING);
				
				if (SCR_SaveLoadComponent.IsLoadOnStart(header))
					m_bBackendStateLoaded = true;
				
				SCR_MissionHeaderCampaign campaignHeader = SCR_MissionHeaderCampaign.Cast(header);
	
				if (campaignHeader)
				{
					m_bCheckAdvancedStageConfig = true;	// config was loaded, check it regardless of setting
					m_bAllowRemnantsDespawn = campaignHeader.m_bDespawnRemnants;
					m_bAdvancedStage = campaignHeader.m_bAdvancedStage;
					m_iCustomRadioRange = campaignHeader.m_iCustomRadioRange;
					m_fXpMultiplier = campaignHeader.m_fXpMultiplier;
					m_bIgnoreMinimumVehicleRank = campaignHeader.m_bIgnoreMinimumVehicleRank;
					
					int suppliesMax = campaignHeader.m_iMaximumBaseSupplies;
					int suppliesMin = campaignHeader.m_iMinimumBaseSupplies;
					int respawnRadiosCnt = campaignHeader.m_iMaximumRespawnRadios;
					
					if (suppliesMax != -1)
						STARTING_SUPPLIES_MAX = suppliesMax;
					
					if (suppliesMin != -1)
						STARTING_SUPPLIES_MIN = suppliesMin;
					
					if (respawnRadiosCnt != -1)
						m_iMaxRespawnRadios = respawnRadiosCnt;
					
					Replication.BumpMe();
				}
			}
		
			// Players limit should be an even number
			if (m_iTotalPlayersLimit % 2 != 0)
				m_iTotalPlayersLimit++;
			
			// Set AI group sizes based on players limit
			m_iAIGroupSizeWest = Math.Round((m_iTotalPlayersLimit / 2) / AI_GROUPS_PER_FACTION);
			m_iAIGroupSizeEast = m_iAIGroupSizeWest;
			
#ifdef TDM_CLI_SELECTION
			m_bRandomizeBases = false;
#endif
			
			// Do we want to flip the Main base ownership?
			if (m_bRandomizeBases)
			{
				Math.Randomize(-1);
				m_bFlipBases = Math.RandomIntInclusive(0, 1);
				
				if (m_bFlipBases)
					Print("HQ locations have been swapped.");
				else
					Print("HQ locations at default.");
			}
		};
		
		//Parse & register XP reward list
		m_aXPRewardList = new array<ref SCR_CampaignXPRewardInfo>;
		Resource container2 = BaseContainerTools.LoadContainer("{E6FC4537B53EA00B}Configs/Campaign/XPRewards.conf");
		SCR_CampaignXPRewardList list2 = SCR_CampaignXPRewardList.Cast(BaseContainerTools.CreateInstanceFromContainer(container2.GetResource().ToBaseContainer()));
		list2.GetRewardList(m_aXPRewardList);
		
		//Register to script invokers
		s_OnBaseCaptured.Insert(OnBaseCaptured);
		SCR_CampaignBaseManager.s_OnAllBasesInitialized.Insert(AfterAllBasesInitialized);
		
#ifdef ENABLE_BUILDING_DEBUG
		Print("Script invoker AfterAllBasesInitialized has been added");
#endif	
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_GameModeCampaignMP()
	{
		m_aRegisteredClients = null;
		m_aAIGroups = null;
		m_rRemnants = null;
		m_rAIVehicleWest = null;
		m_rAIVehicleEast = null;
		m_aBasesWithPlayer = null;
		m_aDedicatedSlots = null;
		m_Preload = null;
		m_aRemnantsPresence.Clear();
		m_aRemnantsPresence = null;
		
		if (m_wXPInfo)
			m_wXPInfo.RemoveFromHierarchy();
		
		if (m_wPlayersList)
			m_wPlayersList.RemoveFromHierarchy();
		
		delete SCR_CampaignBaseManager.GetInstance();
		
		//Unregister from script invokers
		s_OnBaseCaptured.Remove(OnBaseCaptured);
		SCR_UITaskManagerComponent.s_OnTaskListVisible.Remove(ShowXP);
	}

};

enum SCR_ECampaignHints
{
	NONE,
	SIGNAL,
	SERVICES,
	RESPAWN,
	SEIZING,
	SUPPLY_RUNS,
	KILL_XP,
	MOBILE_ASSEMBLY,
	SUPPLIES_UNLOADING,
	REINFORCEMENTS,
	TICKETS,
	BASE_LOST
};