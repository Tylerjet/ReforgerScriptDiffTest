[EntityEditorProps(category: "GameScripted/Campaign", description: "Campaign base manager.", color: "0 0 255 255")]
class SCR_CampaignBaseManagerClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBaseManager : GenericEntity
{
	// Bases components update related
	static const float COMPONENTS_UPDATE_TIME = 5; //In seconds
	protected float m_fComponentUpdateTime = 0;
	protected int m_iCurrentBaseComponentUpdateIndex = 0;
	
	// This array is always emptied upon creation of SCR_CampaignBaseManager -> See constructor
	static ref array<SCR_CampaignBase> s_aBases = new ref array<SCR_CampaignBase>();
	static ref ScriptInvoker s_OnAllBasesInitialized = new ref ScriptInvoker();
	
	protected static SCR_CampaignBaseManager s_Instance = null;
	protected static float s_fTickTime;
	
	protected static const float ITERATION_TIME = 1;
	
	protected float m_fCurrentTickTime = 0;
	protected int m_iCurrentBaseIndex = 0;
	protected int m_iInitializedBasesCount = 0;
	
	protected ref map<CampaignBaseType, ref array<SCR_CampaignBase>> m_mPrefilteredBases = new ref map<CampaignBaseType, ref array<SCR_CampaignBase>>();
	protected bool m_bBasesInitialized = false;
	
	protected ref SCR_CampaignStruct m_CampaignInfo;
	protected bool m_bShowLinks = true;
	protected MapItem m_MobileMapItemBlufor;
	protected MapItem m_MobileMapItemOpfor;
	
#ifdef ENABLE_DIAG
	protected bool m_bCapturedRelays = false;
#endif
#ifdef ENABLE_CAMPAIGN_CONFIGURATOR
	//Doesn't work as an attribute?
	ResourceName m_ConfiguratorMapLayout = "{87D055441739D9C9}UI/layouts/Map/MapCampaignBasesGraph.layout";
	
	protected Widget m_ConfiguratorWidget;
	
	//------------------------------------------------------------------------------------------------
	void OnMapCloseConfigurator()
	{
		if (m_ConfiguratorWidget)
			m_ConfiguratorWidget.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapOpenConfigurator(MapConfiguration config)
	{
		if (!SCR_PlayerController.GetLocalControlledEntity())
			return;
		
		if (m_ConfiguratorWidget)
		{
			m_ConfiguratorWidget.SetVisible(true);
			return;
		}
		
		m_ConfiguratorWidget = GetGame().GetWorkspace().CreateWidgets(m_ConfiguratorMapLayout);
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;
		
		Widget mapWidget = mapEntity.GetMapConfig().RootWidgetRef;
		if (!mapWidget)
			return;
		
		m_ConfiguratorWidget.SetZOrder(mapWidget.GetZOrder() + 1);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateBasesSettings()
	{
		if (!m_ConfiguratorWidget)
			return;
		
		float relaysRange, basesRange;
		
		EditBoxWidget editBoxWidget = EditBoxWidget.Cast(m_ConfiguratorWidget.FindAnyWidget("RelaysRange"));
		if (editBoxWidget)
			relaysRange = editBoxWidget.GetText().ToFloat();
		
		editBoxWidget = EditBoxWidget.Cast(m_ConfiguratorWidget.FindAnyWidget("BasesRange"));
		if (editBoxWidget)
			basesRange = editBoxWidget.GetText().ToFloat();
		
		array<SCR_CampaignBase> bases;
		int count = GetFilteredBases(CampaignBaseType.SMALL | CampaignBaseType.MAJOR | CampaignBaseType.MAIN, bases);
		
		for (int i = 0; i < count; i++)
		{
			bases[i].SetSignalRange(basesRange);
		}
		
		count = GetFilteredBases(CampaignBaseType.RELAY, bases);
		for (int i = 0; i < count; i++)
		{
			bases[i].SetSignalRange(relaysRange);
		}
		
		bases = GetBases();
		count = bases.Count();
		
		for (int i = 0; i < count; i++)
		{
			bases[i].ClearLinks();
		}
		
		for (int i = 0; i < count; i++)
		{
			bases[i].LinkBases();
			bases[i].MapSetup();
			bases[i].HandleMapLinks(true);
		}
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	static SCR_CampaignBase FindClosestBase(vector position)
	{
		array<SCR_CampaignBase> bases = SCR_CampaignBaseManager.GetInstance().GetBases();
		if (!bases)
			return null;
		
		int closestBaseIndex = -1;
		float closestBaseDistance = float.MAX;
		for (int i = bases.Count() - 1; i >= 0; i--)
		{
			float distance = vector.DistanceSq(bases[i].GetOrigin(), position);
			if (distance < closestBaseDistance)
			{
				closestBaseDistance = distance;
				closestBaseIndex = i;
			}
		}
		
		if (closestBaseIndex != -1)
			return bases[closestBaseIndex];
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateBasesSignalCoverage(SCR_CampaignFaction faction = null, bool byMobileHQ = false)
	{
		//Print("Updating");
		if (!s_aBases)
			return;
		
		// Faction was not specified, do it for both
		if (!faction)
		{
			SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();
			
			if (!fManager)
				return;
			
			UpdateBasesSignalCoverage(fManager.GetCampaignFactionByKey(SCR_GameModeCampaignMP.FACTION_BLUFOR), byMobileHQ);
			UpdateBasesSignalCoverage(fManager.GetCampaignFactionByKey(SCR_GameModeCampaignMP.FACTION_OPFOR), byMobileHQ);
			
			return;
		}
		
		//PrintFormat("-------------------------- UPDATE SIGNAL FOR %1", faction.GetFactionKey());
		
		SCR_CampaignBase HQ = faction.GetMainBase();
		
		if (!HQ)
			return;
		
		//PrintFormat("HQ identified as %1", HQ.GetBaseName());
		
		IEntity mobileHQ = faction.GetDeployedMobileAssembly();
		array<SCR_CampaignBase> basesInRangeOfMobileHQ = {};
		MapItem mobileMapItem;
		// Mobile HQ is deployed, grab the covered bases
		if (mobileHQ)
		{
			SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
			SCR_CampaignMobileAssemblyComponent comp;
			
			if (campaign)
				comp = SCR_CampaignMobileAssemblyComponent.Cast(Replication.FindItem(campaign.GetDeployedMobileAssemblyID(faction.GetFactionKey())));
			
			if (comp)
			{
				basesInRangeOfMobileHQ = comp.GetBasesInRange();
				SCR_MapDescriptorComponent desc = SCR_MapDescriptorComponent.Cast(mobileHQ.FindComponent(SCR_MapDescriptorComponent));
				if(!desc)
					return;
				mobileMapItem = desc.Item();
				comp.SetMapItem(desc.Item());
			}
			/*Print("Following bases are in range of mobile HQ:");
			
			foreach (SCR_CampaignBase base: basesInRangeOfMobileHQ)
				Print(base.GetBaseName());*/
		}
		else
		{
			//Print("Mobile HQ not deployed");
		}
		
		bool mobileHQProcessed = false;
		array<SCR_CampaignBase> coveredBases = {};			// Bases stored here are in radio range
		int basesCnt = s_aBases.Count();
		
		for (int i = 0; i < basesCnt; i++)
		{
			SCR_CampaignBase base = s_aBases[i];
			
			if (coveredBases.Contains(base))
			{
				//PrintFormat("Skipping %1...", base.GetBaseName());
				continue;
			}
			
			//PrintFormat("Processing %1...", base.GetBaseName());
			bool covered = false;
			
			if (base == HQ)
				covered = true;
			else
			{
				// Check if this base is able to send to and receive from any of other bases within signal range
				foreach (SCR_CampaignBase coveredBase: coveredBases)
					if (coveredBase.GetOwningFaction() == faction && base.GetBasesInRangeSimple().Contains(coveredBase) && base.GetBasesInRangeSimple(true).Contains(coveredBase))
					{
						covered = true;
						break;
					}
			}
			
			// Is in signal range
			if (covered)
			{
				//Print("Covered!");
				coveredBases.Insert(base);
				
				// If this base is within signal range and within mobile HQ range, add all bases in mobile HQ range to covered list
				if (!mobileHQProcessed && basesInRangeOfMobileHQ.Contains(base) && base.GetOwningFaction() == faction)
				{
					//Print("Mobile HQ is extending the signal to the following bases:");
					mobileHQProcessed = true;
					
					foreach (SCR_CampaignBase baseInRangeOfMobileHQ: basesInRangeOfMobileHQ)
					{
						if (!coveredBases.Contains(baseInRangeOfMobileHQ))
						{
							coveredBases.Insert(baseInRangeOfMobileHQ);
							//Print(baseInRangeOfMobileHQ.GetBaseName());
						}
					}
				}
				
				// Recheck all previous bases in case they are connected via this base
				i = -1;
				//Print("Rechecking!");
			}
		}
		
		// All relevant bases checked; apply changes
		foreach (SCR_CampaignBase base : s_aBases)
		{
			if (!base)
				continue;
			
			base.SetIsBaseInFactionRadioSignal(faction.GetFactionKey(), coveredBases.Contains(base), byMobileHQ);
		}
		
		foreach (SCR_CampaignBase base : s_aBases)
		{
			if (!base)
				continue;
			
			base.HandleMapLinks();
		}
		
		//Print("Done!");
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsEntityInFactionRadioSignal(IEntity entity, Faction faction)
	{
		if (!entity)
			return false;
		
		int count = s_aBases.Count();
		
		// Check if the entity is within range of deployed mobile HQ which is abe to relay the signal
		SCR_CampaignFaction factionC = SCR_CampaignFaction.Cast(faction);
		
		if (factionC)
		{
			IEntity mobileHQ = factionC.GetDeployedMobileAssembly();
			
			if (mobileHQ && mobileHQ != entity)
			{
				SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
				SCR_CampaignMobileAssemblyComponent HQComp = SCR_CampaignMobileAssemblyComponent.Cast(Replication.FindItem(campaign.GetDeployedMobileAssemblyID(faction.GetFactionKey())));
				
				if (HQComp && HQComp.IsInRadioRange())
				{
					IEntity truck = mobileHQ.GetParent();
					
					if (truck)
					{
						BaseRadioComponent comp = BaseRadioComponent.Cast(truck.FindComponent(BaseRadioComponent));
						
						if (comp)
						{
							if (vector.DistanceSq(entity.GetOrigin(), mobileHQ.GetOrigin()) < Math.Pow(comp.GetRange(), 2))
								return true;
						}
					}
				}
			}
		}
		
		for (int i = 0; i < count; i++)
		{
			if (faction != s_aBases[i].GetOwningFaction())
				continue;
			
			if (s_aBases[i].GetIsEntityInMyRange(entity) && s_aBases[i].IsBaseInFactionRadioSignal(SCR_CampaignFaction.Cast(faction)))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	static notnull SCR_CampaignBaseManager GetInstance()
	{
		if (!s_Instance)
			GetGame().SpawnEntity(SCR_CampaignBaseManager, GetGame().GetWorld());
		
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFilteredBases(CampaignBaseType filter, out array<SCR_CampaignBase> bases)
	{
		if (!m_mPrefilteredBases)
			m_mPrefilteredBases = new ref map<CampaignBaseType, ref array<SCR_CampaignBase>>();
		
		array<SCR_CampaignBase> prefilteredBases = m_mPrefilteredBases.Get(filter);
		if (!prefilteredBases)
		{
			prefilteredBases = new ref array<SCR_CampaignBase>();
			for (int i = 0; i < s_aBases.Count(); i++)
			{
				CampaignBaseType baseType = s_aBases[i].GetType();
				if (filter & baseType)
				{
					prefilteredBases.Insert(s_aBases[i]);
				}
			}
			
			m_mPrefilteredBases.Insert(filter, prefilteredBases);
			bases = prefilteredBases;
			return bases.Count();
		}
		
		bases = prefilteredBases;
		return bases.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	static array<SCR_CampaignBase> GetBases()
	{
		return s_aBases;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_CampaignBase FindBaseByID(int baseID, SCR_CampaignBase exclude = null)
	{
		if (!s_aBases)
			return null;
		
		for (int i = s_aBases.Count() - 1; i >= 0; i--)
		{
			if (exclude)
			{
				if (s_aBases[i] == exclude)
					continue;
			}
			
			int otherBaseID = s_aBases[i].GetBaseID();
			if (otherBaseID == baseID)
			{
				return s_aBases[i];
			}
		}
		
		return null;
	}
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	static SCR_CampaignBase _WB_FindBaseByID(int baseID, SCR_CampaignBase exclude = null)
	{
		if (!s_aBases)
			return null;
		
		for (int i = s_aBases.Count() - 1; i >= 0; i--)
		{
			if (exclude && s_aBases[i] == exclude)
				continue;
			
			IEntitySource otherBaseSource = s_aBases[i].GetSource();
			if (!otherBaseSource)
				continue;
			
			int otherBaseID;
			otherBaseSource.Get(SCR_CampaignBase.VARNAME_BASE_ID, otherBaseID);
			
			if (otherBaseID == baseID)
				return s_aBases[i];
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	static void HandleBaseID(notnull SCR_CampaignBase base, notnull IEntitySource src)
	{
		if (!s_aBases)
			s_aBases = new ref array<SCR_CampaignBase>();
		
		int baseID = base.GetBaseID();
		
		if (baseID == -1)
			baseID++;
		
		SCR_CampaignBase otherBase = _WB_FindBaseByID(baseID, base);
		while (otherBase)
		{
			baseID++;
			otherBase = _WB_FindBaseByID(baseID, base);
		}
		
		if (baseID != base.GetBaseID())
			src.Set(base.VARNAME_BASE_ID, baseID);
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	static void UnregisterBase(SCR_CampaignBase base)
	{
		if (!s_aBases)
			return;
		
#ifdef ENABLE_BUILDING_DEBUG
		Print("#");
		Print(base.GetName());
		Print("UNREGISTER");
#endif
		
		s_aBases.RemoveItem(base);
		
		// Check for init here too since delayed RplLoads can cause this method to be called later
		if (GetGame().InPlayMode() && GetInstance())
			GetInstance().CheckBasesInitialized();
	}
	
	//------------------------------------------------------------------------------------------------
	bool AllBasesInitialized()
	{
		return m_bBasesInitialized;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckBasesInitialized()
	{
		if (m_bBasesInitialized)
			return;
		
		if (m_iInitializedBasesCount != 0 && m_iInitializedBasesCount == s_aBases.Count())
		{
			m_bBasesInitialized = true;
			
			if (s_OnAllBasesInitialized)
			{
#ifdef ENABLE_BUILDING_DEBUG
				Print("ALL BASES INITIALIZED");
#endif
				UpdateBasesSignalCoverage();
				s_OnAllBasesInitialized.Invoke();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePrefilteredBases(SCR_CampaignBase base)
	{
		CampaignBaseType baseType = base.GetType();
		if (m_mPrefilteredBases)
		{
			array<SCR_CampaignBase> prefilteredBases = m_mPrefilteredBases.Get(baseType);
			if (prefilteredBases)
			{
				prefilteredBases.Insert(base);
			}
			else
			{
				ref array<SCR_CampaignBase> bases = new ref array<SCR_CampaignBase>();
				bases.Insert(base);
				m_mPrefilteredBases.Insert(baseType, bases);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBaseInitialized(SCR_CampaignBase base)
	{
		m_iInitializedBasesCount++;
		if (!s_aBases)
			return;
		
#ifdef ENABLE_BUILDING_DEBUG
		Print("-");
		Print(base.GetName());
		Print(m_iInitializedBasesCount);
		Print(s_aBases.Count());
#endif
		
		CheckBasesInitialized();
	}
	
	//------------------------------------------------------------------------------------------------
	static void ClearNulls()
	{
		if (!s_aBases)
			return;
		
		for (int i = s_aBases.Count() - 1; i >= 0; i--)
		{
			if (!s_aBases[i])
				s_aBases.Remove(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static void RegisterBase(SCR_CampaignBase base)
	{
		// We call this first, to make sure we are then registering into correct s_aBases array, an emptied one
		SCR_CampaignBaseManager baseManager = GetInstance();
		
		if (!s_aBases)
			return;
		
		ClearNulls();
		
		if (s_aBases.Find(base) != -1)
			return;
		
		s_aBases.Insert(base);
		
		baseManager.UpdatePrefilteredBases(base);
		
		int count = s_aBases.Count();
		
#ifdef ENABLE_BUILDING_DEBUG
		Print("------");
		Print(base.GetName());
		Print("REGISTER");
#endif
		
		if (count != 0)
			s_fTickTime = ITERATION_TIME / count;
		else
			s_fTickTime = ITERATION_TIME;
	}
	
	//------------------------------------------------------------------------------------------------
	void CaptureAllRelays()
	{
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_RespawnSystemComponent.GetLocalPlayerFaction());
		if (!faction)
			return;
		
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		
		for (int i = 0; i < s_aBases.Count(); i++)
		{
			if (s_aBases[i].GetType() != CampaignBaseType.RELAY)
				continue;
			
			if (s_aBases[i].BeginCapture(faction, playerID))
				s_aBases[i].FinishCapture(playerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void StoreBasesStates(out notnull array<ref SCR_CampaignBaseStruct> outEntries)
	{
		if (!s_aBases)
			return;
		
		for (int i = s_aBases.Count() - 1; i >= 0; i--)
		{
			SCR_CampaignBaseStruct struct = new SCR_CampaignBaseStruct();
			s_aBases[i].StoreState(struct);
			outEntries.Insert(struct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadBasesStates(notnull array<ref SCR_CampaignBaseStruct> entries)
	{
		if (!s_aBases)
			return;
		
		int basesCnt = s_aBases.Count();
		int infoCnt = entries.Count();
		
		if (infoCnt != basesCnt)
			return;
		
		for (int i = 0; i < infoCnt; i++)
		{
			SCR_CampaignBaseStruct baseInfo = entries[i];
			
			if (!baseInfo)
				continue;
			
			int baseID = baseInfo.GetBaseID();
			SCR_CampaignBase base = FindBaseByID(baseID);
			
			if (!base)
				continue;
			
			base.LoadState(baseInfo);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void BasesComponentsUpdate(float timeSlice)
	{
		m_fComponentUpdateTime -= timeSlice;
		int basesCount = s_aBases.Count();
		
		if (m_fComponentUpdateTime < 0)
		{
			if (m_iCurrentBaseComponentUpdateIndex >= basesCount)
				m_iCurrentBaseComponentUpdateIndex = 0;
			
			m_fComponentUpdateTime = COMPONENTS_UPDATE_TIME;
		}
		
		if (m_iCurrentBaseComponentUpdateIndex < basesCount)
		{
			s_aBases[m_iCurrentBaseComponentUpdateIndex].UpdateComponents(COMPONENTS_UPDATE_TIME);
			m_iCurrentBaseComponentUpdateIndex++;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetShowMapLinks()
	{
		return m_bShowLinks;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateShowMapLinks(SCR_CheckboxComponent checkboxComponent, bool value)
	{
		m_bShowLinks = value;
		for (int i = s_aBases.Count() - 1; i >= 0; i--)
		{
			s_aBases[i].ShowMapLinks(value);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
#ifdef ENABLE_CAMPAIGN_CONFIGURATOR
		OnMapOpenConfigurator(config);
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapClose()
	{
#ifdef ENABLE_CAMPAIGN_CONFIGURATOR
		OnMapCloseConfigurator();
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!s_aBases || s_aBases.Count() <= 0)
			return;
		
		BasesComponentsUpdate(timeSlice);
		
		m_fCurrentTickTime += timeSlice;
		
		while (m_fCurrentTickTime >= s_fTickTime)
		{
			m_iCurrentBaseIndex++;
			
			if (m_iCurrentBaseIndex >= s_aBases.Count())
				m_iCurrentBaseIndex = 0;
			
			s_aBases[m_iCurrentBaseIndex].CheckIsPlayerInside();
			
			// TICK
			m_fCurrentTickTime -= s_fTickTime;
		}
		
		#ifdef ENABLE_DIAG		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_CAPTURE_RELAYS))
		{
			if (!m_bCapturedRelays)
			{
				m_bCapturedRelays = true;
				CaptureAllRelays();
			}
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
	}

	//------------------------------------------------------------------------------------------------
	void SCR_CampaignBaseManager(IEntitySource src, IEntity parent)
	{
		// We need s_aBases to be empty
		if (!s_aBases)
			s_aBases = new ref array<SCR_CampaignBase>();
		else
			s_aBases.Clear();
		
		if (!s_Instance)
			s_Instance = this;
		
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
		
		//DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_DETECT_RELAYS, "", "Detect relays", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_CAPTURE_RELAYS, "", "Capture relays", "Conflict");
		
		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignBaseManager()
	{
		if (s_Instance == this)
			s_Instance = null;
		
		m_CampaignInfo = null;
	}

};