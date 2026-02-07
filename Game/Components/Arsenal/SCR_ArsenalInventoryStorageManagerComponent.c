class SCR_ArsenalInventoryStorageManagerComponentClass: InventoryStorageManagerComponentClass
{
	
};

class SCR_ArsenalInventoryStorageManagerComponent : InventoryStorageManagerComponent
{
	ref ScriptInvoker m_OnItemAddedInvoker = new ScriptInvoker();
	ref ScriptInvoker m_OnItemRemovedInvoker = new ScriptInvoker();
	
	// Callback when item is added (will be performed locally after server completed the Insert/Move operation)
	override protected void OnItemAdded(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		if (m_OnItemAddedInvoker)
			m_OnItemAddedInvoker.Invoke(item, storageOwner);
	
	}
	// Callback when item is removed (will be performed locally after server completed the Remove/Move operation)
	override protected void OnItemRemoved(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		if (m_OnItemRemovedInvoker && !item.IsDeleted())
			m_OnItemRemovedInvoker.Invoke(item, storageOwner);
	}
	
	protected override void FillInitialPrefabsToStore(out array<ResourceName> prefabsToSpawn)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(GetOwner().FindComponent(SCR_ArsenalComponent));
		if (arsenalComponent)
		{
			arsenalComponent.GetAvailablePrefabs(prefabsToSpawn);
		}
	}
};