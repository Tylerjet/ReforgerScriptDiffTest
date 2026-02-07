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
	static int s_iValidBasesCount;
	
	// This array is always emptied upon creation of SCR_CampaignBaseManager -> See constructor
	static ref array<SCR_CampaignBase> s_aBases = new ref array<SCR_CampaignBase>();
	static ref ScriptInvoker s_OnAllBasesInitialized = new ref ScriptInvoker();
	
	protected static SCR_CampaignBaseManager s_Instance = null;
	protected static float s_fTickTime;
	
	protected static const float ITERATION_TIME = 1;
	protected static const int MAX_HQ_SELECTION_ITERATIONS = 10;
	protected static const float CP_AVG_DISTANCE_TOLERANCE = 0.25;
	
	protected float m_fCurrentTickTime = 0;
	protected int m_iCurrentBaseIndex = 0;
	protected int m_iInitializedBasesCount = 0;
	
	protected ref map<CampaignBaseType, ref array<SCR_CampaignBase>> m_mPrefilteredBases = new ref map<CampaignBaseType, ref array<SCR_CampaignBase>>();
	protected bool m_bBasesInitialized = false;
	
	protected ref array<SCR_CampaignBase> m_aControlPoints = {};
	
	protected bool m_bShowLinks = true;
	protected MapItem m_MobileMapItemBlufor;
	protected MapItem m_MobileMapItemOpfor;
	protected SCR_CampaignBase m_WestHQ;
	protected SCR_CampaignBase m_EastHQ;
	protected bool m_bIsHQSetupDone;
	
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
		int count = GetFilteredBases(CampaignBaseType.base, bases);
		
		for (int i = 0; i < count; i++)
		{
			bases[i].SetSignalRange(basesRange);
		}
		
		count = GetFilteredBases(CampaignBaseType.RELAY, bases);
		for (int i = 0; i < count; i++)
		{
			bases[i].SetSignalRange(relaysRange);
		}
		
		count = s_aBases.Count();
		
		for (int i = 0; i < count; i++)
		{
			s_aBases[i].ClearLinks();
		}
		
		for (int i = 0; i < count; i++)
		{
			s_aBases[i].LinkBases();
			s_aBases[i].MapSetup();
			s_aBases[i].HandleMapLinks(true);
		}
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	static SCR_CampaignBase FindClosestBase(vector position)
	{
		if (!s_aBases)
			return null;
		
		int closestBaseIndex = -1;
		float closestBaseDistance = float.MAX;
		
		for (int i = s_aBases.Count() - 1; i >= 0; i--)
		{
			if (!s_aBases[i].GetIsEnabled())
				continue;
			
			float distance = vector.DistanceSq(s_aBases[i].GetOrigin(), position);
			
			if (distance < closestBaseDistance)
			{
				closestBaseDistance = distance;
				closestBaseIndex = i;
			}
		}
		
		if (closestBaseIndex != -1)
			return s_aBases[closestBaseIndex];
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateBasesSignalCoverage(SCR_CampaignFaction faction = null, bool byMobileHQ = false)
	{
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
		
		SCR_CampaignBase HQ = faction.GetMainBase();
		
		if (!HQ)
			return;
		
		IEntity mobileHQ = faction.GetDeployedMobileAssembly();
		SCR_CampaignMobileAssemblyComponent comp;
		array<SCR_CampaignBase> basesInRangeOfMobileHQ = {};
		
		// Mobile HQ is deployed, grab the covered bases
		if (mobileHQ)
		{
			SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
			
			if (campaign)
				comp = SCR_CampaignMobileAssemblyComponent.Cast(Replication.FindItem(campaign.GetDeployedMobileAssemblyID(faction.GetFactionKey())));
			
			if (comp)
			{
				basesInRangeOfMobileHQ = comp.GetBasesInRange();
				SCR_MapDescriptorComponent desc = SCR_MapDescriptorComponent.Cast(mobileHQ.FindComponent(SCR_MapDescriptorComponent));
				
				if (desc)
					comp.SetMapItem(desc.Item());
			}
		}
		
		bool mobileHQProcessed = false;
		array<SCR_CampaignBase> coveredBases = {};			// Bases stored here are in radio range
		int basesCnt = s_aBases.Count();
		int coveredBasesWithoutMHQ;
		bool isMHQConnected = false;
		SCR_CampaignBase base;
		bool covered;
		
		for (int i = 0; i < basesCnt; i++)
		{
			base = s_aBases[i];
			
			if (!coveredBases.Contains(base))
			{
				covered = false;
				
				if (base == HQ)
					covered = true;
				else
				{
					// Check if this base is able to send to and receive from any of other bases within signal range
					foreach (SCR_CampaignBase coveredBase: coveredBases)
						if (coveredBase.GetOwningFaction() == faction && coveredBase.GetBasesInRangeSimple().Contains(base))
						{
							covered = true;
							break;
						}
				}
				
				// Is in signal range
				if (covered)
				{
					coveredBases.Insert(base);
					
					if (!isMHQConnected && basesInRangeOfMobileHQ.Contains(base) && base.GetOwningFaction() == faction)
						isMHQConnected = true;
					
					// Recheck all previous bases in case they are connected via this base
					i = -1;
				}
			}
			
			// All bases have been processed, take care of MHQ
			if (i == basesCnt - 1 && isMHQConnected && !mobileHQProcessed)
			{
				mobileHQProcessed = true;
				coveredBasesWithoutMHQ = coveredBases.Count();
				
				foreach (SCR_CampaignBase baseInRangeOfMobileHQ: basesInRangeOfMobileHQ)
				{
					if (!coveredBases.Contains(baseInRangeOfMobileHQ))
					{
						// This base is covered by MHQ, recalculate to take care of additional links
						coveredBases.Insert(baseInRangeOfMobileHQ);
						i = -1;
					}
				}
			}
		}
		
		// All relevant bases checked; apply changes
		foreach (SCR_CampaignBase processedBase : s_aBases)
		{
			if (!processedBase)
				continue;
			
			processedBase.SetIsBaseInFactionRadioSignal(faction.GetFactionKey(), coveredBases.Contains(processedBase), byMobileHQ);
		}
		
		foreach (SCR_CampaignBase baseToLink : s_aBases)
		{
			if (!baseToLink)
				continue;
			baseToLink.HandleMapLinks();
		}
		
		if (comp)
			comp.SetCountOfExclusivelyLinkedBases(coveredBases.Count() - coveredBasesWithoutMHQ);
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
						if (comp && comp.TransceiversCount() > 0)
						{
							BaseTransceiver tsv = comp.GetTransceiver(0);
							if (vector.DistanceSq(entity.GetOrigin(), mobileHQ.GetOrigin()) < Math.Pow(tsv.GetRange(), 2))
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
	int GetClosestVictoryControlPointsAvgDistanceSq(notnull SCR_CampaignBase HQ, notnull array<SCR_CampaignBase> controlPoints)
	{
		array<SCR_CampaignBase> nearestControlPoints = {};
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return 0;
		
		int distanceToHQ;
		int controlPointsCount;
		int arrayIndex;
		int nearestControlPointsCount;
		
		vector HQPos = HQ.GetOrigin();
		
		foreach (SCR_CampaignBase controlPoint : controlPoints)
		{
			if (!controlPoint)
				continue;
			
			distanceToHQ = vector.DistanceSqXZ(controlPoint.GetOrigin(), HQPos);
			controlPointsCount = nearestControlPoints.Count();
			arrayIndex = controlPointsCount;
			
			for (int i = 0; i < controlPointsCount; i++)
			{
				if (distanceToHQ < vector.DistanceSqXZ(HQPos, nearestControlPoints[i].GetOrigin()))
				{
					arrayIndex = i;
					break;
				}
			}
			
			nearestControlPointsCount = nearestControlPoints.InsertAt(controlPoint, arrayIndex);
		}
		
		int limit = campaign.GetControlPointTreshold();
		
		// Avoid division by zero
		if (limit == 0)
			return 0;
		
		if (limit < nearestControlPointsCount)
			nearestControlPoints.Resize(limit);
		
		int totalDist;
		
		foreach (SCR_CampaignBase controlPoint : nearestControlPoints)
			totalDist += vector.DistanceSqXZ(HQPos, controlPoint.GetOrigin());
		
		return totalDist / nearestControlPointsCount;
	}
	
	//------------------------------------------------------------------------------------------------
	void SelectHQs()
	{
		// Allow permanent starting HQ for debugging purposes
#ifdef TDM_CLI_SELECTION
		if (m_WestHQ && m_EastHQ)
			return;
#endif
		
		m_WestHQ = null;
		m_EastHQ = null;
		
		int candidatesCount;
		array<SCR_CampaignBase> candidatesForHQ = {};
		array<SCR_CampaignBase> controlPoints = {};
		
		foreach (SCR_CampaignBase base : s_aBases)
		{
			if (!base.GetIsEnabled())
				continue;
			
			if (base.GetCanBeHQ())
			{
				candidatesForHQ.Insert(base);
				candidatesCount++;
			}
			
			if (base.GetIsControlPoint())
				controlPoints.Insert(base);
		}
		
		if (candidatesCount < 2)
			return;
		
#ifdef TDM_CLI_SELECTION
		m_WestHQ = candidatesForHQ[0];
		m_EastHQ = candidatesForHQ[1];
		return;
#endif
		
		// If only two HQs are set up, don't waste time with processing
		if (candidatesCount == 2)
		{
			SelectHQsSimple(candidatesForHQ);
			return;
		}
		
		int iterations;
		int totalBasesDistance;
		SCR_CampaignBase westHQ;
		SCR_CampaignBase eastHQ;
		array<SCR_CampaignBase> eligibleForHQ;
		array<SCR_CampaignBase> nearestControlPoints;
		vector westHQPos;
		int averageHQDistance;
		int averageCPDistance;
		int acceptedCPDistanceDiff;
		
		while (!eastHQ && iterations < MAX_HQ_SELECTION_ITERATIONS)
		{
			iterations++;
			totalBasesDistance = 0;
			eligibleForHQ = {};
			nearestControlPoints = {};
			
			// Pick one of the HQs at random
			Math.Randomize(-1);
			westHQ = candidatesForHQ.GetRandomElement();
			westHQPos = westHQ.GetOrigin();
			
			// Calculate average distance between our HQ and others
			foreach (SCR_CampaignBase otherHQ : candidatesForHQ)
			{
				if (otherHQ == westHQ)
					continue;
				
				totalBasesDistance += vector.DistanceSqXZ(westHQPos, otherHQ.GetOrigin());
			}
			
			averageHQDistance = totalBasesDistance / (candidatesCount - 1);	// Our HQ is substracted
			averageCPDistance = GetClosestVictoryControlPointsAvgDistanceSq(westHQ, controlPoints);
			acceptedCPDistanceDiff = averageCPDistance * CP_AVG_DISTANCE_TOLERANCE;
			
			foreach (SCR_CampaignBase candidate : candidatesForHQ)
			{
				if (candidate == westHQ)
					continue;
				
				// Ignore HQs closer than the average distance
				if (vector.DistanceSqXZ(westHQPos, candidate.GetOrigin()) < averageHQDistance)
					continue;
				
				// Ignore HQs too far from control points (relative to our HQ)
				if (Math.AbsInt(averageCPDistance - GetClosestVictoryControlPointsAvgDistanceSq(candidate, controlPoints)) > acceptedCPDistanceDiff)
					continue;
				
				eligibleForHQ.Insert(candidate);
			}
			
			// No HQs fit the condition, restart loop
			if (eligibleForHQ.Count() == 0)
				continue;
			 
			Math.Randomize(-1);
			eastHQ = eligibleForHQ.GetRandomElement();
		}
		
		// Selection failed, use the simplified but reliable one
		if (!eastHQ)
		{
			SelectHQsSimple(candidatesForHQ);
			return;
		}
		
		// Randomly assign the factions in reverse in case primary selection gets too limited
		if (Math.RandomFloat01() >= 0.5)
		{
			m_WestHQ = westHQ;
			m_EastHQ = eastHQ;
		}
		else
		{
			m_WestHQ = eastHQ;
			m_EastHQ = westHQ;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SelectHQsSimple(notnull array<SCR_CampaignBase> candidates)
	{
		Math.Randomize(-1);
		SCR_CampaignBase westHQ = candidates.GetRandomElement();
		candidates.RemoveItem(westHQ);
		SCR_CampaignBase eastHQ = candidates.GetRandomElement();
		
		if (Math.RandomFloat01() >= 0.5)
		{
			m_WestHQ = westHQ;
			m_EastHQ = eastHQ;
		}
		else
		{
			m_WestHQ = eastHQ;
			m_EastHQ = westHQ;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHQs()
	{
		m_bIsHQSetupDone = true;
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (campaign && campaign.GetApplyPresetOwners())
		{
			SCR_CampaignBase westHQ;
			SCR_CampaignBase eastHQ;
			
			foreach (SCR_CampaignBase base : s_aBases)
			{
				if (base.GetStartingBaseOwnerEnum() == SCR_ECampaignBaseOwner.BLUFOR && base.GetCanBeHQ())
					westHQ = base;
				else if (base.GetStartingBaseOwnerEnum() == SCR_ECampaignBaseOwner.OPFOR && base.GetCanBeHQ())
					eastHQ = base;
				
				if (westHQ && eastHQ)
					break;
			}
			
			if (westHQ && eastHQ)
			{
				westHQ.SetAsHQ(SCR_ECampaignBaseOwner.BLUFOR);
				eastHQ.SetAsHQ(SCR_ECampaignBaseOwner.OPFOR);
				return;
			}
		}
		
		if (!m_WestHQ || !m_EastHQ)
		{
			Print("No suitable starting locations found in current setup. Check 'Can Be HQ' attributes in Conflict base entities!", LogLevel.ERROR);
			return;
		}
		
		m_WestHQ.SetAsHQ(SCR_ECampaignBaseOwner.BLUFOR);
		m_EastHQ.SetAsHQ(SCR_ECampaignBaseOwner.OPFOR);
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsHQSetupDone()
	{
		return m_bIsHQSetupDone;
	}
	
	//------------------------------------------------------------------------------------------------
	static notnull SCR_CampaignBaseManager GetInstance()
	{
		if (!s_Instance)
			GetGame().SpawnEntity(SCR_CampaignBaseManager, GetGame().GetWorld());
		
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	void GetControlPoints(out array<SCR_CampaignBase> bases)
	{
		bases = m_aControlPoints;
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
		
		int index = s_aBases.Find(base);
		
		if (index == -1)
			return;
		
		s_aBases.Remove(index);
		s_iValidBasesCount--;
		ClearNulls();
		SCR_CampaignBaseManager baseManager = GetInstance();
		
		// Check for init here too since delayed RplLoads can cause this method to be called later
		if (GetGame().InPlayMode() && baseManager)
			baseManager.CheckBasesInitialized();
	}
	
	//------------------------------------------------------------------------------------------------
	bool AllBasesInitialized()
	{
		return m_bBasesInitialized;
	}
	
	//------------------------------------------------------------------------------------------------
	static void DisableBase(notnull SCR_CampaignBase base)
	{
		s_iValidBasesCount--;
		
		SCR_CampaignBaseManager baseManager = GetInstance();
		
		if (GetGame().InPlayMode() && baseManager)
			baseManager.CheckBasesInitialized();
	}
	
	//------------------------------------------------------------------------------------------------
	static void EnableBase(notnull SCR_CampaignBase base)
	{
		s_iValidBasesCount++;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckBasesInitialized()
	{
		if (m_bBasesInitialized)
			return;
		
		if (m_iInitializedBasesCount != 0 && m_iInitializedBasesCount == s_iValidBasesCount)
		{
			m_bBasesInitialized = true;
			
			if (s_OnAllBasesInitialized)
			{
				foreach (SCR_CampaignBase baseToLink : s_aBases)
					baseToLink.LinkBases();
				
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
		
		if (base.GetIsControlPoint())
			m_aControlPoints.Insert(base);
		
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
		s_iValidBasesCount++;
		
		if (s_iValidBasesCount != 0)
			s_fTickTime = ITERATION_TIME / s_iValidBasesCount;
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
			if (!s_aBases[i].GetIsEnabled())
				continue;
			
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
		
		m_WestHQ = null;
		m_EastHQ = null;
		m_aControlPoints = {};
		m_bBasesInitialized = false;
		m_iInitializedBasesCount = 0;
		
		int infoCnt = entries.Count();
		array<SCR_CampaignBase> unprocessedBases = {};
		unprocessedBases.Copy(s_aBases);
		
		// Find the HQs and set them up first
		for (int i = 0; i < infoCnt; i++)
		{
			SCR_CampaignBaseStruct baseInfo = entries[i];
			
			if (!baseInfo || !baseInfo.GetIsHQ())
				continue;
			
			int baseID = baseInfo.GetBaseID();
			SCR_CampaignBase base = FindBaseByID(baseID);
			
			if (!base)
				continue;
			
			Faction faction = GetGame().GetFactionManager().GetFactionByIndex(baseInfo.GetOwningFaction());
			
			if (!faction)
				continue;
			
			if (faction.GetFactionKey() == SCR_GameModeCampaignMP.FACTION_BLUFOR)
				m_WestHQ = base;
			else if (faction.GetFactionKey() == SCR_GameModeCampaignMP.FACTION_OPFOR)
				m_EastHQ = base;
			
			if (m_WestHQ && m_EastHQ)
			{
				m_WestHQ.SetAsHQ(SCR_ECampaignBaseOwner.BLUFOR);
				m_EastHQ.SetAsHQ(SCR_ECampaignBaseOwner.OPFOR);
				break;
			}
		}
		
		for (int i = 0; i < infoCnt; i++)
		{
			SCR_CampaignBaseStruct baseInfo = entries[i];
			
			if (!baseInfo)
				continue;
			
			int baseID = baseInfo.GetBaseID();
			SCR_CampaignBase base = FindBaseByID(baseID);
			
			if (!base)
				continue;
			
			if (!base.GetIsEnabled())
				base.EnableBase();
			
			base.InitializeBase();
			base.LoadState(baseInfo);
			unprocessedBases.RemoveItem(base);
		}
		
		foreach (SCR_CampaignBase base : unprocessedBases)
		{
			if (base.GetIsEnabled())
				base.DisableBase();
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
		
		s_iValidBasesCount = 0;
		
		if (!s_Instance)
			s_Instance = this;
		
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
		
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
	}
};