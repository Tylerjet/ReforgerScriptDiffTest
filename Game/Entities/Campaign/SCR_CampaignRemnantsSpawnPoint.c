[EntityEditorProps(category: "GameScripted/Campaign", description: "Spawn point for Remnant forces in Campaign.", color: "0 0 255 255")]
class SCR_CampaignRemnantsSpawnPointClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
//! Spawn point for Remnant forces in Campaign
class SCR_CampaignRemnantsSpawnPoint : GenericEntity
{
	[Attribute("-1", UIWidgets.EditBox, "Waypoints will be placed in ascending sequence. Valid only for hierarchy children.")]
	protected int m_iWaypointIndex;
	
	[Attribute("0", UIWidgets.ComboBox, "Valid only for hierarchy parent.", "", ParamEnumArray.FromEnum(SCR_CampaignRemnantsGroupType))]
	private SCR_CampaignRemnantsGroupType m_eGroupType;
	
	//------------------------------------------------------------------------------------------------
	int GetWaypointIndex()
	{
		return m_iWaypointIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignRemnantsGroupType GetGroupType()
	{
		return m_eGroupType;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// No need to register children unless the group type is specified (indicating default base defenders near HQ)
		if (GetParent() && m_eGroupType == SCR_CampaignRemnantsGroupType.RANDOM)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return;
		
		// Gamemode will take care of the rest
		campaign.RegisterRemnantsPresence(this);
		
		if (SCR_CampaignBase.Cast(GetParent()))
			campaign.RegisterRemnantsPresence(this, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignRemnantsSpawnPoint(IEntitySource src, IEntity parent)
	{
		#ifdef WORKBENCH
			SetFlags(EntityFlags.TRACEABLE, false);
		#else
			SetFlags(EntityFlags.NO_LINK, false);
		#endif
		
		SetEventMask(EntityEvent.INIT);
	}
};

//------------------------------------------------------------------------------------------------
enum SCR_CampaignRemnantsGroupType
{
	RANDOM,
	PATROL,
	MG,
	AT,
	SNIPER,
	FIRETEAM
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignRemnantsPresence
{
	protected int m_iID;
	protected vector m_vCenter;
	protected vector m_vSpawn;
	protected AIWaypoint m_Waypoint;
	protected int m_iMembersAlive = -1;
	protected ResourceName m_sPrefab;
	protected bool m_bSpawned = false;
	protected float m_fDespawnTimer = -1;
	protected SCR_AIGroup m_Group;
	protected bool m_bIsDefendersSpawn;
	protected SCR_CampaignBase m_ParentBase;
	protected Faction m_Faction;
	protected float m_fSpawnTime;
	static const int PARENT_BASE_DISTANCE_THRESHOLD = 300;
	static const int DEFENDERS_SPAWN_DELAY = 90;
	
	//------------------------------------------------------------------------------------------------
	void SetID(int ID)
	{
		m_iID = ID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetID()
	{
		return m_iID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCenter(vector center)
	{
		m_vCenter = center;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetCenter()
	{
		return m_vCenter;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpawnpoint(vector spawn)
	{
		m_vSpawn = spawn;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetSpawnpoint()
	{
		return m_vSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWaypoint(AIWaypoint wp)
	{
		m_Waypoint = wp;
	}
	
	//------------------------------------------------------------------------------------------------
	AIWaypoint GetWaypoint()
	{
		return m_Waypoint;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMembersAlive(int cnt)
	{
		m_iMembersAlive = cnt;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMembersAlive()
	{
		return m_iMembersAlive;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupPrefab(ResourceName prefab)
	{
		m_sPrefab = prefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetGroupPrefab()
	{
		return m_sPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsSpawned(bool spawned)
	{
		m_bSpawned = spawned;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsSpawned()
	{
		return m_bSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDespawnTimer(float time)
	{
		m_fDespawnTimer = time;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDespawnTimer()
	{
		return m_fDespawnTimer;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpawnedGroup(SCR_AIGroup group)
	{
		m_Group = group;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetSpawnedGroup()
	{
		return m_Group;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsDefendersSpawn(bool defender)
	{
		m_bIsDefendersSpawn = defender;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsDefendersSpawn()
	{
		return m_bIsDefendersSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParentBase(notnull SCR_CampaignBase base)
	{
		m_ParentBase = base;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBase GetParentBase()
	{
		return m_ParentBase;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDefendersFaction(notnull Faction faction)
	{
		m_Faction = faction;
	}
	
	//------------------------------------------------------------------------------------------------
	Faction GetDefendersFaction()
	{
		return m_Faction;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpawnTime(float time)
	{
		m_fSpawnTime = time;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSpawnTime()
	{
		return m_fSpawnTime;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_CampaignRemnantsGroup
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Group prefab", "et")]
	ResourceName m_Prefab;
	
	[Attribute("0.5", UIWidgets.Slider, "Probability of presence", "0 1 0.05")]
	float m_fProbability;
	
	[Attribute("1", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_CampaignRemnantsGroupType))]
	SCR_CampaignRemnantsGroupType m_eType;
};