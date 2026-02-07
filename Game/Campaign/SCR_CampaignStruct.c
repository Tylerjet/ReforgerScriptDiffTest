//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_CampaignStruct : SCR_JsonApiStruct
{
	protected int m_iHours = -1;
	protected int m_iMinutes = -1;
	protected int m_iSeconds = -1;
	protected int m_iPlayerXP;	// TODO: Manage properly in MP environment
	protected int m_iPlayerFaction = -1;	// TODO: Manage properly in MP environment
	protected vector m_vMHQLocationBLUFOR;
	protected vector m_vMHQRotationBLUFOR;
	protected vector m_vMHQLocationOPFOR;
	protected vector m_vMHQRotationOPFOR;
	protected ref array<ref SCR_CampaignBaseStruct> m_aBasesStructs = {};
	protected ref array<ref SCR_CampaignRemnantInfoStruct> m_aRemnantsStructs = {};
	protected int m_iTutorialStage = -1;
	protected bool m_bMatchOver;
	
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
			timeManager.GetHoursMinutesSeconds(m_iHours, m_iMinutes, m_iSeconds);
		
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
		
		// TODO: Manage properly in MP environment
		if (RplSession.Mode() == RplMode.None)
		{
			Faction faction = SCR_RespawnSystemComponent.GetLocalPlayerFaction();
			
			if (faction)
				m_iPlayerFaction = GetGame().GetFactionManager().GetFactionIndex(faction);
			
			PlayerController pc = GetGame().GetPlayerController();
			
			if (pc)
			{
				SCR_CampaignNetworkComponent comp = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
				
				if (comp)
					m_iPlayerXP = comp.GetPlayerXP();
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Deserialize()
	{
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
			GetGame().GetCallqueue().CallLater(timeHandler.SetupDaytimeAndWeather, 500, false, m_iHours, m_iMinutes, m_iSeconds);
		
		baseManager.LoadBasesStates(m_aBasesStructs);
		
		SCR_CampaignTutorialComponent tutorial = SCR_CampaignTutorialComponent.Cast(campaign.FindComponent(SCR_CampaignTutorialComponent));
		
		if (tutorial)
		{
			tutorial.SetResumeStage(m_iTutorialStage);
			return true;
		}
		
		campaign.LoadRemnantsStates(m_aRemnantsStructs);
		
		SCR_CampaignFaction factionBLUFOR = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_BLUFOR));
		SCR_CampaignFaction factionOPFOR = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_OPFOR));
		
		if (factionBLUFOR && m_vMHQLocationBLUFOR != vector.Zero)
			GetGame().GetCallqueue().CallLater(campaign.SpawnMobileHQ, 500, false, factionBLUFOR, m_vMHQLocationBLUFOR, m_vMHQRotationBLUFOR);
		
		if (factionOPFOR && m_vMHQLocationOPFOR != vector.Zero)
			GetGame().GetCallqueue().CallLater(campaign.SpawnMobileHQ, 500, false, factionOPFOR, m_vMHQLocationOPFOR, m_vMHQRotationOPFOR);
		
		// TODO: Manage properly in MP environment
		if (RplSession.Mode() == RplMode.None)
		{
			campaign.RegisterSavedPlayerFaction(m_iPlayerFaction);
			campaign.RegisterSavedPlayerXP(m_iPlayerXP);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		m_aBasesStructs.Clear();
		m_aRemnantsStructs.Clear();
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
		RegV("m_iPlayerXP");
		RegV("m_iPlayerFaction");
		RegV("m_vMHQLocationBLUFOR");
		RegV("m_vMHQRotationBLUFOR");
		RegV("m_vMHQLocationOPFOR");
		RegV("m_vMHQRotationOPFOR");
		RegV("m_aBasesStructs");
		RegV("m_aRemnantsStructs");
		RegV("m_iTutorialStage");
		RegV("m_bMatchOver");
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
	string GetServicePrefab(ECampaignServicePointType type)
	{
		switch (type)
		{
			case ECampaignServicePointType.LIGHT_VEHICLE_DEPOT: {return m_sVehicleDepotPrefab;};
			case ECampaignServicePointType.ARMORY: {return m_sArmoryPrefab;};
			case ECampaignServicePointType.HEAVY_VEHICLE_DEPOT: {return m_sHeavyVehicleDepotPrefab;};
			case ECampaignServicePointType.SUPPLY_DEPOT: {return m_sSupplyDepotPrefab;};
			case ECampaignServicePointType.RADIO_ANTENNA: {return m_sAntennaPrefab;};
			case ECampaignServicePointType.FIELD_HOSPITAL: {return m_sFieldHospitalPrefab;};
			case ECampaignServicePointType.BARRACKS: {return m_sBarracksPrefab;};
			case ECampaignServicePointType.FIELD_HOSPITAL: {return m_sHospitalPrefab;};
		}
		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetServicePosition(ECampaignServicePointType type)
	{
		switch (type)
		{
			case ECampaignServicePointType.LIGHT_VEHICLE_DEPOT: {return m_vVehicleDepotPosition;};
			case ECampaignServicePointType.ARMORY: {return m_vArmoryPosition;};
			case ECampaignServicePointType.HEAVY_VEHICLE_DEPOT: {return m_vHeavyVehicleDepotPosition;};
			case ECampaignServicePointType.SUPPLY_DEPOT: {return m_vSupplyDepotPosition;};
			case ECampaignServicePointType.RADIO_ANTENNA: {return m_vAntennaPosition;};
			case ECampaignServicePointType.FIELD_HOSPITAL: {return m_vFieldHospitalPosition;};
			case ECampaignServicePointType.BARRACKS: {return m_vBarracksPosition;};
			case ECampaignServicePointType.FIELD_HOSPITAL: {return m_vHospitalPosition;};
		}
		
		return vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetServiceRotation(ECampaignServicePointType type)
	{
		switch (type)
		{
			case ECampaignServicePointType.LIGHT_VEHICLE_DEPOT: {return m_vVehicleDepotRotation;};
			case ECampaignServicePointType.ARMORY: {return m_vArmoryRotation;};
			case ECampaignServicePointType.HEAVY_VEHICLE_DEPOT: {return m_vHeavyVehicleDepotRotation;};
			case ECampaignServicePointType.SUPPLY_DEPOT: {return m_vSupplyDepotRotation;};
			case ECampaignServicePointType.RADIO_ANTENNA: {return m_vAntennaRotation;};
			case ECampaignServicePointType.FIELD_HOSPITAL: {return m_vFieldHospitalRotation;};
			case ECampaignServicePointType.BARRACKS: {return m_vBarracksRotation;};
			case ECampaignServicePointType.FIELD_HOSPITAL: {return m_vHospitalRotation;};
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
				case ECampaignServicePointType.LIGHT_VEHICLE_DEPOT:
				{
					m_sVehicleDepotPrefab = prefab;
					m_vVehicleDepotPosition = composition.GetOrigin();
					m_vVehicleDepotRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case ECampaignServicePointType.ARMORY:
				{
					m_sArmoryPrefab = prefab;
					m_vArmoryPosition = composition.GetOrigin();
					m_vArmoryRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case ECampaignServicePointType.HEAVY_VEHICLE_DEPOT:
				{
					m_sHeavyVehicleDepotPrefab = prefab;
					m_vHeavyVehicleDepotPosition = composition.GetOrigin();
					m_vHeavyVehicleDepotRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case ECampaignServicePointType.SUPPLY_DEPOT:
				{
					m_sSupplyDepotPrefab = prefab;
					m_vSupplyDepotPosition = composition.GetOrigin();
					m_vSupplyDepotRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case ECampaignServicePointType.RADIO_ANTENNA:
				{
					m_sAntennaPrefab = prefab;
					m_vAntennaPosition = composition.GetOrigin();
					m_vAntennaRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case ECampaignServicePointType.FIELD_HOSPITAL:
				{
					m_sFieldHospitalPrefab = prefab;
					m_vFieldHospitalPosition = composition.GetOrigin();
					m_vFieldHospitalRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case ECampaignServicePointType.BARRACKS:
				{
					m_sBarracksPrefab = prefab;
					m_vBarracksPosition = composition.GetOrigin();
					m_vBarracksRotation = composition.GetYawPitchRoll();
					break;
				}
				
				case ECampaignServicePointType.FIELD_HOSPITAL:
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