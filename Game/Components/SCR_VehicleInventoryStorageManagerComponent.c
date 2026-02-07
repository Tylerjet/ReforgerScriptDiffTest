[EntityEditorProps(category: "GameScripted/UI/Inventory")]
class SCR_VehicleInventoryStorageManagerComponentClass : ScriptedInventoryStorageManagerComponentClass
{
};

class SCR_VehicleInventoryStorageManagerComponent : ScriptedInventoryStorageManagerComponent
{
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
		
		ChimeraWorld world = ChimeraWorld.CastFrom(item.GetWorld());
		if (world)
		{
			GarbageManager garbageManager = world.GetGarbageManager();
			if (garbageManager)
			{
				garbageManager.Withdraw(item);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnItemRemoved(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		super.OnItemRemoved(storageOwner, item);
		
		ChimeraWorld world = ChimeraWorld.CastFrom(item.GetWorld());
		if (world)
		{
			GarbageManager garbageManager = world.GetGarbageManager();
			if (garbageManager)
			{
				if (item.FindComponent(InventoryItemComponent))
					garbageManager.Insert(item);
			}
		}
	}
};
