//------------------------------------------------------------------------------------------------
class SCR_Faction : ScriptedFaction
{
	[Attribute("1 1 1", UIWidgets.ColorPicker, desc: "Outline faction color")]
	private ref Color m_OutlineFactionColor;
	
	[Attribute(defvalue: "1", desc: "Will the faction appear in the respawn menu?")]
	private bool m_bIsPlayable;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Flag icon of this particular faction.", params: "edds")]
	private ResourceName m_sFactionFlag;
		
	[Attribute()]
	protected ref SCR_FactionCallsignInfo m_CallsignInfo;
	
	[Attribute()]
	protected ref SCR_ArsenalItemListConfig m_ArsenalConfig;
	
	[Attribute(desc: "List of vehicles related to this faction.")]
	protected ref SCR_EntityAssetList m_aVehicleList;
	
	protected ref array<string>> m_aAncestors;
	protected ref ScriptInvoker Event_OnFactionPlayableChanged = new ref ScriptInvoker; //Gives Faction and Bool enabled
	
	protected ref map<SCR_EArsenalItemType, ref array<SCR_ArsenalItem>> m_mArsenalItemsByType = new map<SCR_EArsenalItemType, ref array<SCR_ArsenalItem>>();
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetFactionFlag()
	{
		return m_sFactionFlag;
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
			SCR_RespawnSystemComponent respawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
			if (!respawnSystemComponent)
				return;
			
			array<int> playerList = new array<int>;
			GetGame().GetPlayerManager().GetPlayers(playerList);
			
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			IEntity playerEntity;
			
			foreach(int playerId: playerList)
			{
				Faction playerFaction = respawnSystemComponent.GetPlayerFaction(playerId);
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
	
	/*!
	Get SCR_EArsnelItemType for given prefab
	Values taken from SCR_ArsenalItemListConfig
	\param itemType SCR_EArsenalItemType of the prefab
	\return bool False if no config is set
	*/
	bool GetArsenalItemTypeForPrefab(ResourceName prefab, out SCR_EArsenalItemType itemType)
	{
		if (m_ArsenalConfig)
		{
			return m_ArsenalConfig.GetItemTypeForPrefab(prefab, itemType);
		}
		return false;
	}
	
	/*!
	Get SCR_EArsenalItemMode for given prefab
	Values taken from SCR_ArsenalItemListConfig
	\param itemType SCR_EArsenalItemType of the prefab
	\return bool False if no config is set
	*/
	bool GetArsenalItemModeForPrefab(ResourceName prefab, out SCR_EArsenalItemMode itemMode)
	{
		if (m_ArsenalConfig)
		{
			return m_ArsenalConfig.GetItemModeForPrefab(prefab, itemMode);
		}
		return false;
	}
	
	/*!
	Get all arsenal items configured on the faction
	Values taken from SCR_ArsenalItemListConfig
	\param arsenalItems output array
	\return bool False if no config is set and when 0 items are configured
	*/
	bool GetArsenalItems(out array<ref SCR_ArsenalItem> arsenalItems)
	{
		if (m_ArsenalConfig)
		{
			return m_ArsenalConfig.GetArsenalItems(arsenalItems);
		}
		return false;
	}
	
	/*!
	Get arsenal items filtered by SCR_EArsenalItemType filter, caches values
	\param filter Combined flags for available items for this faction (RIFLE, MAGAZINE, EQUIPMENT, RADIOBACKPACK etc.)
	\return array with availabe arsenal items of give filter types
	*/
	array<SCR_ArsenalItem> GetFilteredArsenalItems(SCR_EArsenalItemType typeFilter, SCR_EArsenalItemMode modeFilter)
	{
		array<SCR_ArsenalItem> filteredItems = new array<SCR_ArsenalItem>();
		
		array<SCR_ArsenalItem> itemsByType = m_mArsenalItemsByType.Get(typeFilter);
		if (!itemsByType)
		{
			itemsByType = new ref array<SCR_ArsenalItem>();
			array<ref SCR_ArsenalItem> availableArsenalItems;
			if (GetArsenalItems(availableArsenalItems))
			{
				for (int i = 0, count = availableArsenalItems.Count(); i < count; i++)
				{
					if (availableArsenalItems[i].GetItemType() & typeFilter)
					{
						itemsByType.Insert(availableArsenalItems[i]);
					}
				}
				
				m_mArsenalItemsByType.Insert(typeFilter, itemsByType);
			}
		}
		
		foreach	(SCR_ArsenalItem item : itemsByType)
		{
			if (item.GetItemMode() & modeFilter)
			{
				filteredItems.Insert(item);
			}
		}
		return filteredItems;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the vehicle list assigned to this faction
	\return SCR_EntityAssetList Vehicle List
	*/
	SCR_EntityAssetList GetVehicleList()
	{
		return m_aVehicleList;
	}

	/*!
	Get the number of players assigned to this faction
	*/
	int GetPlayerCount()
	{
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
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

		return playerCount;
	}
};
