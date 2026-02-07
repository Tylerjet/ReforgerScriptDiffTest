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
		
		SCR_CampaignTutorialComponent tutorial = SCR_CampaignTutorialComponent.Cast(campaign.FindComponent(SCR_CampaignTutorialComponent));
		
		if (tutorial)
		{
			m_iTutorialStage = tutorial.GetStage();
			return true;
		}
		
		Clear();
		
		TimeAndWeatherManagerEntity timeManager = GetGame().GetTimeAndWeatherManager();
		
		if (timeManager)
			timeManager.GetHoursMinutesSeconds(m_iHours, m_iMinutes, m_iSeconds);
		
		baseManager.StoreBasesStates(m_aBasesStructs);
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
		
		SCR_CampaignTutorialComponent tutorial = SCR_CampaignTutorialComponent.Cast(campaign.FindComponent(SCR_CampaignTutorialComponent));
		
		if (tutorial)
		{
			tutorial.SetResumeStage(m_iTutorialStage);
			return true;
		}
		
		TimeAndWeatherManagerEntity timeManager = GetGame().GetTimeAndWeatherManager();
		
		if (timeManager && m_iHours >= 0 && m_iMinutes >= 0)
			GetGame().GetCallqueue().CallLater(campaign.SetupDaytime, 500, false, m_iHours, m_iMinutes, m_iSeconds);
		
		baseManager.LoadBasesStates(m_aBasesStructs);
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
	protected int m_BaseID;
	protected int m_iOwningFaction;
	protected int m_iBuildingsFaction;
	protected int m_iSupplies;
	protected bool m_bVehicleDepotBuilt;
	protected bool m_bArmoryBuilt;
	protected int m_iRespawnTickets;
	
	//------------------------------------------------------------------------------------------------
	int GetBaseID()
	{
		return m_BaseID;
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
	bool GetVehicleDepotBuilt()
	{
		return m_bVehicleDepotBuilt;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetArmoryBuilt()
	{
		return m_bArmoryBuilt;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRespawnTickets()
	{
		return m_iRespawnTickets;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBaseID(int baseID)
	{
		m_BaseID = baseID;
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
	void SetVehicleDepotBuilt(bool built)
	{
		m_bVehicleDepotBuilt = built;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetArmoryBuilt(bool built)
	{
		m_bArmoryBuilt = built;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRespawnTickets(int tickets)
	{
		m_iRespawnTickets = tickets;
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
		RegV("m_BaseID");
		RegV("m_iOwningFaction");
		RegV("m_iBuildingsFaction");
		RegV("m_iSupplies");
		RegV("m_bVehicleDepotBuilt");
		RegV("m_bArmoryBuilt");
		RegV("m_iRespawnTickets");
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
	void SetID(int ID)
	{
		m_iID = ID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMembersAlive(int count)
	{
		m_iMembersAlive = count;
	}
	
	//************************//
	//CONSTRUCTOR & DESTRUCTOR//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignRemnantInfoStruct()
	{
		RegV("m_iID");
		RegV("m_iMembersAlive");
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignRemnantInfoStruct()
	{
	}
};