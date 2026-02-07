//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_CampaignStruct : SCR_JsonApiStruct
{
	protected int m_iHours = -1;
	protected int m_iMinutes = -1;
	protected int m_iSeconds = -1;
	protected vector m_vMHQLocationBLUFOR;
	protected vector m_vMHQRotationBLUFOR;
	protected vector m_vMHQLocationOPFOR;
	protected vector m_vMHQRotationOPFOR;
	protected ref array<ref SCR_CampaignBaseStruct> m_aBasesStructs = {};
	protected ref array<ref SCR_CampaignRemnantInfoStruct> m_aRemnantsStructs = {};
	protected ref array<ref SCR_CampaignPlayerStruct> m_aPlayerStructs = {};
	protected int m_iTutorialStage = -1;
	protected bool m_bMatchOver;
	protected string m_sWeatherState;
	
	//------------------------------------------------------------------------------------------------
	override bool Serialize()
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		
		if (!campaign || !baseManager)
			return false;
		
		m_bMatchOver = campaign.GetIsMatchOver();
		
		Clear();
		
		TimeAndWeatherManagerEntity timeManager = GetGame().GetTimeAndWeatherManager();
		
		if (timeManager)
		{
			timeManager.GetHoursMinutesSeconds(m_iHours, m_iMinutes, m_iSeconds);
			WeatherStateTransitionManager transitionManager = timeManager.GetTransitionManager();
			
			if (transitionManager)
				m_sWeatherState = transitionManager.GetCurrentState().GetStateName();
		}
		
		baseManager.StoreBasesStates(m_aBasesStructs);
		
		SCR_CampaignTutorialComponent tutorial = SCR_CampaignTutorialComponent.Cast(campaign.FindComponent(SCR_CampaignTutorialComponent));
		
		if (tutorial)
		{
			m_iTutorialStage = tutorial.GetStage();
			return true;
		}
		
		campaign.StoreRemnantsStates(m_aRemnantsStructs);
		
		SCR_CampaignFaction factionBLUFOR = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_BLUFOR));
		SCR_CampaignFaction factionOPFOR = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_OPFOR));
		
		if (factionBLUFOR && factionBLUFOR.GetDeployedMobileAssembly())
		{
			m_vMHQLocationBLUFOR = factionBLUFOR.GetDeployedMobileAssembly().GetOrigin();
			m_vMHQRotationBLUFOR = factionBLUFOR.GetDeployedMobileAssembly().GetYawPitchRoll();
		}
		
		if (factionOPFOR && factionOPFOR.GetDeployedMobileAssembly())
		{
			m_vMHQLocationOPFOR = factionOPFOR.GetDeployedMobileAssembly().GetOrigin();
			m_vMHQRotationOPFOR = factionOPFOR.GetDeployedMobileAssembly().GetYawPitchRoll();
		}
		
		campaign.WriteAllClientsData();
		array<ref SCR_CampaignClientData> clients = campaign.GetClientsData();
		
		foreach (SCR_CampaignClientData data : clients)
		{
			SCR_CampaignPlayerStruct struct = new SCR_CampaignPlayerStruct();
			struct.SetID(data.GetID());
			struct.SetXP(data.GetXP());
			struct.SetFactionIndex(data.GetFactionIndex());
			
			m_aPlayerStructs.Insert(struct);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Deserialize()
	{
		// No bases data available for load, something is wrong - terminate
		if (m_aBasesStructs.IsEmpty())
			return false;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		
		if (!campaign || !baseManager)
			return false;
		
		// Game was saved after match was over, don't load
		if (m_bMatchOver)
			return false;
		
		SCR_TimeAndWeatherHandlerComponent timeHandler = SCR_TimeAndWeatherHandlerComponent.GetInstance();
		
		// Weather has to be changed after init
		if (timeHandler && m_iHours >= 0 && m_iMinutes >= 0)
		{
			GetGame().GetCallqueue().Remove(timeHandler.SetupDaytimeAndWeather);
			GetGame().GetCallqueue().CallLater(timeHandler.SetupDaytimeAndWeather, 500, false, m_iHours, m_iMinutes, m_iSeconds, m_sWeatherState, true);
		}
		
		baseManager.LoadBasesStates(m_aBasesStructs);
		
		SCR_CampaignTutorialComponent tutorial = SCR_CampaignTutorialComponent.Cast(campaign.FindComponent(SCR_CampaignTutorialComponent));
		
		if (tutorial)
		{
			tutorial.SetResumeStage(m_iTutorialStage);
			return true;
		}
		
		campaign.LoadRemnantsStates(m_aRemnantsStructs);
		campaign.LoadClientData(m_aPlayerStructs);
		
		SCR_CampaignFaction factionBLUFOR = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_BLUFOR));
		SCR_CampaignFaction factionOPFOR = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_OPFOR));
		
		if (factionBLUFOR && m_vMHQLocationBLUFOR != vector.Zero)
			GetGame().GetCallqueue().CallLater(campaign.SpawnMobileHQ, 500, false, factionBLUFOR, m_vMHQLocationBLUFOR, m_vMHQRotationBLUFOR);
		
		if (factionOPFOR && m_vMHQLocationOPFOR != vector.Zero)
			GetGame().GetCallqueue().CallLater(campaign.SpawnMobileHQ, 500, false, factionOPFOR, m_vMHQLocationOPFOR, m_vMHQRotationOPFOR);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		m_aBasesStructs.Clear();
		m_aRemnantsStructs.Clear();
		m_aPlayerStructs.Clear();
		m_vMHQLocationBLUFOR = vector.Zero;
		m_vMHQRotationBLUFOR = vector.Zero;
		m_vMHQLocationOPFOR = vector.Zero;
		m_vMHQRotationOPFOR = vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignStruct()
	{
		RegV("m_iHours");
		RegV("m_iMinutes");
		RegV("m_iSeconds");
		RegV("m_vMHQLocationBLUFOR");
		RegV("m_vMHQRotationBLUFOR");
		RegV("m_vMHQLocationOPFOR");
		RegV("m_vMHQRotationOPFOR");
		RegV("m_aBasesStructs");
		RegV("m_aRemnantsStructs");
		RegV("m_aPlayerStructs");
		RegV("m_iTutorialStage");
		RegV("m_bMatchOver");
		RegV("m_sWeatherState");
	}
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBaseStruct : SCR_JsonApiStruct
{
	protected int m_iBaseID;
	protected bool m_bIsHQ;
	protected int m_iCallsignIndex;
	protected int m_iOwningFaction;
	protected int m_iBuildingsFaction;
	protected int m_iSupplies;
	protected string m_sVehicleDepotPrefab;
	protected vector m_vVehicleDepotPosition;
	protected vector m_vVehicleDepotRotation;
	protected string m_sArmoryPrefab;
	protected vector m_vArmoryPosition;
	protected vector m_vArmoryRotation;
	protected string m_sHeavyVehicleDepotPrefab;
	protected vector m_vHeavyVehicleDepotPosition;
	protected vector m_vHeavyVehicleDepotRotation;
	protected string m_sSupplyDepotPrefab;
	protected vector m_vSupplyDepotPosition;
	protected vector m_vSupplyDepotRotation;
	protected string m_sAntennaPrefab;
	protected vector m_vAntennaPosition;
	protected vector m_vAntennaRotation;	
	protected string m_sFieldHospitalPrefab;
	protected vector m_vFieldHospitalPosition;
	protected vector m_vFieldHospitalRotation;
	protected string m_sBarracksPrefab;
	protected vector m_vBarracksPosition;
	protected vector m_vBarracksRotation;
	protected string m_sHospitalPrefab;
	protected vector m_vHospitalPosition;
	protected vector m_vHospitalRotation;
	
	//------------------------------------------------------------------------------------------------
	int GetBaseID()
	{
		return m_iBaseID;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsHQ()
	{
		return m_bIsHQ;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCallsignIndex()
	{
		return m_iCallsignIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSupplies()
	{
		return m_iSupplies;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetOwningFaction()
	{
		return m_iOwningFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBuildingsFaction()
	{
		return m_iBuildingsFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetServicePrefab(SCR_EServicePointType type)
	{
		switch (type)
		{
			case SCR_EServicePointType.LIGHT_VEHICLE_DEPOT: {return m_sVehicleDepotPrefab;};
			case SCR_EServicePointType.ARMORY: {return m_sArmoryPrefab;};
			case SCR_EServicePointType.HEAVY_VEHICLE_DEPOT: {return m_sHeavyVehicleDepotPrefab;};
			case SCR_EServicePointType.SUPPLY_DEPOT: {return m_sSupplyDepotPrefab;};
			case SCR_EServicePointType.RADIO_ANTENNA: {return m_sAntennaPrefab;};
			case SCR_EServicePointType.FIELD_HOSPITAL: {return m_sFieldHospitalPrefab;};
			case SCR_EServicePointType.BARRACKS: {return m_sBarracksPrefab;};
			case SCR_EServicePointType.FIELD_HOSPITAL: {return m_sHospitalPrefab;};
		}
		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetServicePosition(SCR_EServicePointType type)
	{
		switch (type)
		{
			case SCR_EServicePointType.LIGHT_VEHICLE_DEPOT: {return m_vVehicleDepotPosition;};
			case SCR_EServicePointType.ARMORY: {return m_vArmoryPosition;};
			case SCR_EServicePointType.HEAVY_VEHICLE_DEPOT: {return m_vHeavyVehicleDepotPosition;};
			case SCR_EServicePointType.SUPPLY_DEPOT: {return m_vSupplyDepotPosition;};
			case SCR_EServicePointType.RADIO_ANTENNA: {return m_vAntennaPosition;};
			case SCR_EServicePointType.FIELD_HOSPITAL: {return m_vFieldHospitalPosition;};
			case SCR_EServicePointType.BARRACKS: {return m_vBarracksPosition;};
			case SCR_EServicePointType.FIELD_HOSPITAL: {return m_vHospitalPosition;};
		}
		
		return vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetServiceRotation(SCR_EServicePointType type)
	{
		switch (type)
		{
			case SCR_EServicePointType.LIGHT_VEHICLE_DEPOT: {return m_vVehicleDepotRotation;};
			case SCR_EServicePointType.ARMORY: {return m_vArmoryRotation;};
			case SCR_EServicePointType.HEAVY_VEHICLE_DEPOT: {return m_vHeavyVehicleDepotRotation;};
			case SCR_EServicePointType.SUPPLY_DEPOT: {return m_vSupplyDepotRotation;};
			case SCR_EServicePointType.RADIO_ANTENNA: {return m_vAntennaRotation;};
			case SCR_EServicePointType.FIELD_HOSPITAL: {return m_vFieldHospitalRotation;};
			case SCR_EServicePointType.BARRACKS: {return m_vBarracksRotation;};
			case SCR_EServicePointType.FIELD_HOSPITAL: {return m_vHospitalRotation;};
		}
		
		return vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBaseID(int baseID)
	{
		m_iBaseID = baseID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsHQ(bool hq)
	{
		m_bIsHQ = hq;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCallsignIndex(int callsign)
	{
		m_iCallsignIndex = callsign;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSupplies(int supplies)
	{
		m_iSupplies = supplies;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOwningFaction(int owningFaction)
	{
		m_iOwningFaction = owningFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBuildingsFaction(int buildingsFaction)
	{
		m_iBuildingsFaction = buildingsFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBuildingsData(notnull SCR_CampaignBase base)
	{
		array<SCR_CampaignServiceComponent> services = {};
		base.GetAllBaseServices(services);
		IEntity composition;
		EntityPrefabData data;
		ResourceName prefab;
		
		foreach (SCR_CampaignServiceComponent service : services)
		{
			composition = service.GetOwner();
			
			if (!composition)
				continue;
			
			data = composition.GetPrefabData();
			
			if (!data)
				continue;
			
			prefab = data.GetPrefabName();
			
			if (prefab.IsEmpty())
				continue;
			
			switch (service.GetType())
			{
				case SCR_EServicePointType.LIGHT_VEHICLE_DEPOT:
				{
					m_sVehicleDepotPrefab = prefab;
					m_vVehicleDepotPosition = composition.GetOrigin();
					m_vVehicleDepotRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.ARMORY:
				{
					m_sArmoryPrefab = prefab;
					m_vArmoryPosition = composition.GetOrigin();
					m_vArmoryRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.HEAVY_VEHICLE_DEPOT:
				{
					m_sHeavyVehicleDepotPrefab = prefab;
					m_vHeavyVehicleDepotPosition = composition.GetOrigin();
					m_vHeavyVehicleDepotRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.SUPPLY_DEPOT:
				{
					m_sSupplyDepotPrefab = prefab;
					m_vSupplyDepotPosition = composition.GetOrigin();
					m_vSupplyDepotRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.RADIO_ANTENNA:
				{
					m_sAntennaPrefab = prefab;
					m_vAntennaPosition = composition.GetOrigin();
					m_vAntennaRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.FIELD_HOSPITAL:
				{
					m_sFieldHospitalPrefab = prefab;
					m_vFieldHospitalPosition = composition.GetOrigin();
					m_vFieldHospitalRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.BARRACKS:
				{
					m_sBarracksPrefab = prefab;
					m_vBarracksPosition = composition.GetOrigin();
					m_vBarracksRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.FIELD_HOSPITAL:
				{
					m_sHospitalPrefab = prefab;
					m_vHospitalPosition = composition.GetOrigin();
					m_vHospitalRotation = composition.GetYawPitchRoll();
					break;
				}
			}
		}
	}
	
	//****************//
	//OVERRIDE METHODS//
	//****************//
	
	//------------------------------------------------------------------------------------------------
	override void OnExpand()
	{
		Print("OnExpand()");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnItemObject(int index, string name)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPack()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnObject( string name )
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnError(int errorCode)
	{
		Print("OnError: " + errorCode);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess( int errorCode )
	{
        Print("OnSuccess: " + errorCode );
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnBufferReady()
	{
		Print( "OnBufferReady()" );
		
		string s = AsString();
		Print(s);
	}
	
	//************************//
	//CONSTRUCTOR & DESTRUCTOR//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignBaseStruct()
	{
		RegV("m_iBaseID");
		RegV("m_bIsHQ");
		RegV("m_iCallsignIndex");
		RegV("m_iOwningFaction");
		RegV("m_iBuildingsFaction");
		RegV("m_iSupplies");
		RegV("m_sVehicleDepotPrefab");
		RegV("m_vVehicleDepotPosition");
		RegV("m_vVehicleDepotRotation");
		RegV("m_sArmoryPrefab");
		RegV("m_vArmoryPosition");
		RegV("m_vArmoryRotation");
		RegV("m_sHeavyVehicleDepotPrefab");
		RegV("m_vHeavyVehicleDepotPosition");
		RegV("m_vHeavyVehicleDepotRotation");
		RegV("m_sSupplyDepotPrefab");
		RegV("m_vSupplyDepotPosition");
		RegV("m_vSupplyDepotRotation");
		RegV("m_sAntennaPrefab");
		RegV("m_vAntennaPosition");
		RegV("m_vAntennaRotation");
		RegV("m_sFieldHospitalPrefab");
		RegV("m_vFieldHospitalPosition");
		RegV("m_vFieldHospitalRotation");
		RegV("m_sBarracksPrefab");
		RegV("m_vBarracksPosition");
		RegV("m_vBarracksRotation");
		RegV("m_sHospitalPrefab");
		RegV("m_vHospitalPosition");
		RegV("m_vHospitalRotation");
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignBaseStruct()
	{
		
	}
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignRemnantInfoStruct : SCR_JsonApiStruct
{
	protected int m_iID;
	protected int m_iMembersAlive;
	protected float m_fRespawnTimer;
	
	//------------------------------------------------------------------------------------------------
	int GetID()
	{
		return m_iID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMembersAlive()
	{
		return m_iMembersAlive;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetRespawnTimer()
	{
		return m_fRespawnTimer;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetID(int ID)
	{
		m_iID = ID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMembersAlive(int count)
	{
		m_iMembersAlive = count;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRespawnTimer(float timer)
	{
		m_fRespawnTimer = timer;
	}
	
	//************************//
	//CONSTRUCTOR & DESTRUCTOR//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignRemnantInfoStruct()
	{
		RegV("m_iID");
		RegV("m_iMembersAlive");
		RegV("m_fRespawnTimer");
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignRemnantInfoStruct()
	{
	}
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignPlayerStruct : SCR_JsonApiStruct
{
	static const string LOCAL_PLAYER_IDENTITY_ID = "identitySP";
	
	protected string m_sID;
	protected int m_iFaction = -1;
	protected int m_iXP;
	
	//------------------------------------------------------------------------------------------------
	string GetID()
	{
		return m_sID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFactionIndex()
	{
		return m_iFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetXP()
	{
		return m_iXP;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetID(string ID)
	{
		m_sID = ID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFactionIndex(int index)
	{
		m_iFaction = index;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetXP(int xp)
	{
		m_iXP = xp;
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetPlayerIdentity(int playerId)
	{
		if (playerId <= 0)
			return string.Empty;
		
		if (RplSession.Mode() == RplMode.None)
			return LOCAL_PLAYER_IDENTITY_ID;
		
		BackendApi api = GetGame().GetBackendApi();
		
		if (!api)
			return string.Empty;
		
		return api.GetPlayerUID(playerId);
	}
	
	//************************//
	//CONSTRUCTOR & DESTRUCTOR//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignPlayerStruct()
	{
		RegV("m_sID");
		RegV("m_iFaction");
		RegV("m_iXP");
	}
};