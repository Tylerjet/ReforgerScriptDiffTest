class SCR_ArsenalComponentClass : ScriptComponentClass
{
	
};

class SCR_ArsenalComponent : ScriptComponent
{	
	[Attribute("", UIWidgets.Flags, "Toggle supported SCR_EArsenalItemType by this arsenal, items are gathered from SCR_Faction", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_eSupportedArsenalItemTypes;
	
	[Attribute("", UIWidgets.Flags, "Toggle supported SCR_EArsenalItemMode by this arsenal, items are gathered from SCR_Faction", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode))]
	protected SCR_EArsenalItemMode m_eSupportedArsenalItemModes;
	
	[Attribute("", UIWidgets.Flags, "Toggle which SCR_EArsenalItemType groups are exposed for gamemaster attribute editing", enums: ParamEnumArray.FromEnum(SCR_EArsenalAttributeGroup))]
	protected SCR_EArsenalAttributeGroup m_eEditableAttributeGroups;
	
	protected SCR_ArsenalInventoryStorageManagerComponent m_InventoryComponent;
	protected UniversalInventoryStorageComponent m_StorageComponent;
	
	protected FactionManager m_FactionManager;
	protected SCR_FactionAffiliationComponent m_FactionComponent;
	
	protected bool m_bIsClearingInventory;
	
	SCR_EArsenalItemType GetSupportedArsenalItemTypes()
	{
		return m_eSupportedArsenalItemTypes;
	}
	
	void SetSupportedArsenalItemTypes(SCR_EArsenalItemType types)
	{
		m_eSupportedArsenalItemTypes = types;
		RefreshArsenal();
	}
	
	SCR_EArsenalItemMode GetSupportedArsenalItemModes()
	{
		return m_eSupportedArsenalItemModes;
	}
	
	void SetSupportedArsenalItemModes(SCR_EArsenalItemMode modes)
	{
		m_eSupportedArsenalItemModes = modes;
		RefreshArsenal();
	}
	
	SCR_EArsenalAttributeGroup GetEditableAttributeGroups()
	{
		return m_eEditableAttributeGroups;
	}
	
	bool GetAvailablePrefabs(out notnull array<ResourceName> availablePrefabs, SCR_Faction faction = null)
	{
		array<SCR_ArsenalItem> arsenalItems = {};
		if (!GetFilteredArsenalItems(arsenalItems, faction))
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
	
	bool GetAssignedFaction(out SCR_Faction faction)
	{
		if (m_FactionComponent)
		{
			faction = SCR_Faction.Cast(m_FactionComponent.GetAffiliatedFaction());
		}
		return faction != null;
	}
	
	bool GetFilteredArsenalItems(out notnull array<SCR_ArsenalItem> filteredArsenalItems, SCR_Faction faction = null)
	{
		if (!faction && !GetAssignedFaction(faction))
		{
			return false;
		}
		
		filteredArsenalItems = faction.GetFilteredArsenalItems(m_eSupportedArsenalItemTypes, m_eSupportedArsenalItemModes);
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
		for (int i = 0; i< itemCount ; i++)
		{
			if (!m_InventoryComponent.TryDeleteItem(arsenalItems[i]) && arsenalItems[i])
			{
				RplComponent.DeleteRplEntity(arsenalItems[i], false);
			}
		}
		m_bIsClearingInventory = false;
	}
	
	void RefreshArsenal(SCR_Faction faction = null)
	{
		if (!m_InventoryComponent)
		{
			return;
		}
		
		if (!faction && !GetAssignedFaction(faction))
		{
			return;
		}
		
		ClearArsenal();
		
		array<SCR_ArsenalItem> arsenalItems = {};
		if (!GetFilteredArsenalItems(arsenalItems, faction))
			return;
		
		//SCR_Sorting<SCR_ArsenalItem, SCR_CompareArsenalItemPriority>.HeapSort(arsenalItems);
		
		int arsenalItemCount = arsenalItems.Count();
		for (int i = 0; i < arsenalItemCount; i++)
		{
			SCR_ArsenalItem itemToSpawn = arsenalItems[i % arsenalItemCount];
			if (!itemToSpawn)
				continue;
			
			InsertItem(itemToSpawn.GetItemResource());
		}
	}
	
	protected bool GetItemValid(SCR_EArsenalItemType arsenalItemType, SCR_EArsenalItemMode arsenalItemMode)
	{
		return arsenalItemType & m_eSupportedArsenalItemTypes
			&& arsenalItemMode & m_eSupportedArsenalItemModes;
	}
	
	protected bool GetItemValid(SCR_Faction faction, int index, out bool isEmpty = true)
	{
		return true;
	}
	
	protected void InsertItem(Resource itemPrefab)
	{
		if (!m_InventoryComponent || !itemPrefab || !itemPrefab.IsValid() || m_bIsClearingInventory)
		{
			return;
		}
		
		IEntity itemEntity = GetGame().SpawnEntityPrefab(itemPrefab);
		if (!itemEntity)
		{
			Print(string.Format("Could not spawn to Arsenal/storage %1 Item: %2 Storage: %3", this, itemPrefab, m_StorageComponent), LogLevel.WARNING);
			return;
		}
		// Insert into specific root storage component, will otherwise add attachments to weapons, put items in backpack etc.
		if (!m_InventoryComponent.TryInsertItemInStorage(itemEntity, m_StorageComponent))
		{
			RplComponent.DeleteRplEntity(itemEntity, false);
			Print(string.Format("Could not add item to Arsenal/storage %1 Item: %2 Storage: %3", this, itemPrefab, m_StorageComponent), LogLevel.WARNING);
			return;
		}
	}
	
	protected void OnItemRemoved(IEntity entity, BaseInventoryStorageComponent storage)
	{
		// Only refill root inventory items, ignore attachments and magazines on weapons
		if (SCR_Global.IsEditMode() || !entity || storage != m_StorageComponent || m_bIsClearingInventory || !entity || entity.IsDeleted() || !entity.GetPrefabData() || GetOwner().IsDeleted())
		{
			return;
		}
		
		InsertItem(Resource.Load(entity.GetPrefabData().GetPrefabName()));
	}
	
	protected void OnFactionChanged()
	{
		RefreshArsenal();
	}
	
	override protected void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
		{
			return;
		}
		
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE, true);
		
		m_FactionManager = GetGame().GetFactionManager();
		m_FactionComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		
		// Initialize inventory of arsenal, if applicable (Display racks without additional inventory will return here)
		m_InventoryComponent = SCR_ArsenalInventoryStorageManagerComponent.Cast(owner.FindComponent(SCR_ArsenalInventoryStorageManagerComponent));
		m_StorageComponent = UniversalInventoryStorageComponent.Cast(owner.FindComponent(UniversalInventoryStorageComponent));
		// Arsenal inventory is filled after OnPostInit by SCR_ArsenalInventoryStorageManagerComponent.FillInitialPrefabsToStore
	}
	
	override protected void EOnInit(IEntity owner)
	{
		// Refill item only on authority
		// Hook up OnFactionChanged callback in EOnInit (inventory not ready in OnPostInit)
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rplComponent && rplComponent.Role() == RplRole.Authority)
		{
			if (m_InventoryComponent)
				m_InventoryComponent.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
			if (m_FactionComponent)
				m_FactionComponent.GetOnFactionUpdate().Insert(OnFactionChanged);
		}
	}
	
	override protected void OnDelete(IEntity owner)
	{		
		if (SCR_Global.IsEditMode())
		{
			return;
		}
		
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rplComponent && rplComponent.Role() == RplRole.Authority)
		{
			if (m_InventoryComponent)
				m_InventoryComponent.m_OnItemRemovedInvoker.Remove(OnItemRemoved);
			if (m_FactionComponent)
				m_FactionComponent.GetOnFactionUpdate().Remove(OnFactionChanged);
		}
	}
};

/*
class SCR_CompareArsenalItemPriority : SCR_SortCompare<SCR_ArsenalItem>
{
	override static int Compare(SCR_ArsenalItem left, SCR_ArsenalItem right)
	{
		string name1 = left.GetName();
		string name2 = right.GetName();
		
		if (name1.Compare(name2) == -1)
			return 1;
		else
			return 0;
		return 1;
	}
};
*/