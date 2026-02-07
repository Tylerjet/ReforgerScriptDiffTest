class SCR_WeaponAttachmentsStorageComponentClass: WeaponAttachmentsStorageComponentClass
{
};

class SCR_WeaponAttachmentsStorageComponent : WeaponAttachmentsStorageComponent
{
	ref ScriptInvoker m_OnItemAddedToSlotInvoker = new ScriptInvoker();
	ref ScriptInvoker m_OnItemRemovedFromSlotInvoker = new ScriptInvoker();
	
	protected ref array<typename> m_aActiveAttachmentTypes;
	
	//------------------------------------------------------------------------------------------------
	override bool CanStoreItem(IEntity item, int slotID)
	{
		InventoryItemComponent inventoryItemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!inventoryItemComponent)
			return true;

		SCR_WeaponAttachmentObstructionAttributes obstructionAttributes = SCR_WeaponAttachmentObstructionAttributes.Cast(inventoryItemComponent.FindAttribute(SCR_WeaponAttachmentObstructionAttributes));
		if (!obstructionAttributes)
			return true;

		array<typename> obstructedAttachmentTypes = obstructionAttributes.GetObstructedAttachmentTypes();
		if (obstructedAttachmentTypes && !obstructedAttachmentTypes.IsEmpty())
		{
			foreach (typename attachmentType : obstructedAttachmentTypes)
			{
				if (m_aActiveAttachmentTypes && m_aActiveAttachmentTypes.Contains(attachmentType))
					return false;
			}
		}
				
		array<typename> requiredAttachmentTypes = obstructionAttributes.GetRequiredAttachmentTypes();
		if (requiredAttachmentTypes && !requiredAttachmentTypes.IsEmpty())
		{
			foreach (typename attachmentType : requiredAttachmentTypes)
			{
				if (m_aActiveAttachmentTypes && !m_aActiveAttachmentTypes.Contains(attachmentType))
					return false;
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Callback when item is added (will be performed locally after server completed the Insert/Move operation)
	override protected void OnAddedToSlot(IEntity item, int slotID)
	{
		if (m_OnItemAddedToSlotInvoker)
			m_OnItemAddedToSlotInvoker.Invoke(item, slotID);
		
		InventoryItemComponent inventoryItemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!inventoryItemComponent)
			return;
		
		SCR_WeaponAttachmentObstructionAttributes obstructionAttributes = SCR_WeaponAttachmentObstructionAttributes.Cast(inventoryItemComponent.FindAttribute(SCR_WeaponAttachmentObstructionAttributes));
		if (!obstructionAttributes)
			return;
		
		if (!m_aActiveAttachmentTypes)
			m_aActiveAttachmentTypes = {};
				
		m_aActiveAttachmentTypes.Insert(obstructionAttributes.GetAttachmentType().Type());
	}
	
	//------------------------------------------------------------------------------------------------
	// Callback when item is removed (will be performed locally after server completed the Remove/Move operation)
	override protected void OnRemovedFromSlot(IEntity item, int slotID)
	{
		if (m_OnItemRemovedFromSlotInvoker && !item.IsDeleted())
			m_OnItemRemovedFromSlotInvoker.Invoke(item, slotID);
		
		InventoryItemComponent inventoryItemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!inventoryItemComponent)
			return;
		
		SCR_WeaponAttachmentObstructionAttributes obstructionAttributes = SCR_WeaponAttachmentObstructionAttributes.Cast(inventoryItemComponent.FindAttribute(SCR_WeaponAttachmentObstructionAttributes));
		if (!obstructionAttributes)
			return;
		
		typename attachmentType = obstructionAttributes.GetAttachmentType().Type();
		m_aActiveAttachmentTypes.RemoveItem(attachmentType);
				
		GetGame().GetCallqueue().CallLater(RemoveNestedAttachments, param1: attachmentType); // Wait for attachment to be stored before removing nested ones
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveNestedAttachments(typename removedAttachmentType)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;
		
		InventoryStorageManagerComponent storageManager = InventoryStorageManagerComponent.Cast(character.FindComponent(InventoryStorageManagerComponent));
		if (!storageManager)
			return;
			
		array<IEntity> storedItems = {};
		GetAll(storedItems);	
		foreach (IEntity storedItem : storedItems)
		{
			InventoryItemComponent inventoryItemComponent = InventoryItemComponent.Cast(storedItem.FindComponent(InventoryItemComponent));
			if (!inventoryItemComponent)
				continue;
			
			SCR_WeaponAttachmentObstructionAttributes obstructionAttributes = SCR_WeaponAttachmentObstructionAttributes.Cast(inventoryItemComponent.FindAttribute(SCR_WeaponAttachmentObstructionAttributes));
			if (!obstructionAttributes)
				continue;
			
			array<typename> requiredAttachmentTypes = obstructionAttributes.GetRequiredAttachmentTypes();
			if (requiredAttachmentTypes && requiredAttachmentTypes.Contains(removedAttachmentType))
			{
				BaseInventoryStorageComponent storage = storageManager.FindStorageForItem(storedItem, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT);		
				if (!storage)
					storage = storageManager.FindStorageForItem(storedItem);
					
				storageManager.TryMoveItemToStorage(storedItem, storage);
			}
		}
	}
};
