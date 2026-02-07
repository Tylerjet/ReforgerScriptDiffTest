[EntityEditorProps(category: "GameScripted/UI/Inventory")]
class SCR_VehicleInventoryStorageManagerComponentClass : ScriptedInventoryStorageManagerComponentClass
{
};

class SCR_VehicleInventoryStorageManagerComponent : ScriptedInventoryStorageManagerComponent
{
	ref ScriptInvokerBase<ScriptInvokerEntityAndStorageMethod> m_OnItemAddedInvoker = new ScriptInvokerBase<ScriptInvokerEntityAndStorageMethod>();
	ref ScriptInvokerBase<ScriptInvokerEntityAndStorageMethod> m_OnItemRemovedInvoker = new ScriptInvokerBase<ScriptInvokerEntityAndStorageMethod>();
	
	//------------------------------------------------------------------------------------------------
	protected override void FillInitialStorages(out array<BaseInventoryStorageComponent> storagesToAdd)
	{
		super.FillInitialStorages(storagesToAdd);
		
		if (!GetGame().InPlayMode())
			return;

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		foreach (EntitySlotInfo info : slots)
		{
			IEntity ent = info.GetAttachedEntity();
			if (!ent)
				continue;
			BaseInventoryStorageComponent invComp = BaseInventoryStorageComponent.Cast(ent.FindComponent(BaseInventoryStorageComponent));
			if (invComp)
			{
				storagesToAdd.Insert(invComp);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnItemAdded(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		super.OnItemAdded(storageOwner, item);
		
		InventoryItemComponent pItemComponent = InventoryItemComponent.Cast( item.FindComponent( InventoryItemComponent ) );
		if (!pItemComponent)
			return;
		
		if (m_OnItemAddedInvoker)
			m_OnItemAddedInvoker.Invoke(item, storageOwner);
		
		ChimeraWorld world = ChimeraWorld.CastFrom(item.GetWorld());
		if (world)
		{
			GarbageSystem garbageSystem = world.GetGarbageSystem();
			if (garbageSystem)
			{
				garbageSystem.Withdraw(item);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnItemRemoved(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		super.OnItemRemoved(storageOwner, item);
		
		if (m_OnItemRemovedInvoker)
			m_OnItemRemovedInvoker.Invoke(item, storageOwner);
		
		ChimeraWorld world = ChimeraWorld.CastFrom(item.GetWorld());
		if (world)
		{
			GarbageSystem garbageSystem = world.GetGarbageSystem();
			if (garbageSystem)
				garbageSystem.Insert(item);
		}
	}
};
