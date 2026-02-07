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
	
	[Attribute("0", UIWidgets.EditBox, "How often will the group respawn. (0 = no respawn)")]
	protected int m_iGroupRespawnPeriod;
	
	[Attribute("0", UIWidgets.CheckBox, "This entity is used as a respawn position from which Remnants will move to their target.")]
	protected bool m_bIsRespawn;
	
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
	int GetGroupRespawnPeriod()
	{
		return m_iGroupRespawnPeriod;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsRespawn()
	{
		return m_bIsRespawn;
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
	protected SCR_CampaignBase m_ParentBase;
	protected float m_fRespawnTimestamp;
	protected int m_iRespawnPeriod;
	protected ref array<vector> m_aRespawns = {};
	static const int PARENT_BASE_DISTANCE_THRESHOLD = 300;
	static const int RESPAWN_PLAYER_DISTANCE_THRESHOLD = 200;
	
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
	float GetRespawnTimestamp()
	{
		return m_fRespawnTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRespawnTimestamp(float time)
	{
		m_fRespawnTimestamp = time;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRespawnPeriod()
	{
		return m_iRespawnPeriod;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRespawnPeriod(int time)
	{
		m_iRespawnPeriod = time;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddRespawn(vector respawn)
	{
		m_aRespawns.Insert(respawn);
	}
	
	//------------------------------------------------------------------------------------------------
	array<vector> GetRespawns()
	{
		return m_aRespawns;
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
	void SetParentBase(notnull SCR_CampaignBase base)
	{
		m_ParentBase = base;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBase GetParentBase()
	{
		return m_ParentBase;
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