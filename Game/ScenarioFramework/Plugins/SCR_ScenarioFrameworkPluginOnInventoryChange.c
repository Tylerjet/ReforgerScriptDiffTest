[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPluginOnInventoryChange : SCR_ScenarioFrameworkPlugin
{
	[Attribute(UIWidgets.Auto, desc: "What to do once object inventory has changed by item addition")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnItemAdded;

	[Attribute(UIWidgets.Auto, desc: "What to do once object inventory has changed by item removal")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnItemRemoved;

	IEntity m_Asset;

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		if (!object)
			return;

		super.Init(object);
		IEntity entity = object.GetSpawnedEntity();
		if (!entity)
			return;

		m_Asset = entity;

		//Inventory system is a mess and since different entities have different storage managers that don't have this properly inherited, we need to account for that here
		SCR_InventoryStorageManagerComponent storageManager1 = SCR_InventoryStorageManagerComponent.Cast(m_Asset.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManager1)
		{
			storageManager1.m_OnItemAddedInvoker.Insert(OnItemAdded);
			storageManager1.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
			return;
		}

		SCR_VehicleInventoryStorageManagerComponent storageManager2 = SCR_VehicleInventoryStorageManagerComponent.Cast(m_Asset.FindComponent(SCR_VehicleInventoryStorageManagerComponent));
		if (storageManager2)
		{
			storageManager2.m_OnItemAddedInvoker.Insert(OnItemAdded);
			storageManager2.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
			return;
		}

		SCR_ArsenalInventoryStorageManagerComponent storageManager3 = SCR_ArsenalInventoryStorageManagerComponent.Cast(m_Asset.FindComponent(SCR_ArsenalInventoryStorageManagerComponent));
		if (storageManager3)
		{
			storageManager3.m_OnItemAddedInvoker.Insert(OnItemAdded);
			storageManager3.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemAdded(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemAdded)
		{
			action.OnActivate(m_Asset);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemRemoved(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemRemoved)
		{
			action.OnActivate(m_Asset);
		}
	}
}