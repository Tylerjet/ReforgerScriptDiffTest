class SCR_ArsenalPlayerLoadout
{
	string loadout;
	ref SCR_PlayerLoadoutData loadoutData;
	float suppliesCost = 0.0;
}

//~ Scriptinvokers
void SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged(int playerId, bool hasValidLoadout);
typedef func SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged;

[ComponentEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_ArsenalManagerComponentClass : SCR_BaseGameModeComponentClass
{
	//------------------------------------------------------------------------------------------------
	static override array<typename> Requires(IEntityComponentSource src)
	{
		array<typename> requires = {};
		
		requires.Insert(SerializerInventoryStorageManagerComponent);
		
		return requires;
	}
}

class SCR_ArsenalManagerComponent : SCR_BaseGameModeComponent
{
	protected static SCR_ArsenalManagerComponent s_Instance;
	
	protected static const ref array<string> ARSENALLOADOUT_COMPONENTS_TO_CHECK = {
		"SCR_CharacterInventoryStorageComponent",
		"SCR_UniversalInventoryStorageComponent",
		"EquipedWeaponStorageComponent",
		"ClothNodeStorageComponent"
	};
	
	[Attribute("{27F28CF7C6698FF8}Configs/Arsenal/ArsenalSaveTypeInfoHolder.conf", desc: "Holds a list of save types than can be used for arsenals. Any new arsenal save type should be added to the config to allow it to be set by Editor", params: "conf class=SCR_ArsenalSaveTypeInfoHolder")]
	protected ResourceName m_sArsenalSaveTypeInfoHolder;
	
	[Attribute("{183361B6DA2C304F}Configs/Arsenal/ArsenalLoadoutSaveBlacklists.conf", desc: "This is server only, A blacklist of entities that are not allowed to be saved at arsenals if the blacklist the item is in is enabled. Can be null.",params: "conf class=SCR_LoadoutSaveBlackListHolder")]
	protected ResourceName m_sLoadoutSaveBlackListHolder;
	
	[Attribute()]
	protected bool m_bDisable
	
	//=== Authority
	protected ref map<string, ref SCR_ArsenalPlayerLoadout> m_aPlayerLoadouts = new map<string, ref SCR_ArsenalPlayerLoadout>();
	
	//=== Broadcast
	protected ref ScriptInvokerBase<SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged> m_OnPlayerLoadoutUpdated = new ScriptInvokerBase<SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged>();
	
	protected ref SCR_ArsenalSaveTypeInfoHolder m_ArsenalSaveTypeInfoHolder;
	protected ref SCR_LoadoutSaveBlackListHolder m_LoadoutSaveBlackListHolder;
	
	protected bool m_bLocalPlayerLoadoutAvailable;
	ref SCR_PlayerLoadoutData m_bLocalPlayerLoadoutData;
	
	[Attribute("0", desc: "Cost multiplier for supplies on spawning. 0 means the cost is free. Only used in gamemodes that support supply cost on spawning", params: "-1 inf")]//, RplProp(onRplName: "OnLoadoutSpawnSupplyCostMultiplierChanged")]
	protected float m_fLoadoutSpawnSupplyCostMultiplier;
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] arsenalManager
	//! \return
	static bool GetArsenalManager(out SCR_ArsenalManagerComponent arsenalManager)
	{
		arsenalManager = s_Instance;
		return s_Instance != null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get Save type info holder
	SCR_ArsenalSaveTypeInfoHolder GetArsenalSaveTypeInfoHolder()
	{
		return m_ArsenalSaveTypeInfoHolder;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get Loadout save blacklist holder
	SCR_LoadoutSaveBlackListHolder GetLoadoutSaveBlackListHolder()
	{
		return m_LoadoutSaveBlackListHolder;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerChangedFaction(int playerId, SCR_PlayerFactionAffiliationComponent playerFactionAffiliation, Faction faction)
	{
		string playerUID = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);
		if (!m_aPlayerLoadouts.Contains(playerUID))
			return;
		
		/*Faction playerFaction = playerFactionAffiliation.GetAffiliatedFaction();
		
		//~ Remove loadout from map on server and inform players
		//! Don't remove player loadout if faction is null (aka disconnect)
		if (playerFaction && faction)
		{
			m_aPlayerLoadouts.Remove(playerUID);
			DoPlayerClearHasLoadout(playerId);
			Rpc(DoPlayerClearHasLoadout, playerId);
		}*/
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update BlackList active states (Server only)
	//! \param[in] orderedActiveStates An ordered list of all blacklists and their new active state
	//! \param[in] clearExistingLoadouts
	//! \param[in] editorPlayerIdClearedLoadout
	void SetLoadoutBlackListActiveStates(notnull array<bool> orderedActiveStates, bool clearExistingLoadouts, int editorPlayerIdClearedLoadout = -1)
	{
		if (!m_LoadoutSaveBlackListHolder || !GetGameMode().IsMaster())
			return;
		
		if (editorPlayerIdClearedLoadout > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_CHANGED_LOADOUT_SAVE_BLACKLIST, editorPlayerIdClearedLoadout);
		
		m_LoadoutSaveBlackListHolder.SetOrderedBlackListsActive(orderedActiveStates);
		
		//~ Clear existing loadouts
		if (clearExistingLoadouts)
		{
			//~ Clear local loadout
			Rpc(RPC_OnPlayerLoadoutCleared, editorPlayerIdClearedLoadout);
			RPC_OnPlayerLoadoutCleared(editorPlayerIdClearedLoadout);
			
			//~ Clear existing loadouts on server
			m_aPlayerLoadouts.Clear();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_OnPlayerLoadoutCleared(int playerIdClearedLoadout)
	{
		if (!m_bLocalPlayerLoadoutAvailable)
			return;
		
		//~ No local loadout availible anymore
		m_bLocalPlayerLoadoutAvailable = false;
		
		//~ Player loadout was cleared. This can happen when respawn menu is open and needs to be refreshed
		m_OnPlayerLoadoutUpdated.Invoke(SCR_PlayerController.GetLocalPlayerId(), false);
		
		//~ Send notification to player to inform them their loadout was cleared
		if (playerIdClearedLoadout > 0)
			SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_CLEARED_BY_EDITOR);
		else 
			SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_CLEARED);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetLocalPlayerLoadoutAvailable()
	{
		return m_bLocalPlayerLoadoutAvailable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	 ScriptInvokerBase<SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged> GetOnLoadoutUpdated()
	{
		return m_OnPlayerLoadoutUpdated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Authority side
	//! \param[in] playerId
	//! \param[out] playerLoadout
	//! \return
	bool GetPlayerArsenalLoadout(string playerUID, out SCR_ArsenalPlayerLoadout playerLoadout)
	{
		return m_aPlayerLoadouts.Find(playerUID, playerLoadout) && playerLoadout.loadout != string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ TODO: Add ability to set Cost multiplier in run time
	/*void SetLoadoutSpawnCostMultiplier(float multiplier, int playerID)
	{
		//~ Same or invalid values
		if (m_fLoadoutSpawnSupplyCostMultiplier == multiplier || multiplier < 0)
			return;
		
		m_fLoadoutSpawnSupplyCostMultiplier = multiplier;
		Replication.BumpMe();
		
		//~ Notification
		if (playerID > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_CHANGED_SPAWN_SUPPLYCOST_MULTIPLIER, playerID, m_fLoadoutSpawnSupplyCostMultiplier * 1000);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if ((gameMode && gameMode.IsMaster()) || (!gameMode && Replication.IsServer()))
			OnLoadoutSupplyCostMultiplierChanged();
	}*/
	
	//------------------------------------------------------------------------------------------------
	//! \return The spawn cost multiplier. Will always be 0 if supplies are disabled
	float GetLoadoutSpawnSupplyCostMultiplier()
	{
		if (!SCR_ResourceSystemHelper.IsGlobalResourceTypeEnabled())
			return 0;
		
		return m_fLoadoutSpawnSupplyCostMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	/*protected void OnLoadoutSpawnSupplyCostMultiplierChanged()
	{
	
	}*/
	
	//------------------------------------------------------------------------------------------------
	/*static int GetLoadoutCalculatedSupplyCost_S(SCR_BasePlayerLoadout playerLoadout, int playerID, SCR_CampaignMilitaryBaseComponent base, SCR_ResourceComponent resourceComponent)
	{
		GetLoadoutCalculatedSupplyCost(playerLoadout, playerID, null, base, resourceComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	static int GetLoadoutCalculatedSupplyCost_C(SCR_BasePlayerLoadout playerLoadout, SCR_Faction playerFaction, SCR_CampaignMilitaryBaseComponent base, SCR_ResourceComponent resourceComponent)
	{
		GetLoadoutCalculatedSupplyCost(playerLoadout, playerID, playerFaction, base, resourceComponent);
	}*/
	
	//------------------------------------------------------------------------------------------------
	//~ TODO: Make sure that this function takes the resourceComponent from SpawnPoint alternativly if it does not try to spawn at base
	//! Get the cost of spawning as a player at given base
	//! \param[in] playerLoadout Loadout to check supply cost for
	//! \param[in] getLocalPlayer If true and the loadout is a SCR_PlayerArsenalLoadout then it will get the locally stored data for the arsenal loadout
	//! \param[in] playerID Player ID of player that want's to spawn, Required only if !getLocalPlayer
	//! \param[in] playerFaction (Optional) Faction of player to spawn to speed up getting the spawn cost data from Catalog
	//! \param[in] base Which Base is  the player trying to spawn (if any)
	//! \param[in] resourceComponent Respawn component of spawnpoint/base it is trying to spawn at
	//! \return Returns the total cost of spawning with the given loadout
	static float GetLoadoutCalculatedSupplyCost(notnull SCR_BasePlayerLoadout playerLoadout, bool getLocalPlayer, int playerID, SCR_Faction playerFaction, SCR_CampaignMilitaryBaseComponent base, SCR_ResourceComponent resourceComponent)
	{				
		float multiplier = 1;
		SCR_ArsenalManagerComponent arsenalManager;
		if (!GetArsenalManager(arsenalManager))
			return 0;
		
		multiplier = arsenalManager.GetLoadoutSpawnSupplyCostMultiplier();
		
		//~ Spawn multiplier is 0
		if (multiplier <= 0)
			return 0;
		
		//~ Supplies are disabled
		if (resourceComponent && !resourceComponent.IsResourceTypeEnabled())
			return 0;
		else if (!resourceComponent && !SCR_ResourceSystemHelper.IsGlobalResourceTypeEnabled())
			return 0;

		float baseMultiplier = 0;
		
		//~ TODO: Deploy menu does not know which base (Or Spawnpoint Resource Component) the player is trying to spawn at
		if (base)
		{
			if (!base.CostSuppliesToSpawn())
				return 0;
			
			baseMultiplier = base.GetBaseSpawnCostFactor();
		}
			
		SCR_PlayerArsenalLoadout playerArsenalLoadout = SCR_PlayerArsenalLoadout.Cast(playerLoadout);
		
		if (playerArsenalLoadout)
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			
			//~ Local
			if (getLocalPlayer && arsenalManager.m_bLocalPlayerLoadoutData)
			{
				if (baseMultiplier > 0)
					return Math.Clamp(arsenalManager.m_bLocalPlayerLoadoutData.LoadoutCost * baseMultiplier, 0, float.MAX);
				else
					return Math.Clamp(arsenalManager.m_bLocalPlayerLoadoutData.LoadoutCost, 0, float.MAX);
			}
			//~ Server
			else if ((gameMode && gameMode.IsMaster()) || (!gameMode && Replication.IsServer()))
			{
				if (baseMultiplier > 0)
					return Math.Clamp(playerArsenalLoadout.GetLoadoutSuppliesCost(playerID) * baseMultiplier, 0, float.MAX);
				else
					return Math.Clamp(playerArsenalLoadout.GetLoadoutSuppliesCost(playerID), 0, float.MAX);
			}
			
			//~ No custom loadout found so spawn cost is base cost or 0 if no base is given
			if (base)
				return Math.Clamp(base.GetBaseSpawnCost(), 0, float.MAX);
			
			return 0;
		}
		//~ Not an arsenal loadout so take cost from catalog
		else 
		{
			SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
			if (!entityCatalogManager)
				return 0;
			
			ResourceName loadoutResource = playerLoadout.GetLoadoutResource();
			if (!loadoutResource)
				return 0;
				
			Resource resource = Resource.Load(loadoutResource);
			if (!resource)
				return 0;
			
			SCR_EntityCatalogEntry entry = entityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.CHARACTER, resource.GetResource().GetResourceName(), playerFaction);
			if (!entry)
			{
				Print("Loadout: '" + resource.GetResource().GetResourceName() + "' is not in catalog so supply cost is 0!", LogLevel.WARNING);
				return 0;
			}
				
			SCR_EntityCatalogLoadoutData data = SCR_EntityCatalogLoadoutData.Cast(entry.GetEntityDataOfType(SCR_EntityCatalogLoadoutData));
			float supplyCost;
			
			if (data)
			{
				supplyCost = data.GetLoadoutSpawnCost();
			}
			//~ No data so try to get Campaign budget for supply cost
			else 
			{
				Print("Loadout: '" + resource.GetResource().GetResourceName() + "' has no 'SCR_EntityCatalogLoadoutData' assigned in catalog so supply cost is taken from Campaign budget on prefab!", LogLevel.WARNING);
				
				SCR_EditableEntityUIInfo editableUIInfo = SCR_EditableEntityUIInfo.Cast(entry.GetEntityUiInfo());
				if (!editableUIInfo)
					return 0;
				
				array<ref SCR_EntityBudgetValue> budgets = {};
				
				if (!editableUIInfo.GetEntityBudgetCost(budgets))
					return 0;
				
				//~ Get campaign budget
				foreach (SCR_EntityBudgetValue budget : budgets)
				{
					if (budget.GetBudgetType() == EEditableEntityBudget.CAMPAIGN)
					{
						supplyCost = budget.GetBudgetValue();
						break;
					}
				}
				
				//~ No campaign budget assigned
				if (supplyCost <= 0)
					return 0;
			}
			
			//~ Add multipliers to the supply cost
			if (baseMultiplier > 0)
				return Math.Clamp((supplyCost * multiplier) * baseMultiplier, 0, float.MAX);
			else 
				return Math.Clamp(supplyCost * multiplier, 0, float.MAX);
		}
		
		//~ If all fails set supply cost 0
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CanSaveLoadout(int playerId, notnull GameEntity characterEntity, FactionAffiliationComponent playerFactionAffiliation, SCR_ArsenalComponent arsenalComponent, bool sendNotificationOnFailed)
	{
		//~ Always allow saving if no arsenal component or no restrictions
		if (!arsenalComponent || (arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.NO_RESTRICTIONS && (!m_LoadoutSaveBlackListHolder || m_LoadoutSaveBlackListHolder.GetBlackListsCount() == 0)))
			return true;
		
		SCR_InventoryStorageManagerComponent characterInventory = SCR_InventoryStorageManagerComponent.Cast(characterEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!characterInventory)
		{
			Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but character has no inventory manager!", LogLevel.ERROR);
			
			if (sendNotificationOnFailed)
				SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED);
			
			return false;
		}
			
		SCR_EntityCatalog itemCatalog;
		SCR_ArsenalInventoryStorageManagerComponent arsenalInventory;
		
		//~ Check either faction or arsenal inventory if item is in it
		if (arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.FACTION_ITEMS_ONLY || arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.IN_ARSENAL_ITEMS_ONLY)
		{			
			if (arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.FACTION_ITEMS_ONLY)
			{				
				//~ Player has no faction affiliation comp
				if (!playerFactionAffiliation)
				{
					Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but player has no faction affiliation component, arsenal saving will simply be allowed", LogLevel.WARNING);
					return true;
				}
				
				SCR_Faction playerFaction = SCR_Faction.Cast(playerFactionAffiliation.GetAffiliatedFaction());
				//~ Player has no faction
				if (!playerFaction)
				{
					Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but player has no SCR_Faction, arsenal saving will simply be allowed", LogLevel.WARNING);
					return true;
				}
				
				itemCatalog = playerFaction.GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM);
				if (!itemCatalog)
				{
					Print(string.Format("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but player faction '%1' has no ITEM catalog!", playerFaction.GetFactionKey()), LogLevel.ERROR);
					
					if (sendNotificationOnFailed)
						SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED);
					
					return false;
				}
			}
			else 
			{
				arsenalInventory = SCR_ArsenalInventoryStorageManagerComponent.Cast(arsenalComponent.GetArsenalInventoryComponent());
				if (!arsenalInventory)
				{
					Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' and arsenal check type is 'IN_ARSENAL_ITEMS_ONLY' but arsenal has no SCR_ArsenalInventoryStorageManagerComponent, so saving is simply allowed", LogLevel.WARNING);
					return true;
				}
			}
		}

		set<string> checkedEntities = new set<string>();
		array<IEntity> allPlayerItems = {};
		characterInventory.GetAllRootItems(allPlayerItems);
		
		EntityPrefabData prefabData;
		RplComponent itemRplComponent;
		string resourceName;
		
		int invalidItemCount = 0;
		
		foreach (IEntity item : allPlayerItems)
		{			
			prefabData = item.GetPrefabData();
			
			//~ Ignore if no prefab data
			if (!prefabData)
				continue;
			
			//~ Ignore if no prefab resource name
			resourceName = prefabData.GetPrefabName();
			if (resourceName.IsEmpty())
				continue;
			
			//~ Do not check the same item twice
			if (checkedEntities.Contains(resourceName))
				continue;
				
			checkedEntities.Insert(resourceName);
			
			//~ Check if item is blacklisted if there is a blacklist
			if (m_LoadoutSaveBlackListHolder && m_LoadoutSaveBlackListHolder.GetBlackListsCount() != 0)
			{
				//~ Item is blackListed so do not allow saving
				if (m_LoadoutSaveBlackListHolder.IsPrefabBlacklisted(resourceName))
				{
					if (sendNotificationOnFailed)
					{
						itemRplComponent = RplComponent.Cast(item.FindComponent(RplComponent));
						if (itemRplComponent)
							SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_ITEM_FAILED_ITEM_BLACKLISTED, itemRplComponent.Id());
					}
				
					invalidItemCount++;
					continue;
				}
			}
			
			//~ Check arsenal inventory
			if (arsenalInventory)
			{
				//~ Not in inventory of arsenal so cannot save
				if (!arsenalInventory.IsPrefabInArsenalStorage(resourceName))
				{
					if (sendNotificationOnFailed)
					{
						itemRplComponent = RplComponent.Cast(item.FindComponent(RplComponent));
						if (itemRplComponent)
							SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_ITEM_FAILED_NOT_IN_ARSENAL, itemRplComponent.Id());
					}
						
					invalidItemCount++;
					continue;
				}
			}
			//~ Check item catalog
			else if (itemCatalog)
			{
				//~ Item not in faction catalog so cannot save
				if (!itemCatalog.GetEntryWithPrefab(resourceName))
				{
					if (sendNotificationOnFailed)
					{
						itemRplComponent = RplComponent.Cast(item.FindComponent(RplComponent));
						if (itemRplComponent)
							SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_ITEM_FAILED_NOT_FACTION, itemRplComponent.Id());
					}
						
					invalidItemCount++;
					continue;
				}
			}
		}
		
		//~ Saving failed because of invalid items
		if (invalidItemCount > 0 && sendNotificationOnFailed)
			SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED_INVALID_ITEMS, invalidItemCount);
		
		return invalidItemCount == 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Authority side
	//! \param[in] playerId
	//! \param[in] characterEntity
	//! \param[in] arsenalComponent
	//! \param[in] arsenalSupplyType
	void SetPlayerArsenalLoadout(int playerId, GameEntity characterEntity, SCR_ArsenalComponent arsenalComponent, SCR_EArsenalSupplyCostType arsenalSupplyType)
	{
		//~ If Not Authority return
		if (!GetGameMode().IsMaster())
			return;
		
		string playerUID = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);
		if (!characterEntity)
		{
			DoSetPlayerLoadout(playerId, string.Empty, characterEntity, arsenalSupplyType);
			return;
		}
		
		SCR_PlayerController clientPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!clientPlayerController || clientPlayerController.IsPossessing())
			return;
		
		string factionKey = SCR_PlayerArsenalLoadout.ARSENALLOADOUT_FACTIONKEY_NONE;
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(characterEntity.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
			factionKey = factionAffiliation.GetAffiliatedFaction().GetFactionKey();
		
		if (!CanSaveLoadout(playerId, characterEntity, factionAffiliation, arsenalComponent, true))
			return;
		
		SCR_JsonSaveContext context = new SCR_JsonSaveContext();
		if (!context.WriteValue(SCR_PlayerArsenalLoadout.ARSENALLOADOUT_FACTION_KEY, factionKey) || !context.WriteValue(SCR_PlayerArsenalLoadout.ARSENALLOADOUT_KEY, characterEntity))
			return;
		
		DoSetPlayerLoadout(playerId, context.ExportToString(), characterEntity, arsenalSupplyType);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_PlayerLoadoutData GetPlayerLoadoutData(GameEntity characterEntity)
	{
		SCR_PlayerLoadoutData loadoutData();
		
		EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(characterEntity.FindComponent(EquipedLoadoutStorageComponent));
		if (loadoutStorage)
		{
			int slotsCount = loadoutStorage.GetSlotsCount();
			for (int i = 0; i < slotsCount; ++i)
			{
				InventoryStorageSlot slot = loadoutStorage.GetSlot(i);
				if (!slot)
					continue;
				
				IEntity attachedEntity = slot.GetAttachedEntity();
				if (!attachedEntity)
					continue;
				
				ResourceName prefabName;
				BaseContainer prefab = attachedEntity.GetPrefabData().GetPrefab();
				while (prefabName.IsEmpty() && prefab)
				{
					prefabName = prefab.GetResourceName();
					prefab = prefab.GetAncestor();
				}
				
				if (prefabName.IsEmpty())
					continue;
				
				SCR_ClothingLoadoutData clothingData();
				clothingData.SlotIdx = i;
				clothingData.ClothingPrefab = prefabName;
				
				loadoutData.Clothings.Insert(clothingData);
			}
		}
		
		EquipedWeaponStorageComponent weaponStorage = EquipedWeaponStorageComponent.Cast(characterEntity.FindComponent(EquipedWeaponStorageComponent));
		if (weaponStorage)
		{
			int slotsCount = weaponStorage.GetSlotsCount();
			for (int i = 0; i < slotsCount; ++i)
			{
				InventoryStorageSlot slot = weaponStorage.GetSlot(i);
				if (!slot)
					continue;
				
				IEntity attachedEntity = slot.GetAttachedEntity();
				if (!attachedEntity)
					continue;
				
				ResourceName prefabName;
				BaseContainer prefab = attachedEntity.GetPrefabData().GetPrefab();
				while (prefabName.IsEmpty() && prefab)
				{
					prefabName = prefab.GetResourceName();
					prefab = prefab.GetAncestor();
				}
				
				if (prefabName.IsEmpty())
					continue;
				
				SCR_WeaponLoadoutData weaponData();
				weaponData.SlotIdx = i;
				weaponData.WeaponPrefab = prefabName;
				
				loadoutData.Weapons.Insert(weaponData);
			}
		}
		
		return loadoutData;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ComputeSuppliesCost(notnull SCR_Faction faction, notnull SCR_ArsenalPlayerLoadout playerLoadout, SCR_EArsenalSupplyCostType arsenalSupplyType)
	{
		playerLoadout.suppliesCost = 0.0;
		
		SCR_JsonLoadContext context = new SCR_JsonLoadContext(false);
		if (!context.ImportFromString(playerLoadout.loadout))
			return;
		
		ComputeEntity(context, faction, playerLoadout, SCR_PlayerArsenalLoadout.ARSENALLOADOUT_KEY, arsenalSupplyType);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ComputeStorage(notnull SCR_JsonLoadContext context, notnull SCR_Faction faction, notnull SCR_ArsenalPlayerLoadout playerLoadout, string storageName, SCR_EArsenalSupplyCostType arsenalSupplyType)
	{
		if (!context.StartObject(storageName))
			return;
		
		if (!context.StartObject("Native"))
		{
			context.EndObject();
			return;
		}
		
		if (!context.StartObject("slots"))
		{
			context.EndObject();
			context.EndObject();
			return;
		}
		
		int itemsCount;
		if (!context.ReadValue("itemsCount", itemsCount))
			return;
		
		for (int i = 0; i < itemsCount; ++i)
		{
			bool valid = false;
			if (!context.ReadValue("slot-" + i +"-valid", valid))
				return;
			
			if (!valid)
				continue;
			
			if (!context.StartObject("slot-" + i))
				return;
			
			ComputeStorageEntity(context, faction, playerLoadout, "entity", arsenalSupplyType);
			
			if (!context.EndObject())
				return;
		}
		
		if (!context.EndObject())
			return;
		
		if (!context.EndObject())
			return;
			
		if (!context.EndObject())
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ComputeStorageEntity(notnull SCR_JsonLoadContext context, notnull SCR_Faction faction, notnull SCR_ArsenalPlayerLoadout playerLoadout, string entityName, SCR_EArsenalSupplyCostType arsenalSupplyType)
	{
		if (!context.StartObject("entity"))
			return;
		
		ResourceName prefab;
		if (!context.ReadValue("prefabGUID", prefab))
			return;
		
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		SCR_EntityCatalogEntry entry = entityCatalogManager.GetEntryWithPrefabFromGeneralOrFactionCatalog(EEntityCatalogType.ITEM, prefab, faction);
		if (entry)
		{
			SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
			if (data)
				playerLoadout.suppliesCost += data.GetSupplyCost(arsenalSupplyType);
		}
		
		ComputeEntity(context, faction, playerLoadout, "entity", arsenalSupplyType);
		
		if (!context.EndObject())
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ComputeEntity(notnull SCR_JsonLoadContext context, notnull SCR_Faction faction, notnull SCR_ArsenalPlayerLoadout playerLoadout, string entityName, SCR_EArsenalSupplyCostType arsenalSupplyType)
	{
		if (!context.StartObject(entityName))
			return;
		
		if (!context.StartObject("Native"))
			return;
		
		if (!context.StartObject("components"))
			return;
		
		foreach (string componentName: ARSENALLOADOUT_COMPONENTS_TO_CHECK)
		{
			ComputeStorage(context, faction, playerLoadout, componentName, arsenalSupplyType);
		}
		
		if (!context.EndObject())
			return;
		
		if (!context.EndObject())
			return;
		
		if (!context.EndObject())
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DoSetPlayerLoadout(int playerId, string loadoutString, GameEntity characterEntity, SCR_EArsenalSupplyCostType arsenalSupplyType)
	{
		bool loadoutValid = !loadoutString.IsEmpty();
		
		string playerUID = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);
		SCR_ArsenalPlayerLoadout playerLoadout = m_aPlayerLoadouts.Get(playerUID);
		if (!playerLoadout)
			playerLoadout = new SCR_ArsenalPlayerLoadout();

		bool loadoutChanged = loadoutValid && loadoutString != playerLoadout.loadout;
		
		playerLoadout.loadout = loadoutString;
		
		m_aPlayerLoadouts.Set(playerUID, playerLoadout);

		if (loadoutChanged)
		{
			FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(characterEntity.FindComponent(FactionAffiliationComponent));
			if (factionAffiliation && factionAffiliation.GetAffiliatedFaction())
				ComputeSuppliesCost(SCR_Faction.Cast(factionAffiliation.GetAffiliatedFaction()), playerLoadout, arsenalSupplyType);
		}
		
		if (loadoutValid && loadoutChanged)
		{
			playerLoadout.loadoutData = GetPlayerLoadoutData(characterEntity);
			playerLoadout.loadoutData.LoadoutCost = SCR_PlayerArsenalLoadout.GetLoadoutSuppliesCost(playerUID);
			DoSendPlayerLoadout(playerId, playerLoadout.loadoutData);
			Rpc(DoSendPlayerLoadout, playerId, playerLoadout.loadoutData);
		}

		DoSetPlayerHasLoadout(playerId, loadoutValid, loadoutChanged, true);
		Rpc(DoSetPlayerHasLoadout, playerId, loadoutValid, loadoutChanged, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPlayerAuditSuccess(int playerId)
	{
		super.OnPlayerAuditSuccess(playerId);
		
		string playerUID = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);
		SCR_ArsenalPlayerLoadout playerLoadout = m_aPlayerLoadouts.Get(playerUID);
		if (!playerLoadout)
			return;
		
		bool loadoutValid = !playerLoadout.loadout.IsEmpty();
		if (!loadoutValid)
			return;
		
		DoSendPlayerLoadout(playerId, playerLoadout.loadoutData);
		Rpc(DoSendPlayerLoadout, playerId, playerLoadout.loadoutData);
		
		DoSetPlayerHasLoadout(playerId, loadoutValid, false, false);
		Rpc(DoSetPlayerHasLoadout, playerId, loadoutValid, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DoSetPlayerHasLoadout(int playerId, bool loadoutValid, bool loadoutChanged, bool notification)
	{
		if (playerId == SCR_PlayerController.GetLocalPlayerId())
		{
			if (notification)
			{
				if (m_bLocalPlayerLoadoutAvailable != loadoutValid || loadoutChanged)
				{
					//~ Send notification with loadout cost
					if (GetLoadoutSpawnSupplyCostMultiplier() > 0 && m_bLocalPlayerLoadoutData)
						SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_SAVED_SUPPLY_COST, m_bLocalPlayerLoadoutData.LoadoutCost);
					//~ Set notification without loadout cost
					else
						SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_SAVED);
				}
				else
				{
					SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_NOT_SAVED_UNCHANGED);
				} 
			}
			
			m_bLocalPlayerLoadoutAvailable = loadoutValid;
		}
		m_OnPlayerLoadoutUpdated.Invoke(playerId, loadoutValid);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DoSendPlayerLoadout(int playerId, SCR_PlayerLoadoutData loadoutData)
	{
		if (playerId == SCR_PlayerController.GetLocalPlayerId())
			m_bLocalPlayerLoadoutData = loadoutData;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DoPlayerClearHasLoadout(int playerId)
	{
		if (playerId == SCR_PlayerController.GetLocalPlayerId())
			m_bLocalPlayerLoadoutAvailable = false;

		m_OnPlayerLoadoutUpdated.Invoke(playerId, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		//~ Init item black list (Server only). Call one frame later to make sure catalogs are initalized
		if (m_LoadoutSaveBlackListHolder)
			GetGame().GetCallqueue().CallLater(m_LoadoutSaveBlackListHolder.Init);
		
		if (!GetGameMode().IsMaster())
			return; 
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			factionManager.GetOnPlayerFactionChanged_S().Insert(OnPlayerChangedFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		if (s_Instance || SCR_Global.IsEditMode())
			return;

		s_Instance = SCR_ArsenalManagerComponent.Cast(GetGameMode().FindComponent(SCR_ArsenalManagerComponent));
		SetEventMask(owner, EntityEvent.INIT);
		
		//~ Get config for saveType holder. This is used by arsenals in the world
		m_ArsenalSaveTypeInfoHolder = SCR_ArsenalSaveTypeInfoHolder.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sArsenalSaveTypeInfoHolder, true));
		if (!m_ArsenalSaveTypeInfoHolder)
			Print("'SCR_ArsenalManagerComponent' failed to load Arsenal Save Type Holder config!", LogLevel.ERROR);
		
		//~ Get loadout save blacklist config, Can be null (Server Only)
		if (GetGameMode().IsMaster())
			m_LoadoutSaveBlackListHolder = SCR_LoadoutSaveBlackListHolder.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sLoadoutSaveBlackListHolder, false));		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (SCR_Global.IsEditMode() || !GetGameMode() || !GetGameMode().IsMaster())
			return; 
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			factionManager.GetOnPlayerFactionChanged_S().Remove(OnPlayerChangedFaction);
	}
}
