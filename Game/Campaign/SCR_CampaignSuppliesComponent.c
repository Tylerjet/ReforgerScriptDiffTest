[EntityEditorProps(category: "GameScripted/Campaign", description: "Makes a vehicle able to carry Campaign resources.", color: "0 0 255 255")]
class SCR_CampaignSuppliesComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Makes a vehicle able to carry Campaign resources
class SCR_CampaignSuppliesComponent : ScriptComponent
{
	// Member variables 
	protected SCR_CampaignBase m_LastLoadedAt;
	protected SCR_CampaignBase m_LastUnloadedAt;
	protected SCR_CampaignBase m_LastXPAwardedAt;
	protected bool m_bAwardUnloadXP = true;
	
	[RplProp()]
	protected ref array<int> m_aLoadingPlayerIDs = {};
	
	[RplProp()]
	protected ref array<int> m_aUnloadingPlayerIDs = {};
	
	// Script Invoker
	ref ScriptInvoker m_OnSuppliesChanged = new ScriptInvoker();
	ref ScriptInvoker m_OnSuppliesTruckDeleted = new ScriptInvoker();
	
	// Synced variables
	[Attribute("0", desc: "How many supplies this component holds when it is created."), RplProp(onRplName: "OnSuppliesChanged")]
	protected int m_iSupplies;
	
	// Synced variables
	[Attribute("0", desc: "Maximum supplies this component can hold.")]
	protected int m_iSuppliesMax;
	
	[Attribute("0", desc: "Maximum distance from a supply depot a player can still (un)load their truck")]
	protected float m_fOperationalRadius;
	
	//------------------------------------------------------------------------------------------------
	// Returns first player from loading/unloading arrays 
	// Use True parameters, to return from Unloading array. Otherwise, it will look in loading players
	int GetLoadingPlayer(bool unloading = false)
	{
		array<int> loadingArray;
		//Switch between loading and unloading players
		if(!unloading)
			loadingArray = m_aLoadingPlayerIDs;
		else
			loadingArray = m_aUnloadingPlayerIDs;
		
		//return id of first player (if there is any)
		if(!loadingArray.IsEmpty())
			return loadingArray[0];	
		else
			return 0;
	}
	//------------------------------------------------------------------------------------------------
	// Setter for Loading players array
	void SetSupplyLoadingPlayer(int playerID)
	{
		if(!m_aLoadingPlayerIDs.Contains(playerID))
		{
			m_aLoadingPlayerIDs.Insert(playerID);
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Deletes from loading players array
	void DeleteSupplyLoadingPlayer(int playerID)
	{
		if(m_aLoadingPlayerIDs.Contains(playerID))
		{
			m_aLoadingPlayerIDs.RemoveItemOrdered(playerID);
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Setter for Unloading players array
	void SetSupplyUnloadingPlayer(int playerID)
	{
		if(!m_aUnloadingPlayerIDs.Contains(playerID))
		{
			m_aUnloadingPlayerIDs.Insert(playerID);
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Deletes from Unloading players array
	void DeleteSupplyUnloadingPlayer(int playerID)
	{
		if(m_aUnloadingPlayerIDs.Contains(playerID))
		{
			m_aUnloadingPlayerIDs.RemoveItemOrdered(playerID);
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSuppliesChanged()
	{
		// script invoker executed when ammout of supplies in vehicle changed. m_iSupplies is ammout of currently loaded supplies.
		m_OnSuppliesChanged.Invoke(m_iSupplies);
		
		SCR_CampaignBase base = SCR_CampaignBase.Cast(GetOwner().GetParent());
		
		if (base)
		{
			if (RplSession.Mode() != RplMode.Dedicated)
				base.HandleMapInfo();
			
			RplComponent rplC = RplComponent.Cast(base.FindComponent(RplComponent));
			
			if (rplC && !rplC.IsProxy())
			{
				SCR_CampaignDeliveryPoint servicePoint = SCR_CampaignDeliveryPoint.Cast(GetOwner());
				
				if (servicePoint)
					servicePoint.UpdateCompositionPrefab();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSupplies()
	{
		return m_iSupplies;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSuppliesMax()
	{
		return m_iSuppliesMax;
	}
	
	//------------------------------------------------------------------------------------------------
	// Get the load / unload radius. If set custom use this. if it's set to zero, use default.
	float GetOperationalRadius()
	{
		if (m_fOperationalRadius != 0)
			return m_fOperationalRadius;
		
		return SCR_GameModeCampaignMP.SUPPLY_TRUCK_UNLOAD_RADIUS;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLastLoadedAt(SCR_CampaignBase base)
	{
		m_LastLoadedAt = base;
		
		if (m_LastLoadedAt == m_LastUnloadedAt && m_LastUnloadedAt == m_LastXPAwardedAt)
			m_bAwardUnloadXP = false;
		else
			m_bAwardUnloadXP = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLastUnloadedAt(SCR_CampaignBase base)
	{
		m_LastUnloadedAt = base;
		
		if (m_LastLoadedAt == m_LastUnloadedAt)
			m_bAwardUnloadXP = false;
		else
			m_LastXPAwardedAt = base;
	}
	
	//------------------------------------------------------------------------------------------------
	bool AwardXP()
	{
		return m_bAwardUnloadXP;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddSupplies(int supplies)
	{
		m_iSupplies += supplies;
		Replication.BumpMe();
		OnSuppliesChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (m_iSupplies > m_iSuppliesMax)
		{
			string err = string.Format("SCR_CampaignSuppliesComponent on %1 carries more supplies (%2) than its maximum (%3)!", owner, m_iSupplies, m_iSuppliesMax);
			Print(err, LogLevel.ERROR);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Constructor
	void SCR_CampaignSuppliesComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	// Destructor
	void ~SCR_CampaignSuppliesComponent()
	{
		m_OnSuppliesTruckDeleted.Invoke();
	}
};