[EntityEditorProps(category: "GameScripted/Campaign", description: "Base class for campaign bases.", color: "0 0 255 255")]
class SCR_CampaignBaseClass: SCR_BaseCampaignInstallationClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBase : SCR_BaseCampaignInstallation
{
	static const string VARNAME_BASE_ID = "m_iBaseID";
	static const int DEFENDER_GROUPS_CNT = 4;
	static const float RADIO_RECONFIGURATION_DURATION = 20.0;
	static const int UNDER_ATTACK_WARNING_PERIOD = 60;
	protected static const ResourceName GARAGE_BOARD_US = "{92442E76D55E8F36}Assets/Props/Civilian/NoticeBoardCork_01/Data/NoticeBoardCork_01_US.emat";
	protected static const ResourceName GARAGE_BOARD_USSR = "{D396E8B04E0CC347}Assets/Props/Civilian/NoticeBoardCork_01/Data/NoticeBoardCork_01_USSR.emat";
	protected static const float ICON_FLASH_DURATION = 20;
	protected static const float ICON_FLASH_PERIOD = 0.5;
	
	//This script invoker is called on clients both for JIP and already playing players
	static ref ScriptInvoker s_OnBaseOwnerChanged = new ScriptInvoker();
	static ref ScriptInvoker s_OnSpawnPointOwnerChanged = new ScriptInvoker();
	static ref ScriptInvoker s_OnMapItemInfoChanged = new ScriptInvoker();
		
	//**********//
	//ATTRIBUTES//
	//**********//
	[Attribute("Base", desc: "The display name of this base.")]
	protected string m_sBaseName;
	
	[Attribute("BASE", desc: "The display name of this base, in upper case.")]
	protected string m_sBaseNameUpper;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECampaignBaseOwner))]
	protected SCR_ECampaignBaseOwner m_eStartingBaseOwner;
	
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
	
	[Attribute("", desc: "Name of the Slot on which to spawn a Repair Depot.")]
	protected string m_sSlotNameRepair;	
	
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
	protected SCR_CampaignFaction m_OwningFaction;
	protected SCR_CampaignFaction m_OwningFactionPrevious;
	protected bool m_bLocalPlayerPresent = false;
	protected AIWaypointCycle m_CycleWaypoint;
	protected float m_fReinforcementsArrivalTime = int.MAX;
	protected SCR_CampaignDeliveryPoint m_VehicleDepot;
	protected SCR_CampaignDeliveryPoint m_SupplyDepot;
	protected SCR_CampaignDeliveryPoint m_FuelDepot;
	protected SCR_CampaignDeliveryPoint m_Armory;
	protected SCR_CampaignDeliveryPoint m_RepairDepot;
	protected SCR_CampaignDeliveryPoint m_FieldHospital;
	protected SCR_CampaignDeliveryPoint m_Barracks;
	protected SCR_CampaignDeliveryPoint m_RadioAntenna;
	protected SCR_SiteSlotEntity m_SlotHQ;
	protected SCR_SiteSlotEntity m_SlotSupplies;
	protected SCR_SiteSlotEntity m_SlotFuel;
	protected SCR_SiteSlotEntity m_SlotArmory;
	protected SCR_SiteSlotEntity m_SlotRepair;
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
	protected bool m_bDefendersSpawned = false;
	protected ref array<MapLink> m_aMapLinks = {};
	protected bool m_bIsHovered = false;
	protected SCR_TimedWaypoint m_SeekDestroyWP;
	protected SCR_SmartActionWaypoint m_RetakeWP;
	protected FactionKey m_sStartingBaseOwner;
	protected ResourceName m_sBuildingIconImageset = "{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset";
	protected bool m_bLoadStateVehicleDepotBuilt;
	protected bool m_bLoadStateArmoryBuilt;
	protected float m_fLastEnemyContactTimestamp;
	protected bool m_bBuildingsSpawned;
	
	//Set only on server
	//The faction that last sent reinforcements to this base (SendReinforcements())
	protected SCR_CampaignFaction m_SuppliesFaction;
	
	ref ScriptInvoker m_OnReinforcementsArrived = new ScriptInvoker();
	
	//As s_OnBaseOwnerChanged, however it is called on all clients
	ref ScriptInvoker m_OnFactionChanged = new ScriptInvoker();
	
	ref ScriptInvoker m_OnServiceBuild = new ScriptInvoker();
	
	//********************************//
	//RUNTIME SYNCHED MEMBER VARIABLES//
	//********************************//
	[RplProp(onRplName: "OnCapturingFactionChanged")]
	protected int m_iCapturingFaction = INVALID_FACTION_INDEX;
	[RplProp(onRplName: "OnOwningFactionChanged")]
	protected int m_iOwningFaction = INVALID_FACTION_INDEX;
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
	[RplProp(onRplName: "OnRespawnTicketsChanged")]
	protected int m_iRespawnTickets;
	[RplProp(onRplName: "OnRespawnTicketsMaxChanged")]
	protected int m_iRespawnTicketsMax = SCR_GameModeCampaignMP.RESPAWN_TICKETS_MAX_DEFAULT;
	[RplProp()]
	protected bool m_bEnabled = true;
	[RplProp()]
	protected bool m_bIsHQ = false;
	[RplProp(onRplName: "OnOverrunChanged")]
	protected bool m_bIsOverrun = false;
	[RplProp(onRplName: "OnAttackingFactionChanged")]
	protected int m_iAttackingFaction = -1;
	[RplProp(onRplName: "OnCallsignAssigned")]
	protected int m_iCallsign = INVALID_BASE_INDEX;
		
	//*********//
	//CONSTANTS//
	//*********//
	
	static const int INVALID_PLAYER_INDEX = -1;
	static const int INVALID_FACTION_INDEX = -1;
	static const int INVALID_BASE_INDEX = -1;
	static const int INVALID_FREQUENCY = -1;
	static const int RESPAWN_DELAY_AFTER_CAPTURE = 180000;
	static const int OVERRUN_CHECK_FREQUENCY = 5000;
	static const float OVERRUN_FRACTION = 1/4;
	
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
	static SCR_CampaignBase GetRandomBase()
	{
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		
		array<SCR_CampaignBase> bases = baseManager.GetBases();
		
		int randomIndex = Math.RandomInt(0, bases.Count());
		return bases[randomIndex];
	}
	
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
		HandleMapLinks();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AfterAllBasesInitialized()
	{
		if (RplSession.Mode() != RplMode.Dedicated)
			HandleMapLinks();
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
	void LinkBases()
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
		if (!newOwningFaction || m_OwningFaction == newOwningFaction)
			return;
		
		bool retakenByRemnants = !newOwningFaction.IsPlayable();
		bool firstTime = false;
		
		if (!m_OwningFaction)
		{
			firstTime = true;
			m_SuppliesFaction = newOwningFaction;
		}
		
		SCR_CampaignFaction prevOwningFaction = m_OwningFaction;
		
		m_OwningFactionPrevious = m_OwningFaction;
		
		if (retakenByRemnants)
			m_OwningFaction = null;
		else
			m_OwningFaction = newOwningFaction;
		
		if (m_aCampaignComponents)
		{
			for (int i = 0, count = m_aCampaignComponents.Count(); i < count; i++)
			{
				m_aCampaignComponents[i].OnBaseOwnerChanged(newOwningFaction);
			}
		}
		
		// Delay respawn possibility at newly-captured bases
		if (!changedAtStart)
		{
			m_fRespawnAvailableSince = Replication.Time() + RESPAWN_DELAY_AFTER_CAPTURE;
			
			switch (GetType())
			{
				case CampaignBaseType.MAIN:
				{
					newOwningFaction.SendHQMessage(SCR_ERadioMsg.SEIZED_MAIN, m_iCallsign);
					break;
				}
				
				case CampaignBaseType.MAJOR:
				{
					newOwningFaction.SendHQMessage(SCR_ERadioMsg.SEIZED_MAJOR, m_iCallsign);
					break;
				}
				
				case CampaignBaseType.SMALL:
				{
					newOwningFaction.SendHQMessage(SCR_ERadioMsg.SEIZED_SMALL, m_iCallsign);
					break;
				}
			}
			
			if (prevOwningFaction)
				prevOwningFaction.SendHQMessage(SCR_ERadioMsg.BASE_LOST, m_iCallsign);
			
			if (RplSession.Mode() == RplMode.Dedicated)
			{
				BackendApi bApi = GetGame().GetBackendApi();
				
				if (bApi)
				{				
					if ( SCR_GameModeCampaignMP.GetBackendFilename() != string.Empty)
						bApi.GetStorage().RequestSave(SCR_GameModeCampaignMP.GetBackendFilename());
				}
			}
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
		
		m_iOwningFaction = SCR_CampaignFactionManager.GetInstance().GetFactionIndex(m_OwningFaction);
		Replication.BumpMe();
		OnOwningFactionChanged();	// Server execution
		
		// Reconfigure RadioComponent used for communication.		
		BaseRadioComponent radio = BaseRadioComponent.Cast(FindComponent(BaseRadioComponent));
		
		if (!radio)
			return;
		
		if (retakenByRemnants)
			radio.TogglePower(false);
		else
		{
			radio.TogglePower(true);
			radio.SetRange(GetSignalRange());
			radio.SetFrequency(newOwningFaction.GetFactionRadioFrequency());
			radio.SetEncryptionKey(newOwningFaction.GetFactionRadioEncryptionKey());
		}
		
		// When a Major base is seized for the first time, set up which vehicles will be shown in the widget, even if they're not available yet
		if (!retakenByRemnants && firstTime && !changedAtStart && GetType() == CampaignBaseType.MAJOR)
			FillAssetPools(false, 0);
		else
			if (changedAtStart && GetType() == CampaignBaseType.MAJOR)
				FillAssetPools(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateBaseDefenders(notnull SCR_CampaignRemnantsPresence presence)
	{
		if (!m_OwningFaction)
			return;
		
		if (presence.GetDefendersFaction() == m_OwningFaction)
			return;
		
		presence.SetDefendersFaction(m_OwningFaction);
		presence.SetGroupPrefab(m_OwningFaction.GetDefendersGroupPrefab());
		presence.SetMembersAlive(-1);
		presence.SetDespawnTimer(-1);
		presence.SetIsSpawned(false);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SiteSlotEntity GetAssignedSlot(ECampaignCompositionType type)
	{
		switch (type)
		{
			case ECampaignCompositionType.HQ: {return m_SlotHQ; break;};
			case ECampaignCompositionType.SUPPLIES: {return m_SlotSupplies; break;};
			case ECampaignCompositionType.FUEL: {return m_SlotFuel; break;};
			case ECampaignCompositionType.REPAIR: {return m_SlotRepair; break;};
			case ECampaignCompositionType.ARMORY: {return m_SlotArmory; break;};
			case ECampaignCompositionType.HOSPITAL: {return m_SlotHospital; break;};
			case ECampaignCompositionType.BARRACKS: {return m_SlotBarracks; break;};
			case ECampaignCompositionType.RADIO_ANTENNA: {return m_SlotRadioAntenna; break;};
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
					
					if (playerFaction != m_OwningFaction)
						continue;
					
					if (vector.DistanceSq(playerEntity.GetOrigin(), this.GetOrigin()) < 90000)	// 300m
						campaign.AwardXP(GetGame().GetPlayerManager().GetPlayerController(players[i]), CampaignXPRewards.BASE_SEIZED);
				}
			}
		}
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
		if (faction == m_OwningFaction)
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
	protected void SpawnDefenders()
	{
		if (m_bDefendersSpawned)
			return;
		
		m_bDefendersSpawned = true;
		
		array<SCR_CampaignRemnantsSpawnPoint> spawnPoints = {};
		IEntity child = GetChildren();
		
		while (child)
		{	
			if (child.Type() == SCR_CampaignRemnantsSpawnPoint)
				spawnPoints.Insert(SCR_CampaignRemnantsSpawnPoint.Cast(child));
			
			child = child.GetSibling();
		};
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		int waypointPointIndex;
		
		for (int spawnPointIndex = 0, count = spawnPoints.Count(); spawnPointIndex < count; spawnPointIndex++)
		{
			spawnPoints[spawnPointIndex].GetWorldTransform(params.Transform);
			Resource res = Resource.Load(m_OwningFaction.GetDefendersGroupPrefab());
			
			if (!res)
				return;
			
			SCR_AIGroup grp = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(res, null, params));
			
			if (!grp)
				return;
			
			grp.SpawnUnits();
			waypointPointIndex = spawnPoints[spawnPointIndex].GetWaypointIndex();
			
			if (waypointPointIndex < 0) // default index value in instance of SCR_CampaignRemnantsSpawnPoint
				waypointPointIndex = spawnPointIndex;
			
			PermuteBaseCycleWaypoint(m_CycleWaypoint, waypointPointIndex);
			grp.AddWaypoint(m_CycleWaypoint);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Changes the faction which can spawn on spawn point groups owned by this base
	protected void SetSpawnPointFaction(Faction faction)
	{
		string factionKey;
		bool isLinked = false;
		
		if (faction)
			isLinked = IsBaseInFactionRadioSignal(SCR_CampaignFaction.Cast(faction));
		
		// Allow respawn on small bases only if they have enough supplies and are within main base radio range
		if (!m_bIsOverrun && faction && (GetType() != CampaignBaseType.SMALL || m_iRespawnTickets > 0) && (Replication.Time() >= m_fRespawnAvailableSince || GetType() != CampaignBaseType.SMALL) && isLinked)
			factionKey = faction.GetFactionKey();
		
		if(!m_SpawnPoint)
			return;
		
		if (factionKey == m_SpawnPoint.GetFactionKey())
			return;

		m_SpawnPoint.SetFactionKey(factionKey);

		s_OnSpawnPointOwnerChanged.Invoke();
	}
	//------------------------------------------------------------------------------------------------
	SCR_SpawnPoint GetBaseSpawnPoint()
	{
		return m_SpawnPoint;
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
					m_PlayedRadio = comp.SoundEvent("SOUND_RADIO_ESTABLISH_ACTION");
				}
				else
				{
					if (m_PlayedRadio != AudioHandle.Invalid)
						comp.TerminateAll();
					
					ToggleRadioChatter(true);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHasSignalChanged()
	{
		SetSpawnPointFaction(m_OwningFaction);
		
		if (RplSession.Mode() != RplMode.Dedicated)
			HandleMapInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	//! In Main and Major bases, check if the base is overrun by enemies and disable respawn if it is
	protected void CheckIsOverrun()
	{
		// The trigger can be unregistered dynamically, so we need to do nullPtr check here
		if (!m_Trigger)
			return;
		
		// Base is controlled by Remnants, cannot be overrun
		if (!GetOwningFaction())
		{
			SetIsOverrun(false);
			return;
		}
		
		SCR_CampaignFaction enemyFaction;
		
		if (GetOwningFaction().GetFactionKey() == SCR_GameModeCampaignMP.FACTION_BLUFOR)
			enemyFaction = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_OPFOR));
		else
			enemyFaction = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_BLUFOR));
		
		if (!enemyFaction)
			return;
		
		// Base is outside of enemy radio range, cannot be overrun
		if (!IsBaseInFactionRadioSignal(enemyFaction))
		{
			SetIsOverrun(false);
			return;
		}
		
		// How many players are there in the enemy faction
		array<int> playerIDs = {};
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		int enemyFactionCnt;
		
		foreach (int ID: playerIDs)
		{
			SCR_ChimeraCharacter playerChar = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerController(ID).GetControlledEntity());
			
			if (!playerChar)
				continue;
			
			Faction faction = playerChar.GetFaction();
			
			if (faction == enemyFaction)
				enemyFactionCnt++;
		}
		
		// Enemy faction is empty, don't bother with the remaining calculations
		if (enemyFactionCnt == 0)
		{
			SetIsOverrun(false);
			return;
		}
		
		array<IEntity> inside = {};
		int enemiesInsideCnt;
		m_Trigger.QueryEntitiesInside();
		m_Trigger.GetEntitiesInside(inside);
		
		// Get alive enemy units inside the trigger
		foreach (IEntity unit: inside)
		{
			SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(unit);
			
			if (!char)
				continue;
			
			CharacterControllerComponent comp = CharacterControllerComponent.Cast(char.FindComponent(CharacterControllerComponent));
			
			if (!comp)
				continue;
			
			if (comp.IsDead())
				continue;
			
			SCR_Faction faction = SCR_Faction.Cast(char.GetFaction());
			
			if (faction && faction.IsPlayable() && faction == enemyFaction)
				enemiesInsideCnt++;
		}
		
		// The base is overrun if the predefined percentage of enemy team is inside the trigger
		SetIsOverrun(enemiesInsideCnt != 0 && enemiesInsideCnt >= enemyFactionCnt * OVERRUN_FRACTION);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetIsOverrun(bool overrun)
	{
		if (m_bIsOverrun != overrun)
		{
			m_bIsOverrun = overrun;
			OnOverrunChanged();
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnOverrunChanged()
	{
		SetSpawnPointFaction(m_OwningFaction);
		
		if (RplSession.Mode() != RplMode.Dedicated)
		{
			if (SCR_PlayerController.GetLocalMainEntityFaction() == GetOwningFaction())
			{
				if (m_bIsOverrun)
					SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_BaseOverrun-UC", 7, text2: "#AR-Campaign_BaseOverrun_Details", prio: SCR_ECampaignPopupPriority.BASE_OVERRUN, param1: GetBaseNameUpperCase());
				else
					SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_BaseSecured-UC", 5, prio: SCR_ECampaignPopupPriority.BASE_OVERRUN, param1: GetBaseNameUpperCase());
			}
			
			HandleMapInfo();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRespawnTicketsChanged()
	{
		SetSpawnPointFaction(m_OwningFaction);
		
		if (RplSession.Mode() != RplMode.Dedicated)
			HandleMapInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRespawnTicketsMaxChanged()
	{
		if (RplSession.Mode() != RplMode.Dedicated)
			HandleMapInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowRespawnCooldown()
	{
		SCR_GameModeCampaignMP camp = SCR_GameModeCampaignMP.GetInstance();

		if (!camp)
			return;

		Faction playerFaction = camp.GetLastPlayerFaction();

		if (!playerFaction)
			return;
		
		if (Replication.Time() < m_fRespawnAvailableSince || !m_bRespawnBecameAvailable)
		{
			// Cooldown finished, enable respawn (client only, server handles this in EOnFrame)
			if (!m_bRespawnBecameAvailable && IsProxy())
				SetSpawnPointFaction(m_OwningFaction);
			
			// Update cooldown timer in map UI
			if (GetBaseUI())
				GetBaseUI().SetIconInfoText();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Event which is triggered when the owning faction changes
	protected void OnOwningFactionChanged()
	{
#ifdef ENABLE_DIAG
		m_aDebugShapes.Clear();
#endif
		
		SCR_CampaignFaction newFaction = SCR_CampaignFactionManager.GetInstance().GetCampaignFactionByIndex(m_iOwningFaction);
		if (newFaction != m_OwningFaction)
		{
			m_OwningFactionPrevious = m_OwningFaction;
			m_OwningFaction = newFaction;
		}
		
		SetSpawnPointFaction(m_OwningFaction);
		
		if (!IsProxy())
		{
			if (GetType() == CampaignBaseType.MAIN || GetType() == CampaignBaseType.MAJOR)
				CheckIsOverrun();
			
			// Update signal coverage only if the base was seized during normal play, not at the start
			if (GetGame().GetWorld().GetWorldTime() != 0 || SCR_GameModeCampaignMP.IsBackendStateLoaded())
			{
				SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
				
				if (baseManager)
				{
					// If the base was neutral before, update signal just for the capturing faction
					if (m_OwningFactionPrevious)
						baseManager.UpdateBasesSignalCoverage();
					else
						baseManager.UpdateBasesSignalCoverage(m_OwningFaction);
				}
			}
			
			// Reset timer for reinforcements
			if (GetType() == CampaignBaseType.MAIN || GetType() == CampaignBaseType.MAJOR)
			{
				ReinforcementsTimer(true);
				Replication.BumpMe();
			}
			
			// Handle defenders
			if (m_OwningFaction)
			{
				foreach (SCR_CampaignRemnantsPresence presence: m_aRemnants)
				{
					if (!presence || !presence.GetIsDefendersSpawn())
						continue;
					
					if (presence.GetIsSpawned() || presence.GetMembersAlive() > 0)
						break;
					
					presence.SetSpawnTime(Replication.Time() + (SCR_CampaignRemnantsPresence.DEFENDERS_SPAWN_DELAY * 1000));
					UpdateBaseDefenders(presence);
					break;
				}
			}
		}
		else
		{
			//On clients we want to invoke this script invoker for local usage
			//First parameter is SCR_CampaignBase -> Cannot be null, we are passing "this"
			//Second parameter is SCR_CampaignFaction -> Could be null
			s_OnBaseOwnerChanged.Invoke(this, m_OwningFaction);
		}
		
		if (RplSession.Mode() != RplMode.Dedicated)
		{
			GetGame().GetCallqueue().Remove(FlashBaseIcon);
			HandleMapInfo();
			HandleMapLinks();
			
			// Update motor pool notice board texture
			if (m_GarageBoard && m_OwningFaction)
			{
				IEntity paper = m_GarageBoard.GetChildren();
				
				if (paper)
				{
					if (m_OwningFaction.GetFactionKey() == SCR_GameModeCampaignMP.FACTION_BLUFOR)
						SCR_Global.SetMaterial(paper, GARAGE_BOARD_US);
					else if (m_OwningFaction.GetFactionKey() == SCR_GameModeCampaignMP.FACTION_OPFOR)
						SCR_Global.SetMaterial(paper, GARAGE_BOARD_USSR);
						
				}
			}
			
			SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_RespawnSystemComponent.GetLocalPlayerFaction());
			SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
			
			// TODO: Move this to PlayRadioMsg so it is checked for player being inside radio range
			if (campaign)
			{
				if (m_OwningFaction == playerFaction)
				{
					campaign.ShowHint(SCR_ECampaignHints.TICKETS);
					
					//Play music theme
					SCR_MusicManager musicManager = SCR_MusicManager.GetInstance();
					if (musicManager)
						musicManager.PlayMusicOneShot("SOUND_ONBASECAPTURE", true, false);			
				}
				
				if (playerFaction && playerFaction != m_OwningFaction && IsBaseInFactionRadioSignal(playerFaction))
				{
					if (m_OwningFaction)
						SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_BaseSeized-UC", prio: SCR_ECampaignPopupPriority.BASE_LOST, param1: m_OwningFaction.GetFactionNameUpperCase(), param2: GetBaseNameUpperCase());
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
			Print(m_iOwningFaction);
			Print(m_OwningFaction);
		}
		
		
	}
	
	//------------------------------------------------------------------------------------------------
	void AddRespawnTickets(int amount, bool loaded = false)
	{
		if (amount == 0)
			return;
		
		if (!m_OwningFaction)
			return;
		
		if (!IsBaseInFactionRadioSignal(m_OwningFaction))
			return;
		
		if (!loaded)
			amount = Math.Min(amount, m_iRespawnTicketsMax - m_iRespawnTickets);
		
		m_iRespawnTickets += amount;
		
		Replication.BumpMe();
		OnRespawnTicketsChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void AddRespawnTicketsMax(int amount)
	{
		m_iRespawnTicketsMax += amount;
		Replication.BumpMe();
		OnRespawnTicketsMaxChanged();
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
			if (m_aInRangeOf.Find(m_aInMyRange[i]) == -1)
				continue;
								//Link already exists
			if (FindMapLink(this, m_aInMyRange[i]))
				continue;
			
			if(myFaction != localPlayerFaction)
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
		
		// Set callsign based on player's faction
		if (GetType() != CampaignBaseType.RELAY && m_sCallsign.IsEmpty())
			SetCallsign(playerFactionCampaign);
		
		s_OnMapItemInfoChanged.Invoke(playerFaction);
		
		// Update base icon color
		EFactionMapID factionMapID = EFactionMapID.UNKNOWN;
		bool isInRange = IsBaseInFactionRadioSignal(playerFactionCampaign);
		
		// Show proper faction color only for HQs or bases within radio signal
		if (!m_OwningFaction && isInRange)
			factionMapID = EFactionMapID.FIA;
		else
			if (m_OwningFaction && (GetType() == CampaignBaseType.MAIN || isInRange))
				switch (m_OwningFaction.GetFactionKey())
				{
					case SCR_GameModeCampaignMP.FACTION_OPFOR: {factionMapID = EFactionMapID.EAST; break;};
					case SCR_GameModeCampaignMP.FACTION_BLUFOR: {factionMapID = EFactionMapID.WEST; break;};
				}
		
		m_MapItem.SetFactionIndex(factionMapID);
		
		array<SCR_CampaignDeliveryPoint> services = {};
		GetAllBaseServices(services);
		
		foreach (SCR_CampaignDeliveryPoint service: services)
		{
			SCR_CampaignServiceMapDescriptorComponent comp = SCR_CampaignServiceMapDescriptorComponent.Cast(service.FindComponent(SCR_CampaignServiceMapDescriptorComponent));
			
			if (comp)
			{
				if (isInRange && service.IsBuilt())
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
		
		m_bRespawnBecameAvailable = true;
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
		if (m_iRespawnTicketsMax != 0 && respawnCooldown > 0 && m_OwningFaction && m_OwningFaction == playerFaction)
		{
			m_bRespawnBecameAvailable = false;
			shownRespawnCooldown = "#AR-Campaign_BaseCooldown_Suffix";
			SCR_Global.ConvertSecondsToDaysHoursMinutesSeconds(respawnCooldown, d, h, m, s);
			sStr = string.ToString(s);
			
			if (s < 10)
				sStr = "0" + sStr;
		}

		
		// Compose proper map info based on the base type and ownership
		LocalizedString suppliesString;
		LocalizedString respawnString;
		string suppliesInfo;
		string respawnsInfo;
		
		
		
		
		if (GetType() != CampaignBaseType.RELAY)
		{
			Supplies.SetVisible(true);
			SuppliesImg.SetVisible(true);
			if (m_OwningFaction && m_OwningFaction == playerFaction)
			{
				suppliesString = "#AR-Campaign_BaseSuppliesAmount";
				suppliesInfo = GetSupplies().ToString();
				
				if (GetType() == CampaignBaseType.SMALL)
				{
					Respawn.SetVisible(true);
					RespawnImg.SetVisible(true);
					
					if (m_iRespawnTicketsMax == 0)
					{
						respawnString = "#AR-Campaign_BaseRespawnsNone";
					}
					else
					{
						respawnString = "#AR-Campaign_BaseRespawnsAmount";
						respawnsInfo = m_iRespawnTickets.ToString();
						respawnString = respawnString + " " + shownRespawnCooldown;
					}
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
		Respawn.SetTextFormat(respawnString, respawnsInfo, m_iRespawnTicketsMax, m, sStr);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCallsignIndex(int index)
	{
		m_iCallsign = index;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetCallsign(notnull SCR_CampaignFaction faction)
	{	
#ifdef ENABLE_BUILDING_DEBUG
		PrintFormat("SetCallsign() - %1", GetName());
#endif
		if (m_iCallsign < 0 || m_iCallsign >= SCR_GameModeCampaignMP.BASE_CALLSIGNS_COUNT)
			return;
		
#ifdef ENABLE_BUILDING_DEBUG
		PrintFormat("SetCallsign data: m_iCallsign: %1, faction: %2", m_iCallsign, faction.GetFactionKey());
#endif
		
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
		
#ifdef ENABLE_BUILDING_DEBUG
		Print(baseCallsignsArray.Count());
#endif
	
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
	//! Checks children for valit depots
	protected void RegisterDepots()
	{
		IEntity child = GetChildren();
		
		while (child)
		{	
			SCR_CampaignDeliveryPoint depot = SCR_CampaignDeliveryPoint.Cast(child);
			child = child.GetSibling();
			
			if (!depot)
				continue;
			
			ECampaignServicePointType depotType = depot.GetServiceType();
			
			switch (depotType)
			{
				case ECampaignServicePointType.VEHICLE_DEPOT:
				{
					m_VehicleDepot = depot;
					
					if (!IsProxy() && m_OwningFaction)
						m_VehicleDepot.FillAssetPool(true);
					
					break;
				};
				case ECampaignServicePointType.SUPPLY_DEPOT:
				{
					m_SupplyDepot = depot;
					depot.SetSlot(GetAssignedSlot(ECampaignCompositionType.SUPPLIES));
					SCR_CampaignServiceMapDescriptorComponent comp = SCR_CampaignServiceMapDescriptorComponent.Cast(depot.FindComponent(SCR_CampaignServiceMapDescriptorComponent));
					
					if (comp)
						comp.SetServiceMarker(GetOwningFaction());
					
					break;
				};
				case ECampaignServicePointType.FUEL_DEPOT:
				{
					m_FuelDepot = depot;
					depot.SetSlot(GetAssignedSlot(ECampaignCompositionType.FUEL));
					break;
				};
				case ECampaignServicePointType.ARMORY:
				{
					m_Armory = depot;
					depot.SetSlot(GetAssignedSlot(ECampaignCompositionType.ARMORY));
					break;
				};
				case ECampaignServicePointType.REPAIR_DEPOT:
				{
					m_RepairDepot = depot;
					depot.SetSlot(GetAssignedSlot(ECampaignCompositionType.REPAIR));
					break;
				};
				case ECampaignServicePointType.FIELD_HOSPITAL:
				{
					m_FieldHospital = depot;
					depot.SetSlot(GetAssignedSlot(ECampaignCompositionType.HOSPITAL));
					break;
				};
				case ECampaignServicePointType.BARRACKS:
				{
					m_Barracks = depot;
					depot.SetSlot(GetAssignedSlot(ECampaignCompositionType.BARRACKS));
					break;
				};
				case ECampaignServicePointType.RADIO_ANTENNA:
				{
					m_RadioAntenna = depot;
					depot.SetSlot(GetAssignedSlot(ECampaignCompositionType.RADIO_ANTENNA));
					break;
				};
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
			
			m_fReinforcementsArrivalTime = curTime + (SCR_GameModeCampaignMP.GetReinforcementsPeriod(this) * 1000);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Reinforcements timer has finished, send in reinforcements 
	protected void SendReinforcements()
	{
		m_OnReinforcementsArrived.Invoke(this, m_SuppliesFaction, m_OwningFaction);
		m_SuppliesFaction = m_OwningFaction;
		FillAssetPools();
		AddSupplies(Math.Min(SCR_GameModeCampaignMP.REINFORCEMENTS_SUPPLIES, GetSuppliesMax() - GetSupplies()));
		m_OwningFaction.SendHQMessage(SCR_ERadioMsg.REINFORCEMENTS, m_iCallsign, param: GetBaseID())
	};
	
	//------------------------------------------------------------------------------------------------
	//! Add assets to delivery point stocks
	protected void FillAssetPools(bool firstTime = false, int overrideAmount = -1)
	{
		if (m_eType == CampaignBaseType.RELAY || m_eType == CampaignBaseType.SMALL)
			return;
		
		if (!m_VehicleDepot)
			return;
		
		m_VehicleDepot.FillAssetPool(firstTime, overrideAmount)
	};
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnComposition(ECampaignCompositionType slotType)
	{
		SCR_SiteSlotEntity slot = GetAssignedSlot(slotType);
		SCR_CampaignFaction buildingsFaction = GetBuildingsFaction();
		
		if (!buildingsFaction)
			return;
		
		SCR_CampaignDeliveryPoint service;
		int ticketsBonus;
			
		switch (slotType)
		{
			case ECampaignCompositionType.SUPPLIES: {service = GetBaseService(ECampaignServicePointType.SUPPLY_DEPOT); break;};
			case ECampaignCompositionType.FUEL: {service = GetBaseService(ECampaignServicePointType.FUEL_DEPOT); ticketsBonus = SCR_GameModeCampaignMP.RESPAWN_TICKETS_MAX_BONUS_FUEL; break;};
			case ECampaignCompositionType.REPAIR: {service = GetBaseService(ECampaignServicePointType.REPAIR_DEPOT); ticketsBonus = SCR_GameModeCampaignMP.RESPAWN_TICKETS_MAX_BONUS_REPAIR; break;};
			case ECampaignCompositionType.ARMORY: {service = GetBaseService(ECampaignServicePointType.ARMORY); ticketsBonus = SCR_GameModeCampaignMP.RESPAWN_TICKETS_MAX_BONUS_ARMORY; break;};
			case ECampaignCompositionType.HOSPITAL: {service = GetBaseService(ECampaignServicePointType.FIELD_HOSPITAL); ticketsBonus = SCR_GameModeCampaignMP.RESPAWN_TICKETS_MAX_BONUS_HOSPITAL; break;};
			case ECampaignCompositionType.BARRACKS: {service = GetBaseService(ECampaignServicePointType.BARRACKS); ticketsBonus = SCR_GameModeCampaignMP.RESPAWN_TICKETS_MAX_BONUS_BARRACKS; break;};
		}
		
		// If the slot is defined for a composition, use the slot
		if (slot)
		{
			if (slotType == ECampaignCompositionType.SUPPLIES)
				slotType = SCR_CampaignDeliveryPoint.CalculateSupplyDepotCompositionType(GetSuppliesMax(), GetSupplies());
			
			ResourceName compType = buildingsFaction.GetBuildingPrefab(slotType, GetType());
			Resource composition = Resource.Load(compType);
			
			if (!composition)
				return;

			slot.SpawnEntityInSlot(composition);
			
			if (service)
			{
				service.UpdateCompositionPrefab(compType);
				
				if (slotType == ECampaignCompositionType.REPAIR && !IsProxy())
				{
					GetGame().GetCallqueue().CallLater(service.HandleVehicleSpawn, 1000); // Spawn the first vehicle right away
					GetGame().GetCallqueue().CallLater(service.HandleVehicleSpawn, SCR_GameModeCampaignMP.GARAGE_VEHICLE_SPAWN_INTERVAL, true);
				}
			}
			
			if (!IsProxy())
			{
				AddRespawnTicketsMax(ticketsBonus);
				
				if (!SCR_GameModeCampaignMP.IsBackendStateLoaded())
					AddRespawnTickets(ticketsBonus);
			}
		}
		// if the slot is not defined spawn a composition on the SCR_Delivery entity or base position
		else if (m_bSpawnNoSlotComposition)
		{
			if (slotType == ECampaignCompositionType.SUPPLIES)
				slotType = SCR_CampaignDeliveryPoint.CalculateSupplyDepotCompositionType(GetSuppliesMax(), GetSupplies());
			
			IEntity deliveryPoint;
			switch (slotType)
			{
				case ECampaignCompositionType.HQ: {deliveryPoint = this; break;};
				case ECampaignCompositionType.SUPPLIES: {deliveryPoint = m_SupplyDepot; break;};
				case ECampaignCompositionType.SUPPLIES_LOW: {deliveryPoint = m_SupplyDepot; break;};
				case ECampaignCompositionType.SUPPLIES_HIGH: {deliveryPoint = m_SupplyDepot; break;};
				case ECampaignCompositionType.SUPPLIES_FULL: {deliveryPoint = m_SupplyDepot; break;};
				case ECampaignCompositionType.SUPPLIES_EMPTY: {deliveryPoint = m_SupplyDepot; break;};
				case ECampaignCompositionType.FUEL: {deliveryPoint = m_FuelDepot; break;};
				case ECampaignCompositionType.REPAIR: {deliveryPoint = m_RepairDepot; break;};
				case ECampaignCompositionType.ARMORY: {deliveryPoint = m_Armory; break;};
				case ECampaignCompositionType.HOSPITAL: {deliveryPoint = m_FieldHospital; break;};
				case ECampaignCompositionType.BARRACKS: {deliveryPoint = m_Barracks; break;};
				case ECampaignCompositionType.RADIO_ANTENNA: {deliveryPoint = m_RadioAntenna; break;};
			}
			
			if (!deliveryPoint)
				return;
			
			ResourceName compType = buildingsFaction.GetBuildingPrefabNoSlot(slotType, GetType());
			Resource compositionRes = Resource.Load(compType);
			
			if (!compositionRes.IsValid())
				return;

			EntitySpawnParams spawnParams = EntitySpawnParams();	
			spawnParams.TransformMode = ETransformMode.WORLD;		
			deliveryPoint.GetWorldTransform(spawnParams.Transform);
		
			IEntity composition = GetGame().SpawnEntityPrefab(compositionRes, GetGame().GetWorld(),spawnParams);
			m_OnServiceBuild.Invoke();
			SCR_CampaignServiceCompositionComponent campServComp = SCR_CampaignServiceCompositionComponent.Cast(composition.FindComponent(SCR_CampaignServiceCompositionComponent));
			if (campServComp)
			{
				campServComp.SetCompositionType(slotType);
				campServComp.SetDeliveryPoint(deliveryPoint);
			}
				
			if (service)
				service.SetIsBuilt(true);
		}
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
		if (m_SupplyDepot)
		{
			SCR_CampaignSuppliesComponent supp = SCR_CampaignSuppliesComponent.Cast(m_SupplyDepot.FindComponent(SCR_CampaignSuppliesComponent));
			
			if (supp)
				return supp.GetSupplies();
		}
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSuppliesMax()
	{
		if (m_SupplyDepot)
		{
			SCR_CampaignSuppliesComponent supp = SCR_CampaignSuppliesComponent.Cast(m_SupplyDepot.FindComponent(SCR_CampaignSuppliesComponent));
			
			if (supp)
				return supp.GetSuppliesMax();
		}
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnBuildings()
	{
		m_bBuildingsSpawned = true;
		
		// These compositions need to be built everywhere
		SpawnComposition(ECampaignCompositionType.HQ);
		SpawnComposition(ECampaignCompositionType.SUPPLIES);
		SpawnComposition(ECampaignCompositionType.RADIO_ANTENNA);
				
		if (GetType() != CampaignBaseType.RELAY && (GetType() == CampaignBaseType.MAIN || GetType() == CampaignBaseType.MAJOR || m_bPrebuilt))
		{
			// Build all available services automatically in big bases or if small base should be prebuilt
			SpawnComposition(ECampaignCompositionType.FUEL);
			SpawnComposition(ECampaignCompositionType.REPAIR);
			SpawnComposition(ECampaignCompositionType.ARMORY);
			SpawnComposition(ECampaignCompositionType.HOSPITAL);
			SpawnComposition(ECampaignCompositionType.BARRACKS);
			
			// Fill respawn tickets
			if (GetType() == CampaignBaseType.SMALL)
				AddRespawnTickets(m_iRespawnTicketsMax);
		}
		else
		{
			if (m_bLoadStateVehicleDepotBuilt)
				SpawnComposition(ECampaignCompositionType.REPAIR);
			
			if (m_bLoadStateArmoryBuilt)
				SpawnComposition(ECampaignCompositionType.ARMORY);
		}
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
		
		if (!m_OwningFaction)
			m_PlayedRadio = comp.SoundEvent("SOUND_RADIO_CHATTER_EV");
		else
		{
			if (m_OwningFaction.GetFactionKey() == SCR_GameModeCampaignMP.FACTION_BLUFOR)
				m_PlayedRadio = comp.SoundEvent("SOUND_RADIO_CHATTER_US");
			else
				m_PlayedRadio = comp.SoundEvent("SOUND_RADIO_CHATTER_RU");
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
	SCR_CampaignDeliveryPoint GetBaseService(ECampaignServicePointType type, bool hasToBeBuilt = false)
	{
		SCR_CampaignDeliveryPoint foundDepot;
		
		switch (type)
		{
			case ECampaignServicePointType.VEHICLE_DEPOT: {foundDepot = m_VehicleDepot; break;};
			case ECampaignServicePointType.SUPPLY_DEPOT: {foundDepot = m_SupplyDepot; break;};
			case ECampaignServicePointType.FUEL_DEPOT: {foundDepot = m_FuelDepot; break;};
			case ECampaignServicePointType.ARMORY: {foundDepot = m_Armory; break;};
			case ECampaignServicePointType.REPAIR_DEPOT: {foundDepot = m_RepairDepot; break;};
			case ECampaignServicePointType.FIELD_HOSPITAL: {foundDepot = m_FieldHospital; break;};
			case ECampaignServicePointType.BARRACKS: {foundDepot = m_Barracks; break;};
		}
		
		if (hasToBeBuilt)
		{
			if (foundDepot && foundDepot.IsBuilt())
			{
				return foundDepot;
			}
			else
			{
				return null;
			}
		}
		
		return foundDepot;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetAllBaseServices(out array<SCR_CampaignDeliveryPoint> baseServices = null, bool builtOnly = false)
	{
		int cnt = 0;
		SCR_CampaignDeliveryPoint service;
		
		service = GetBaseService(ECampaignServicePointType.VEHICLE_DEPOT, builtOnly); if (service) {cnt ++; if (baseServices) baseServices.Insert(service)};
		service = GetBaseService(ECampaignServicePointType.SUPPLY_DEPOT, builtOnly); if (service) {cnt ++; if (baseServices) baseServices.Insert(service)};
		service = GetBaseService(ECampaignServicePointType.FUEL_DEPOT, builtOnly); if (service) {cnt ++; if (baseServices) baseServices.Insert(service)};
		service = GetBaseService(ECampaignServicePointType.ARMORY, builtOnly); if (service) {cnt ++; if (baseServices) baseServices.Insert(service)};
		service = GetBaseService(ECampaignServicePointType.REPAIR_DEPOT, builtOnly); if (service) {cnt ++; if (baseServices) baseServices.Insert(service)};
		service = GetBaseService(ECampaignServicePointType.FIELD_HOSPITAL, builtOnly); if (service) {cnt ++; if (baseServices) baseServices.Insert(service)};
		service = GetBaseService(ECampaignServicePointType.BARRACKS, builtOnly); if (service) {cnt ++; if (baseServices) baseServices.Insert(service)};
		
		return cnt;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add delivered supplies
	void AddSupplies(int suppliesCnt)
	{
		if (!m_SupplyDepot)
			return;
		
		SCR_CampaignSuppliesComponent supp = SCR_CampaignSuppliesComponent.Cast(m_SupplyDepot.FindComponent(SCR_CampaignSuppliesComponent));
			
		if (!supp)
			return;
		
		supp.AddSupplies(suppliesCnt);
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
			
			if (m_OwningFaction)
				rangeColor = m_OwningFaction.GetFactionColor();
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
		GetGame().GetCallqueue().CallLater(HandleMapInfo, 3000, true);
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
					SCR_CampaignTaskManager tManager = SCR_CampaignTaskManager.GetCampaignTaskManagerInstance();
					
					if (tManager)
					{
						SCR_CampaignTask task = tManager.GetTask(this, faction, SCR_CampaignTaskType.CAPTURE);
						
						if (task)
							tManager.FailTask(task);
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
	//! Returns a random base in range that suits the filter. Filter = allowed types.
	SCR_CampaignBase GetRandomBaseInRange(CampaignBaseType filter, array<SCR_CampaignBase> excludeBases = null)
	{
		if (!m_aInRangeOf)
			return null;
		
		array<SCR_CampaignBase> filteredBases = new array<SCR_CampaignBase>();
		
		foreach (SCR_CampaignBase base: m_aInRangeOf)
		{
			if (filter & base.GetType() || filter == base.GetType())
			{
				if (!excludeBases)
					filteredBases.Insert(base);
				else
				{
					bool exclude = false;
					foreach (SCR_CampaignBase excludeBase: excludeBases)
					{
						if (base == excludeBase)
						{
							exclude = true;
							break;
						}
					}
					if (!exclude)
						filteredBases.Insert(base);
				}
			}
		}
		
		if (filteredBases.Count() <= 0)
			return null;
		
		return filteredBases[Math.RandomInt(0, filteredBases.Count())];
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
	//! Returns the starting faction
	SCR_CampaignFaction GetStartingFaction()
	{
		if (m_sStartingBaseOwner.IsEmpty())
			return null;
		
		ChimeraGame chimeraGame = GetGame();
		if (!chimeraGame)
			return null;
		
		// Need to find faction manager to find the faction object by it's name
		return SCR_CampaignFactionManager.GetInstance().GetCampaignFactionByKey(m_sStartingBaseOwner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the owning faction
	SCR_CampaignFaction GetOwningFaction()
	{
		return m_OwningFaction;
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
	void FlashBaseIcon(float remainingTime = ICON_FLASH_DURATION, Faction faction = null, bool changeToDefault = false)
	{
		GetGame().GetCallqueue().Remove(FlashBaseIcon);
		
		if (!faction)
			return;
		
		Faction iconFaction;
		
		if (changeToDefault)
			iconFaction = m_OwningFaction;
		else
			iconFaction = faction;
		
		if (GetBaseUI())
			GetBaseUI().SetBaseIconFactionColor(iconFaction);
		
		if ((remainingTime - ICON_FLASH_PERIOD) > 0 || !changeToDefault)
			GetGame().GetCallqueue().CallLater(FlashBaseIcon, ICON_FLASH_PERIOD * 1000, false, remainingTime - ICON_FLASH_PERIOD, faction, !changeToDefault)
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
	//! In case this base is blacklisted in server config, disable all its functionality
	void DisableBase()
	{
		SCR_CampaignBaseManager.UnregisterBase(this);
		ClearEventMask(EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.DIAG);
		ClearFlags(EntityFlags.ACTIVE, true);
		
		IEntity ent = this;
		
		// Hide all map descriptors
		while (ent)
		{
			MapDescriptorComponent mapDesc = MapDescriptorComponent.Cast(ent.FindComponent(MapDescriptorComponent));
			
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
	void InitializeBase()
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
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
			
			// Repair depot
			if (!m_sSlotNameRepair.IsEmpty())
			{
				m_SlotRepair = SCR_SiteSlotEntity.Cast(world.FindEntityByName(m_sSlotNameRepair));
				
				if (!m_SlotRepair)
					Print("Unable to register Slot " + m_sSlotNameRepair + " in " + baseName + "!", LogLevel.WARNING);
				else
					FilterSlots(m_SlotRepair);
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
		
		LinkBases();
		
		if (RplSession.Mode() != RplMode.Dedicated)
			MapSetup();
		
		if (!IsProxy())
		{
			if (campaign.AreBasesFlipped() && !campaign.IsBackendStateLoaded())
				switch (m_sStartingBaseOwner)
				{
					case SCR_GameModeCampaignMP.FACTION_OPFOR: {m_sStartingBaseOwner = SCR_GameModeCampaignMP.FACTION_BLUFOR; break;};
					case SCR_GameModeCampaignMP.FACTION_BLUFOR: {m_sStartingBaseOwner = SCR_GameModeCampaignMP.FACTION_OPFOR; break;};
				}
			
			if (!m_sStartingBaseOwner.IsEmpty() && (campaign.AllowStartingBaseOwners() || GetType() == CampaignBaseType.MAIN))
			{
				SetStartingBaseOwner();
				
				if (m_eType == CampaignBaseType.MAIN)
					m_BuildingsFaction = GetOwningFaction();
			}
		
			if (GetType() == CampaignBaseType.MAIN || GetType() == CampaignBaseType.MAJOR)
			{
				GetGame().GetCallqueue().CallLater(ReinforcementsTimer, SCR_GameModeCampaignMP.REINFORCEMENTS_CHECK_PERIOD, true, false);
				GetGame().GetCallqueue().CallLater(CheckIsOverrun, OVERRUN_CHECK_FREQUENCY, true);
			}
			
			int defendersSpawnTimeout = 250;
			
			if (RplSession.Mode() == RplMode.Dedicated)
				defendersSpawnTimeout = 5000;
			
			// Spawn defenders at HQ (longer timeout for dedicated in case the state is loaded from backend			
			if (m_OwningFaction && GetType() == CampaignBaseType.MAIN && GetGame().AreGameFlagsSet( EGameFlags.SpawnAI ))
				GetGame().GetCallqueue().CallLater(SpawnDefenders, defendersSpawnTimeout);
			
			if (GetType() == CampaignBaseType.SMALL)
				GetGame().GetCallqueue().CallLater(AddRespawnTickets, SCR_GameModeCampaignMP.RESPAWN_TICKETS_REPLENISH_PERIOD, true, SCR_GameModeCampaignMP.RESPAWN_TICKETS_REPLENISH_AMOUNT, false);
		}
		else
		{
			if (m_bIsHQ)
			{
				SCR_CampaignFaction HQFaction = SCR_CampaignFactionManager.GetInstance().GetCampaignFactionByIndex(m_iOwningFaction);
				
				if (HQFaction)
					HQFaction.SetMainBase(this);
			}
		}
		
		if (GetType() != CampaignBaseType.RELAY)
			RegisterDepots();
		
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		baseManager.OnBaseInitialized(this);
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
			
			if (name == m_sSlotNameRepair)
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
	
	//****************//
	//OVERRIDE METHODS//
	//****************//
	
	//------------------------------------------------------------------------------------------------
	override void LoadState(SCR_CampaignBaseStruct baseStruct)
	{
		if (!baseStruct)
			return;
		
		if (m_iOwningFaction != baseStruct.GetOwningFaction())
		{
			m_iOwningFaction = baseStruct.GetOwningFaction();
			OnOwningFactionChanged();
		}
		
		SCR_CampaignFaction faction = SCR_CampaignFactionManager.GetInstance().GetCampaignFactionByIndex(m_iOwningFaction);
		m_BuildingsFaction = SCR_CampaignFactionManager.GetInstance().GetCampaignFactionByIndex(baseStruct.GetBuildingsFaction());
		AddRespawnTickets(baseStruct.GetRespawnTickets() - m_iRespawnTickets, true);
		
		if (faction && GetType() == CampaignBaseType.MAIN)
			faction.SetMainBase(this);
		
		AddSupplies(baseStruct.GetSupplies() - GetSupplies());
		
		if (!m_bDefendersSpawned && GetType() == CampaignBaseType.MAIN && GetGame().AreGameFlagsSet( EGameFlags.SpawnAI ))
			SpawnDefenders();
		
		FillAssetPools(true);
		
		// Inform the base that it has some services saved so it can spawn them along with other buildings
		m_bLoadStateVehicleDepotBuilt = baseStruct.GetVehicleDepotBuilt();
		m_bLoadStateArmoryBuilt = baseStruct.GetArmoryBuilt();
		
		// Building spawning has already been processed, take care of saved structures here instead
		if (m_bBuildingsSpawned && GetType() == CampaignBaseType.SMALL && !m_bPrebuilt)
		{
			if (m_bLoadStateVehicleDepotBuilt)
				SpawnComposition(ECampaignCompositionType.REPAIR);
			
			if (m_bLoadStateArmoryBuilt)
				SpawnComposition(ECampaignCompositionType.ARMORY);
		}
		
		IEntity armoryService = GetBaseService(ECampaignServicePointType.ARMORY, true);
		
		if (armoryService)
		{
			SCR_CampaignArmoryComponent armory = SCR_CampaignArmoryComponent.Cast(armoryService.FindComponent(SCR_CampaignArmoryComponent));
			
			if (armory)
				armory.RespawnAllWeapons(faction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void StoreState(out SCR_CampaignBaseStruct baseStruct)
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();
		
		if (!fManager)
			return;
		
		baseStruct.SetBaseID(GetBaseID());
		baseStruct.SetOwningFaction(m_iOwningFaction);
		baseStruct.SetBuildingsFaction(fManager.GetFactionIndex(GetBuildingsFaction()));
		baseStruct.SetSupplies(GetSupplies());
		baseStruct.SetVehicleDepotBuilt(GetBaseService(ECampaignServicePointType.REPAIR_DEPOT, true) != null);
		baseStruct.SetArmoryBuilt(GetBaseService(ECampaignServicePointType.ARMORY, true) != null);
		baseStruct.SetRespawnTickets(m_iRespawnTickets);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteInt(m_iOwningFaction);
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
			OnOwningFactionChanged();
		
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
		IEntity child = GetChildren();
		while(child)
		{
			if (child.Type() == SCR_SpawnPoint)
				m_SpawnPoint = SCR_SpawnPoint.Cast(child);
				
			child = child.GetSibling();
		}	
		
		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (campaign && !IsProxy() && m_bEnabled)
			InitializeBase();
		else if (!m_bEnabled)
			DisableBase();		// Hide unused MapDescriptors
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (!IsProxy())
		{
			// Handle respawn cooldown on server
			if (!m_bRespawnBecameAvailable && m_OwningFaction && Replication.Time() >= m_fRespawnAvailableSince)
			{
				// Hide map UI timer for hosting player, skip for dedicated
				if (RplSession.Mode() != RplMode.Dedicated)
					HandleMapInfo();
				else
					m_bRespawnBecameAvailable = true;
				
				// Allow respawn
				SetSpawnPointFaction(m_OwningFaction);
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
			if (m_OwningFaction)
				color = m_OwningFaction.GetFactionColor();
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
		
		if (!IsProxy())
		{
			SCR_MissionHeaderCampaign header = SCR_MissionHeaderCampaign.Cast(GetGame().GetMissionHeader());
			
			if (header)
			{
				// Disable this base if it's been blacklisted in header
				array<ref SCR_CampaignCustomBase> baseList = header.m_aCampaignCustomBaseList;
				bool whitelist = header.m_bCustomBaseWhitelist;
				bool found = false;
				bool isHQWest = !header.m_sCustomHQWest.IsEmpty() && GetName() == header.m_sCustomHQWest;
				bool isHQEast = !header.m_sCustomHQEast.IsEmpty() && GetName() == header.m_sCustomHQEast;
				
				if (!isHQWest && !isHQEast)	// Skip check for predefined custom HQs
				{
					foreach (SCR_CampaignCustomBase base: baseList)
					{
						string name = base.GetBaseName();
					
						if (!name.IsEmpty() && name == GetName())
						{
							found = true;
							
							if (whitelist)
								m_bEnabled = true;
							else
								m_bEnabled = false;
							
							break;
						}
					}
				
					if (!found && whitelist)
						m_bEnabled = false;
					
					if (!m_bEnabled)
					{
						DisableBase();
						return;
					}
				}
				
				// Set up custom HQs defined in header
				if (isHQWest || isHQEast)
				{
					if (GetType() == CampaignBaseType.MAIN || GetType() == CampaignBaseType.MAJOR)
					{
						m_bIsHQ = true;
						
						if (isHQWest)
							m_eStartingBaseOwner = SCR_ECampaignBaseOwner.BLUFOR;
						else
							m_eStartingBaseOwner = SCR_ECampaignBaseOwner.OPFOR;
					}
					else
						Print("Attempting to set up custom HQ at " + GetBaseName() + " - must be a Main or Major base, skipping...", LogLevel.ERROR);
				}
				else
				{
					// Check if this HQ is about to be overriden later, if not, keep this one
					if (GetType() == CampaignBaseType.MAIN)
					{
						BaseWorld world = GetGame().GetWorld();
						SCR_CampaignBase base;
						
						if (world)
						{
							switch (m_eStartingBaseOwner)
							{
								case SCR_ECampaignBaseOwner.BLUFOR:
								{
									if (!header.m_sCustomHQWest.IsEmpty())
										base = SCR_CampaignBase.Cast(world.FindEntityByName(header.m_sCustomHQWest));
									
									break;
								}
							
								case SCR_ECampaignBaseOwner.OPFOR:
								{
									if (!header.m_sCustomHQEast.IsEmpty())
										base = SCR_CampaignBase.Cast(world.FindEntityByName(header.m_sCustomHQEast));
									
									break;
								}
							}
							
							if (base && (base.GetType() == CampaignBaseType.MAIN || base.GetType() == CampaignBaseType.MAJOR))
							{
							}
							else
							{
								m_bIsHQ = true;
							}
						}
					}
				}
			}
			else
				if (GetType() == CampaignBaseType.MAIN)
					m_bIsHQ = true;
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
	
	//------------------------------------------------------------------------------------------------
	string GetBaseName()
	{
		return m_sBaseName;
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
	REPAIR,
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
	REPAIR,
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