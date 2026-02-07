[EntityEditorProps(category: "GameScripted/UI/Inventory")]
class SCR_VehicleInventoryStorageManagerComponentClass : InventoryStorageManagerComponentClass
{
};

class SCR_VehicleInventoryStorageManagerComponent : InventoryStorageManagerComponent
{
	//------------------------------------------------------------------------------------------------
	protected override void FillInitialStorages(out array<BaseInventoryStorageComponent> storagesToAdd)
	{
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
		GarbageManager garbageManager = GetGame().GetGarbageManager();
		if (garbageManager)
		{
			if (item.FindComponent(InventoryItemComponent))
				garbageManager.Withdraw(item);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnItemRemoved(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		GarbageManager garbageManager = GetGame().GetGarbageManager();
		if (garbageManager)
		{
			if (item.FindComponent(InventoryItemComponent))
				garbageManager.Insert(item);
		}
	}
};
