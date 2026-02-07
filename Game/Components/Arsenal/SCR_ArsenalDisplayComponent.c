class SCR_ArsenalDisplayComponentClass : SCR_ArsenalComponentClass
{
	[Attribute()]
	protected ref SCR_ArsenalItemDisplayConfig m_ItemDisplayConfig;
	
	array<ref SCR_ArsenalItemDisplayData> GetAllDisplayData()
	{
		if (m_ItemDisplayConfig)
		{
			return m_ItemDisplayConfig.GetAllDisplayData();
		}
		return new array<ref SCR_ArsenalItemDisplayData>();
	}
	
	bool GetItemDisplayData(ResourceName prefab, out SCR_ArsenalItemDisplayData displayData)
	{
		if (m_ItemDisplayConfig)
		{
			return m_ItemDisplayConfig.GetItemDisplayData(prefab, displayData);
		}
		return false;
	}
};

class SCR_ArsenalDisplayComponent : SCR_ArsenalComponent
{
	protected ref array<SCR_WeaponRackSlotEntity> m_aSlots = new array<SCR_WeaponRackSlotEntity>();
	protected SCR_ArsenalDisplayComponentClass m_ArsenalComponentData;
	
	override void SetSupportedArsenalItemTypes(SCR_EArsenalItemType types)
	{
		ClearArsenal();
		super.SetSupportedArsenalItemTypes(types);
	}
	
	override void SetSupportedArsenalItemModes(SCR_EArsenalItemMode modes)
	{
		ClearArsenal();
		super.SetSupportedArsenalItemModes(modes);
	}
	
	protected void RegisterSlot(SCR_WeaponRackSlotEntity slot)
	{
		if (!slot || m_aSlots.Find(slot) != -1)
		{
			return;
		}
		
		m_aSlots.Insert(slot);
	}
	
	override void OnFactionChanged()
	{
		ClearArsenal();
		RefreshArsenal();
	}
	
	override void ClearArsenal()
	{
		for (int i = 0; i < m_aSlots.Count(); i++)
		{
			m_aSlots[i].RemoveItem();
		}
	}
	
	override void RefreshArsenal(SCR_Faction faction = null)
	{
		if (!faction && !GetAssignedFaction(faction))
		{
			return;
		}
		
		array<SCR_ArsenalItem> availableItems = {};
		if (!GetFilteredArsenalItems(availableItems, faction))
		{
			return;
		}
		
		int availableItemCount = availableItems.Count();
		int availableSlotCount = m_aSlots.Count();
		
		SCR_ArsenalItem itemToSpawn;
		SCR_ArsenalItemDisplayData itemDisplayData;
		SCR_WeaponRackSlotEntity currentSlot = null;
		for (int i = 0; i < availableSlotCount; i++)
		{
			currentSlot = m_aSlots[i];
			if (GetSlotValid(faction, currentSlot))
			{
				continue;
			}
			else if(currentSlot && currentSlot.GetChildren())
			{
				currentSlot.RemoveItem();
			}
			else if (!currentSlot)
			{
				m_aSlots.RemoveOrdered(i--);
				availableSlotCount--;
				continue;
			}
			
			if (!currentSlot.CanSpawnItem())
			{
				continue;
			}
			
			int index = i % availableItemCount;
			for (int j = 0; j < availableItemCount; j++)
			{
				itemToSpawn = availableItems[index];
				if (GetItemValidForSlot(itemToSpawn.GetItemType(), itemToSpawn.GetItemMode(), currentSlot.GetSlotSupportedItemTypes(), currentSlot.GetSlotSupportedItemModes()))
				{
					break;
				}
				itemToSpawn = null;
				if (++index >= availableItemCount)
				{
					index = 0;
				}
			}
			
			if (!itemToSpawn)
			{
				continue;
			}
			
			itemDisplayData = null;
			if (!m_ArsenalComponentData || !m_ArsenalComponentData.GetItemDisplayData(itemToSpawn.GetItemResourceName(), itemDisplayData))
			{
				//Print("DEBUG LINE | " + FilePath.StripPath(__FILE__) + ":" + __LINE__ + " Weapon rack; component data or item offset not found on weapon display prefab", LogLevel.DEBUG);
			}
			
			currentSlot.SpawnNewItem(itemToSpawn.GetItemResource(), itemDisplayData);
		}
	}
	
	override protected bool GetItemValid(SCR_Faction faction, int index, out bool isEmpty = true)
	{
		if (!faction || index < 0 || index > m_aSlots.Count() - 1)
		{
			return false;
		}
		
		return GetSlotValid(faction, m_aSlots[index], isEmpty);
	}
	
	protected bool GetSlotValid(SCR_Faction faction, SCR_WeaponRackSlotEntity slotEntity, out bool isEmpty = true)
	{
		if (!faction || !slotEntity)
		{
			return false;
		}
		
		if (slotEntity.GetChildren() && slotEntity.GetChildren().GetPrefabData())
		{
			isEmpty = false;
			ResourceName itemPrefab = slotEntity.GetChildren().GetPrefabData().GetPrefabName();
			SCR_EArsenalItemType currentItemType;
			SCR_EArsenalItemMode currentItemMode;
			
			if (faction.GetArsenalItemTypeForPrefab(itemPrefab, currentItemType)
				&& faction.GetArsenalItemModeForPrefab(itemPrefab, currentItemMode))
			{
				return GetItemValidForSlot(currentItemType, currentItemMode, slotEntity.GetSlotSupportedItemTypes(), slotEntity.GetSlotSupportedItemModes());
			}
		}
		return false;
	}
	
	protected bool GetItemValidForSlot(SCR_EArsenalItemType itemType, SCR_EArsenalItemMode itemMode, SCR_EArsenalItemType supportedSlotTypes, SCR_EArsenalItemMode supportedSlotModes)
	{
		return GetItemValid(itemType, itemMode) && itemType & supportedSlotTypes && itemMode & supportedSlotModes;
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		IEntity child = owner.GetChildren();
		while (child)
		{
			SCR_WeaponRackSlotEntity slot = SCR_WeaponRackSlotEntity.Cast(child);
			if (slot)
			{
				RegisterSlot(slot);
			}
			child = child.GetSibling();
		}
		
		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rpl && rpl.Role() == RplRole.Authority)
		{
			RefreshArsenal();
		}
	}	
	
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
		{
			return;
		}
		super.OnPostInit(owner);
		
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE, true);
		
		m_ArsenalComponentData = SCR_ArsenalDisplayComponentClass.Cast(GetComponentData(owner));
	}
	
	override protected void OnDelete(IEntity owner)
	{		
		if (SCR_Global.IsEditMode())
		{
			return;
		}
		super.OnDelete(owner);
		
		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rpl && rpl.Role() == RplRole.Authority)
		{
		//	GetGame().GetCallqueue().Remove(RefreshArsenal);
		}
	}
};