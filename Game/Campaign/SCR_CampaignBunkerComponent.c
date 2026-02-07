[EntityEditorProps(category: "GameScripted/Campaign", description: "Handles additional functionality at bunkers built in campaign.", color: "0 0 255 255")]
class SCR_CampaignBunkerComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBunkerComponent : ScriptComponent
{
	[Attribute("150", desc: "Range in which should component search for base.")]
	protected float m_fBaseSearchDistance;
	
	protected SCR_CampaignBase m_Base;
	protected SCR_CampaignSpawnPointGroup m_SpawnPoint;
	
	//------------------------------------------------------------------------------------------------
	protected void AddSpawnPositions()
	{
		if (!m_SpawnPoint)
			return;
		
		SCR_Position position;

		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			position = SCR_Position.Cast(child);
			if (position)
				m_SpawnPoint.InsertChildrenPosition(position);
			
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback for base search function. Returns true, if no base was found
	protected bool BaseSearchCallback(IEntity ent)
	{
		SCR_CampaignBase base = SCR_CampaignBase.Cast(ent);
		if (base && base.GetType() != CampaignBaseType.RELAY)
		{
			m_Base = base;
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AssignBaseSpawnPoint()
	{
		if (!m_Base)
			return;
		
		m_SpawnPoint = SCR_CampaignSpawnPointGroup.Cast(m_Base.GetBaseSpawnPoint());
		if (!m_SpawnPoint)
			return;
		
		AddSpawnPositions();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		// returns if query was finished without finding base (TEMPORARY, will be changed as soon as Freeform basebuilding is available
		if (GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), m_fBaseSearchDistance, BaseSearchCallback, null, EQueryEntitiesFlags.ALL))
		{
			Print("Bunker didn't find any base in its vicinity", LogLevel.DEBUG);
			return;
		}
		
		if (!m_Base.GetBaseSpawnPoint())
			m_Base.m_OnSpawnPointAssigned.Insert(AssignBaseSpawnPoint);
		else
			AssignBaseSpawnPoint();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignBunkerComponent()
	{
		if (m_SpawnPoint)
			m_SpawnPoint.RemoveEmptyChildren();
	}
};