[EntityEditorProps(category: "GameScripted/GameMode", description: "Spawn point entity", visible: false)]
class SCR_SpawnPointClass : SCR_PositionClass
{
	// [Attribute()]
	// protected ref SCR_UIInfo m_Info;

	// SCR_UIInfo GetInfo()
	// {
	// 	return m_Info;
	// }
};

//------------------------------------------------------------------------------------------------
//! Spawn point entity defines positions on which players can possibly spawn.
class SCR_SpawnPoint : SCR_Position
{
	[Attribute("0", desc: "Find empty position for spawning within given radius. When none is found, entity position will be used.")]
	protected float m_fSpawnRadius;
	
	[Attribute("10", UIWidgets.EditBox, "Determines how close a player has to be to disable this spawn point.")]
	private float m_fNoPlayerRadius;

	[Attribute("50", UIWidgets.EditBox, "Determines how close a player looking at the spawn point has to be to disable it.")]
	private float m_fNoSightRadius;

	[Attribute("Red", UIWidgets.EditBox, "Determines which faction can spawn on this spawn point."), RplProp(onRplName: "OnSetFactionKey")]
	private string m_sFaction;

	[Attribute("0")]
	protected bool m_bShowInDeployMapOnly;

	protected SCR_UIInfo m_LinkedInfo;
	protected SCR_FactionControlComponent m_FactionControl;

	[Attribute()]
	protected ref SCR_UIInfo m_Info;

	// List of all spawn points
	private static ref array<SCR_SpawnPoint> m_aSpawnPoints = new ref array<SCR_SpawnPoint>();

	static ref ScriptInvoker Event_OnSpawnPointCountChanged = new ScriptInvoker();
	static ref ScriptInvoker Event_SpawnPointFactionAssigned = new ScriptInvoker();
	static ref ScriptInvoker Event_SpawnPointRemoved = new ScriptInvoker();

	protected static ref map<SCR_SpawnPoint, RplId> s_mSpawnPoints = new map<SCR_SpawnPoint, RplId>();
	static RplId s_LastUsed = RplId.Invalid();

	// spawn point will work as a spawn point group if it has any SCR_Position as its children
	protected ref array<SCR_Position> m_aChildren = {};

	//------------------------------------------------------------------------------------------------
	/*!
	\return Radius in which players can be spawned on empty position.
	*/
	float GetSpawnRadius()
	{
		return m_fSpawnRadius;
	}
	//------------------------------------------------------------------------------------------------
	static void ShowSpawnPointDescriptors(bool show, Faction faction)
	{
		if (!m_aSpawnPoints)
			return;

		string factionKey = string.Empty;
		if (faction)
			factionKey = faction.GetFactionKey();

		foreach (SCR_SpawnPoint spawnPoint : m_aSpawnPoints)
		{
			if (!spawnPoint)
				continue;

			auto mapDescriptor = SCR_MapDescriptorComponent.Cast(spawnPoint.FindComponent(SCR_MapDescriptorComponent));
			if (!mapDescriptor)
				continue;

			bool visible = show && !factionKey.IsEmpty() && spawnPoint.GetFactionKey() == factionKey;
			if (mapDescriptor.Item())
				mapDescriptor.Item().SetVisible(visible);
		}
	}

	//------------------------------------------------------------------------------------------------
	static RplId GetSpawnPointRplId(SCR_SpawnPoint sp)
	{
		return s_mSpawnPoints.Get(sp);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_SpawnPoint GetSpawnPointByRplId(RplId id)
	{
		return s_mSpawnPoints.GetKeyByValue(id);
	}

	//------------------------------------------------------------------------------------------------
	void GetPositionAndRotation(out vector pos, out vector rot)
	{
		if (m_aChildren.Count() > 1)
		{
			int id = m_aChildren.GetRandomIndex();
			pos = m_aChildren[id].GetOrigin();
			rot = m_aChildren[id].GetAngles();
		}
		else
		{
			SCR_WorldTools.FindEmptyTerrainPosition(pos, GetOrigin(), GetSpawnRadius());
			rot = GetAngles();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Return spawn point or null if out of bounds
	static SCR_SpawnPoint GetSpawnPointByIndex(int spawnPointIndex)
	{
		if (spawnPointIndex >= 0 && spawnPointIndex < m_aSpawnPoints.Count())
			return m_aSpawnPoints[spawnPointIndex];

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Return spawn point index or -1 if not existant
	static int GetSpawnPointIndex(SCR_SpawnPoint spawnPoint)
	{
		return m_aSpawnPoints.Find(spawnPoint);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	\return Number of spawn points in the world
	*/
	static int CountSpawnPoints()
	{
		return m_aSpawnPoints.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param faction determines which faction is allied.
	//! \return a spawn point that is safe.
	static SCR_SpawnPoint FindSafeSpawnPoint(string faction)
	{
		// todo(koudelkaluk): revise
		if (!s_mSpawnPoints.IsEmpty())
		{
			array<SCR_SpawnPoint> spawnPoints = GetSpawnPointsForFaction(faction);
			if (!spawnPoints.IsEmpty())
			{
				int index = Math.RandomInt(0, spawnPoints.Count());
				return spawnPoints[index];
			}
		}

		if (!m_aSpawnPoints)
			return null;

		int spawnPointsCount = m_aSpawnPoints.Count();
		int randomIndex = Math.RandomInt(0, spawnPointsCount);

		// Pick spawn point
		for (int i = 0; i < spawnPointsCount; i++)
		{
			int offsetIndex = i + randomIndex;

			if (offsetIndex > spawnPointsCount-1)
				offsetIndex = spawnPointsCount - 1 - i;

			SCR_SpawnPoint sp = m_aSpawnPoints[offsetIndex];
			if (sp && sp.GetIsSafe(faction))
				return sp;
		}

		//No safe spawn point found, pick a random one
		return SCR_SpawnPoint.GetRandomSpawnPointDeathmatch();

		// Get spawn point with lowest threat level, TODO
	}


	//------------------------------------------------------------------------------------------------
	//! \param faction determines which faction is allied.
	//! \return whether is safe or not.
	bool GetIsSafe(string faction)
	{
		if (m_sFaction != faction)
			return false;

		bool playerFree = GetIsPlayerFree(faction);
		if (!playerFree)
			return false;

		bool visible = GetIsVisible(faction);
		if (visible)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param faction determines which faction is allied.
	//! \return whether is safe or not.
	bool GetIsPlayerFree(string faction)
	{
		array<SCR_ChimeraCharacter> characters = SCR_CharacterRegistrationComponent.GetChimeraCharacters();
		if (!characters)
			return true;

		foreach (auto character: characters)
		{
			string characterFaction = character.GetFactionKey();
			if (characterFaction == FactionKey.Empty && faction == FactionKey.Empty)
				return !GetIsInRange(character, m_fNoPlayerRadius);

			if (characterFaction != string.Empty && characterFaction != faction) //TODO: Check faction friendliness.
				return !GetIsInRange(character, m_fNoPlayerRadius);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param faction determines which faction is allied.
	//! \return whether is visible or not.
	bool GetIsVisible(string faction)
	{
		array<SCR_ChimeraCharacter> characters = SCR_CharacterRegistrationComponent.GetChimeraCharacters();
		if (!characters)
			return true;

		foreach (auto character: characters)
		{
			string characterFaction = character.GetFactionKey();
			if (characterFaction != string.Empty && characterFaction != faction && GetIsInRange(character, m_fNoSightRadius)) //TODO: Check faction friendliness.
			{
				auto characterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
				return characterController && characterController.GetPositionInView(GetOrigin(), 50);
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool GetVisibleInDeployMapOnly()
	{
		return m_bShowInDeployMapOnly;
	}

	//------------------------------------------------------------------------------------------------
	void SetFaction(Faction faction)
	{
		if (!faction)
			SetFactionKey(string.Empty);
		else
			SetFactionKey(faction.GetFactionKey());
	}

	//------------------------------------------------------------------------------------------------
	void SetFactionKey(string factionKey)
	{
		if (factionKey == m_sFaction)
			return;
		
		m_sFaction = factionKey;
		OnSetFactionKey();
		Replication.BumpMe();
	}
	protected void OnSetFactionKey()
	{
		Event_SpawnPointFactionAssigned.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	array<SCR_Position> GetChildSpawnPoints()
	{
		return m_aChildren;
	}

	//------------------------------------------------------------------------------------------------
	//! \return string name of the faction of the spawn point.
	string GetFactionKey()
	{
		return m_sFaction;
	}

	//------------------------------------------------------------------------------------------------
	//! \param character determines which entity is being checked.
	//! \param range determines the max range.
	//! \return whether is in range or not.
	private bool GetIsInRange(ChimeraCharacter character, float range)
	{
		return character && vector.DistanceSq(character.GetOrigin(), GetOrigin()) < range*range;
	}

	//------------------------------------------------------------------------------------------------
	//! \return an array of all spawn point entities.
	static array<SCR_SpawnPoint> GetSpawnPoints()
	{
		return m_aSpawnPoints;
	}

	//------------------------------------------------------------------------------------------------
	//! \return random spawn point without limitations.
	static SCR_SpawnPoint GetRandomSpawnPointDeathmatch()
	{
		if (!m_aSpawnPoints || m_aSpawnPoints.IsEmpty())
			return null;

		return m_aSpawnPoints.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! \return random spawn point for a character.
	static SCR_SpawnPoint GetRandomSpawnPoint(SCR_ChimeraCharacter character)
	{
		if (!character)
			return null;

		return GetRandomSpawnPointForFaction(character.GetFactionKey());
	}

	//------------------------------------------------------------------------------------------------
	//! \return spawn points valid for for local player.
	static array<SCR_SpawnPoint> GetSpawnPointsForPlayer(SCR_ChimeraCharacter character)
	{
		string factionKey = string.Empty;
		if (character)
			factionKey = character.GetFactionKey();

		return GetSpawnPointsForFaction(factionKey);
	}

	//------------------------------------------------------------------------------------------------
	//! Get spawn points valid for given faction
	//! \param factionKey Valid faction key
	static array<SCR_SpawnPoint> GetSpawnPointsForFaction(string factionKey)
	{
		array<SCR_SpawnPoint> factionSpawnPoints = new array<SCR_SpawnPoint>();
		if (factionKey.IsEmpty())
			return factionSpawnPoints;

		array<SCR_SpawnPoint> spawnPoints = GetSpawnPoints();
		foreach (SCR_SpawnPoint spawnPoint : spawnPoints)
		{
			if (spawnPoint && spawnPoint.GetFactionKey() == factionKey)
				factionSpawnPoints.Insert(spawnPoint);
		}
		return factionSpawnPoints;
	}
	//------------------------------------------------------------------------------------------------
	//! Get count of spawn points belonging to given faction.
	//! \param factionKey Valid faction key
	//! \return Number of spawn points
	static int GetSpawnPointCountForFaction(string factionKey)
	{
		if (factionKey.IsEmpty())
			return 0;

		int count = 0;
		foreach (SCR_SpawnPoint spawnPoint: GetSpawnPoints())
		{
			if (!spawnPoint) continue;
			if (spawnPoint.GetFactionKey() == factionKey)
				count++;
		}
		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! \return random spawn point valid for a specific faction.
	static SCR_SpawnPoint GetRandomSpawnPointForFaction(string factionKey)
	{
		array<SCR_SpawnPoint> spawnPoints = GetSpawnPointsForFaction(factionKey);
		if (!spawnPoints.IsEmpty())
			return spawnPoints.GetRandomElement();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	// todo(koudelkaluk): get back to this

	//------------------------------------------------------------------------------------------------
	// SCR_UIInfo GetInfo()
	// {
	// 	if (m_LinkedInfo)
	// 		return m_LinkedInfo;
	// 	else
	// 	    return GetInfoFromPrefab();
	// }

	// protected SCR_UIInfo GetInfoFromPrefab()
	// {

		// SCR_SpawnPointClass prefabData = SCR_SpawnPointClass.Cast(GetPrefabData());
		// if (!prefabData)
		// 	return null;

		// return prefabData.GetInfo();
	// }

	//------------------------------------------------------------------------------------------------
	SCR_UIInfo GetInfo()
	{
	 	if (m_LinkedInfo)
	 		return m_LinkedInfo;
	 	else
	 	    return m_Info;
	}

	//------------------------------------------------------------------------------------------------
	void LinkInfo(SCR_UIInfo info)
	{
		m_LinkedInfo = info;
	}
	//------------------------------------------------------------------------------------------------
	/*!
	Event called on server when player is spawned on this point.
	To be overriden by inherited classes.
	\param entity Player entity
	*/
	event void EOnPlayerSpawn(IEntity entity)
	{
	}
	//------------------------------------------------------------------------------------------------

#ifdef WORKBENCH
	override void SetColorAndText()
	{
		m_sText = m_sFaction;
		
		// Fetch faction data
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			Faction faction = factionManager.GetFactionByKey(m_sFaction);
			if (faction)
			{
				m_iColor = faction.GetFactionColor().PackToInt();
			}
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		RplId id = Replication.FindId(this);
		s_mSpawnPoints.Set(this, id);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		if (!GetGame().GetWorldEntity())
  			return;

		RplComponent rpl = RplComponent.Cast(FindComponent(RplComponent));
		if (rpl)
		{
			rpl.InsertToReplication();

			RplId id = Replication.FindId(this);
			s_mSpawnPoints.Set(this, id);
		}

		IEntity child = GetChildren();
		while (child)
		{
			if (SCR_Position.Cast(child))
				m_aChildren.Insert(SCR_Position.Cast(child));
			child = child.GetSibling();
		}
		
		m_FactionControl = SCR_FactionControlComponent.Cast(owner.FindComponent(SCR_FactionControlComponent));
		if (m_FactionControl)
		{
			m_FactionControl.GetOnFactionChanged().Insert(SetFaction);
			Faction faction = m_FactionControl.GetFaction();
			if (faction)
			{
				m_sFaction = faction.GetFactionKey();
			}
		}		

		ClearFlags(EntityFlags.ACTIVE, false);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_SpawnPoint(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.STATIC, true);

		if (GetGame().GetWorldEntity())
		{
			m_aSpawnPoints.Insert(this);
			Event_OnSpawnPointCountChanged.Invoke(m_sFaction);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SpawnPoint()
	{
		if (m_aSpawnPoints)
		{
			m_aSpawnPoints.RemoveItem(this);
		}
		Event_OnSpawnPointCountChanged.Invoke(m_sFaction);
		Event_SpawnPointRemoved.Invoke(this);
		if (s_mSpawnPoints)
		{
			if (GetSpawnPointByRplId(s_LastUsed) == this)
				s_LastUsed = RplId.Invalid();
			s_mSpawnPoints.Remove(this);
		}
		if (m_FactionControl)
		{
			m_FactionControl.GetOnFactionChanged().Remove(SetFaction);
		}
	}
};
