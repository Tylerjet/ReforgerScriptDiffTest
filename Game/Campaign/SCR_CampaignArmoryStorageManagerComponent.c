class SCR_CampaignArmoryStorageManagerComponentClass : InventoryStorageManagerComponentClass
{
};

class SCR_CampaignArmoryStorageManagerComponent :  InventoryStorageManagerComponent
{
	//Used to prevent certain storage related actions to be called in same frame, which could cause replication errors
	static const int STORAGE_INIT_DELAY = 500;
	
	//Member variables
	protected ref array<SCR_CampaignArmoryStorageComponent> m_aStorages = {};
	protected bool m_bAllowItemRespawn = true;
	protected IEntity m_Owner;
	protected SCR_CampaignArmoryComponent m_CampaignArmoryComponent;
	protected SCR_CampaignBase m_OwnerBase;
	protected SCR_Faction m_CurrentFaction;
	protected RplComponent m_RplComponent;
	
	//Callback
	ref SCR_StorageItemRemovedCallback m_ItemRemovedCallback = new SCR_StorageItemRemovedCallback;
	
	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return count of all items in registered storages
	int GetItemCount()
	{
		array<IEntity> items = {};
		return GetItems(items);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable respawning items in storages after their removal
	void EnableItemRespawn(bool allowRespawn)
	{
		m_bAllowItemRespawn = allowRespawn;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return current faction
	SCR_Faction GetCurrentFaction()
	{
		return m_CurrentFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clears all managed storages and fills them with appropriate faction related items using SetFactionItems
	void RefreshFactionItems(SCR_Faction faction)
	{
		if (IsProxy())
			return;
		
		if (!m_ItemRemovedCallback.CanRefill())
			return;
		
		//Prevents automatic refilling of deleted items
		m_bAllowItemRespawn = false;
		
		if(GetItemCount()==0)
		{
			SetFactionItems(faction);
			m_bAllowItemRespawn = true;
			return;
		}
		
		
		foreach (SCR_CampaignArmoryStorageComponent storageComp : m_aStorages)
		{
			storageComp.GetAll(m_ItemRemovedCallback.m_aItemsToRemove);
		}
		m_ItemRemovedCallback.RemoveAndRefill();
	}
	
	//---------------------------------------------------------------------------------------
	//! Goes through registered storages and fills them with items according to input faction and storage item filter
	void SetFactionItems(SCR_Faction faction)
	{
		if (IsProxy())
			return;
		
		//condition will be met also if base is recaptured by FIA
		if (!faction)
			return;
		
		if (m_aStorages.IsEmpty())
		{
			Print("ArmoryStorageManager could't find any storages", LogLevel.WARNING);
			return;
		}
		
		array<SCR_ArsenalItem> items;
		foreach (SCR_CampaignArmoryStorageComponent storageComp : m_aStorages)
		{	
			items = faction.GetFilteredArsenalItems(storageComp.GetSupportedItems(), storageComp.GetAmmunitionMode());
			storageComp.AllowStoringItems(true);
			
			foreach (SCR_ArsenalItem item : items)
				TrySpawnPrefabToStorage(item.GetItemResourceName(), storageComp);
			
			storageComp.AllowStoringItems(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepares storages to be used and assigns its initial owner
	void SetupStorages()
	{			
		//This will prevent creating items before replication is prepared for it
		if (m_RplComponent.Id() == RplId.Invalid())
			return;
		
		GetGame().GetCallqueue().Remove(SetupStorages);
		
		m_ItemRemovedCallback.SetManager(this);
		
		//set initial owning faction. NOTE: Independent are null.
		m_CurrentFaction = m_OwnerBase.GetOwningFaction();
		
		RefreshFactionItems(m_OwnerBase.GetOwningFaction());
		m_OwnerBase.m_OnReinforcementsArrived.Insert(HandleReinforcements);
	}
	//------------------------------------------------------------------------------------------------
	void HandleReinforcements()
	{
		SCR_Faction owningFaction = m_OwnerBase.GetOwningFaction();
		
		if (m_CurrentFaction != owningFaction)
		{
			m_CurrentFaction = owningFaction;
			RefreshFactionItems(m_CurrentFaction);
		}
	}
	
	//---------------------------------------------------------------------------------------
	//! Override of OnItemRemoved event, used spawning new items in Storage once they were removed from it. 
	override protected void OnItemRemoved(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		if (IsProxy())
			return;
		
		if (m_bAllowItemRespawn)
		{
			SCR_CampaignArmoryStorageComponent storageComp = SCR_CampaignArmoryStorageComponent.Cast(storageOwner);
			if (!storageComp)
				return;
			
			storageComp.AllowStoringItems(true);
			
			Resource prefabRes = Resource.Load( item.GetPrefabData().GetPrefabName() );
			IEntity spawnedItem = GetGame().SpawnEntityPrefab(prefabRes);
			TryInsertItemInStorage(spawnedItem, storageComp);
			
			storageComp.AllowStoringItems(false);
		}
	}
	
	//---------------------------------------------------------------------------------------
	override void FillInitialStorages(out array<BaseInventoryStorageComponent> storagesToAdd)
	{
		if (!GetGame().InPlayMode())
			return;
				
		m_RplComponent = RplComponent.Cast(m_Owner.FindComponent(RplComponent));
		if (!m_RplComponent)
			return;
		
		if (IsProxy())
			return;
		
		m_CampaignArmoryComponent = SCR_CampaignArmoryComponent.GetNearestArmory(m_Owner.GetOrigin());
		if (!m_CampaignArmoryComponent)
			return;
		
		m_OwnerBase = m_CampaignArmoryComponent.GetBase();
		if (!m_OwnerBase)
			return;
		
		IEntity child = m_Owner.GetChildren();
		
		SCR_CampaignArmoryStorageComponent storage;
		while (child)
		{
			storage = SCR_CampaignArmoryStorageComponent.Cast(child.FindComponent(SCR_CampaignArmoryStorageComponent));
			if (storage)
			{		
				storagesToAdd.Insert(storage);
				m_aStorages.Insert(storage);
			}
			child = child.GetSibling();
		}
		
		GetGame().GetCallqueue().CallLater(SetupStorages, STORAGE_INIT_DELAY, true);
	}
	
	//---------------------------------------------------------------------------------------
	void SCR_CampaignArmoryStorageManagerComponent( IEntityComponentSource src, IEntity ent, IEntity parent )
	{
		if (!GetGame().InPlayMode())
			return;
		
		m_Owner = GetOwner();
	}
	//---------------------------------------------------------------------------------------
	void ~SCR_CampaignArmoryStorageManagerComponent()
	{
		if (m_aStorages)
			m_aStorages.Clear();
		
		m_aStorages = null;
	}
};


//------------------------------------------------------------------------------------------------

class SCR_StorageItemRemovedCallback: ScriptedInventoryOperationCallback
{
	ref array <IEntity> m_aItemsToRemove = {};
	private int m_iItemCounter = 0;
	protected SCR_CampaignArmoryStorageManagerComponent m_Manager;
	void SetManager(SCR_CampaignArmoryStorageManagerComponent manager)
	{
		m_Manager = manager;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanRefill()
	{
		return m_iItemCounter == 0;
	}
	
	//------------------------------------------------------------------------------------------------
	private void TryRefill()
	{
		if (CanRefill())
		{
			m_aItemsToRemove.Clear();
			m_Manager.SetFactionItems(m_Manager.GetCurrentFaction());
			m_Manager.EnableItemRespawn(true);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveAndRefill()
	{
		if (m_iItemCounter > 0)
			return;
		m_iItemCounter = m_aItemsToRemove.Count();
		foreach (IEntity item : m_aItemsToRemove)
		{
			m_Manager.TryDeleteItem(item, this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnComplete()
	{
		m_iItemCounter = m_iItemCounter - 1;
		TryRefill();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFailed()
	{
		Print("Failed to remove items", LogLevel.WARNING);
		m_aItemsToRemove.Clear();
	}
};