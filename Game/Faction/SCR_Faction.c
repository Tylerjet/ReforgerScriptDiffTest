//------------------------------------------------------------------------------------------------
class SCR_Faction : ScriptedFaction
{
	[Attribute(defvalue: "0", desc: "Order in which the faction appears in the list. Lower values are first.")]
	protected int m_iOrder;

	[Attribute("1 1 1", UIWidgets.ColorPicker, desc: "Outline faction color")]
	private ref Color m_OutlineFactionColor;

	[Attribute(defvalue: "1", desc: "Will the faction appear in the respawn menu?")]
	private bool m_bIsPlayable;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Flag icon of this particular faction.", params: "edds")]
	private ResourceName m_sFactionFlag;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Faction flag material, used on flag poles.", params: "emat")]
	protected ResourceName m_FactionFlagMaterial;

	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected EEditableEntityLabel m_FactionLabel;
	
	[Attribute("1", desc: "If this is false it would mean that every AI will be hostile towards their own faction members and essentially allow for Deathmatch. Use with caution, only checked on init, you can still set the faction hostile towards itself in runtime. This essentially makes sure it adds itself to FriendlyFactionsIds.")]
	protected bool m_bFriendlyToSelf;
	
	[Attribute(desc: "List of faction IDs that are considered friendly for this faction. Note: If factionA has factionB as friendly but FactionB does not have FactionA as friendly then they are still both set as friendly so for init it is only required for one faction.")]
	protected ref array<string> m_aFriendlyFactionsIds;

	[Attribute()]
	protected ref SCR_FactionCallsignInfo m_CallsignInfo;

	[Attribute(desc: "Group preset for predefined groups")]
	protected ref array<ref SCR_GroupPreset> m_aPredefinedGroups;

	[Attribute(desc: "Create only predefined groups")]
	protected bool m_bCreateOnlyPredefinedGroups;

	[Attribute("", uiwidget: UIWidgets.EditBox)]
	protected string m_sFactionRadioEncryptionKey;

	[Attribute("0", uiwidget: UIWidgets.EditBox)]
	protected int m_iFactionRadioFrequency;

	protected ref array<string>> m_aAncestors;
	protected ref ScriptInvoker Event_OnFactionPlayableChanged = new ref ScriptInvoker; //Gives Faction and Bool enabled

	[Attribute("", UIWidgets.Object, "List of ranks")]
	protected ref array<ref SCR_CharacterRank> m_aRanks;
	
	[Attribute(desc: "List of Entity catalogs. Each holds a list of entity Prefab and data of a given type. Catalogs of the same type are merged into one. Note this array is moved to a map on init and set to null")]
	protected ref array<ref SCR_EntityCatalog> m_aEntityCatalogs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Flag icons for group.", params: "edds")]
	protected ref array<ResourceName> m_aGroupFlags;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Flag imageset for groups of this faction.", params: "imageset")]
	protected ResourceName m_sGroupFlagsImageSet;

	[Attribute("List of flags from imageset")]
	protected ref array<string> m_aFlagNames;

	//~ Catalog map for quicker obtaining the catalog using EEntityCatalogType
	protected ref map<EEntityCatalogType, ref SCR_EntityCatalog> m_mEntityCatalogs = new ref map<EEntityCatalogType, ref SCR_EntityCatalog>();
	
	protected ref set<Faction> m_FriendlyFactions = new set<Faction>;
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Order in which the faction appears in the list. Lower values are first.
	*/
	int GetOrder()
	{
		return m_iOrder;
	}

	//------------------------------------------------------------------------------------------------
	bool GetCanCreateOnlyPredefinedGroups()
	{
		return m_bCreateOnlyPredefinedGroups;
	}
	
	//------------------------------------------------------------------------------------------------
	void GetPredefinedGroups(notnull array<ref SCR_GroupPreset> groupArray)
	{
		for (int i = 0, count = m_aPredefinedGroups.Count(); i < count; i++)
		{
			groupArray.Insert(m_aPredefinedGroups[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetFlagName(int index)
	{
		return m_aFlagNames[index];
	}

	//------------------------------------------------------------------------------------------------
	int GetFlagNames(out array<string> flagNames)
	{
		return flagNames.Copy(m_aFlagNames);
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetGroupFlagImageSet()
	{
		return m_sGroupFlagsImageSet;
	}

	//------------------------------------------------------------------------------------------------
	int GetGroupFlagTextures(out array<ResourceName> textures)
	{
		return textures.Copy(m_aGroupFlags);
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetFactionFlag()
	{
		return m_sFactionFlag;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetFactionFlagMaterial()
	{
		return m_FactionFlagMaterial;
	}

	//------------------------------------------------------------------------------------------------
	void SetFactionFlagMaterial(ResourceName materialResource)
	{
		m_FactionFlagMaterial = materialResource;
	}

	//------------------------------------------------------------------------------------------------
	EEditableEntityLabel GetFactionLabel()
	{
		return m_FactionLabel;
	}

	//------------------------------------------------------------------------------------------------
	SCR_FactionCallsignInfo GetCallsignInfo()
	{
		return m_CallsignInfo;
	}

	//------------------------------------------------------------------------------------------------
	Color GetOutlineFactionColor()
	{
		return m_OutlineFactionColor;
	}

	//------------------------------------------------------------------------------------------------
	//Called everywhere, used to generate initial data for this faction
	void InitializeFaction()
	{
	}

	void SetAncestors(array<string> ancestors)
	{
		m_aAncestors = {};
		m_aAncestors.Copy(ancestors);
	}

	//------------------------------------------------------------------------------------------------\
	/*!
	Check if the faction is playable.
	Non-playable factions will not appear in the respawn menu.
	\return True when playable
	*/
	bool IsPlayable()
	{
		return m_bIsPlayable;
	}

	/*!
	Init faction is playable.
	Called on Init (if server) and on server join (is Client)
	\param isPlayable Bool to set is playable
	*/
	void InitFactionIsPlayable(bool isPlayable)
	{
		m_bIsPlayable = isPlayable;
	}

	/*!
	Check if the faction is inherited from a faction with given faction key.
	\param factionKey Ancestor faction key
	\return True when inherited
	*/
	bool IsInherited(string factionKey)
	{
		return m_aAncestors && m_aAncestors.Contains(factionKey);
	}

	/*!
	Set if the faction is playable.
	Non-playable factions will not appear in the respawn menu.
	Note that this is not broadcasted on the SCR_Faction side
	\param isPlayable Bool to set is playable
	\param killPlayersIfNotPlayable Bool kills all players if on server if faction is set isplayable false
	*/
	void SetIsPlayable(bool isPlayable, bool killPlayersIfNotPlayable = false)
	{
		if (m_bIsPlayable == isPlayable)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager || !factionManager.CanChangeFactionsPlayable())
			return;

		m_bIsPlayable = isPlayable;
		Event_OnFactionPlayableChanged.Invoke(this, m_bIsPlayable);

		//Kill players if m_bIsPlayable is false, killPlayersIfNotPlayable is true, of the same faction and is server
		if (!m_bIsPlayable && killPlayersIfNotPlayable && Replication.IsServer())
		{
			array<int> playerList = new array<int>;
			GetGame().GetPlayerManager().GetPlayers(playerList);

			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			IEntity playerEntity;

			foreach (int playerId : playerList)
			{
				Faction playerFaction = factionManager.GetPlayerFaction(playerId);
				if (!playerFaction)
					continue;

				//Check if has the same faction as the one disabled
				if (playerFaction.GetFactionKey() != GetFactionKey())
					continue;

				//Do not kill GMs
				if (core)
				{
					SCR_EditorManagerEntity editorManager = core.GetEditorManager(playerId);
					if (editorManager)
					{
						if (!editorManager.IsLimited())
							continue;
					}
				}

				playerEntity = SCR_PossessingManagerComponent.GetPlayerMainEntity(playerId);
				if (!playerEntity)
					continue;

				DamageManagerComponent damageManager = DamageManagerComponent.Cast(playerEntity.FindComponent(DamageManagerComponent));
				if (damageManager)
					damageManager.SetHealthScaled(0);
			}
		}
	}

	/*!
	Get On Playable Changed Script Invoker
	\return ScriptInvoker Event_OnFactionPlayableChanged
	*/
	ScriptInvoker GetOnFactionPlayableChanged()
	{
		return Event_OnFactionPlayableChanged;
	}
	
	//======================================== FACTION RELATIONS ========================================\\
	//~ Check if faction is friendly
	override bool DoCheckIfFactionFriendly(Faction faction)
	{
		return m_FriendlyFactions.Contains(faction);
	}
	
	//------------------------------------------------------------------------------------------------	
	/*!
	Add given faction as friendly towards this faction (Server Only)
	Called by SCR_FactionManager please use the SCR_FactionManager.SetFactionsFriendly function instead of this to make sure the setting is mirrored!
	If you add factionA friendly towards factionB but not the other way around then factionA will not retaliate if shot at by factionB
	\param faction Faction to set friendly
	*/
	void SetFactionFriendly(notnull Faction faction)
	{
		m_FriendlyFactions.Insert(faction);	
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Remove given faction as friendly towards this faction (Server Only)
	Called by SCR_FactionManager please use the SCR_FactionManager.SetFactionsHostile function instead of this to make sure the setting is mirrored!
	If you add factionB hostile towards factionA but not the other way around then factionA will not retaliate if shot at by factionB
	\param faction Faction to set hostile
	*/
	void SetFactionHostile(notnull Faction faction)
	{
		int index = m_FriendlyFactions.Find(faction);
		
		if (index >= 0)
			m_FriendlyFactions.Remove(index);
	}

	/*!
	Get the number of players assigned to this faction
	*/
	int GetPlayerCount()
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return -1;
		
		return factionManager.GetFactionPlayerCount(this);
		/*SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnSystem)
			return -1;

		array<int> players = {};
		int playerCount = 0;

		PlayerManager pm = GetGame().GetPlayerManager();
		pm.GetPlayers(players);

		foreach (int playerId : players)
		{
			Faction playerFaction = respawnSystem.GetPlayerFaction(playerId);
			if (playerFaction == this)
				playerCount++;
		}

		return playerCount;*/
	}
	
	//------------------------------------------------------------------------------------------------
	/*
	Get a list of all players beloning to the faction
	\param[out] player ids List of players belonging to the faction
	\return Number of players in faction
	*/
	int GetPlayersInFaction(notnull out array<int> players)
	{
		players.Clear();
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return -1;

		array<int> allPlayers = {};
		GetGame().GetPlayerManager().GetPlayers(allPlayers);

		foreach (int playerId : allPlayers)
		{
			Faction playerFaction = factionManager.GetPlayerFaction(playerId);
			if (playerFaction == this)
				players.Insert(playerId);
		}

		return players.Count();
	}

	//------------------------------------------------------------------------------------------------
	string GetFactionRadioEncryptionKey()
	{
		return m_sFactionRadioEncryptionKey;
	}

	//------------------------------------------------------------------------------------------------
	int GetFactionRadioFrequency()
	{
		return m_iFactionRadioFrequency;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CharacterRank GetRankByID(SCR_ECharacterRank rankID)
	{
		if (!m_aRanks)
			return null;

		foreach (SCR_CharacterRank rank : m_aRanks)
		{
			if (rank.GetRankID() == rankID)
				return rank;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	string GetRankName(SCR_ECharacterRank rankID)
	{
		SCR_CharacterRank rank = GetRankByID(rankID);

		if (rank)
			return rank.GetRankName();
		else
			return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	string GetRankNameUpperCase(SCR_ECharacterRank rankID)
	{
		SCR_CharacterRank rank = GetRankByID(rankID);

		if (rank)
			return rank.GetRankNameUpperCase();
		else
			return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	string GetRankNameShort(SCR_ECharacterRank rankID)
	{
		SCR_CharacterRank rank = GetRankByID(rankID);

		if (rank)
			return rank.GetRankNameShort();
		else
			return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	string GetRankInsignia(SCR_ECharacterRank rankID)
	{
		SCR_CharacterRank rank = GetRankByID(rankID);

		if (rank)
			return rank.GetRankInsignia();
		else
			return string.Empty;
	}
	
	//======================================== FACTION ENTITY CATALOG ========================================\\
	//--------------------------------- Get Entity Catalog ---------------------------------\\
	/*!
	Get Entity catalog of specific type linked to faction
	The catalog contains all entities part of the faction of that specific type
	\param catalogType Type to get catalog of
	\return Catalog. Can be null if not found. Note that Only one catalog of each type can be in the list
	*/
	SCR_EntityCatalog GetFactionEntityCatalogOfType(EEntityCatalogType catalogType)
	{
		//~ Get catalog 
		SCR_EntityCatalog entityCatalog = m_mEntityCatalogs.Get(catalogType);
		
		if (entityCatalog)
			return entityCatalog;
	
		//~ No data found
		Print(string.Format("'SCR_Faction' trying to get entity list of type '%1' but there is no catalog with that type for faction '%2'", typename.EnumToString(EEntityCatalogType, catalogType), GetFactionKey()), LogLevel.WARNING);
		return null;
	}
	
	//--------------------------------- Get Entity All Catalogs ---------------------------------\\
	/*!
	Get all entity catalogs within the faction
	The catalogs contain all entities part of the faction
	\param[out] List of all catalogs within the faction
	\return List size
	*/
	int GetAllFactionEntityCatalogs(notnull out array<SCR_EntityCatalog> outEntityCatalogs)
	{
		outEntityCatalogs.Clear();
		foreach (SCR_EntityCatalog entityCatalog: m_mEntityCatalogs)
	    {
	        outEntityCatalogs.Insert(entityCatalog);
	    }
		
		return outEntityCatalogs.Count();
	}
	
	//--------------------------------- Init ---------------------------------\\
	override void Init(IEntity owner)
	{
		super.Init(owner);
		
		if (SCR_Global.IsEditMode()) 
			return;
		
		//~ Set faction friendly
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
		{
			//~ Still make sure faction is friendly towards itself	
			if (m_bFriendlyToSelf)
				SetFactionFriendly(this);
			
			Print("'SCR_Faction' is trying to set friendly factions but no SCR_FactionManager could be found!", LogLevel.ERROR);
		}
		else 
		{
			//~ Make sure faction is friendly towards itself
			if (m_bFriendlyToSelf)
				factionManager.SetFactionsFriendly(this, this);
			
			//~ On init friendly factions assigning
			if (!m_aFriendlyFactionsIds.IsEmpty())
			{			
				SCR_Faction faction;
				
				//~ Set each given faction ID as friendly
				foreach (string factionId : m_aFriendlyFactionsIds)
				{
					faction = SCR_Faction.Cast(factionManager.GetFactionByKey(factionId));
					
					if (!faction)
					{
						Print(string.Format("'SCR_Faction' is trying to set friendly factions on init but '%1' is not a valid SCR_Faction!", factionId), LogLevel.ERROR);
						continue;
					}
					
					//~ Don't set self as friendly
					if (faction == this)
						continue;
					
					//~ Assign as friendly
					factionManager.SetFactionsFriendly(this, faction);
				}
			}
		}
		
		//~ Init the catalog for faster processing
		SCR_EntityCatalogManagerComponent.InitCatalogs(m_aEntityCatalogs, m_mEntityCatalogs);
		
		//~ Clear array as no longer needed
		m_aEntityCatalogs = null;
	}
};
