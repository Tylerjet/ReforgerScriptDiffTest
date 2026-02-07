[EntityEditorProps(category: "GameScripted/Campaign", description: "Handles additional functionality at bunkers built in campaign.", color: "0 0 255 255")]
class SCR_BunkerServiceComponentClass: SCR_ServicePointComponentClass
{
	[Attribute("150", desc: "Range in which should component search for SCR_CampaignSpawnPointGroup.")]
	float m_fBaseSearchDistance;
};

//------------------------------------------------------------------------------------------------
class SCR_BunkerServiceComponent : SCR_ServicePointComponent
{	
	protected SCR_CampaignSpawnPointGroup m_SpawnPoint;
	
	//------------------------------------------------------------------------------------------------
	void SetSpawnPointGroup(SCR_CampaignSpawnPointGroup sp)
	{
		if (!sp)
			return;
		
		m_SpawnPoint = sp;
		AddSpawnPositions();
	}
	
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
	//! Callback for spawnpoint search function. Returns true, if no base was found
	protected bool SpawnpointSearchCallback(IEntity ent)
	{
		SCR_CampaignSpawnPointGroup sp = SCR_CampaignSpawnPointGroup.Cast(ent);
		if (!sp)
			return true;
		
		m_SpawnPoint = sp;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		SCR_BunkerServiceComponentClass prefabData = SCR_BunkerServiceComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;
		
		// returns if query was finished without finding base (TEMPORARY, will be changed as soon as Freeform basebuilding is available
		if (GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), prefabData.m_fBaseSearchDistance, SpawnpointSearchCallback, null, EQueryEntitiesFlags.ALL))
			Print("Bunker didn't find any Spawnpoint in its vicinity", LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	//------------------------------------------------------------------------------------------------
	void ~SCR_BunkerServiceComponent()
	{
		if (m_SpawnPoint)
			m_SpawnPoint.RemoveEmptyChildren();
	}
};