class SCR_WeaponAttachmentsStorageComponentClass: WeaponAttachmentsStorageComponentClass
{
};

class SCR_WeaponAttachmentsStorageComponent : WeaponAttachmentsStorageComponent
{
	ref ScriptInvoker m_OnItemAddedToSlotInvoker = new ScriptInvoker();
	ref ScriptInvoker m_OnItemRemovedFromSlotInvoker = new ScriptInvoker();
	
	protected bool m_bHasUnderbarrelAttachment;
	
	//------------------------------------------------------------------------------------------------
	override bool CanStoreItem(IEntity item, int slotID)
	{
		if (m_bHasUnderbarrelAttachment && (MuzzleInMagComponent.Cast(item.FindComponent(MuzzleInMagComponent)) || SCR_BayonetComponent.Cast(item.FindComponent(SCR_BayonetComponent))))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Callback when item is added (will be performed locally after server completed the Insert/Move operation)
	override protected void OnAddedToSlot(IEntity item, int slotID)
	{
		if (MuzzleInMagComponent.Cast(item.FindComponent(MuzzleInMagComponent)) || SCR_BayonetComponent.Cast(item.FindComponent(SCR_BayonetComponent)))
			m_bHasUnderbarrelAttachment = true;
		
		if (m_OnItemAddedToSlotInvoker)
			m_OnItemAddedToSlotInvoker.Invoke(item, slotID);
	}
	
	//------------------------------------------------------------------------------------------------
	// Callback when item is removed (will be performed locally after server completed the Remove/Move operation)
	override protected void OnRemovedFromSlot(IEntity item, int slotID)
	{
		if (MuzzleInMagComponent.Cast(item.FindComponent(MuzzleInMagComponent)) || SCR_BayonetComponent.Cast(item.FindComponent(SCR_BayonetComponent)))
			m_bHasUnderbarrelAttachment = false;
		
		if (m_OnItemRemovedFromSlotInvoker && !item.IsDeleted())
			m_OnItemRemovedFromSlotInvoker.Invoke(item, slotID);
	}
};
