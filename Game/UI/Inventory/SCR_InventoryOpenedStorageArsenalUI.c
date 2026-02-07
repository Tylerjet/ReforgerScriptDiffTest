class SCR_VirtualArsenalCacheManager
{
	protected static ref SCR_VirtualArsenalCacheManager INSTANCE;
	protected ref map<BaseResourceObject, IEntity> m_mCachedEntities;

	static SCR_VirtualArsenalCacheManager GetInstance()
	{
		if (!SCR_VirtualArsenalCacheManager.INSTANCE)
			SCR_VirtualArsenalCacheManager.INSTANCE = new SCR_VirtualArsenalCacheManager();
		
		return SCR_VirtualArsenalCacheManager.INSTANCE;
	}

	IEntity RegisterResource(notnull Resource resource)
	{
		if (!resource.IsValid())
			return null;
		
		BaseResourceObject resourceObject = resource.GetResource();
		
		if (!m_mCachedEntities.Contains(resourceObject) && GetGame().CanSpawnEntityPrefab(resource))
		{
			IEntity cachedEntity = GetGame().SpawnEntityPrefabLocal(resource);
			
			m_mCachedEntities.Set(resourceObject, cachedEntity);
			
			return cachedEntity;
		}

		return m_mCachedEntities.Get(resourceObject);
	}

	void UnregisterResource(notnull Resource resource)
	{
		if (!resource.IsValid())
			return;
		
		BaseResourceObject resourceObject = resource.GetResource();
		
		if (!m_mCachedEntities.Contains(resourceObject))
			return;
		
		delete m_mCachedEntities.Get(resourceObject);

		m_mCachedEntities.Remove(resourceObject);
	}

	void SCR_VirtualArsenalCacheManager()
	{
		m_mCachedEntities = new map<BaseResourceObject, IEntity>();
	}

	void ~SCR_VirtualArsenalCacheManager()
	{
		foreach (BaseResourceObject resource, IEntity entity: m_mCachedEntities)
		{
			delete entity;
		}
	}
}

class SCR_InventoryOpenedStorageArsenalUI : SCR_InventoryOpenedStorageUI
{
	//------------------------------------------------------------------------------------------------
	override protected void GetAllItems(out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null)
	{
		if (pStorage)
		{
			super.GetAllItems(pItemsInStorage, pStorage);

			return;
		}
		
		SCR_ArsenalComponent arsenalComponent	= SCR_ArsenalComponent.Cast(m_Storage.GetOwner().FindComponent(SCR_ArsenalComponent));
		array<ResourceName> prefabsToSpawn		= new array<ResourceName>();
		
		if (!arsenalComponent)
			return;
		
		arsenalComponent.GetAvailablePrefabs(prefabsToSpawn);
		
		foreach (ResourceName resourceName: prefabsToSpawn)
		{
			pItemsInStorage.Insert(SCR_VirtualArsenalCacheManager.GetInstance().RegisterResource(Resource.Load(resourceName)));
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	override void RefreshResources()
	{
		if (!m_wResourceDisplay || !m_wResourceText)
			return;
		
		if (!GetStorage())
			return;
		
		IEntity owner = GetStorage().GetOwner();
		
		if (!owner)
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(owner);
		
		if (!resourceComponent)
			return;
		
		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		
		if (!consumer)
			return;
		
		m_wResourceText.SetText(WidgetManager.Translate("#AR-Supplies_Arsenal_Availability", consumer.GetAggregatedResourceValue().ToString()));
		
		if (!m_wResourceDisplay.IsVisible())
			m_wResourceDisplay.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
		
		m_wWeightDisplay.SetVisible(false);
		m_wCapacityDisplay.SetVisible(false);
		
		m_wWeightText = null;
		m_wWeightDisplay = null;
		m_wCapacityDisplay = null;
		m_wCapacityPercentageText = null;
		
		m_wResourceText = TextWidget.Cast(m_widget.FindAnyWidget("ResourceText"));
		m_wResourceDisplay = Widget.Cast(m_widget.FindAnyWidget("ResourceDisplay"));
		
		if (!m_wResourceDisplay || !m_wResourceText)
			return;
		
		if (m_Storage && m_Storage.GetOwner())
			m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(m_Storage.GetOwner());
		
		if (!m_ResourceComponent)
			return;
		
		m_ResourceConsumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		
		if (!m_ResourceConsumer)
			return;
		
		m_wResourceDisplay.SetVisible(true);
		m_wResourceText.SetText(WidgetManager.Translate("#AR-Supplies_Arsenal_Availability", m_ResourceConsumer.GetAggregatedResourceValue().ToString()));
		m_ResourceComponent.TEMP_GetOnInteractorReplicated().Insert(Refresh);	
	}
	
	//------------------------------------------------------------------------------------------------
	override protected SCR_InventorySlotUI CreateSlotUI( InventoryItemComponent pComponent, SCR_ItemAttributeCollection pAttributes = null )
	{
		if (!pComponent)
		{
			return new SCR_InventorySlotUI(null, this, false, -1, pAttributes);
		}
		else
		{
			SCR_ArsenalInventorySlotUI slotUI = SCR_ArsenalInventorySlotUI(pComponent, this, false, -1, null); //creates the slot
			
			slotUI.SetArsenalResourceComponent(m_ResourceComponent);
			SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Insert(slotUI, GetCurrentNavigationStorage().GetOwner());
			
			return slotUI;
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	override void Refresh()
	{
		super.Refresh();
		
		if (m_ResourceComponent && m_ResourceConsumer)
			RefreshResources();
	}
};