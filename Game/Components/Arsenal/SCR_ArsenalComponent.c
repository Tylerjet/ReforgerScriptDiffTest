class SCR_ArsenalComponentClass : ScriptComponentClass
{

};

class SCR_ArsenalComponent : ScriptComponent
{
	[Attribute("", desc: "Toggle supported SCR_EArsenalItemType by this arsenal, items are gathered from SCR_Faction or from the overwrite config", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType), category: "Settings")]
	protected SCR_EArsenalItemType m_eSupportedArsenalItemTypes;

	[Attribute("", desc: "Toggle supported SCR_EArsenalItemMode by this arsenal, items are gathered from SCR_Faction or from the overwrite config", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode), category: "Settings")]
	protected SCR_EArsenalItemMode m_eSupportedArsenalItemModes;

	[Attribute("", desc: "Toggle which SCR_EArsenalItemType groups are exposed for gamemaster attribute editing", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalAttributeGroup), category: "Settings")]
	protected SCR_EArsenalAttributeGroup m_eEditableAttributeGroups;

	[Attribute(category: "Overwrite", desc: "If empty this will be ignored. For ease of use do not edit this directly in the prefab, use a config instead. The items added the config will allow the arsenal to spawn these items within it. Note that it will still filter on type, meaning that if weapons are added to the list but disabled in the Supported types then they will not show in the arsenal. Changing faction will not have any effect if this is not null")]
	protected ref SCR_ArsenalItemListConfig m_OverwriteArsenalConfig;

	[Attribute("1", desc: "Get default faction if current faction could not be found")]
	protected bool m_bGetDefaultIfNoFaction;
	
	[Attribute("0", desc: "If true will try and find the SCR_FactionAffiliationComponent from parent if it was not found on self", category: "Settings")]
	protected bool m_bAllowGetFactionFromParent;
	
	[Attribute("0", desc: "Save type of Arsenal. Only applicable if there is a save arsenal action attached to the arsenal.\nSAVING_DISABLED: Saving action is disabled.\nIN_ARSENAL_ITEMS_ONLY: Only allow saving if all the items the player has are in the arsenal as well.\nFACTION_ITEMS_ONLY: Only allow saving if all the items the player has in their inventory are of the faction of the arsenal.\nNO_RESTRICTIONS: No restriction as what the arsenal is allowed to save.", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalSaveType), category: "Settings")]
	protected SCR_EArsenalSaveType m_eArsenalSaveType;
	
	[Attribute("0", desc: "If false it will get the arsenals current faction and change when the faction is updated. If true it will never check the on faction and use the default faction assigned. Use this if you want the content to never change or if there is never a current faction")]
	protected bool m_bAlwaysUseDefaultFaction;
	
	protected bool m_bArsenalSavingDisplayedIfDisabled = true; ///< This is auto set on init and when arsenal save type is changed. If the prefab has saving disabled than the arsenal save action will never show unless saving has been changed in runtime

	protected bool m_bHasSaveArsenalAction = false; ///< Set by action in action manager for the editor to know if the Arsenal save action exists on this arsenal
	
	protected SCR_ArsenalInventoryStorageManagerComponent m_InventoryComponent;
	protected UniversalInventoryStorageComponent m_StorageComponent;

	protected FactionManager m_FactionManager;
	protected SCR_FactionAffiliationComponent m_FactionComponent;

	protected bool m_bIsClearingInventory;

	//------------------------------------------------------------------------------------------------
	/*!
	\return Get Current Arsenal save type
	*/
	SCR_EArsenalSaveType GetArsenalSaveType()
	{
		return m_eArsenalSaveType;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return If arsenal action can be displayed if disabled. This is auto set on init and when arsenal save type is changed. If the prefab has saving disabled than the arsenal save action will never show unless saving has been changed in runtime
	*/
	bool IsArsenalSavingDisplayedIfDisabled()
	{
		return m_bArsenalSavingDisplayedIfDisabled;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set if arsenal save action is on the action manager.
	Set by the save action on init
	\param Set true if action is availible
	*/
	void SetHasSaveArsenalAction(bool hasSaveArsenalAction)
	{
		m_bHasSaveArsenalAction = hasSaveArsenalAction;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Check if Arsenal has an arsenal action attached to it. If true it will allow the editor to set if saving is allowed
	\return True if Arsenal action is availible
	*/
	bool HasSaveArsenalAction()
	{
		return m_bHasSaveArsenalAction;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set Arsenal Save type (Server Only)
	\param saveType New Save type to set
	*/
	void SetArsenalSaveType(SCR_EArsenalSaveType saveType)
	{		
		if (m_eArsenalSaveType == saveType)
			return;
		
		RPL_SetArsenalSaveType(saveType);
		Rpc(RPL_SetArsenalSaveType, saveType);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPL_SetArsenalSaveType(SCR_EArsenalSaveType saveType)
	{
		//~ Nothing changed so make sure the action displayed if disabled is not set
		if (m_eArsenalSaveType == saveType)
			return;
		
		m_eArsenalSaveType = saveType;

		if (saveType != SCR_EArsenalSaveType.SAVING_DISABLED)
			m_bArsenalSavingDisplayedIfDisabled = true;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EArsenalItemType GetSupportedArsenalItemTypes()
	{
		return m_eSupportedArsenalItemTypes;
	}
	
		
	//------------------------------------------------------------------------------------------------
	/*!
	Returns the first arsenal component found on the children of the given parent
	\param parent To check children for arsenal component
	\return Arsenal Component if any is found
	*/
	static SCR_ArsenalComponent GetArsenalComponentFromChildren(notnull IEntity parent)
	{
		IEntity child = parent.GetChildren();
		SCR_ArsenalComponent arsenalComponent;		
		
		while (child)
		{
			arsenalComponent = SCR_ArsenalComponent.Cast(child.FindComponent(SCR_ArsenalComponent));
			if (arsenalComponent)
				return arsenalComponent;
			
			child = child.GetSibling();
		}
		
		//~ Not found
		return null;
	}

	//------------------------------------------------------------------------------------------------
	void SetSupportedArsenalItemTypes(SCR_EArsenalItemType types)
	{
		m_eSupportedArsenalItemTypes = types;
		RefreshArsenal();
	}

	//------------------------------------------------------------------------------------------------
	SCR_EArsenalItemMode GetSupportedArsenalItemModes()
	{
		return m_eSupportedArsenalItemModes;
	}

	//------------------------------------------------------------------------------------------------
	void SetSupportedArsenalItemModes(SCR_EArsenalItemMode modes)
	{
		m_eSupportedArsenalItemModes = modes;
		RefreshArsenal();
	}

	//------------------------------------------------------------------------------------------------
	SCR_EArsenalAttributeGroup GetEditableAttributeGroups()
	{
		return m_eEditableAttributeGroups;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ArsenalInventoryStorageManagerComponent GetArsenalInventoryComponent()
	{
		return m_InventoryComponent;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get overwrite Arsenal config
	\return Config of overwrite arsenal. Can be null if not overwritten
	*/
	SCR_ArsenalItemListConfig GetOverwriteArsenalConfig()
	{
		return m_OverwriteArsenalConfig;
	}

	//------------------------------------------------------------------------------------------------
	bool GetAvailablePrefabs(out notnull array<ResourceName> availablePrefabs)
	{
		array<SCR_ArsenalItem> arsenalItems = {};
		if (!GetFilteredArsenalItems(arsenalItems))
		{
			return false;
		}

		for (int i = 0; i < arsenalItems.Count(); i++)
		{
			SCR_ArsenalItem itemToSpawn = arsenalItems[i % arsenalItems.Count()];
			if (!itemToSpawn)
				continue;

			availablePrefabs.Insert(itemToSpawn.GetItemResourceName());
		}
		return !availablePrefabs.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	SCR_Faction GetAssignedFaction()
	{
		if (m_FactionComponent)
		{
			//~ Always use default faction
			if (m_bAlwaysUseDefaultFaction)
				return SCR_Faction.Cast(m_FactionComponent.GetDefaultAffiliatedFaction());
			
			SCR_Faction faction = SCR_Faction.Cast(m_FactionComponent.GetAffiliatedFaction());
			if (!faction && m_bGetDefaultIfNoFaction)
			{
				faction = SCR_Faction.Cast(m_FactionComponent.GetDefaultAffiliatedFaction());
			}
			
			return faction;
		}
		
		return null;
	}

	//------------------------------------------------------------------------------------------------
	bool GetFilteredArsenalItems(out notnull array<SCR_ArsenalItem> filteredArsenalItems, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		//~ Is overwritting Arsenal
		if (m_OverwriteArsenalConfig)
		{
			GetFilteredOverwriteArsenalItems(filteredArsenalItems, requiresDisplayType);
			return !filteredArsenalItems.IsEmpty();
		}

		//~ Cannot get items if no entity catalog manager
		SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalogManager)
			return false;

		//~ Get filtered list from faction (if any) or default
		filteredArsenalItems = catalogManager.GetFilteredArsenalItems(m_eSupportedArsenalItemTypes, m_eSupportedArsenalItemModes, GetAssignedFaction(), requiresDisplayType);
		return !filteredArsenalItems.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get filtered overwrite Arsenal EWorkshopItemState
	\param[out] List of all filtered arsenal items
	\return true if any filtered items were found
	*/
	bool GetFilteredOverwriteArsenalItems(out notnull array<SCR_ArsenalItem> filteredArsenalItems, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		filteredArsenalItems = m_OverwriteArsenalConfig.GetFilteredArsenalItems(m_eSupportedArsenalItemTypes, m_eSupportedArsenalItemModes, requiresDisplayType);
		return !filteredArsenalItems.IsEmpty();
	}

	void ClearArsenal()
	{
		if (!m_InventoryComponent)
		{
			return;
		}

		m_bIsClearingInventory = true;
		array<IEntity> arsenalItems = {};
		int itemCount = m_InventoryComponent.GetItems(arsenalItems);
		for (int i = 0; i< itemCount; i++)
		{
			if (!m_InventoryComponent.TryDeleteItem(arsenalItems[i]) && arsenalItems[i])
			{
				RplComponent.DeleteRplEntity(arsenalItems[i], false);
			}
		}
		m_bIsClearingInventory = false;
	}

	//------------------------------------------------------------------------------------------------
	void RefreshArsenal(SCR_Faction faction = null)
	{
		if (!m_InventoryComponent)
		{
			return;
		}

		ClearArsenal();

		array<SCR_ArsenalItem> arsenalItems = {};
		if (!GetFilteredArsenalItems(arsenalItems))
			return;

		//SCR_Sorting<SCR_ArsenalItem, SCR_CompareArsenalItemPriority>.HeapSort(arsenalItems);

		int arsenalItemCount = arsenalItems.Count();
		for (int i = 0; i < arsenalItemCount; i++)
		{
			SCR_ArsenalItem itemToSpawn = arsenalItems[i % arsenalItemCount];
			if (!itemToSpawn)
				continue;

			InsertItem(itemToSpawn.GetItemResourceName());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetItemValid(SCR_EArsenalItemType arsenalItemType, SCR_EArsenalItemMode arsenalItemMode)
	{
		return arsenalItemType & m_eSupportedArsenalItemTypes
			&& arsenalItemMode & m_eSupportedArsenalItemModes;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetItemValid(SCR_Faction faction, int index, out bool isEmpty = true)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void InsertItem(ResourceName itemPrefab)
	{
		if (!m_InventoryComponent || !itemPrefab || m_bIsClearingInventory)
		{
			return;
		}

		// Insert into specific root storage component, will otherwise add attachments to weapons, put items in backpack etc.
		if (!m_InventoryComponent.TrySpawnPrefabToStorage(itemPrefab, m_StorageComponent))
		{
			Print(string.Format("Could not add item to Arsenal/storage %1 Item: %2 Storage: %3", this, itemPrefab, m_StorageComponent), LogLevel.WARNING);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemRemoved(IEntity entity, BaseInventoryStorageComponent storage)
	{
		// Only refill root inventory items, ignore attachments and magazines on weapons
		if (SCR_Global.IsEditMode() || !entity || storage != m_StorageComponent || m_bIsClearingInventory || !entity || entity.IsDeleted() || !entity.GetPrefabData() || GetOwner().IsDeleted())
		{
			return;
		}

		InsertItem(entity.GetPrefabData().GetPrefabName());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction)
	{
		RefreshArsenal();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		//~ Make sure it knows that it is disabled at the start
		if (GetArsenalSaveType() == SCR_EArsenalSaveType.SAVING_DISABLED)
			m_bArsenalSavingDisplayedIfDisabled = false;

		SetEventMask(owner, EntityEvent.INIT);

		m_FactionManager = GetGame().GetFactionManager();
		m_FactionComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		
		if (!m_FactionComponent && m_bAllowGetFactionFromParent && owner.GetParent())
			m_FactionComponent = SCR_FactionAffiliationComponent.Cast(owner.GetParent().FindComponent(SCR_FactionAffiliationComponent));
		
		// Initialize inventory of arsenal, if applicable (Display racks without additional inventory will return here)
		m_InventoryComponent = SCR_ArsenalInventoryStorageManagerComponent.Cast(owner.FindComponent(SCR_ArsenalInventoryStorageManagerComponent));
		m_StorageComponent = UniversalInventoryStorageComponent.Cast(owner.FindComponent(UniversalInventoryStorageComponent));
		// Arsenal inventory is filled after OnPostInit by SCR_ArsenalInventoryStorageManagerComponent.FillInitialPrefabsToStore
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{		
		// Refill item only on authority
		// Hook up OnFactionChanged callback in EOnInit (inventory not ready in OnPostInit)
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rplComponent && rplComponent.Role() == RplRole.Authority)
		{
			if (m_InventoryComponent)
				m_InventoryComponent.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
			
			if (!m_bAlwaysUseDefaultFaction && m_FactionComponent)
				m_FactionComponent.GetOnFactionChanged().Insert(OnFactionChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rplComponent && rplComponent.Role() == RplRole.Authority)
		{
			if (m_InventoryComponent)
				m_InventoryComponent.m_OnItemRemovedInvoker.Remove(OnItemRemoved);
			
			if (!m_bAlwaysUseDefaultFaction && m_FactionComponent)
				m_FactionComponent.GetOnFactionChanged().Remove(OnFactionChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
    {	
        writer.WriteInt(GetArsenalSaveType()); 
        return true;
    }
	
	//------------------------------------------------------------------------------------------------
    override bool RplLoad(ScriptBitReader reader)
    {
		int saveType;
		
        reader.ReadInt(saveType);
		RPL_SetArsenalSaveType(saveType);
		
        return true;
	}
};

//------------------------------------------------------------------------------------------------
enum SCR_EArsenalSaveType
{
	SAVING_DISABLED = 0, ///< Saving is disabled
	IN_ARSENAL_ITEMS_ONLY = 10, ///< Only allows arsenal saving if all the items the player has are in the arsenal
	FACTION_ITEMS_ONLY = 20, ///< Only allow saving if items the player has are items of the player faction
	NO_RESTRICTIONS = 30, ///< Always allow saving of arsenal
};
