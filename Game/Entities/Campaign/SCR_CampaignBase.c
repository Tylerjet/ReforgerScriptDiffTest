[EntityEditorProps(category: "GameScripted/Campaign", description: "Base class for campaign bases.", color: "0 0 255 255")]
class SCR_CampaignBaseClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBase : GenericEntity
{
	static const string VARNAME_BASE_ID = "m_iBaseID";
	static const float RADIO_RECONFIGURATION_DURATION = 20.0;
	static const int UNDER_ATTACK_WARNING_PERIOD = 60;
	protected static const ResourceName GARAGE_BOARD_US = "{92442E76D55E8F36}Assets/Props/Civilian/NoticeBoardCork_01/Data/NoticeBoardCork_01_US.emat";
	protected static const ResourceName GARAGE_BOARD_USSR = "{D396E8B04E0CC347}Assets/Props/Civilian/NoticeBoardCork_01/Data/NoticeBoardCork_01_USSR.emat";
	protected static const float ICON_FLASH_DURATION = 20;
	protected static const float ICON_FLASH_PERIOD = 0.5;
	protected static const float HQ_VEHICLE_SPAWN_RADIUS = 32;
	protected static const float HQ_VEHICLE_QUERY_SPACE = 4.5;
	protected static const int REINFORCEMENTS_PERIOD = 5;	//s: how ofter reinforcements arrive at Main bases
	
	//This script invoker is called on clients both for JIP and already playing players
	static ref ScriptInvoker s_OnBaseOwnerChanged = new ScriptInvoker();
	static ref ScriptInvoker s_OnBaseCapture = new ScriptInvoker();
	static ref ScriptInvoker s_OnSpawnPointOwnerChanged = new ScriptInvoker();
	static ref ScriptInvoker s_OnMapItemInfoChanged = new ScriptInvoker();
	static ref ScriptInvoker s_OnServiceRegistered = new ScriptInvoker();
		
	//**********//
	//ATTRIBUTES//
	//**********//
	[Attribute("Base", desc: "The display name of this base.")]
	protected string m_sBaseName;
	
	[Attribute("BASE", desc: "The display name of this base, in upper case.")]
	protected string m_sBaseNameUpper;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECampaignBaseOwner))]
	protected SCR_ECampaignBaseOwner m_eStartingBaseOwner;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox), RplProp()]
	protected bool m_bIsControlPoint;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Can this base be picked as a faction's main base?")]
	protected bool m_bCanBeHQ;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bDisableWhenUnusedAsHQ;
	
	[Attribute("", desc: "Name of associated map location (to hide its label)")]
	protected string m_sMapLocationName;
	
	[Attribute("", desc: "Name of the Slot on which to spawn a HQ.")]
	protected string m_sSlotNameHQ;
	
	[Attribute("", desc: "Name of the Slot on which to spawn a Supply Depot.")]
	protected string m_sSlotNameSupplies;
	
	[Attribute("", desc: "Name of the Slot on which to spawn a Fuel Depot.")]
	protected string m_sSlotNameFuel;
	
	[Attribute("", desc: "Name of the Slot on which to spawn an Armory.")]
	protected string m_sSlotNameArmory;
	
	[Attribute("", desc: "Name of the Slot on which to spawn a Light Vehicle Depot.")]
	protected string m_sSlotNameLightVehicleDepot;	
	
	[Attribute("", desc: "Name of the Slot on which to spawn a Heavy Vehicle Depot.")]
	protected string m_sSlotNameHeavyVehicleDepot;	
	
	[Attribute("", desc: "Name of the Slot on which to spawn a Field Hospital.")]
	protected string m_sSlotNameHospital;	
	
	[Attribute("", desc: "Name of the Slot on which to spawn Barracks.")]
	protected string m_sSlotNameBarracks;
	
	[Attribute("", desc: "Name of the Slot on which to spawn Radio antenna.")]
	protected string m_sSlotNameRadioAntenna;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Type", enums: ParamEnumArray.FromEnum(CampaignBaseType))]
	protected CampaignBaseType m_eType;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Prebuilt services")]
	protected bool m_bPrebuilt;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Spawn composition if slot field is empty")]
	protected bool m_bSpawnNoSlotComposition;
	
	[Attribute("0", desc: "Print debug for this base?", category: "Debug")]
	protected bool m_bPrintDebug;
	
	[Attribute("0.2", desc: "Radio range alpha", params: "0 1")]
	protected float m_fRadioRangeAlpha;
	
	[Attribute("32", UIWidgets.Slider, "Radio frequency (MHz) for players operating at this base (US)", "32 68 4.0")]
	protected int m_iFreqWest;
	
	[Attribute("38", UIWidgets.Slider, "Radio frequency (MHz) for players operating at this base (USSR)", "38 54 2")]
	protected int m_iFreqEast;
	
	[Attribute("{C35F29E48086221A}Configs/Campaign/CampaignGraphLinesConfig.conf")]
	protected ref SCR_GraphLinesData m_GraphLinesData;
	
	[Attribute("1000")]
	protected int m_iAntennaSignalBoost;
	
	// Update VARNAME_BASE_ID if renaming this (m_iBaseID) variable!
	[Attribute("-1")]
	protected int m_iBaseID;
	
	[Attribute("3")]
	protected float m_fLineWidth;
	
	//************************//
	//RUNTIME MEMBER VARIABLES//
	//************************//
	protected MapItem m_MapItem;
	protected ref array<SCR_CampaignBase> m_aInRangeOf = new ref array<SCR_CampaignBase>();
	protected ref array<SCR_CampaignBase> m_aInMyRange = new ref array<SCR_CampaignBase>();
	protected ref array<ref SCR_CampaignRemnantsPresence> m_aRemnants = new array<ref SCR_CampaignRemnantsPresence>;
	protected SCR_SpawnPoint m_SpawnPoint;
	protected SCR_CampaignFaction m_CapturingFaction;
	protected SCR_CampaignTriggerEntity m_Trigger;
	protected int m_iOwningFaction = INVALID_FACTION_INDEX;
	protected SCR_CampaignFaction m_OwningFactionPrevious;
	protected bool m_bLocalPlayerPresent = false;
	protected AIWaypointCycle m_CycleWaypoint;
	protected float m_fReinforcementsArrivalTime = int.MAX;
	protected ref map<ECampaignServicePointType, SCR_CampaignServiceComponent> m_aServices = new map<ECampaignServicePointType, SCR_CampaignServiceComponent>();
	protected SCR_SiteSlotEntity m_SlotHQ;
	protected SCR_SiteSlotEntity m_SlotSupplies;
	protected SCR_SiteSlotEntity m_SlotFuel;
	protected SCR_SiteSlotEntity m_SlotArmory;
	protected SCR_SiteSlotEntity m_SlotLightVehicleDepot;
	protected SCR_SiteSlotEntity m_SlotHeavyVehicleDepot;
	protected SCR_SiteSlotEntity m_SlotHospital;
	protected SCR_SiteSlotEntity m_SlotBarracks;
	protected SCR_SiteSlotEntity m_SlotRadioAntenna;
	protected SCR_CampaignFaction m_BuildingsFaction;
	protected BaseRadioComponent m_RadioComponent = null;
	protected RplComponent m_RplComponent;
	protected bool m_bRespawnBecameAvailable = true;
	protected IEntity m_HQRadio;
	protected IEntity m_GarageBoard;
	protected SCR_CampaignMapUIBase m_UIElement;
	protected bool m_bShowMapLinks = true;
	protected string m_sCallsign;
	protected string m_sCallsignUpper;
	protected string m_sCallsignNameOnly;
	protected string m_sCallsignNameOnlyUC;
	protected AudioHandle m_PlayedRadio = AudioHandle.Invalid;
	protected ref array<MapLink> m_aMapLinks = {};
	protected bool m_bIsHovered = false;
	protected SCR_TimedWaypoint m_SeekDestroyWP;
	protected SCR_SmartActionWaypoint m_RetakeWP;
	protected FactionKey m_sStartingBaseOwner;
	protected ResourceName m_sBuildingIconImageset = "{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset";
	protected bool m_bLoadStateVehicleDepotBuilt;
	protected bool m_bLoadStateArmoryBuilt;
	protected bool m_bLoadStateHeavyVehicleDepotBuilt;
	protected bool m_bLoadStateSupplyDepotBuilt;
	protected bool m_bLoadStateAntennaBuilt;
	protected bool m_bLoadStateBarracksBuilt;
	protected float m_fLastEnemyContactTimestamp;
	protected bool m_bBuildingsSpawned;
	protected SCR_FactionAffiliationComponent m_FactionControl;
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	protected SCR_CampaignBarracksComponent m_BarrackComponent;
	protected SCR_ArmoryComponent m_ArmoryComponent;
	protected ref array <SCR_CampaignEntitySpawnerComponent> m_aVehicleSpawners = new array <SCR_CampaignEntitySpawnerComponent>();
	protected SCR_AIGroup m_DefendersGroup;
	protected bool m_bCanAnswerHQ;
	
	//Set only on server
	//The faction that last sent reinforcements to this base (SendReinforcements())
	protected SCR_CampaignFaction m_SuppliesFaction;
	
	ref ScriptInvoker m_OnReinforcementsArrived = new ScriptInvoker();
	
	//As s_OnBaseOwnerChanged, however it is called on all clients
	ref ScriptInvoker m_OnFactionChanged = new ScriptInvoker();
	
	//Called once spawnpoint entity was assigned
	ref ScriptInvoker m_OnSpawnPointAssigned = new ScriptInvoker();
	
	//********************************//
	//RUNTIME SYNCHED MEMBER VARIABLES//
	//********************************//
	[RplProp(onRplName: "OnCapturingFactionChanged")]
	protected int m_iCapturingFaction = INVALID_FACTION_INDEX;
	[RplProp()]
	protected float m_fCaptureStartTimestamp;
	[RplProp(onRplName: "OnHasSignalChanged")]
	protected bool m_bIsLinkedToMainWest = false;
	[RplProp(onRplName: "OnHasSignalChanged")]
	protected bool m_bIsLinkedToMainEast = false;
	[RplProp()]
	protected int m_iReconfiguredBy = INVALID_PLAYER_INDEX;
	[RplProp()]
	protected float m_fRespawnAvailableSince;
	[RplProp()]
	protected bool m_bEnabled = true;
	[RplProp(onRplName: "OnHQSet")]
	protected bool m_bIsHQ = false;
	[RplProp(onRplName: "OnAttackingFactionChanged")]
	protected int m_iAttackingFaction = -1;
	[RplProp(onRplName: "OnCallsignAssigned")]
	protected int m_iCallsign = INVALID_BASE_INDEX;
	[RplProp(onRplName: "OnAntennaBuilt")]
	protected bool m_bAntennaBuilt = false;
		
	//*********//
	//CONSTANTS//
	//*********//
	
	static const int INVALID_PLAYER_INDEX = -1;
	static const int INVALID_FACTION_INDEX = -1;
	static const int INVALID_BASE_INDEX = -1;
	static const int INVALID_FREQUENCY = -1;
	static const int RESPAWN_DELAY_AFTER_CAPTURE = 180000;
	
#ifdef WORKBENCH
	protected IEntitySource m_Source;
	
	IEntitySource GetSource()
	{
		return m_Source;
	}
#endif
	
	//**************//
	//STATIC METHODS//
	//**************//
	
	//------------------------------------------------------------------------------------------------
	static SCR_CampaignBase FindBaseByEntityID(EntityID baseID)
	{
		if (!GetGame())
			return null;
		
		BaseWorld world = GetGame().GetWorld();
		
		IEntity foundEntity = world.FindEntityByID(baseID);
		
		SCR_CampaignBase base = null;
		
		if (foundEntity)
			base = SCR_CampaignBase.Cast(foundEntity);
		
		return base;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Helper method to save lines when finding DamageManagerComponent
	static DamageManagerComponent FindDamageManagerComponent(GenericEntity entity)
	{
		auto foundComponent = entity.FindComponent(DamageManagerComponent);
		if (foundComponent)
		{
			auto castComponent = DamageManagerComponent.Cast(foundComponent);
			return castComponent;
		}
		
		return null;
	}
	
	//********************//
	//MEMBER EVENT METHODS//
	//********************//
	
	//------------------------------------------------------------------------------------------------
	protected void OnLocalPlayerFactionAssigned(Faction assignedFaction)
	{
		if (!m_bIsHQ || GetOwningFaction() == assignedFaction)
			MapSetup();
		
		HandleMapLinks();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AfterAllBasesInitialized()
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;
		
		HandleMapLinks();
		HideMapLocationLabel();
	}
	
	//------------------------------------------------------------------------------------------------
	//! An event called when the local player leaves the trigger of this base.
	protected void OnLocalPlayerLeft()
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_BASES))
			return;
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! An event called when the local player enters the trigger of this base.
	protected void OnLocalPlayerEntered()
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_BASES))
			return;
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! An event called when the trigger of this base has been changed, the trigger parameter is always the previous trigger (the one being unregistered)!
	protected void OnUnregisterTrigger(SCR_CampaignTriggerEntity trigger)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_BASES))
			return;
#endif
	}
	
	//************************//
	//PROTECTED MEMBER METHODS//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	int GetBaseSpawnCost()
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetCampaign();
		if (!campaign)
			return -1;
		
		//reduce spawning cost if barracks are built
		if (m_aServices.Get(ECampaignServicePointType.BARRACKS))
			return campaign.GetSpawnCost() / 2;
		else 
			return campaign.GetSpawnCost();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateBaseDisplayName(string newDisplayName)
	{
		m_MapItem.SetDisplayName(newDisplayName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBaseCaptured(SCR_CampaignBase base)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns bool whether the local player's character is present in this bases trigger zone
	protected bool CheckForLocalPlayer(array<ChimeraCharacter> presentCharacters)
	{
		// We try to get the local players character
		ChimeraCharacter chimeraCharacter = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		
		if (presentCharacters.Find(chimeraCharacter) != -1)
		{
			if (!m_bLocalPlayerPresent)
				OnLocalPlayerEntered();
			return true;
		}
		else
		{
			if (m_bLocalPlayerPresent)
				OnLocalPlayerLeft();
			return false;
		}
		
		// Automatically we assume he isn't, return false
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the base that is closest to the other base
	protected SCR_CampaignBase GetLinkedBaseClosestTo(SCR_CampaignBase otherBase)
	{
		if (!otherBase || !m_aInRangeOf)
			return null;
		
		int index = INVALID_BASE_INDEX;
		float lowestSqDistance = float.MAX;
		vector otherBaseOrigin = otherBase.GetOrigin();
		for (int i = 0, count = m_aInRangeOf.Count(); i < count; i++)
		{
			// Measure distance of each base to the other base
			float distanceSq = vector.DistanceSq(otherBaseOrigin, m_aInRangeOf[i].GetOrigin());
			
			// Store the lowest distance and it's base index in the m_aInRangeOf array
			if (distanceSq < lowestSqDistance)
			{
				index = i;
				lowestSqDistance = distanceSq;
			}
		}
		
		// No base found
		if (index == INVALID_BASE_INDEX)
			return null;
		
		// A base has been found, return the closest one
		return m_aInRangeOf[index];
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stores an array of bases in radio range for this base
	void LinkBases(bool refreshTasks = false)
	{
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		array<SCR_CampaignBase> bases = baseManager.GetBases();
		
		if (!m_aInRangeOf || !m_aInMyRange)
			return;
		
		m_aInMyRange.Clear();
		m_aInRangeOf.Clear();
		
		// Go through all existing bases
		for (int i = 0, count = bases.Count(); i < count; i++)
		{
			// Ignore self
			if (this == bases[i])
				continue;
			
			if (!bases[i].GetIsEnabled())
				continue;
			
			// Measure the distance between this base and the other base in the world
			float distanceSq = vector.DistanceSq(GetOrigin(), bases[i].GetOrigin());
		
			// Load the other base's radio range 
			float signalRange = bases[i].GetSignalRange();
			
			// If the other base is in range of this one (or the other way around), add it to the m_aInRangeOf array
			if (distanceSq <= Math.Pow(signalRange, 2))
				m_aInRangeOf.Insert(bases[i]);
			
			
			if (distanceSq <= Math.Pow(GetSignalRange(), 2))
				m_aInMyRange.Insert(bases[i]);
		}
		
		CalculateSignal();
		baseManager.UpdateBasesSignalCoverage();
		
		if (refreshTasks && !IsProxy())
		{
			SCR_CampaignTaskSupportEntity supportClass = SCR_CampaignTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignTaskSupportEntity));
			
			if (supportClass)
				supportClass.GenerateCaptureTasks(this);
		}
		SCR_GameModeCampaignMP.s_OnSignalChanged.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void CalculateSignal()
	{
		if (m_bIsHQ)
		{
			m_bCanAnswerHQ = true;
			return;
		}
		
		foreach (SCR_CampaignBase inRange: m_aInMyRange)
		{
			if (m_aInRangeOf.Contains(inRange))
			{
				if (inRange.GetIsHQ() || inRange.GetIsLinkedToHQ())
				{
					m_bCanAnswerHQ = true;
					return;
				}
				else
					m_bCanAnswerHQ = false;
			}
			else
				m_bCanAnswerHQ = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsLinkedToHQ()
	{
		return m_bCanAnswerHQ;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnHQSet()
	{
		m_bCanAnswerHQ = true;
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_CampaignBase> GetBasesInRangeSimple(bool inRangeOf = false)
	{
		if (inRangeOf)
			return m_aInRangeOf;
		else
			return m_aInMyRange;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Changes the owner faction of this base to the newOwningFaction parameter value
	//! If a base has been captured, then this should only be called from FinishCapture, since the evaluation of game state is done there!
	protected void ChangeOwner(SCR_CampaignFaction newOwningFaction, bool changedAtStart = false)
	{
		if (!newOwningFaction || GetOwningFaction() == newOwningFaction)
			return;
		
		bool retakenByRemnants = !newOwningFaction.IsPlayable();
		bool firstTime = false;
		
		if (!GetOwningFaction())
		{
			firstTime = true;
			m_SuppliesFaction = newOwningFaction;
		}
		
		SCR_CampaignFaction prevOwningFaction = GetOwningFaction();
		
		if (retakenByRemnants)
			SetOwningFaction(null);
		else
			SetOwningFaction(newOwningFaction);
		
		// Delay respawn possibility at newly-captured bases
		if (!changedAtStart)
		{
			m_fRespawnAvailableSince = Replication.Time() + RESPAWN_DELAY_AFTER_CAPTURE;
			m_bRespawnBecameAvailable = false;
			
			HandleSpawnPointFaction();
			
			SCR_SaveLoadComponent saveComp = SCR_SaveLoadComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_SaveLoadComponent));
			
			if (saveComp)
				saveComp.Save();
		}
		else
		{
			m_SpawnPoint.SetFactionKey(newOwningFaction.GetFactionKey());
		}
		
		// If some Remnants live, send them to recapture
		if (!retakenByRemnants)
			foreach (SCR_CampaignRemnantsPresence remnants: m_aRemnants)
			{
				AIGroup grp = remnants.GetSpawnedGroup();
				
				if (!grp)
					continue;
				
				if (!m_RetakeWP && m_HQRadio)
				{
					EntitySpawnParams params = EntitySpawnParams();
					params.TransformMode = ETransformMode.WORLD;
					params.Transform[3] = m_HQRadio.GetOrigin();
					m_SeekDestroyWP = SCR_TimedWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(SCR_GameModeCampaignMP.GetInstance().GetSeekDestroyWaypointPrefab()), null, params));
					m_SeekDestroyWP.SetHoldingTime(60);
					m_RetakeWP = SCR_SmartActionWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(SCR_GameModeCampaignMP.GetInstance().GetRetakeWaypointPrefab()), null, params));
					m_RetakeWP.SetSmartActionEntity(m_HQRadio, "CapturePoint");
				}
				
				if (m_RetakeWP)
					grp.AddWaypointAt(m_RetakeWP, 0);
				if (m_SeekDestroyWP)
					grp.AddWaypointAt(m_SeekDestroyWP, 0);
			}
		
		// Change barrack group according to owner's faction, if it was built
		if (m_BarrackComponent && m_FactionControl)
		{
			SCR_CampaignFaction owningFaction = SCR_CampaignFaction.Cast(m_FactionControl.GetAffiliatedFaction());
			
			if (!owningFaction)
				m_BarrackComponent.StopHandler();
			else
			{
				ResourceName defenderGroup = owningFaction.GetDefendersGroupPrefab();
			
				if (defenderGroup)
				{
					m_BarrackComponent.EnableSpawning(false);
					m_BarrackComponent.SetGroupPrefab(defenderGroup);
					if (!m_BarrackComponent.IsInitialized())
						m_BarrackComponent.InitializeBarrack();
					
					GetGame().GetCallqueue().CallLater(m_BarrackComponent.EnableSpawning, m_BarrackComponent.GetRespawnDelay()*1000, false, true);
				}
			}
		}
		
		// Reconfigure RadioComponent used for communication.		
		BaseRadioComponent radio = BaseRadioComponent.Cast(FindComponent(BaseRadioComponent));
		
		if (!radio)
			return;
		
		if (retakenByRemnants)
		{
			radio.TogglePower(false);
		}
		else
		{
			radio.TogglePower(true);
			radio.SetFrequency(newOwningFaction.GetFactionRadioFrequency());
			radio.SetEncryptionKey(newOwningFaction.GetFactionRadioEncryptionKey());
		}
		
		// Change owner of assigned armory 
		if (m_ArmoryComponent)
			m_ArmoryComponent.ChangeOwningFaction(GetOwningFaction());
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SiteSlotEntity GetAssignedSlot(ECampaignCompositionType type)
	{
		switch (type)
		{
			case ECampaignCompositionType.HQ: {return m_SlotHQ;};
			case ECampaignCompositionType.SUPPLIES: {return m_SlotSupplies;};
			case ECampaignCompositionType.FUEL: {return m_SlotFuel;};
			case ECampaignCompositionType.LIGHT_VEHICLE_DEPOT: {return m_SlotLightVehicleDepot;};
			case ECampaignCompositionType.HEAVY_VEHICLE_DEPOT: {return m_SlotHeavyVehicleDepot;};
			case ECampaignCompositionType.ARMORY: {return m_SlotArmory;};
			case ECampaignCompositionType.HOSPITAL: {return m_SlotHospital;};
			case ECampaignCompositionType.BARRACKS: {return m_SlotBarracks;};
			case ECampaignCompositionType.RADIO_ANTENNA: {return m_SlotRadioAntenna;};
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Capturing has been successfully finished
	void FinishCapture(int playerID = INVALID_PLAYER_INDEX)
	{
		if (IsProxy())
			return;
		
		if (m_bPrintDebug)
			Print("Captured by " + m_CapturingFaction.GetFactionKey());
		
		//Let the players know a base has been seized - moved to tasks
		//TODO FOR CLIENTS AS WELL
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		ChangeOwner(m_CapturingFaction);
		
		EndCapture();
		
		if (campaign)
		{
			SCR_GameModeCampaignMP.s_OnBaseCaptured.Invoke(this);

			if (GetType() != CampaignBaseType.RELAY)
			{			
				// Reward XP for seizing the base
				array<int> players = {};
				int playersCount = GetGame().GetPlayerManager().GetPlayers(players);
				
				for (int i = 0; i < playersCount; i++)
				{
					IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(players[i]);
					
					if (!playerEntity)
						continue;
					
					Faction playerFaction = SCR_CampaignReconfigureRelayUserAction.GetPlayerFaction(playerEntity);
					
					if (playerFaction != GetOwningFaction())
						continue;
					
					if (vector.DistanceSq(playerEntity.GetOrigin(), this.GetOrigin()) < 90000)	// 300m
						campaign.AwardXP(GetGame().GetPlayerManager().GetPlayerController(players[i]), CampaignXPRewards.BASE_SEIZED);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyAboutEnemyAttack(notnull Faction attackingFaction)
	{
		if (!GetOwningFaction())
			return;
		
		if (!IsBaseInFactionRadioSignal(GetOwningFaction()))
			return;
		
		if (GetLastEnemyContactTimestamp() != 0 && GetLastEnemyContactTimestamp() > (Replication.Time() - (SCR_CampaignBase.UNDER_ATTACK_WARNING_PERIOD * 1000)))
			return;
		
		GetOwningFaction().SendHQMessage(SCR_ERadioMsg.BASE_UNDER_ATTACK, GetCallsign(), param: GetBaseID());
		SetLastEnemyContactTimestamp(Replication.Time());
		
		if (!GetCapturingFaction())
			SetAttackingFaction(GetGame().GetFactionManager().GetFactionIndex(attackingFaction));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if the session is run as client
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Capturing has been terminated
	void EndCapture()
	{
		if (IsProxy())
			return;
		
		if (!m_CapturingFaction)
			return;
		
		if (m_bPrintDebug)
			Print("Capturing by " + m_CapturingFaction.GetFactionKey() + " stopped");
		
		m_CapturingFaction = null;
		
		m_iCapturingFaction = INVALID_FACTION_INDEX;
		Replication.BumpMe();
		OnCapturingFactionChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBaseID(int baseID)
	{
		m_iBaseID = baseID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBaseID()
	{
		return m_iBaseID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetReconfiguredByID()
	{
		return m_iReconfiguredBy;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsControlPoint(bool toggle)
	{
		m_bIsControlPoint = toggle;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsControlPoint()
	{
		return m_bIsControlPoint;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetCanBeHQ()
	{
		return m_bCanBeHQ;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetDisableWhenUnusedAsHQ()
	{
		return m_bDisableWhenUnusedAsHQ;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetSuppliesFaction()
	{
		return m_SuppliesFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnAIWaypoints()
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return;
		
		array<IEntity> spawnPoints = new array<IEntity>();
		array<AIWaypoint> queueOfWaypoints = new array<AIWaypoint>();
		IEntity child = GetChildren();
		AIWaypoint wp;
		SCR_DefendWaypoint defendWP;
		
		while (child)
		{	
			if (child.Type() == SCR_CampaignRemnantsSpawnPoint)
				spawnPoints.Insert(child);
			
			child = child.GetSibling();
		};
		
		foreach (IEntity spawnPoint: spawnPoints)
		{
			if (!spawnPoint)
				continue;
			
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = spawnPoint.GetOrigin(); 
			// we spawn patrol Waypoint and Defend waypoint for each spawnPoint
			wp = AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(campaign.GetPatrolWaypointPrefab()), null, params));
			if (wp)
				queueOfWaypoints.Insert(wp);
			
			/* commented out due to not working compositions for EA
			defendWP = SCR_DefendWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(campaign.GetDefendWaypointPrefab()), null, params));
			if (defendWP)
			{
				defendWP.SetCompletionRadius(30);
				defendWP.SetHoldingTime(30);
				queueOfWaypoints.Insert(defendWP);
			}
			*/
		}
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = GetOrigin();
		m_CycleWaypoint = AIWaypointCycle.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(campaign.GetCycleWaypointPrefab()), null, params));
		
		if (m_CycleWaypoint)
		{
			m_CycleWaypoint.SetWaypoints(queueOfWaypoints);
			m_CycleWaypoint.SetRerunCounter(-1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Capturing has begun
	bool BeginCapture(SCR_CampaignFaction faction, int playerID = INVALID_PLAYER_INDEX)
	{
		if (IsProxy() || !faction)
			return false;
		
		// The capturing faction already owns this base, return
		if (faction == GetOwningFaction())
			return false;
		
		// Reset the capture timer
		m_fCaptureStartTimestamp = Replication.Time();
		
		// Change the capturing faction
		m_CapturingFaction = faction;
		
		m_iCapturingFaction = SCR_CampaignFactionManager.GetInstance().GetFactionIndex(faction);
		
		if (m_bPrintDebug)
			Print("Capturing by " + m_CapturingFaction.GetFactionKey() + " has begun");
		
		m_iReconfiguredBy = playerID;
		Replication.BumpMe();
		OnCapturingFactionChanged();
		NotifyAboutEnemyAttack(faction);
		
		return true;
	}
	
	// method will permute waypoints in the list such that it starts from startingIndex and ends with startingIndex -1 
	// the expected cycle waypoint from base is made by pairs of waypoints PatrolWP1, DefendWP1, PatrolWP2, DefendWP2,..., 
	//------------------------------------------------------------------------------------------------
	protected void PermuteBaseCycleWaypoint(notnull AIWaypointCycle cycleWP, int startingIndex)
	{
		array<AIWaypoint> defaultWPs = {};
		array<AIWaypoint> cycleWPsPermutation = {};
		cycleWP.GetWaypoints(defaultWPs);
		
		/* commented out due to not working compositions for EA 
		startingIndex *= 2;
		int countOfWPs = defaultWPs.Count();
		if (startingIndex == 0 || defaultWPs.IsEmpty() || countOfWPs < startingIndex || countOfWPs % 2 == 1)
			return;
		
		for (int i = 0; i < startingIndex; i += 2 )
		{
			cycleWPsPermutation.Insert(defaultWPs[i]);
			cycleWPsPermutation.Insert(defaultWPs[i + 1]);
		}
		
		for (int i = countOfWPs - 1; i > startingIndex; i -= 2)
		{
			cycleWPsPermutation.InsertAt(defaultWPs[i],0);
			cycleWPsPermutation.InsertAt(defaultWPs[i-1],0);
		}
		*/ 
		int countOfWPs = defaultWPs.Count();
		if (startingIndex == 0 || defaultWPs.IsEmpty() || countOfWPs < startingIndex)
			return;
		
		for (int i = 0; i < startingIndex; i ++ )
		{
			cycleWPsPermutation.Insert(defaultWPs[i]);			
		}
		
		for (int i = countOfWPs - 1; i >= startingIndex; i -= 1)
		{
			cycleWPsPermutation.InsertAt(defaultWPs[i],0);			
		}
		
		cycleWP.SetWaypoints(cycleWPsPermutation);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Changes the faction which can spawn on spawn point groups owned by this base
	protected void HandleSpawnPointFaction()
	{
		if (!m_SpawnPoint)
			return;
		
		SCR_CampaignFaction owner = GetOwningFaction();
		FactionKey currentKey = m_SpawnPoint.GetFactionKey();
		FactionKey ownerKey;
		FactionKey finalKey;
		
		if (owner)
		{
			ownerKey = owner.GetFactionKey();
			finalKey = ownerKey;
		}
		
		if (ownerKey == FactionKey.Empty)
		{
			if (currentKey != FactionKey.Empty)
			{
				m_SpawnPoint.SetFactionKey(FactionKey.Empty);
				s_OnSpawnPointOwnerChanged.Invoke();
			}
			
			return;
		}
		
		if (!IsBaseInFactionRadioSignal(owner))
			finalKey = FactionKey.Empty;
		
		if (Replication.Time() < m_fRespawnAvailableSince)
			finalKey = FactionKey.Empty;
		
		if (GetSupplies() < GetBaseSpawnCost() && !m_bIsHQ)
			finalKey = FactionKey.Empty;
		
		if (finalKey == currentKey)
			return;
		
		m_SpawnPoint.SetFactionKey(finalKey);
		s_OnSpawnPointOwnerChanged.Invoke();
	}
	//------------------------------------------------------------------------------------------------
	SCR_SpawnPoint GetBaseSpawnPoint()
	{
		return m_SpawnPoint;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ECampaignBaseOwner GetStartingBaseOwnerEnum()
	{
		return m_eStartingBaseOwner;
	}
	
	//------------------------------------------------------------------------------------------------
	FactionKey GetStartingBaseOwner()
	{
		return m_sStartingBaseOwner;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Changes the owning faction to proper one if starting base owner parameter is set
	void SetStartingBaseOwner()
	{
		// Need to find faction manager to find the faction object by it's name
		SCR_CampaignFaction faction = SCR_CampaignFactionManager.GetInstance().GetCampaignFactionByKey(m_sStartingBaseOwner);
		bool change = true;
		
		if (GetType() == CampaignBaseType.MAIN)
		{
			if (m_bIsHQ)
			{
				if (faction)
					faction.SetMainBase(this);
			}
			else
				change = false;
		}
		
		if (faction && change)
			ChangeOwner(faction, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAsHQ(SCR_ECampaignBaseOwner owner)
	{
		if (IsProxy())
			return;
		
		m_eType = CampaignBaseType.MAIN;
		m_eStartingBaseOwner = owner;
		m_bIsHQ = true;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsEnabled()
	{
		return m_bEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsHQ()
	{
		return m_bIsHQ;
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterAsParentBase(notnull SCR_CampaignServiceComponent comp, bool add)
	{
		if (IsProxy())
			return;
		
		comp.SetParentBaseID(m_RplComponent.Id(), add);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Event which is triggered when the capturing faction changes
	protected void OnCapturingFactionChanged()
	{
		m_CapturingFaction = SCR_CampaignFactionManager.GetInstance().GetCampaignFactionByIndex(m_iCapturingFaction);
		
		if (m_bPrintDebug)
		{
			Print("OnCapturingFactionChanged");
			Print(m_iCapturingFaction);
			Print(m_CapturingFaction);
		}
		
		// Play or stop radio tuning SFX
		if (m_HQRadio)
		{
			SoundComponent comp = SoundComponent.Cast(m_HQRadio.FindComponent(SoundComponent));
			
			if (comp)
			{
				if (m_CapturingFaction)
				{
					ToggleRadioChatter(false);
					m_PlayedRadio = comp.SoundEvent(SCR_SoundEvent.SOUND_RADIO_ESTABLISH_ACTION);
				}
				else
				{
					if (m_PlayedRadio != AudioHandle.Invalid)
						comp.TerminateAll();
					
					ToggleRadioChatter(true);
				}
			}
		}
		
		if (!IsProxy())
		{
			SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
			
			if (campaign)
				campaign.EvaluateGame();
		}
		
		if (m_CapturingFaction && GetOwningFaction() == SCR_RespawnSystemComponent.GetLocalPlayerFaction())
			FlashBaseIcon(faction: m_CapturingFaction, infiniteTimer: true);
		else
			FlashBaseIcon(changeToDefault: true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHasSignalChanged()
	{
		if (!IsProxy())
		{
			HandleSpawnPointFaction();
			SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
			
			if (campaign)
				campaign.EvaluateGame();
		}
		
		if (RplSession.Mode() != RplMode.Dedicated)
			HandleMapInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSuppliesChanged()
	{
		if (!IsProxy())
			HandleSpawnPointFaction();
		
		if (RplSession.Mode() != RplMode.Dedicated)
			HandleMapInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update cooldown timer in map UI
	protected void ShowRespawnCooldown()
	{
		if (Replication.Time() >= m_fRespawnAvailableSince)
			return;
		
		SCR_GameModeCampaignMP camp = SCR_GameModeCampaignMP.GetInstance();

		if (!camp)
			return;

		Faction playerFaction = camp.GetLastPlayerFaction();

		if (!playerFaction)
			return;
		
		if (GetBaseUI())
			GetBaseUI().SetIconInfoText();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdatePreviousFaction()
	{
		m_OwningFactionPrevious = GetOwningFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Event which is triggered when the owning faction changes
	protected void OnOwningFactionChanged(Faction newFaction)
	{
#ifdef ENABLE_DIAG
		m_aDebugShapes.Clear();
#endif
		
		GetGame().GetCallqueue().CallLater(UpdatePreviousFaction, 1000, false);
		
		RefreshVehicleSpawners();
		
		if (!IsProxy())
		{
			// Update signal coverage only if the base was seized during normal play, not at the start
			if (GetGame().GetWorld().GetWorldTime() != 0)
			{
				SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
				
				if (baseManager)
					baseManager.UpdateBasesSignalCoverage();
			}
			
			// Reset timer for reinforcements
			if (GetType() == CampaignBaseType.MAIN)
			{
				ReinforcementsTimer(true);
				Replication.BumpMe();
			}
		}
		else
		{
			//On clients we want to invoke this script invoker for local usage
			//First parameter is SCR_CampaignBase -> Cannot be null, we are passing "this"
			//Second parameter is SCR_CampaignFaction -> Could be null
			s_OnBaseOwnerChanged.Invoke(this, GetOwningFaction());
		}
		
		LinkBases();
		FlashBaseIcon(changeToDefault: true);
		
		if (RplSession.Mode() != RplMode.Dedicated)
		{
			HandleMapInfo();
			HandleMapLinks();
			
			// Update motor pool notice board texture
			if (m_GarageBoard && GetOwningFaction())
			{
				IEntity paper = m_GarageBoard.GetChildren();
				
				if (paper)
				{
					if (GetOwningFaction().GetFactionKey() == SCR_GameModeCampaignMP.FACTION_BLUFOR)
						SCR_Global.SetMaterial(paper, GARAGE_BOARD_US);
					else if (GetOwningFaction().GetFactionKey() == SCR_GameModeCampaignMP.FACTION_OPFOR)
						SCR_Global.SetMaterial(paper, GARAGE_BOARD_USSR);
						
				}
			}
			
			SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_RespawnSystemComponent.GetLocalPlayerFaction());
			SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
			IEntity player = SCR_PlayerController.GetLocalControlledEntity();
			
			// TODO: Move this to PlayRadioMsg so it is checked for player being inside radio range
			if (campaign)
			{
				if (player && GetOwningFaction() == playerFaction)
				{
					campaign.ShowHint(SCR_ECampaignHints.TICKETS);
					
					s_OnBaseCapture.Invoke()		
				}
				
				if (player && playerFaction && playerFaction != GetOwningFaction() && IsBaseInFactionRadioSignal(playerFaction))
				{
					if (GetOwningFaction())
						SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_BaseSeized-UC", prio: SCR_ECampaignPopupPriority.BASE_LOST, param1: GetOwningFaction().GetFactionNameUpperCase(), param2: GetBaseNameUpperCase());
					else
					{
						SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
						
						if (fManager)
						{
							SCR_CampaignFaction factionR = fManager.GetCampaignFactionByKey(SCR_GameModeCampaignMP.FACTION_INDFOR);
							
							if (factionR)
								SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_BaseSeized-UC", prio: SCR_ECampaignPopupPriority.BASE_LOST, param1: factionR.GetFactionNameUpperCase(), param2: GetBaseNameUpperCase());
						}
					}
					
					if (playerFaction == m_OwningFactionPrevious)
						campaign.ShowHint(SCR_ECampaignHints.BASE_LOST);
				}
				
				if (campaign.GetBasePlayerPresence() == this)
					ToggleRadioChatter(true);
			}
			
			m_OnFactionChanged.Invoke();
		}
		
		if (m_bPrintDebug)
		{
			Print("OnOwningFactionChanged");
			Print(GetOwningFaction());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearLinks()
	{
		MapItem myMapItem = GetMapItem();
		if (!myMapItem)
			return;
		
		//Clear previous links
		myMapItem.ClearLinks();
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterRemnants(notnull SCR_CampaignRemnantsPresence remnants)
	{
		m_aRemnants.Insert(remnants);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterMapLink(notnull MapLink link)
	{
		m_aMapLinks.RemoveItem(link);
	}
	
	//------------------------------------------------------------------------------------------------
	bool FindMapLink(IEntity owner, IEntity target)
	{
		for (int i = m_aMapLinks.Count() - 1; i >= 0; i--)
		{
			if(!m_aMapLinks[i])
				return false;
			if ((m_aMapLinks[i].Owner() == owner && m_aMapLinks[i].Target() == target) || (m_aMapLinks[i].Owner() == target && m_aMapLinks[i].Target() == owner))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterMapLink(notnull MapLink link)
	{
		m_aMapLinks.Insert(link);
	}
	
	//------------------------------------------------------------------------------------------------
	void ColorMapLink(notnull MapLink link)
	{
		MapLinkProps props = link.GetMapLinkProps();
		if (!props)
			return;
		props.SetLineWidth(m_fLineWidth);
		
		Color c = props.GetLineColor();
		if (!c)
			return;
		
		if (m_bIsHovered)
			c.SetA(m_GraphLinesData.GetHighlightedAlpha());
		else
			c.SetA(m_GraphLinesData.GetDefaultAlpha());
		props.SetLineColor(c);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterMyMapLinks()
	{
		for (int i = m_aMapLinks.Count() - 1; i >= 0; i--)
		{
			if(!m_aMapLinks[i])
				return;
			SCR_CampaignBase otherBase;
			
			if (m_aMapLinks[i].Owner().Entity() == this)
				otherBase = SCR_CampaignBase.Cast(m_aMapLinks[i].Target().Entity());
			else
				otherBase = SCR_CampaignBase.Cast(m_aMapLinks[i].Owner().Entity());
			
			if (!otherBase)
				return;
			
			otherBase.UnregisterMapLink(m_aMapLinks[i]);
			
			UnregisterMapLink(m_aMapLinks[i]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveObsoleteLinks()
	{
		MapItem myMapItem = GetMapItem();
		myMapItem.ClearLinks();
	}
	
	//------------------------------------------------------------------------------------------------
	void HandleMapLinks(bool showAllLinks = false)
	{
		if (m_bIsHQ && GetOwningFaction() != SCR_RespawnSystemComponent.GetLocalPlayerFaction())
			return;
		
		MapItem myMapItem = GetMapItem();
		MapItem mobilehqMapItem;
		array<SCR_CampaignBase> basesInRangeOfMobileHQ = {};
		
		if (!myMapItem)
			return;

		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CampaignFaction myFaction = GetOwningFaction();
		SCR_CampaignFaction localPlayerFaction = SCR_CampaignFaction.Cast(campaign.GetLastPlayerFaction());
		
		if (!localPlayerFaction)
			return;
		
		SCR_MapDescriptorComponent desc;
		IEntity mobileHQ = localPlayerFaction.GetDeployedMobileAssembly();
		SCR_CampaignMobileAssemblyComponent comp = SCR_CampaignMobileAssemblyComponent.Cast(Replication.FindItem(campaign.GetDeployedMobileAssemblyID(localPlayerFaction.GetFactionKey())));
		
		if (mobileHQ)
			desc = SCR_MapDescriptorComponent.Cast(mobileHQ.FindComponent(SCR_MapDescriptorComponent));
		
		if (desc && comp.IsInRadioRange())
		{
			basesInRangeOfMobileHQ = campaign.GetBasesInRangeOfMobileHQ(mobileHQ);
			
			foreach (SCR_CampaignBase baseInRangeOfMobileHQ: basesInRangeOfMobileHQ)
			{
				if (this == baseInRangeOfMobileHQ)
					mobilehqMapItem = desc.Item();	
			}
		}
		
		if (!showAllLinks)
		{	
			RemoveObsoleteLinks();	
			if (!mobilehqMapItem)
			{
				if (!IsBaseInFactionRadioSignal(localPlayerFaction) || localPlayerFaction != myFaction)
					return;
			}
		}	
		
		UnregisterMyMapLinks();
		ClearLinks();
		CreateLinks(myFaction, myMapItem, localPlayerFaction, mobilehqMapItem);
			
		/*	
		//TODO: Make lines alpha in and out
		array<MapLink> links = {};
		int count = myMapItem.GetLinks(links);
		for (int j = 0; j < count; j++)
		{
			MapDescriptorProps props = links[j].GetProps();
			if (!props)
				return;
			
			Color c = props.GetOutlineColor();
			//c.SetA(m_GraphLinesData.GetDefaultAlpha() / 255);
			c.SetR(1);
			c.SetB(1);
			c.SetG(0);
			props.SetOutlineColor(c);
			links[j].SetProps(props);
		}
		*/
	}

	//------------------------------------------------------------------------------------------------
	void GetMapLinks(out array<MapLink> links)
	{
		links = m_aMapLinks;
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateLinks(Faction myFaction, MapItem myMapItem, Faction localPlayerFaction, MapItem mobilehq = null)
	{
		if(mobilehq)
		{
			MapLink link = myMapItem.LinkTo(mobilehq);
			ColorMapLink(link);
			RegisterMapLink(link);
			SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
			SCR_CampaignMobileAssemblyComponent comp;
			comp = SCR_CampaignMobileAssemblyComponent.Cast(Replication.FindItem(campaign.GetDeployedMobileAssemblyID(localPlayerFaction.GetFactionKey())));
			if(comp)
				comp.AddMapLink(link);
		}
		
		for (int i = m_aInMyRange.Count() - 1; i >= 0; i--)
		{
			if (m_aInMyRange[i].GetIsHQ() && m_aInMyRange[i].GetOwningFaction() != localPlayerFaction)
				continue;
			
			if (FindMapLink(this, m_aInMyRange[i]))
				continue;
			
			if (myFaction != localPlayerFaction)
				continue;
		
			MapItem otherMapItem = m_aInMyRange[i].GetMapItem();
			
			if (!otherMapItem)
				continue;
			
			MapLink link = myMapItem.LinkTo(otherMapItem);
			ColorMapLink(link);
			RegisterMapLink(link);
			m_aInMyRange[i].RegisterMapLink(link);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Shows info about the base in the map
	void HandleMapInfo(Faction playerFaction = null)
	{
		if (!m_MapItem)
			return;

		SCR_GameModeCampaignMP campaignGameMode = SCR_GameModeCampaignMP.GetInstance();
		
		if (!playerFaction)
		{
			if (campaignGameMode)
				playerFaction = campaignGameMode.GetLastPlayerFaction();
		}
		
		SCR_CampaignFaction playerFactionCampaign = SCR_CampaignFaction.Cast(playerFaction);
		
		if (!playerFactionCampaign)
			return;
		
		if (m_bIsHQ && GetOwningFaction() != playerFactionCampaign)
			return;
		
		// Set callsign based on player's faction
		if (GetType() != CampaignBaseType.RELAY && m_sCallsign.IsEmpty())
			SetCallsign(playerFactionCampaign);
		
		s_OnMapItemInfoChanged.Invoke(playerFaction);
		
		// Update base icon color
		EFactionMapID factionMapID = EFactionMapID.UNKNOWN;
		bool isInRange = IsBaseInFactionRadioSignal(playerFactionCampaign);
		
		// Show proper faction color only for HQs or bases within radio signal
		if (!GetOwningFaction() && isInRange)
			factionMapID = EFactionMapID.FIA;
		else
			if (GetOwningFaction() && (GetType() == CampaignBaseType.MAIN || isInRange))
				switch (GetOwningFaction().GetFactionKey())
				{
					case SCR_GameModeCampaignMP.FACTION_OPFOR: {factionMapID = EFactionMapID.EAST; break;};
					case SCR_GameModeCampaignMP.FACTION_BLUFOR: {factionMapID = EFactionMapID.WEST; break;};
				}
		
		m_MapItem.SetFactionIndex(factionMapID);
		
		array<SCR_CampaignServiceComponent> services = {};
		GetAllBaseServices(services);
		
		foreach (SCR_CampaignServiceComponent service: services)
		{
			IEntity owner = service.GetOwner();
			
			if (!owner)
				continue;
			
			SCR_CampaignServiceMapDescriptorComponent comp = SCR_CampaignServiceMapDescriptorComponent.Cast(owner.FindComponent(SCR_CampaignServiceMapDescriptorComponent));
			
			if (comp)
			{
				if (isInRange)
					comp.SetServiceMarker(GetOwningFaction());
				else
					comp.SetServiceMarker(visible: false);
			}
		}
		
		if (GetBaseUI())
			GetBaseUI().UpdateBaseIcon(factionMapID);
	}
	
	//------------------------------------------------------------------------------------------------
	void HandleMapInfo_NoParam()
	{
		HandleMapInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateIconText(Widget w, Faction playerFaction = null)
	{
		SCR_GameModeCampaignMP campaignGameMode = SCR_GameModeCampaignMP.GetInstance();
		
		if (!playerFaction)
		{
			if (campaignGameMode)
				playerFaction = campaignGameMode.GetLastPlayerFaction();
		}
		
		SCR_CampaignFaction playerFactionCampaign = SCR_CampaignFaction.Cast(playerFaction);
		
		if (!playerFactionCampaign)
			return;
		
		float respawnCooldown = Math.Ceil((m_fRespawnAvailableSince - Replication.Time()) / 1000);
		string shownRespawnCooldown;
		int d;
		int h;
		int m;
		int s;
		string sStr;
		
		TextWidget Respawn = TextWidget.Cast(w.FindAnyWidget("Respawn"));
		RichTextWidget Freq = RichTextWidget.Cast(w.FindAnyWidget("Freq"));
		TextWidget Supplies = TextWidget.Cast(w.FindAnyWidget("Supplies"));
		ImageWidget RespawnImg = ImageWidget.Cast(w.FindAnyWidget("RespawnIMG"));
		ImageWidget FreqImg = ImageWidget.Cast(w.FindAnyWidget("FreqIMG"));
		ImageWidget SuppliesImg = ImageWidget.Cast(w.FindAnyWidget("SuppliesIMG"));
		ImageWidget bg = ImageWidget.Cast(w.FindAnyWidget("Bg"));
		
		// Cooldown in progress, update UI timer
		if (GetSupplies() >= GetBaseSpawnCost() && respawnCooldown > 0 && GetOwningFaction() && GetOwningFaction() == playerFaction)
		{
			shownRespawnCooldown = "#AR-Campaign_BaseCooldown_Suffix";
			SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(respawnCooldown, d, h, m, s);
			sStr = string.ToString(s);
			
			if (s < 10)
				sStr = "0" + sStr;
		}

		
		// Compose proper map info based on the base type and ownership
		LocalizedString suppliesString;
		LocalizedString respawnString;
		string suppliesInfo;
		
		if (GetType() != CampaignBaseType.RELAY)
		{
			Supplies.SetVisible(true);
			SuppliesImg.SetVisible(true);
			if (GetOwningFaction() && GetOwningFaction() == playerFaction)
			{
				suppliesString = "#AR-Campaign_BaseSuppliesAmount";
				suppliesInfo = GetSupplies().ToString();
				
				if (!GetIsHQ())
				{
					Respawn.SetVisible(true);
					RespawnImg.SetVisible(true);
					
					int respawns = GetSupplies() / GetBaseSpawnCost();
					respawnString = respawns.ToString() + " " + shownRespawnCooldown;
				}
			}
			else
			{
				suppliesString = "#AR-Campaign_BaseSuppliesUnknown";
				
				if (GetType() == CampaignBaseType.SMALL)
					respawnString = "#AR-Campaign_BaseRespawnsUnknown";
			}
		}
		
		RespawnImg.LoadImageFromSet(0, m_sBuildingIconImageset, "RespawnBig");
		FreqImg.LoadImageFromSet(0, m_sBuildingIconImageset, "FrequencyBig");
		SuppliesImg.LoadImageFromSet(0, m_sBuildingIconImageset, "SuppliesBig");
		
		Freq.SetVisible(true);
		FreqImg.SetVisible(true);
		bg.SetVisible(true);
		
		Freq.SetTextFormat("#AR-Campaign_BaseRadioFrequency", GetRadioFrequency(playerFaction.GetFactionKey()));
		Supplies.SetTextFormat(suppliesString, suppliesInfo, GetSuppliesMax());
		//TO-DO - Remove empty chars for old parameters and edit string table approprietly
		Respawn.SetTextFormat(respawnString," "," ", m, sStr);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCallsignIndex(int index)
	{
		m_iCallsign = index;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetAntennaBuilt()
	{
		return m_bAntennaBuilt;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDefendersGroup(SCR_AIGroup grp)
	{
		m_DefendersGroup = grp;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetDefendersGroup()
	{
		return m_DefendersGroup;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetCallsign(notnull SCR_CampaignFaction faction)
	{
		if (m_iCallsign < 0 || m_iCallsign >= SCR_GameModeCampaignMP.BASE_CALLSIGNS_COUNT)
			return;
		
		array<LocalizedString> baseCallsignsArray = {};
		array<LocalizedString> baseCallsignsArrayNameOnly = {};
		array<LocalizedString> baseCallsignsArrayNameOnlyUC = {};
		
		switch (faction.GetFactionKey())
		{
			case SCR_GameModeCampaignMP.FACTION_BLUFOR:
			{
				baseCallsignsArray = {
					"#AR-Campaign_PointAlabama",
					"#AR-Campaign_PointAlaska",
					"#AR-Campaign_PointArizona",
					"#AR-Campaign_PointArkansas",
					"#AR-Campaign_PointCalifornia",
					"#AR-Campaign_PointColorado",
					"#AR-Campaign_PointConnecticut",
					"#AR-Campaign_PointDelaware",
					"#AR-Campaign_PointFlorida",
					"#AR-Campaign_PointGeorgia",
					"#AR-Campaign_PointHawaii",
					"#AR-Campaign_PointIdaho",
					"#AR-Campaign_PointIllinois",
					"#AR-Campaign_PointIndiana",
					"#AR-Campaign_PointIowa",
					"#AR-Campaign_PointKansas",
					"#AR-Campaign_PointKentucky",
					"#AR-Campaign_PointLouisiana",
					"#AR-Campaign_PointMaine",
					"#AR-Campaign_PointMaryland",
					"#AR-Campaign_PointMassachusetts",
					"#AR-Campaign_PointMichigan",
					"#AR-Campaign_PointMinnesota",
					"#AR-Campaign_PointMississippi",
					"#AR-Campaign_PointMissouri",
					"#AR-Campaign_PointMontana",
					"#AR-Campaign_PointNebraska",
					"#AR-Campaign_PointNevada",
					"#AR-Campaign_PointNewYork",
					"#AR-Campaign_PointOhio",
					"#AR-Campaign_PointOklahoma",
					"#AR-Campaign_PointOregon",
					"#AR-Campaign_PointPennsylvania",
					"#AR-Campaign_PointTexas",
					"#AR-Campaign_PointUtah",
					"#AR-Campaign_PointVermont",
					"#AR-Campaign_PointVirginia",
					"#AR-Campaign_PointWashington",
					"#AR-Campaign_PointWisconsin",
					"#AR-Campaign_PointWyoming"
				};
				
				baseCallsignsArrayNameOnly = {
					"#AR-Comm_Variable_Base_Alabama_US",
					"#AR-Comm_Variable_Base_Alaska_US",
					"#AR-Comm_Variable_Base_Arizona_US",
					"#AR-Comm_Variable_Base_Arkansas_US",
					"#AR-Comm_Variable_Base_California_US",
					"#AR-Comm_Variable_Base_Colorado_US",
					"#AR-Comm_Variable_Base_Connecticut_US",
					"#AR-Comm_Variable_Base_Delaware_US",
					"#AR-Comm_Variable_Base_Florida_US",
					"#AR-Comm_Variable_Base_Georgia_US",
					"#AR-Comm_Variable_Base_Hawaii_US",
					"#AR-Comm_Variable_Base_Idaho_US",
					"#AR-Comm_Variable_Base_Illinois_US",
					"#AR-Comm_Variable_Base_Indiana_US",
					"#AR-Comm_Variable_Base_Iowa_US",
					"#AR-Comm_Variable_Base_Kansas_US",
					"#AR-Comm_Variable_Base_Kentucky_US",
					"#AR-Comm_Variable_Base_Louisiana_US",
					"#AR-Comm_Variable_Base_Maine_US",
					"#AR-Comm_Variable_Base_Maryland_US",
					"#AR-Comm_Variable_Base_Massachusetts_US",
					"#AR-Comm_Variable_Base_Michigan_US",
					"#AR-Comm_Variable_Base_Minnesota_US",
					"#AR-Comm_Variable_Base_Mississippi_US",
					"#AR-Comm_Variable_Base_Missouri_US",
					"#AR-Comm_Variable_Base_Montana_US",
					"#AR-Comm_Variable_Base_Nebraska_US",
					"#AR-Comm_Variable_Base_Nevada_US",
					"#AR-Comm_Variable_Base_NewYork_US",
					"#AR-Comm_Variable_Base_Ohio_US",
					"#AR-Comm_Variable_Base_Oklahoma_US",
					"#AR-Comm_Variable_Base_Oregon_US",
					"#AR-Comm_Variable_Base_Pennsylvania_US",
					"#AR-Comm_Variable_Base_Texas_US",
					"#AR-Comm_Variable_Base_Utah_US",
					"#AR-Comm_Variable_Base_Vermont_US",
					"#AR-Comm_Variable_Base_Virginia_US",
					"#AR-Comm_Variable_Base_Washington_US",
					"#AR-Comm_Variable_Base_Wisconsin_US",
					"#AR-Comm_Variable_Base_Wyoming_US"
				};
				
				baseCallsignsArrayNameOnlyUC = {
					"#AR-Comm_Variable_Base_Alabama_US_UC",
					"#AR-Comm_Variable_Base_Alaska_US_UC",
					"#AR-Comm_Variable_Base_Arizona_US_UC",
					"#AR-Comm_Variable_Base_Arkansas_US_UC",
					"#AR-Comm_Variable_Base_California_US_UC",
					"#AR-Comm_Variable_Base_Colorado_US_UC",
					"#AR-Comm_Variable_Base_Connecticut_US_UC",
					"#AR-Comm_Variable_Base_Delaware_US_UC",
					"#AR-Comm_Variable_Base_Florida_US_UC",
					"#AR-Comm_Variable_Base_Georgia_US_UC",
					"#AR-Comm_Variable_Base_Hawaii_US_UC",
					"#AR-Comm_Variable_Base_Idaho_US_UC",
					"#AR-Comm_Variable_Base_Illinois_US_UC",
					"#AR-Comm_Variable_Base_Indiana_US_UC",
					"#AR-Comm_Variable_Base_Iowa_US_UC",
					"#AR-Comm_Variable_Base_Kansas_US_UC",
					"#AR-Comm_Variable_Base_Kentucky_US_UC",
					"#AR-Comm_Variable_Base_Louisiana_US_UC",
					"#AR-Comm_Variable_Base_Maine_US_UC",
					"#AR-Comm_Variable_Base_Maryland_US_UC",
					"#AR-Comm_Variable_Base_Massachusetts_US_UC",
					"#AR-Comm_Variable_Base_Michigan_US_UC",
					"#AR-Comm_Variable_Base_Minnesota_US_UC",
					"#AR-Comm_Variable_Base_Mississippi_US_UC",
					"#AR-Comm_Variable_Base_Missouri_US_UC",
					"#AR-Comm_Variable_Base_Montana_US_UC",
					"#AR-Comm_Variable_Base_Nebraska_US_UC",
					"#AR-Comm_Variable_Base_Nevada_US_UC",
					"#AR-Comm_Variable_Base_NewYork_US_UC",
					"#AR-Comm_Variable_Base_Ohio_US_UC",
					"#AR-Comm_Variable_Base_Oklahoma_US_UC",
					"#AR-Comm_Variable_Base_Oregon_US_UC",
					"#AR-Comm_Variable_Base_Pennsylvania_US_UC",
					"#AR-Comm_Variable_Base_Texas_US_UC",
					"#AR-Comm_Variable_Base_Utah_US_UC",
					"#AR-Comm_Variable_Base_Vermont_US_UC",
					"#AR-Comm_Variable_Base_Virginia_US_UC",
					"#AR-Comm_Variable_Base_Washington_US_UC",
					"#AR-Comm_Variable_Base_Wisconsin_US_UC",
					"#AR-Comm_Variable_Base_Wyoming_US_UC"
				};

				break;
			}
			
			case SCR_GameModeCampaignMP.FACTION_OPFOR:
			{
				baseCallsignsArray = {
					"#AR-Campaign_PointArena",
					"#AR-Campaign_PointAvrora",
					"#AR-Campaign_PointBekas",
					"#AR-Campaign_PointBizon",
					"#AR-Campaign_PointBolero",
					"#AR-Campaign_PointCafftan",
					"#AR-Campaign_PointKaban",
					"#AR-Campaign_PointConsul",
					"#AR-Campaign_PointDimok",
					"#AR-Campaign_PointEffir",
					"#AR-Campaign_PointFakir",
					"#AR-Campaign_PointGips",
					"#AR-Campaign_PointGussar",
					"#AR-Campaign_PointIgla",
					"#AR-Campaign_PointKhek",
					"#AR-Campaign_PointLednik",
					"#AR-Campaign_PointMatros",
					"#AR-Campaign_PointMuskat",
					"#AR-Campaign_PointNagan",
					"#AR-Campaign_PointNitka",
					"#AR-Campaign_PointOmega",
					"#AR-Campaign_PointPlazma",
					"#AR-Campaign_PointPolyn",
					"#AR-Campaign_PointProlog",
					"#AR-Campaign_PointRabkor",
					"#AR-Campaign_PointRadon",
					"#AR-Campaign_PointSadko",
					"#AR-Campaign_PointSekret",
					"#AR-Campaign_PointShtanga",
					"#AR-Campaign_PointShuka",
					"#AR-Campaign_PointTabor",
					"#AR-Campaign_PointTenor",
					"#AR-Campaign_PointTsikson",
					"#AR-Campaign_PointUlan",
					"#AR-Campaign_PointVermout",
					"#AR-Campaign_PointViympel",
					"#AR-Campaign_PointYantar",
					"#AR-Campaign_PointYolka",
					"#AR-Campaign_PointZheton",
					"#AR-Campaign_PointZurna"
				};

				baseCallsignsArrayNameOnly = {
					"#AR-Comm_Variable_Base_Arena_RU",
					"#AR-Comm_Variable_Base_Avrora_RU",
					"#AR-Comm_Variable_Base_Bekas_RU",
					"#AR-Comm_Variable_Base_Bizon_RU",
					"#AR-Comm_Variable_Base_Bolero_RU",
					"#AR-Comm_Variable_Base_Cafftan_RU",
					"#AR-Comm_Variable_Base_Kaban_RU",
					"#AR-Comm_Variable_Base_Consul_RU",
					"#AR-Comm_Variable_Base_Dimok_RU",
					"#AR-Comm_Variable_Base_Effir_RU",
					"#AR-Comm_Variable_Base_Fakir_RU",
					"#AR-Comm_Variable_Base_Gips_RU",
					"#AR-Comm_Variable_Base_Gussar_RU",
					"#AR-Comm_Variable_Base_Igla_RU",
					"#AR-Comm_Variable_Base_Khek_RU",
					"#AR-Comm_Variable_Base_Lednik_RU",
					"#AR-Comm_Variable_Base_Matros_RU",
					"#AR-Comm_Variable_Base_Muskat_RU",
					"#AR-Comm_Variable_Base_Nagan_RU",
					"#AR-Comm_Variable_Base_Nitka_RU",
					"#AR-Comm_Variable_Base_Omega_RU",
					"#AR-Comm_Variable_Base_Plazma_RU",
					"#AR-Comm_Variable_Base_Polyn_RU",
					"#AR-Comm_Variable_Base_Prolog_RU",
					"#AR-Comm_Variable_Base_Rabkor_RU",
					"#AR-Comm_Variable_Base_Radon_RU",
					"#AR-Comm_Variable_Base_Sadko_RU",
					"#AR-Comm_Variable_Base_Sekret_RU",
					"#AR-Comm_Variable_Base_Shtanga_RU",
					"#AR-Comm_Variable_Base_Shuka_RU",
					"#AR-Comm_Variable_Base_Tabor_RU",
					"#AR-Comm_Variable_Base_Tenor_RU",
					"#AR-Comm_Variable_Base_Tsikson_RU",
					"#AR-Comm_Variable_Base_Ulan_RU",
					"#AR-Comm_Variable_Base_Vermout_RU",
					"#AR-Comm_Variable_Base_Viympel_RU",
					"#AR-Comm_Variable_Base_Yantar_RU",
					"#AR-Comm_Variable_Base_Yolka_RU",
					"#AR-Comm_Variable_Base_Zheton_RU",
					"#AR-Comm_Variable_Base_Zurna_RU"
				};
				
				baseCallsignsArrayNameOnlyUC = {
					"#AR-Comm_Variable_Base_Arena_RU_UC",
					"#AR-Comm_Variable_Base_Avrora_RU_UC",
					"#AR-Comm_Variable_Base_Bekas_RU_UC",
					"#AR-Comm_Variable_Base_Bizon_RU_UC",
					"#AR-Comm_Variable_Base_Bolero_RU_UC",
					"#AR-Comm_Variable_Base_Cafftan_RU_UC",
					"#AR-Comm_Variable_Base_Kaban_RU_UC",
					"#AR-Comm_Variable_Base_Consul_RU_UC",
					"#AR-Comm_Variable_Base_Dimok_RU_UC",
					"#AR-Comm_Variable_Base_Effir_RU_UC",
					"#AR-Comm_Variable_Base_Fakir_RU_UC",
					"#AR-Comm_Variable_Base_Gips_RU_UC",
					"#AR-Comm_Variable_Base_Gussar_RU_UC",
					"#AR-Comm_Variable_Base_Igla_RU_UC",
					"#AR-Comm_Variable_Base_Khek_RU_UC",
					"#AR-Comm_Variable_Base_Lednik_RU_UC",
					"#AR-Comm_Variable_Base_Matros_RU_UC",
					"#AR-Comm_Variable_Base_Muskat_RU_UC",
					"#AR-Comm_Variable_Base_Nagan_RU_UC",
					"#AR-Comm_Variable_Base_Nitka_RU_UC",
					"#AR-Comm_Variable_Base_Omega_RU_UC",
					"#AR-Comm_Variable_Base_Plazma_RU_UC",
					"#AR-Comm_Variable_Base_Polyn_RU_UC",
					"#AR-Comm_Variable_Base_Prolog_RU_UC",
					"#AR-Comm_Variable_Base_Rabkor_RU_UC",
					"#AR-Comm_Variable_Base_Radon_RU_UC",
					"#AR-Comm_Variable_Base_Sadko_RU_UC",
					"#AR-Comm_Variable_Base_Sekret_RU_UC",
					"#AR-Comm_Variable_Base_Shtanga_RU_UC",
					"#AR-Comm_Variable_Base_Shuka_RU_UC",
					"#AR-Comm_Variable_Base_Tabor_RU_UC",
					"#AR-Comm_Variable_Base_Tenor_RU_UC",
					"#AR-Comm_Variable_Base_Tsikson_RU_UC",
					"#AR-Comm_Variable_Base_Ulan_RU_UC",
					"#AR-Comm_Variable_Base_Vermout_RU_UC",
					"#AR-Comm_Variable_Base_Viympel_RU_UC",
					"#AR-Comm_Variable_Base_Yantar_RU_UC",
					"#AR-Comm_Variable_Base_Yolka_RU_UC",
					"#AR-Comm_Variable_Base_Zheton_RU_UC",
					"#AR-Comm_Variable_Base_Zurna_RU_UC"
				};
				
				break;
			}
		}
		
		if (!baseCallsignsArray.IsEmpty())
		{
			m_sCallsign = baseCallsignsArray[m_iCallsign];
			m_sCallsignNameOnly = baseCallsignsArrayNameOnly[m_iCallsign];
			m_sCallsignNameOnlyUC = baseCallsignsArrayNameOnlyUC[m_iCallsign];
		}
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCallsign()
	{
		return m_iCallsign;
	}

	//------------------------------------------------------------------------------------------------
	// return callsign name only (eg. "Matros" instead of "Point Matros")
	LocalizedString GetCallsignDisplayNameOnly()
	{
		return m_sCallsignNameOnly;
	}
	
	//------------------------------------------------------------------------------------------------
	LocalizedString GetCallsignDisplayName()
	{
		return m_sCallsign;
	}
	
	//------------------------------------------------------------------------------------------------
	LocalizedString GetCallsignDisplayNameOnlyUC()
	{
		return m_sCallsignNameOnlyUC;
	}
	
	//------------------------------------------------------------------------------------------------
	LocalizedString GetCallsignDisplayNameUpperCase()
	{
		return m_sCallsignUpper;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetAllChildComponentsOfType(GenericComponent parentComponent, typename type, out array<GenericComponent> outChildren)
	{
		if (!outChildren || !parentComponent)
			return;
		
		array<GenericComponent> children = new array<GenericComponent>();
		
		parentComponent.FindComponents(type, children);
		
		foreach (GenericComponent component: children)
		{
			outChildren.Insert(component);
			GetAllChildComponentsOfType(component, type, outChildren);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetPresentSCR_ChimeraCharacters(out array<ChimeraCharacter> characterList)
	{
		array<SCR_ChimeraCharacter> chimeraCharacters = SCR_CharacterRegistrationComponent.GetChimeraCharacters();
		if (!m_Trigger || !chimeraCharacters)
			return;
		
		for (int i = 0, count = chimeraCharacters.Count(); i < count; i++)
		{
			if (m_Trigger.QueryEntityInside(chimeraCharacters[i]))
			{
				characterList.Insert(chimeraCharacters[i]);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnServiceBuilt(notnull SCR_CampaignServiceComponent service, bool add)
	{
		ECampaignServicePointType serviceType = service.GetType();
		
		if (add)
			m_aServices.Insert(serviceType, service);
		else
			m_aServices.Remove(serviceType);
		
		IEntity structure = service.GetOwner();
		
		s_OnServiceRegistered.Invoke(this);
		
		if (structure)
		{
			SCR_CampaignServiceMapDescriptorComponent comp = SCR_CampaignServiceMapDescriptorComponent.Cast(structure.FindComponent(SCR_CampaignServiceMapDescriptorComponent));
		
			if (comp && add)
				comp.SetParentBase(this);
			else
				comp.SetParentBase(null);
		}
		
		//Has to be done also on clients
		if (serviceType == ECampaignServicePointType.LIGHT_VEHICLE_DEPOT || serviceType == ECampaignServicePointType.HEAVY_VEHICLE_DEPOT)
		{
			SCR_CampaignEntitySpawnerComponent spawnerComponent = SCR_CampaignEntitySpawnerComponent.Cast(service.GetOwner().FindComponent(SCR_CampaignEntitySpawnerComponent)); //AssignBase
			if (!spawnerComponent)
				return;
				
			if (add)
				spawnerComponent.AssignBase(this);
			else
				m_aVehicleSpawners.RemoveItem(spawnerComponent);
				
			RefreshVehicleSpawners();
		}
		
		if (IsProxy())
			return;
		
		SCR_CampaignFaction owner = GetOwningFaction();
		
		switch (serviceType)
		{
			case ECampaignServicePointType.SUPPLY_DEPOT:
			{
				if (add)
					SetSuppliesMax(GetSuppliesMax() + SCR_GameModeCampaignMP.SUPPLY_DEPOT_CAPACITY);
				else
					SetSuppliesMax(GetSuppliesMax() - SCR_GameModeCampaignMP.SUPPLY_DEPOT_CAPACITY);
				break;
				
			}
			
			case ECampaignServicePointType.ARMORY:
			{
				if (add)
					m_ArmoryComponent = SCR_ArmoryComponent.Cast(service.GetOwner().FindComponent(SCR_ArmoryComponent));
				else
					m_ArmoryComponent = null;
				break;
			}
			
			case ECampaignServicePointType.RADIO_ANTENNA:
			{
				if (add)
				{
					m_bAntennaBuilt = true;
					OnAntennaBuilt();
				}
				else
				{
					m_bAntennaBuilt = false;
					OnAntennaBuilt();
				}
				break;
			}
			
			case ECampaignServicePointType.BARRACKS:
			{
				SCR_CampaignBarracksComponent barrackComp = SCR_CampaignBarracksComponent.Cast(service.GetOwner().FindComponent(SCR_CampaignBarracksComponent));
					if (!barrackComp)
						return;
				
				if (add)
				{
					AssignBarracks(barrackComp);
					barrackComp.SetBase(this);
				}
			}
		}
		
		if (owner && IsBaseInFactionRadioSignal(owner))
		{
			switch (serviceType)
			{
				case ECampaignServicePointType.SUPPLY_DEPOT:
				{
					if (add)
						owner.SendHQMessage(SCR_ERadioMsg.BUILT_SUPPLY, m_iCallsign, param: m_iBaseID);
					break;
				}
				
				/*case ECampaignServicePointType.FUEL_DEPOT:
				{
					if (add)
						owner.SendHQMessage(SCR_ERadioMsg.BUILT_FUEL, m_iCallsign, param: m_iBaseID);
					break;
				}*/
				
				case ECampaignServicePointType.ARMORY:
				{
					if (add)
						owner.SendHQMessage(SCR_ERadioMsg.BUILT_ARMORY, m_iCallsign, param: m_iBaseID);
					break;
				}
				
				case ECampaignServicePointType.LIGHT_VEHICLE_DEPOT:
				{
					if (add)
						owner.SendHQMessage(SCR_ERadioMsg.BUILT_VEHICLES_LIGHT, m_iCallsign, param: m_iBaseID);
					break;
				}
				
				case ECampaignServicePointType.HEAVY_VEHICLE_DEPOT:
				{
					if (add)
						owner.SendHQMessage(SCR_ERadioMsg.BUILT_VEHICLES_HEAVY, m_iCallsign, param: m_iBaseID);
					break;
				}
				
				case ECampaignServicePointType.RADIO_ANTENNA:
				{
					if (add)
						owner.SendHQMessage(SCR_ERadioMsg.BUILT_ANTENNA, m_iCallsign, param: m_iBaseID);
					break;
				}
				
				case ECampaignServicePointType.BARRACKS:
				{
					if (add)
						owner.SendHQMessage(SCR_ERadioMsg.BUILT_BARRACKS, m_iCallsign, param: m_iBaseID);
					break;
				}
				
				case ECampaignServicePointType.FIELD_HOSPITAL:
				{
					if (add)
						owner.SendHQMessage(SCR_ERadioMsg.BUILT_FIELD_HOSPITAL, m_iCallsign, param: m_iBaseID);
					break;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles the timer for reinforcements arrival at this base 
	//! \param reset Resets the current timer
	protected void ReinforcementsTimer(bool reset = false)
	{	
		BaseWorld world = GetWorld();
		
		if (!GetOwningFaction() || !world)
			return;
		
		float curTime = world.GetWorldTime();
		
		if (curTime >= m_fReinforcementsArrivalTime || reset)
		{
			if (!reset)
				SendReinforcements();
			
			m_fReinforcementsArrivalTime = curTime + (REINFORCEMENTS_PERIOD * 1000);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Reinforcements timer has finished, send in reinforcements 
	protected void SendReinforcements()
	{
		m_OnReinforcementsArrived.Invoke(this, m_SuppliesFaction, GetOwningFaction());
		m_SuppliesFaction = GetOwningFaction();
		AddSupplies(Math.Min(SCR_GameModeCampaignMP.REINFORCEMENTS_SUPPLIES, GetSuppliesMax() - GetSupplies()));
		//GetOwningFaction().SendHQMessage(SCR_ERadioMsg.REINFORCEMENTS, m_iCallsign, param: GetBaseID())
	};
	
	//------------------------------------------------------------------------------------------------
	void SpawnComposition(ECampaignCompositionType slotType, bool noSlotOnly = false)
	{
		SCR_SiteSlotEntity slot = GetAssignedSlot(slotType);
		SCR_CampaignFaction buildingsFaction = GetBuildingsFaction();
		
		if (!buildingsFaction)
			return;
		
		// Spawn no-slot HQ composition if required
		if (slotType == ECampaignCompositionType.HQ && !slot)
		{
			ResourceName compType = buildingsFaction.GetBuildingPrefabNoSlot(slotType, GetType());
			Resource compositionRes = Resource.Load(compType);
			
			if (!compositionRes.IsValid())
				return;

			EntitySpawnParams spawnParams = EntitySpawnParams();	
			spawnParams.TransformMode = ETransformMode.WORLD;		
			GetWorldTransform(spawnParams.Transform);
			GetGame().SpawnEntityPrefab(compositionRes, GetGame().GetWorld(), spawnParams);
			
			return;
		}
		
		// Build everything on bases without access to a building interface
		if (noSlotOnly && slot && !GetAssignedSlot(ECampaignCompositionType.HQ))
			noSlotOnly = false;
		
		// If the slot is defined for a composition, use the slot
		if (!slot || noSlotOnly)
			return;
		
		if (slot.GetOccupant())
			return;
		
		ResourceName compType = buildingsFaction.GetBuildingPrefab(slotType, GetType());
		Resource composition = Resource.Load(compType);
		
		if (!composition)
			return;
		
		IEntity structure = slot.SpawnEntityInSlot(composition);
		
		if (!structure)
			return;
		
		SCR_CampaignServiceComponent service = SCR_CampaignServiceComponent.Cast(structure.FindComponent(SCR_CampaignServiceComponent));
		
		if (!service)
			return;
		
		RegisterAsParentBase(service, true);
	};
	
	//*********************//
	//PUBLIC MEMBER METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	void ShowMapLinks(bool show)
	{
		m_bShowMapLinks = show;
		
		if (!m_MapItem)
			return;
		
		if (!m_bShowMapLinks)
		{
			UnregisterMyMapLinks();
			ClearLinks();
			m_MapItem.SetRange(GetSignalRange()); //HACK TODO: PROPERLY HIDE IT, ONCE LINKS ARE INDEPENDANT
		}
		else
		{
			HandleMapLinks();
			m_MapItem.SetRange(0); //HACK TODO: PROPERLY HIDE IT, ONCE LINKS ARE INDEPENDANT
		}
	}
	
	//------------------------------------------------------------------------------------------------
	MapItem GetMapItem()
	{
		return m_MapItem;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsEntityInMyRange(notnull IEntity entity)
	{
		return vector.Distance(entity.GetOrigin(), GetOrigin()) <= GetSignalRange();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSupplies()
	{
		if (!m_SuppliesComponent)
			return 0;
		
		return m_SuppliesComponent.GetSupplies();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSuppliesMax()
	{
		if (!m_SuppliesComponent)
			return 0;
		
		return m_SuppliesComponent.GetSuppliesMax();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSuppliesMax(int maxSupplies)
	{
		if (!m_SuppliesComponent)
			return;
		
		m_SuppliesComponent.SetSuppliesMax(maxSupplies);
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnBuildings(SCR_CampaignBaseStruct loadedData = null)
	{
		// These compositions need to be built everywhere
		if (GetType() != CampaignBaseType.RELAY)
			SpawnComposition(ECampaignCompositionType.HQ);
		
		if (loadedData)
		{
			array<ECampaignServicePointType> types = {};
			SCR_Enum.GetEnumValues(ECampaignServicePointType, types);
			
			ResourceName prefab;
			vector position;
			vector rotation;
			IEntity service;
			SCR_CampaignServiceComponent serviceComponent;
			EntitySpawnParams params;
			
			foreach (ECampaignServicePointType type : types)
			{
				if (GetBaseService(type))
					continue;
					
				prefab = loadedData.GetServicePrefab(type);
				
				if (!prefab)
					continue;
				
				position = loadedData.GetServicePosition(type);
				
				if (position == vector.Zero)
					continue;
				
				rotation = loadedData.GetServiceRotation(type);
				
				params = EntitySpawnParams();
				GetWorldTransform(params.Transform);
				params.TransformMode = ETransformMode.WORLD;
				Math3D.AnglesToMatrix(rotation, params.Transform);
				params.Transform[3] = position;
				
				service = GetGame().SpawnEntityPrefab(Resource.Load(prefab), null, params);
				
				if (!service)
					continue;
				
				serviceComponent = SCR_CampaignServiceComponent.Cast(service.FindComponent(SCR_CampaignServiceComponent));
		
				if (!serviceComponent)
					continue;
				
				RegisterAsParentBase(serviceComponent, true);
			}
			
			return;
		}
		
		if (GetType() != CampaignBaseType.RELAY)
		{
			// Build services automatically when prebuilt attribute is checked or no-slot composition is used
			SpawnComposition(ECampaignCompositionType.FUEL, !m_bPrebuilt);
			SpawnComposition(ECampaignCompositionType.LIGHT_VEHICLE_DEPOT, !m_bPrebuilt);
			SpawnComposition(ECampaignCompositionType.HEAVY_VEHICLE_DEPOT, !m_bPrebuilt);
			SpawnComposition(ECampaignCompositionType.ARMORY, !m_bPrebuilt);
			SpawnComposition(ECampaignCompositionType.HOSPITAL, !m_bPrebuilt);
			SpawnComposition(ECampaignCompositionType.BARRACKS, !m_bPrebuilt);
			SpawnComposition(ECampaignCompositionType.SUPPLIES, !m_bPrebuilt);
			SpawnComposition(ECampaignCompositionType.RADIO_ANTENNA, !m_bPrebuilt);
		}
		
		if (m_bIsHQ && !SCR_GameModeCampaignMP.GetInstance().IsTutorial())
			SpawnStartingVehicles();
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterHQRadio(notnull IEntity radio)
	{
		m_HQRadio = radio;
	}
	
	void RegisterGarageBoard(notnull IEntity board)
	{
		m_GarageBoard = board;
	}
	
	//------------------------------------------------------------------------------------------------
	void ToggleRadioChatter(bool play)
	{
		if (!m_HQRadio)
			return;
		
		SoundComponent comp = SoundComponent.Cast(m_HQRadio.FindComponent(SoundComponent));
		
		if (!comp)
			return;
		
		if (m_PlayedRadio != AudioHandle.Invalid)
				comp.TerminateAll();
		
		if (!play)
			return;
		
		if (!GetOwningFaction())
			m_PlayedRadio = comp.SoundEvent(SCR_SoundEvent.SOUND_RADIO_CHATTER_EV);
		else
		{
			if (GetOwningFaction().GetFactionKey() == SCR_GameModeCampaignMP.FACTION_BLUFOR)
				m_PlayedRadio = comp.SoundEvent(SCR_SoundEvent.SOUND_RADIO_CHATTER_US);
			else
				m_PlayedRadio = comp.SoundEvent(SCR_SoundEvent.SOUND_RADIO_CHATTER_RU);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetHQRadio()
	{
		return m_HQRadio;
	}
	
	//------------------------------------------------------------------------------------------------
	void HideMapLocationLabel()
	{
		if (m_sMapLocationName.IsEmpty() || !GetWorld())
			return;
		
		GenericEntity ent = GenericEntity.Cast(GetWorld().FindEntityByName(m_sMapLocationName));
		
		if (!ent)
			return;
		
		MapDescriptorComponent comp = MapDescriptorComponent.Cast(ent.FindComponent(MapDescriptorComponent));
		
		if (!comp)
			return;
		
		MapItem item = comp.Item();
		
		if (!item)
			return;
		
		item.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	int GetRadioFrequency(FactionKey faction)
	{
		switch (faction)
		{
			case SCR_GameModeCampaignMP.FACTION_BLUFOR: {return m_iFreqWest;};
			case SCR_GameModeCampaignMP.FACTION_OPFOR: {return m_iFreqEast;};
		}
		
		return INVALID_FREQUENCY;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBuildingsFaction(SCR_CampaignFaction faction)
	{
		m_BuildingsFaction = faction;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetBuildingsFaction()
	{
		return m_BuildingsFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignServiceComponent GetBaseService(ECampaignServicePointType type)
	{
		return m_aServices.Get(type);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetAllBaseServices(out array<SCR_CampaignServiceComponent> baseServices = null)
	{
		int cnt;
		SCR_CampaignServiceComponent service;
		array<ECampaignServicePointType> types = {};
		
		SCR_Enum.GetEnumValues(ECampaignServicePointType, types);
		
		foreach (ECampaignServicePointType type : types)
		{
			service = GetBaseService(type);
			
			if (service)
			{
				cnt ++;
				
				if (baseServices)
					baseServices.Insert(service)
			}
		}
		
		return cnt;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add delivered supplies
	void AddSupplies(int suppliesCnt)
	{
		if (!m_SuppliesComponent)
			return;
		
		m_SuppliesComponent.AddSupplies(suppliesCnt);
	}
	
	//------------------------------------------------------------------------------------------------
	void MapSetup()
	{
		// Try getting the map icon component of this base
		SCR_MapDescriptorComponent mapDescriptorComponent = SCR_MapDescriptorComponent.Cast(FindComponent(SCR_MapDescriptorComponent));
		
		if (mapDescriptorComponent)
		{
			m_MapItem = mapDescriptorComponent.Item();
			m_MapItem.SetDisplayName(GetBaseName());
			
			MapDescriptorProps props = m_MapItem.GetProps();
			props.SetDetail(96);
			
			Color rangeColor;
			
			if (GetOwningFaction())
				rangeColor = GetOwningFaction().GetFactionColor();
			else
				rangeColor = Color(1, 1, 1, 1);
			
			props.SetOutlineColor(rangeColor);
			rangeColor.SetA(0.1);
			props.SetBackgroundColor(rangeColor);
			
			m_MapItem.SetProps(props);
			
			if (!m_bShowMapLinks)
				m_MapItem.SetRange(GetSignalRange());
			else
				m_MapItem.SetRange(0);
		}
		
		GetGame().GetCallqueue().CallLater(ShowRespawnCooldown, 250, true);
		GetGame().GetCallqueue().CallLater(HandleMapInfo_NoParam, 3000, true);
		SCR_MapEntity.GetOnMapOpen().Insert(HandleMapInfo_NoParam);
		SCR_MapCampaignUI.Event_OnIconsInit.Insert(HandleMapInfo);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsBaseInFactionRadioSignal(FactionKey factionKey, bool status, bool byMobileHQ = false)
	{
		if (IsProxy())
			return;
		
		bool update = false;
		
		switch (factionKey)
		{
			case SCR_GameModeCampaignMP.FACTION_BLUFOR:
			{
				if (status != m_bIsLinkedToMainWest)
					update = true;
				
				m_bIsLinkedToMainWest = status;
				break;
			};
			
			case SCR_GameModeCampaignMP.FACTION_OPFOR:
			{
				if (status != m_bIsLinkedToMainEast)
					update = true;
				
				m_bIsLinkedToMainEast = status;
				break;
			};
		}
		
		// Fail capture tasks if mobile HQ was dismantled
		if (!status && byMobileHQ)
		{
			SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();
			
			if (fManager)
			{
				SCR_CampaignFaction faction = fManager.GetCampaignFactionByKey(factionKey);
				
				if (faction)
				{
					if (GetTaskManager())
					{
						SCR_CampaignTaskSupportEntity supportEntity = SCR_CampaignTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignTaskSupportEntity));
						
						if (supportEntity)
						{
							SCR_CampaignTask task = supportEntity.GetTask(this, faction, SCR_CampaignTaskType.CAPTURE);
							
							if (task)
								supportEntity.FailTask(task);
						}
					}
				}
			}
		}
		
		Replication.BumpMe();
		
		// Server execution
		if (update)
			OnHasSignalChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsBaseInFactionRadioSignal(notnull SCR_CampaignFaction faction)
	{
		switch (faction.GetFactionKey())
		{
			case SCR_GameModeCampaignMP.FACTION_BLUFOR: {return m_bIsLinkedToMainWest; break;};
			case SCR_GameModeCampaignMP.FACTION_OPFOR: {return m_bIsLinkedToMainEast; break;};
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change reinforcements timer
	//! \param time This number is added to the timer
	void AlterReinforcementsTimer(float time)
	{
		m_fReinforcementsArrivalTime += time
	}
	
	//------------------------------------------------------------------------------------------------
	//! Outs all bases in range that suit the filter. Filter = allowed types.
	int GetBasesInRange(CampaignBaseType filter, notnull out array<SCR_CampaignBase> basesInRange)
	{
		int count = 0;
		if (!m_aInMyRange)
			return count;
		
		foreach (SCR_CampaignBase base: m_aInMyRange)
		{
			CampaignBaseType baseType = base.GetType();
			if ((filter & baseType) == baseType)
			{
				basesInRange.Insert(base);
				count++;
			}
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns whether the local player is present
	bool IsLocalPlayerPresent()
	{
		return m_bLocalPlayerPresent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the capturing faction
	Faction GetCapturingFaction()
	{
		return m_CapturingFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetOwningFaction(SCR_CampaignFaction faction)
	{
		if (m_FactionControl)
			m_FactionControl.SetAffiliatedFaction(faction);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Returns the owning faction
	SCR_CampaignFaction GetOwningFaction()
	{
		if (m_FactionControl)
			return SCR_CampaignFaction.Cast(m_FactionControl.GetAffiliatedFaction());
		else return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns this base's area trigger
	SCR_CampaignTriggerEntity GetTrigger()
	{
		return m_Trigger;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns remaining time before the base is captured
	float GetCaptureStartTimestamp()
	{
		return m_fCaptureStartTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLastEnemyContactTimestamp(float timestamp)
	{
		m_fLastEnemyContactTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetLastEnemyContactTimestamp()
	{
		return m_fLastEnemyContactTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns type of this base
	CampaignBaseType GetType()
	{
		return m_eType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the name of this base
	string GetBaseName()
	{
		return m_sBaseName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the upper-case name of this base
	string GetBaseNameUpperCase()
	{
		return m_sBaseNameUpper;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns whether this base is being captured
	bool IsBeingCaptured()
	{
		// The capturing faction exists, return true
		if (m_CapturingFaction)
			return true;
		
		// No faction is capturing this base, return false
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAttackingFaction(int enemyFaction)
	{
		m_iAttackingFaction = enemyFaction;
		
		if (m_iAttackingFaction < 0)
			return;
		
		Replication.BumpMe();
		OnAttackingFactionChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnAttackingFactionChanged()
	{
		Faction enemyFaction = GetGame().GetFactionManager().GetFactionByIndex(m_iAttackingFaction);
		
		if (!IsProxy())
			GetGame().GetCallqueue().CallLater(SetAttackingFaction, 1000, false, -1);
		else
			m_iAttackingFaction = -1;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (campaign && campaign.GetLastPlayerFaction() == GetOwningFaction())
			FlashBaseIcon(faction: enemyFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCallsignAssigned()
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();

		if (!campaign)
			return;
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(campaign.GetLastPlayerFaction());

		if (!faction)
			return;
		
		SetCallsign(faction);
	}
	
	//------------------------------------------------------------------------------------------------	
	void OnAntennaBuilt()
	{
		if (!IsProxy())
		{
			if (m_bAntennaBuilt)
				SetSignalRange(GetSignalRange() + m_iAntennaSignalBoost);
			else
				SetSignalRange(GetSignalRange() - m_iAntennaSignalBoost);
			
			LinkBases(true);
			
		}
		else
		{
			// Let signal range catch up on clients
			GetGame().GetCallqueue().CallLater(LinkBases, 2000, false, false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void FlashBaseIcon(float remainingTime = ICON_FLASH_DURATION, Faction faction = null, bool changeToDefault = false, bool infiniteTimer = false)
	{
		GetGame().GetCallqueue().Remove(FlashBaseIcon);
		
		SCR_CampaignMapUIBase baseUI = GetBaseUI();
		
		if (!faction)
		{
			if (!changeToDefault)
			{
				return;
			}
			else
			{
				if (baseUI)
					baseUI.SetBaseIconFactionColor(GetOwningFaction());
				
				return;
			}
		}
		
		Faction iconFaction;
		
		if (baseUI)
		{
			if (changeToDefault)
				iconFaction = GetOwningFaction();
			else
				iconFaction = faction;
			
			if (baseUI)
				baseUI.SetBaseIconFactionColor(iconFaction);
		}
		
		float remainingTimeReduced = remainingTime - ICON_FLASH_PERIOD;
		
		if (infiniteTimer || remainingTimeReduced > 0 || !changeToDefault)
			GetGame().GetCallqueue().CallLater(FlashBaseIcon, ICON_FLASH_PERIOD * 1000, false, remainingTimeReduced, faction, !changeToDefault, infiniteTimer);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find out if local player is inside this base
	void CheckIsPlayerInside()
	{
		if (!m_Trigger)
			return;
		
		ChimeraCharacter localPlayer = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		
		if (!localPlayer)
			return;
		
		DamageManagerComponent comp = DamageManagerComponent.Cast(localPlayer.FindComponent(DamageManagerComponent));
		
		if (!comp)
			return;
		
		if (comp.GetState() == EDamageState.DESTROYED)
			return;
		
		m_bLocalPlayerPresent = m_Trigger.QueryEntityInside(localPlayer);
		
		// Save this base as the one with player if he's inside 
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (campaign && m_bLocalPlayerPresent)
			campaign.AddBaseWithPlayer(this, localPlayer);
		else 
		{
			if (campaign.IsPlayerInBase(this))
				campaign.RemoveBaseWithPlayer(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Allows to register a trigger for this base
	void RegisterTrigger(SCR_CampaignTriggerEntity trigger)
	{
		if (m_Trigger)
			OnUnregisterTrigger(m_Trigger);
		
		m_Trigger = trigger;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSignalRange(float newRange)
	{
		if (!m_RadioComponent)
			return;
		
		if (newRange == 0)
			newRange = GetSignalRange();
		
		m_RadioComponent.SetRange(newRange);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when SCR_CampaignMapUIBase element is selected / hovered
	void OnIconSelected(SCR_CampaignMapUIBase icon)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnIconHovered(SCR_CampaignMapUIBase icon, bool hovering)
	{
		m_bIsHovered = hovering;
		for (int i = m_aMapLinks.Count() - 1; i >= 0; i--)
		{
			if (!m_aMapLinks[i])
				continue;
			
			ColorMapLink(m_aMapLinks[i]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called from SCR_MapCampaignUI when base UI elements are initialized
	void SetBaseUI(SCR_CampaignMapUIBase base)
	{
		m_UIElement = base;
	}
	
	SCR_CampaignMapUIBase GetBaseUI()
	{
		return m_UIElement;
	}

	//------------------------------------------------------------------------------------------------
	//! Getter for signal range
	float GetSignalRange()
	{
		if (!m_RadioComponent)
			m_RadioComponent = BaseRadioComponent.Cast(FindComponent(BaseRadioComponent));

		if (m_RadioComponent)
			return m_RadioComponent.GetRange();
		else
			return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for task waypoints
	AIWaypoint GetTaskWaypoint()
	{
		return m_CycleWaypoint;
	}

	//------------------------------------------------------------------------------------------------
	//! Assign Barrack component
	void AssignBarracks(SCR_CampaignBarracksComponent barracksComp)
	{
		m_BarrackComponent = barracksComp;
		
		if (!m_BarrackComponent)
			return;
		
		if (m_SuppliesComponent)
			m_BarrackComponent.SetSuppliesComponent(m_SuppliesComponent);
		
		if (!m_FactionControl)
			return;
		
		SCR_CampaignFaction owningFaction = GetOwningFaction();
		if (!owningFaction)
			return;
		
		ResourceName defenderGroup = owningFaction.GetDefendersGroupPrefab();
			
		if (defenderGroup)
		{
			m_BarrackComponent.SetGroupPrefab(defenderGroup);
			m_BarrackComponent.InitializeBarrack();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register Vehicle Spawner 
	void RegisterVehicleSpawner(notnull SCR_CampaignEntitySpawnerComponent comp)
	{
		if (!m_SuppliesComponent)
			return;
		
		comp.SetSpawnerSupplies(m_SuppliesComponent.GetSupplies());
		
		if (GetOwningFaction())
			comp.ChangeAssetList(GetOwningFaction().GetVehicleList());
		
		if (GetIsHQ())
			comp.EnableRadiusUsage(true);
		
		m_aVehicleSpawners.Insert(comp);
	}
	
	//------------------------------------------------------------------------------------------------
	void RefreshVehicleSpawners()
	{
		if (m_aVehicleSpawners.IsEmpty())
			return;
		
		if (!GetOwningFaction())
			return;
		
		SCR_EntityAssetList assetList = GetOwningFaction().GetVehicleList();
		
		foreach (SCR_CampaignEntitySpawnerComponent spawner : m_aVehicleSpawners)
		{
			if (spawner)
				spawner.ChangeAssetList(assetList);
		}
	}
	//------------------------------------------------------------------------------------------------
	//! In case this base is blacklisted in server config, disable all its functionality
	void DisableBase()
	{
		m_bEnabled = false;
		SCR_CampaignBaseManager.DisableBase(this);
		ClearEventMask(EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.DIAG);
		ClearFlags(EntityFlags.ACTIVE, true);
		
		if (!IsProxy())
		{
			ChangeOwner(null, true);
			GetGame().GetCallqueue().Remove(ReinforcementsTimer);
		}
		
		IEntity ent = this;
		MapDescriptorComponent mapDesc;
		
		// Hide all map descriptors
		while (ent)
		{
			mapDesc = MapDescriptorComponent.Cast(ent.FindComponent(MapDescriptorComponent));
			
			if (mapDesc && mapDesc.Item())
				mapDesc.Item().SetVisible(false);
			
			IEntity sibling = ent.GetSibling();
			
			if (sibling)
				ent = sibling;
			else
				ent = ent.GetChildren();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableBase()
	{
		m_bEnabled = true;
		SCR_CampaignBaseManager.EnableBase(this);
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.DIAG);
		SetFlags(EntityFlags.ACTIVE, true);
		
		IEntity ent = this;
		
		while (ent)
		{
			MapDescriptorComponent mapDesc = MapDescriptorComponent.Cast(ent.FindComponent(MapDescriptorComponent));
			
			if (mapDesc && mapDesc.Item())
				mapDesc.Item().SetVisible(true);
			
			IEntity sibling = ent.GetSibling();
			
			if (sibling)
				ent = sibling;
			else
				ent = ent.GetChildren();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void InitializeBase()
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		SCR_MapDescriptorComponent mapDescriptorComponent = SCR_MapDescriptorComponent.Cast(FindComponent(SCR_MapDescriptorComponent));
		
		if (mapDescriptorComponent)
		{
			m_MapItem = mapDescriptorComponent.Item();
		}
		
		// HQ overriden from header, update base type
		if (GetType() == CampaignBaseType.MAIN && !m_bIsHQ)
			m_eType = CampaignBaseType.MAJOR;
		
		if (GetType() == CampaignBaseType.MAJOR && m_bIsHQ)
			m_eType = CampaignBaseType.MAIN;
		
		// Register Slots
		BaseWorld world = GetWorld();
		string baseName = GetBaseName();
		
		switch (m_eStartingBaseOwner)
		{
			case SCR_ECampaignBaseOwner.INDFOR: {m_sStartingBaseOwner = FactionKey.Empty; break;};
			case SCR_ECampaignBaseOwner.BLUFOR: {m_sStartingBaseOwner = SCR_GameModeCampaignMP.FACTION_BLUFOR; break;};
			case SCR_ECampaignBaseOwner.OPFOR: {m_sStartingBaseOwner = SCR_GameModeCampaignMP.FACTION_OPFOR; break;};
		}
		
		if (world)
		{
			// HQ
			if (!m_sSlotNameHQ.IsEmpty())
			{
				m_SlotHQ = SCR_SiteSlotEntity.Cast(world.FindEntityByName(m_sSlotNameHQ));
				
				if (!m_SlotHQ)
					Print("Unable to register Slot " + m_sSlotNameHQ + " in " + baseName + "!", LogLevel.WARNING);
				else
					FilterSlots(m_SlotHQ);
			}
			
			// Supply depot
			if (!m_sSlotNameSupplies.IsEmpty())
			{
				m_SlotSupplies = SCR_SiteSlotEntity.Cast(world.FindEntityByName(m_sSlotNameSupplies));
				
				if (!m_SlotSupplies)
					Print("Unable to register Slot " + m_sSlotNameSupplies + " in " + baseName + "!", LogLevel.WARNING);
				else
					FilterSlots(m_SlotSupplies);
			}
			
			// Fuel depot
			if (!m_sSlotNameFuel.IsEmpty())
			{
				m_SlotFuel = SCR_SiteSlotEntity.Cast(world.FindEntityByName(m_sSlotNameFuel));
				
				if (!m_SlotFuel)
					Print("Unable to register Slot " + m_sSlotNameFuel + " in " + baseName + "!", LogLevel.WARNING);
				else
					FilterSlots(m_SlotFuel);
			}
			
			// Armory
			if (!m_sSlotNameArmory.IsEmpty())
			{
				m_SlotArmory = SCR_SiteSlotEntity.Cast(world.FindEntityByName(m_sSlotNameArmory));
				
				if (!m_SlotArmory)
					Print("Unable to register Slot " + m_sSlotNameArmory + " in " + baseName + "!", LogLevel.WARNING);
				else
					FilterSlots(m_SlotArmory);
			}
			
			// Light vehicle Depot
			if (!m_sSlotNameLightVehicleDepot.IsEmpty())
			{
				m_SlotLightVehicleDepot = SCR_SiteSlotEntity.Cast(world.FindEntityByName(m_sSlotNameLightVehicleDepot));
				
				if (!m_SlotLightVehicleDepot)
					Print("Unable to register Slot " + m_sSlotNameLightVehicleDepot + " in " + baseName + "!", LogLevel.WARNING);
				else
					FilterSlots(m_SlotLightVehicleDepot);
			}
			
			// Heavy vehicle Depot
			if (!m_sSlotNameHeavyVehicleDepot.IsEmpty())
			{
				m_SlotHeavyVehicleDepot = SCR_SiteSlotEntity.Cast(world.FindEntityByName(m_sSlotNameHeavyVehicleDepot));
				
				if (!m_SlotHeavyVehicleDepot)
					Print("Unable to register Slot " + m_sSlotNameHeavyVehicleDepot + " in " + baseName + "!", LogLevel.WARNING);
				else
					FilterSlots(m_SlotHeavyVehicleDepot);
			}
			
			// Military hospital
			if (!m_sSlotNameHospital.IsEmpty())
			{
				m_SlotHospital = SCR_SiteSlotEntity.Cast(world.FindEntityByName(m_sSlotNameHospital));
				
				if (!m_SlotHospital)
					Print("Unable to register Slot " + m_sSlotNameHospital + " in " + baseName + "!", LogLevel.WARNING);
				else
					FilterSlots(m_SlotHospital);
			}
			
			// Barracks
			if (!m_sSlotNameBarracks.IsEmpty())
			{
				m_SlotBarracks = SCR_SiteSlotEntity.Cast(world.FindEntityByName(m_sSlotNameBarracks));
				
				if (!m_SlotBarracks)
					Print("Unable to register Slot " + m_sSlotNameBarracks + " in " + baseName + "!", LogLevel.WARNING);
				else
					FilterSlots(m_SlotBarracks);
			}
			
			// Radio Antenna
			if (!m_sSlotNameRadioAntenna.IsEmpty())
			{
				m_SlotRadioAntenna = SCR_SiteSlotEntity.Cast(world.FindEntityByName(m_sSlotNameRadioAntenna));
				if (!m_SlotRadioAntenna)
					Print("Unable to register Slot " + m_sSlotNameRadioAntenna + " in " + baseName + "!", LogLevel.WARNING);
				else
					FilterSlots(m_SlotRadioAntenna);
			}
		}
		
		if (!IsProxy())
		{
			if (!m_sStartingBaseOwner.IsEmpty() && (campaign.GetApplyPresetOwners() || GetType() == CampaignBaseType.MAIN))
			{
				SetStartingBaseOwner();
				
				if (m_bIsHQ)
				{
					m_BuildingsFaction = GetOwningFaction();
					PrintFormat("%1 MOB set up in %2", GetOwningFaction().GetFactionKey(), GetName());
					SCR_MissionHeaderCampaign header = SCR_MissionHeaderCampaign.Cast(GetGame().GetMissionHeader());
				
					if (header && header.m_iMaximumHQSupplies != -1 && ! header.m_iStartingHQSupplies != -1)
					{
						SetSuppliesMax(header.m_iMaximumHQSupplies);
						AddSupplies(header.m_iStartingHQSupplies - GetSupplies());
					}
				}
			}
		
			if (GetType() == CampaignBaseType.MAIN)
			{
				GetGame().GetCallqueue().Remove(ReinforcementsTimer);
				GetGame().GetCallqueue().CallLater(ReinforcementsTimer, SCR_GameModeCampaignMP.REINFORCEMENTS_CHECK_PERIOD, true, false);
			}
		}
		else if (m_bIsHQ)
		{
			SCR_CampaignFaction HQFaction = SCR_CampaignFactionManager.GetInstance().GetCampaignFactionByIndex(m_iOwningFaction);
			
			if (HQFaction)
				HQFaction.SetMainBase(this);
		}
		
		SCR_CampaignBaseManager.GetInstance().OnBaseInitialized(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find and process nearby slots for collisions
	void FilterSlots(notnull SCR_SiteSlotEntity slot)
	{
		GetGame().GetWorld().QueryEntitiesBySphere(slot.GetOrigin(), 20, ProcessFilteredSlots, null, EQueryEntitiesFlags.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disables nearby vanilla slots which collide with Conflict-specific ones
	bool ProcessFilteredSlots(IEntity ent)
	{
		if (ent.Type() != SCR_SiteSlotEntity)
			return true;
		
		string name = ent.GetName();
		
		if (!name.IsEmpty())
		{
			if (name == m_sSlotNameHQ)
				return true;
			
			if (name == m_sSlotNameSupplies)
				return true;
			
			if (name == m_sSlotNameFuel)
				return true;
			
			if (name == m_sSlotNameArmory)
				return true;
			
			if (name == m_sSlotNameLightVehicleDepot)
				return true;
			
			if (name == m_sSlotNameHeavyVehicleDepot)
				return true;
			
			if (name == m_sSlotNameHospital)
				return true;
			
			if (name == m_sSlotNameBarracks)
				return true;
			
			if (name == m_sSlotNameRadioAntenna)
				return true;
		}
		
		SCR_EditableEntityComponent comp = SCR_EditableEntityComponent.Cast(ent.FindComponent(SCR_EditableEntityComponent));
		
		if (comp)
			comp.SetVisible(false);
		
		return true;
	}
	//------------------------------------------------------------------------------------------------
	void SendHQMessageBaseCaptured()
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (!campaign)
			return;
		
		SCR_CampaignFaction newOwner = GetOwningFaction();
		if (!newOwner)
			return;

		int newOwnerPoints = campaign.GetFactionControlPointsCount(newOwner.GetFactionKey());
		
		if (newOwnerPoints == campaign.GetControlPointTreshold() && GetIsControlPoint())
		{
			newOwner.SendHQMessage(SCR_ERadioMsg.WINNING);
			
			SCR_CampaignFaction losingFaction;
			FactionManager factionManager = GetGame().GetFactionManager();
			if (!factionManager)
				return;
			
			if (newOwner.GetFactionKey() == campaign.FACTION_BLUFOR)
				losingFaction = SCR_CampaignFaction.Cast(factionManager.GetFactionByKey(campaign.FACTION_OPFOR));
			else 
				losingFaction = SCR_CampaignFaction.Cast(factionManager.GetFactionByKey(campaign.FACTION_BLUFOR));
			
			if (losingFaction)
				losingFaction.SendHQMessage(SCR_ERadioMsg.LOSING);	
		}
		else
		{
			switch (GetType())
			{
				case CampaignBaseType.MAIN:
				{
					newOwner.SendHQMessage(SCR_ERadioMsg.SEIZED_MAIN, m_iCallsign);
					break;
				}
				
				case CampaignBaseType.MAJOR:
				{
					newOwner.SendHQMessage(SCR_ERadioMsg.SEIZED_MAJOR, m_iCallsign);					
					break;
				}
				
				case CampaignBaseType.SMALL:
				{
					newOwner.SendHQMessage(SCR_ERadioMsg.SEIZED_SMALL, m_iCallsign);
					break;
				}
			}
			
			if (m_OwningFactionPrevious)
				m_OwningFactionPrevious.SendHQMessage(SCR_ERadioMsg.BASE_LOST, m_iCallsign);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnStartingVehicles()
	{
		if (IsProxy() || !GetIsHQ())
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetCampaign();
		if (!campaign)
			return;
		
		int count = campaign.GetStartingVehiclesCount();
		if (count < 1)
			return;
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		vector pos;
		
		for (int i = 0; i<=count-1; i++)
		{
			SCR_WorldTools.FindEmptyTerrainPosition(pos, GetOrigin(), HQ_VEHICLE_SPAWN_RADIUS, HQ_VEHICLE_QUERY_SPACE);
			params.Transform[3] = pos;
			Resource veh = Resource.Load(GetOwningFaction().GetDefaultTransportPrefab());
			GetGame().SpawnEntityPrefab(veh, GetWorld(), params);
		}
	}
	
	//****************//
	//OVERRIDE METHODS//
	//****************//
	
	//------------------------------------------------------------------------------------------------
	void LoadState(SCR_CampaignBaseStruct baseStruct)
	{
		if (!baseStruct)
			return;
		
		if (!m_FactionControl)
			return;
		
		if (GetGame().GetFactionManager().GetFactionIndex(GetOwningFaction()) != baseStruct.GetOwningFaction())
		{
			FactionManager manager = GetGame().GetFactionManager();
			if (!manager)
				return;
			
			SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(manager.GetFactionByIndex(baseStruct.GetOwningFaction()));
			if (!faction)
				return;
			
			SetOwningFaction(faction);
		}
		
		SCR_CampaignFaction faction = GetOwningFaction();
		m_BuildingsFaction = SCR_CampaignFactionManager.GetInstance().GetCampaignFactionByIndex(baseStruct.GetBuildingsFaction());
		
		if (faction && GetType() == CampaignBaseType.MAIN)
			faction.SetMainBase(this);
		
		if (!baseStruct.GetIsHQ())
			m_bIsHQ = false;
		
		SetCallsignIndex(baseStruct.GetCallsignIndex());
		AddSupplies(baseStruct.GetSupplies() - GetSupplies());
		
		// Delay so we don't spawn stuff during init, and make sure prebuilt structures are already in the scene
		GetGame().GetCallqueue().CallLater(SpawnBuildings, 1000, false, baseStruct);
	}
	
	//------------------------------------------------------------------------------------------------
	void StoreState(out SCR_CampaignBaseStruct baseStruct)
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();
		
		if (!fManager)
			return;
		
		baseStruct.SetBaseID(GetBaseID());
		baseStruct.SetIsHQ(GetIsHQ());
		baseStruct.SetCallsignIndex(GetCallsign());
		baseStruct.SetOwningFaction(GetGame().GetFactionManager().GetFactionIndex(GetOwningFaction()));
		baseStruct.SetBuildingsFaction(fManager.GetFactionIndex(GetBuildingsFaction()));
		baseStruct.SetSupplies(GetSupplies());
		
		if (!m_bPrebuilt)
			baseStruct.SetBuildingsData(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (m_FactionControl)
			writer.WriteInt(GetGame().GetFactionManager().GetFactionIndex(m_FactionControl.GetAffiliatedFaction()));
		else
			writer.WriteInt(INVALID_FACTION_INDEX);
		
		writer.WriteBool(m_bIsLinkedToMainWest);
		writer.WriteBool(m_bIsLinkedToMainEast);
		writer.WriteBool(m_bEnabled);
		writer.WriteBool(m_bIsHQ);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadInt(m_iOwningFaction);
		reader.ReadBool(m_bIsLinkedToMainWest);
		reader.ReadBool(m_bIsLinkedToMainEast);
		reader.ReadBool(m_bEnabled);
		reader.ReadBool(m_bIsHQ);
		
		if (m_iOwningFaction != INVALID_FACTION_INDEX)
			OnOwningFactionChanged(GetGame().GetFactionManager().GetFactionByIndex(m_iOwningFaction));
		
		if (m_bIsLinkedToMainWest || m_bIsLinkedToMainEast)
			OnHasSignalChanged();
		
		if (m_bEnabled)
			InitializeBase();
		else
			DisableBase();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (SCR_GameModeCampaignMP.NotPlaying())
			return;
		
		super.EOnInit(owner);
		
	#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_BASES))
			return;
	#endif
		
		//getting spawn point entity from prefab
		SCR_SpawnPoint spawnpoint;
		IEntity child = GetChildren();
		while (child)
		{
			spawnpoint = SCR_SpawnPoint.Cast(child);
			if (spawnpoint)
			{
				m_SpawnPoint = spawnpoint;
				m_OnSpawnPointAssigned.Invoke();
				break;
			}
				
			child = child.GetSibling();
		}
		
		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		m_FactionControl = SCR_FactionAffiliationComponent.Cast(FindComponent(SCR_FactionAffiliationComponent));
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return;
		
		if (m_FactionControl)
			m_FactionControl.GetOnFactionUpdate().Insert(OnOwningFactionChanged);
		
		//Not all bases will have supply component
		m_SuppliesComponent = SCR_CampaignSuppliesComponent.Cast(FindComponent(SCR_CampaignSuppliesComponent));
		
		if (m_SuppliesComponent)
			m_SuppliesComponent.m_OnSuppliesChanged.Insert(OnSuppliesChanged);
		
		if (!IsProxy())
		{
			SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
			
			if (!baseManager.GetIsHQSetupDone())
				baseManager.SetHQs();
			
			if (m_bDisableWhenUnusedAsHQ && m_bCanBeHQ && !m_bIsHQ)
			{
				if (m_bEnabled)
					DisableBase();
			}
			else if (m_bEnabled)
				InitializeBase();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (!IsProxy())
		{
			// Handle respawn cooldown on server
			if (!m_bRespawnBecameAvailable && GetOwningFaction() && Replication.Time() >= m_fRespawnAvailableSince)
			{
				m_bRespawnBecameAvailable = true;
				HandleSpawnPointFaction();
			}
		}
	}

	//****//
	//DIAG//
	//****//

#ifdef ENABLE_DIAG
	protected bool m_bDebugShowRelayMapIcons = false;
	protected ref array<ref Shape> m_aDebugShapes = new ref array<ref Shape>();
	
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_BASES_DIAG))
			return;
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_BASES))
			return;
		
		DiagUpdate(timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called per frame
	void DiagUpdate(float timeSlice)
	{
		DiagDrawDebugShapes(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Draws debug shapes
	protected void DiagDrawDebugShapes(float timeSlice)
	{
		if (!m_aDebugShapes || m_aDebugShapes.Count() > 0)
			return;
		
		bool drawDebug = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_SHOW_RELAYS);
		if (!drawDebug)
		{
			m_aDebugShapes.Clear();
			return;
		}
		
		if (m_eType == CampaignBaseType.RELAY)
		{
			Color color = new Color(0.5, 0.5, 0.5);
			if (GetOwningFaction())
				color = GetOwningFaction().GetFactionColor();
			Shape shape = Shape.CreateSphere(ARGB(color.A() * COLOR_1_TO_255, color.R() * COLOR_1_TO_255, color.G() * COLOR_1_TO_255, color.B() * COLOR_1_TO_255), ShapeFlags.NOOUTLINE | ShapeFlags.NOZBUFFER, GetOrigin() + "0 45 0", 3);
			m_aDebugShapes.Insert(shape);
		}
	}
#endif
	//************************//
	//CONSTRUCTOR / DESTRUCTOR//
	//************************//
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_OnInit(inout vector mat[4], IEntitySource src)
	{
		if (!SCR_GameModeCampaignMP.NotPlaying())
			return;
		
		SCR_CampaignBaseManager.HandleBaseID(this, src);
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignBase(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH
		m_Source = src;
#endif
		if (SCR_GameModeCampaignMP.NotPlaying())
			return;
		
		SCR_CampaignBaseManager.RegisterBase(this);
		
		if (Replication.IsServer())
		{
			SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
			SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
			SCR_MissionHeaderCampaign headerCampaign = SCR_MissionHeaderCampaign.Cast(header);
			
			if (headerCampaign)
			{
				// Disable this base if it's been blacklisted in header
				array<ref SCR_CampaignCustomBase> baseList = headerCampaign.m_aCampaignCustomBaseList;
				bool whitelist = headerCampaign.m_bCustomBaseWhitelist;
				bool found = false;
				
				foreach (SCR_CampaignCustomBase base: baseList)
				{
					string name = base.GetBaseName();
				
					if (!name.IsEmpty() && name == GetName())
					{
						SetIsControlPoint(base.GetIsControlPoint());
						found = true;
						m_bCanBeHQ = base.GetCanBeHQ();
						m_bDisableWhenUnusedAsHQ = base.GetDisableWhenUnusedAsHQ();
						
						if (!whitelist)
							m_bEnabled = false;
						
						break;
					}
				}
				
				if (!found && whitelist)
					m_bEnabled = false;
				
				if (m_bEnabled)
				{
					if (m_bCanBeHQ || m_bIsControlPoint)
						baseManager.SelectHQs();	
				}
				else
				{
					DisableBase();
					return;
				}
			}
			else
			{
				if (m_bCanBeHQ || m_bIsControlPoint)
					baseManager.SelectHQs();
			}
		}
		
		SCR_CampaignBaseManager.s_OnAllBasesInitialized.Insert(AfterAllBasesInitialized);
		SCR_GameModeCampaignMP.s_OnFactionAssignedLocalPlayer.Insert(OnLocalPlayerFactionAssigned);
		SCR_GameModeCampaignMP.s_OnFactionAssignedLocalPlayer.Insert(HandleMapInfo);
		SCR_GameModeCampaignMP.s_OnBaseCaptured.Insert(OnBaseCaptured);
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_MENU, "Conflict", "");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_EXECUTE_BASES, "", "Execute bases", "Conflict", true);
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_SHOW_RELAYS, "", "Show relay position", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_INSTANT_CAPTURE, "", "Instant base capture", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_BASES_DIAG, "", "Enable bases diag", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_NEW_BUILDING, "", "Enable free form building", "Conflict");
#endif
		if (!GetGame().GetWorldEntity())
			return;
		
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.DIAG);
		SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignBase()
	{
		SCR_GameModeCampaignMP.s_OnBaseCaptured.Remove(OnBaseCaptured);
		SCR_MapCampaignUI.Event_OnIconsInit.Remove(HandleMapInfo);
		SCR_MapEntity.GetOnMapOpen().Remove(HandleMapInfo_NoParam);
		
#ifdef ENABLE_DIAG
		if (m_aDebugShapes)
			m_aDebugShapes.Clear();
		m_aDebugShapes = null;
#endif
		
		m_aInRangeOf = null;
		m_aInMyRange = null;
		m_aRemnants = null;
		
		SCR_CampaignBaseManager.UnregisterBase(this);
	}

};

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sBaseName", true)]
class SCR_CampaignCustomBase
{
	[Attribute("", UIWidgets.EditBox, "Base entity name as set up in World Editor.")]
	protected string m_sBaseName;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bIsControlPoint;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bCanBeHQ;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bDisableWhenUnusedAsHQ;
	
	//------------------------------------------------------------------------------------------------
	string GetBaseName()
	{
		return m_sBaseName;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsControlPoint()
	{
		return m_bIsControlPoint;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetCanBeHQ()
	{
		return m_bCanBeHQ;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetDisableWhenUnusedAsHQ()
	{
		return m_bDisableWhenUnusedAsHQ;
	}
};

enum SCR_ECampaignPresentFactionsFlags
{
	NOTHING = 0,
	CAN_SEIZE = 1,
	CAN_DEFEND = 2
};

enum CampaignBaseType
{
	RELAY = 1,
	SMALL = 2,
	MAJOR = 4,
	MAIN = 8
};

enum ECampaignCompositionType
{
	HQ,
	SUPPLIES,
	SUPPLIES_EMPTY,
	SUPPLIES_LOW,
	SUPPLIES_HIGH,
	SUPPLIES_FULL,
	FUEL,
	FUEL_EMPTY,
	FUEL_LOW,
	FUEL_HIGH,
	FUEL_FULL,
	LIGHT_VEHICLE_DEPOT,
	HEAVY_VEHICLE_DEPOT,
	ARMORY,
	HOSPITAL,
	BARRACKS,
	RADIO_ANTENNA,
	LAST // this needs to be last Enum. In case of adding new one, please use a line above. This is used for for loop in OnWorldPostProcess() method in SCR_GameModeCampaignMP. 
};

enum ECampaignPrebuiltCompositionType
{
	NONE,
	FUEL,
	LIGHT_VEHICLE_DEPOT,
	HEAVY_VEHICLE_DEPOT,
	ARMORY,
	HOSPITAL,
	BARRACKS
};

enum EFactionMapID
{
	UNKNOWN = 0,
	EAST = 1,
	WEST = 2,
	FIA = 3
};

enum SCR_ECampaignBaseOwner
{
	INDFOR,
	BLUFOR,
	OPFOR
};