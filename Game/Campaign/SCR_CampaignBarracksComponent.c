[EntityEditorProps(category: "GameScripted/Campaign", description: "Component to be used with Conflict barrack compositions, handling unit spawning.", color: "0 0 255 255")]
class SCR_CampaignBarracksComponentClass: SCR_BarracksComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBarracksComponent: SCR_BarracksComponent
{
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	protected int m_iDeadOnDespawn;
	protected bool m_bInitialized;
	protected SCR_CampaignBase m_Base;
	
	[Attribute("25", UIWidgets.Auto, "How much it cost to spawn one soldier?", category: "Campaign")]
	protected int m_iUnitCost;
	
	[Attribute("50", desc: "Range in which should component search for base.", category: "Campaign")]
	protected float m_fBaseSearchDistance;
	
	bool IsInitialized()
	{
		return m_bInitialized;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSuppliesComponent(SCR_CampaignSuppliesComponent supplyComp)
	{
		m_SuppliesComponent = supplyComp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DespawnGroup(SCR_AIBarracksGroup grp)
	{
		SCR_AIGroup group = grp.GetGroup();
		if (group)
			m_iDeadOnDespawn = grp.GetGroupSize() - group.GetAgentsCount();
		
		super.DespawnGroup(grp);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void SpawnUnit(ResourceName unitResource, SCR_AIBarracksGroup barrackGrp)
	{
		if (!m_SuppliesComponent)
			return;
		
		if (m_SuppliesComponent.GetSupplies() < m_iUnitCost)
			return;
		
		if (!barrackGrp.GetIsDespawned())
			m_SuppliesComponent.AddSupplies(-m_iUnitCost);
		else if (m_iDeadOnDespawn > 0)
		{
			m_iDeadOnDespawn--;
			m_SuppliesComponent.AddSupplies(-m_iUnitCost);
		}
		
		super.SpawnUnit(unitResource, barrackGrp);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnGroupSpawned(notnull SCR_AIGroup grp)
	{
		super.OnGroupSpawned(grp);
		
		SCR_AIGroupUtilityComponent comp = SCR_AIGroupUtilityComponent.Cast(grp.FindComponent(SCR_AIGroupUtilityComponent));
		
		if (!comp)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return;
		
		ScriptInvoker onEnemyDetected = comp.GetOnEnemyDetected();
		
		if (!onEnemyDetected)
			return;
		
		onEnemyDetected.Insert(campaign.OnEnemyDetectedByDefenders);
		
		if (m_Base)
			m_Base.SetDefendersGroup(grp);
	}
	
	//------------------------------------------------------------------------------------------------
	void InitializeBarrack()
	{
		if (IsProxy())
			return;
		
		if (!m_SuppliesComponent)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign || campaign.IsTutorial())
			return;
		
		//Initializes handling of units. If there is not enough supplies, it will use invoker to check when there is change in them
		m_bInitialized = true;
		if (m_SuppliesComponent.GetSupplies() > m_iUnitCost)
			InitHandler();
		else
			m_SuppliesComponent.m_OnSuppliesChanged.Insert(InitHandler);
	}
	
	//------------------------------------------------------------------------------------------------
	void StopHandler()
	{
		m_bInitialized = false;
		GetGame().GetCallqueue().Remove(HandleGroups);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitHandler()
	{
		if (m_SuppliesComponent)
		{
			GetGame().GetCallqueue().CallLater(HandleGroups, GROUP_HANDLER_DELAY, true);
			m_SuppliesComponent.m_OnSuppliesChanged.Remove(InitHandler);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool BaseSearchCB(IEntity ent)
	{
		m_Base = SCR_CampaignBase.Cast(ent);
		if (m_Base && m_Base.GetType() != CampaignBaseType.RELAY)
		{
			m_Base.AssignBarracks(this);
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{	
		if (!GetGame().InPlayMode())
			return;
		
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!m_RplComponent)
			return;
		
		if (IsProxy())
			return;
		
		// returns if query was finished without finding base
		if (GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), m_fBaseSearchDistance, BaseSearchCB, null, EQueryEntitiesFlags.ALL))
			return;
	}
}