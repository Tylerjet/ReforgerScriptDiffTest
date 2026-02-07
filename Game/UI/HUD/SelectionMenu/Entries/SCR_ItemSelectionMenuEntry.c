class SCR_ItemSelectionMenuEntry : SCR_BaseGroupEntry
{
	protected SCR_CharacterInventoryStorageComponent m_pStorage;
	protected int m_iQuickSlotID = -1;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateVisuals()
	{	
		SCR_SelectionEntryWidgetComponent entryWidget = SCR_SelectionEntryWidgetComponent.Cast(GetEntryComponent());
		if (!entryWidget)
			return;
		
		entryWidget.SetIconFaded(true);
		
		// Weapon and info
		IEntity item = GetItem();
		if (!item)
			return;
		
		// Widget setup 
		entryWidget.SetLabelText(string.Empty);
		entryWidget.SetPreviewItem(item);
		entryWidget.SetIcon(string.Empty);
		entryWidget.SetIconVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetQuickSlotID()
	{
		return m_iQuickSlotID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetQuickSlotID(int id)
	{
		m_iQuickSlotID = id;
		UpdateVisuals();
	}
	
	//------------------------------------------------------------------------------------------------
	protected IEntity GetItem()
	{	
		if (m_iQuickSlotID < 0)
			return null;
		
		if (!m_pStorage)
			return null;
		
		array<IEntity> quickSlotItems = m_pStorage.GetQuickSlotItems();
		if (quickSlotItems && m_iQuickSlotID < quickSlotItems.Count())
			return quickSlotItems[m_iQuickSlotID];
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback for when this entry is supposed to be performed
	protected override event void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
		if (!user)
			return;
		
		if (m_pStorage)
			m_pStorage.UseItem(GetItem());
		
		super.OnPerform(user, sourceMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! Can this entry be shown?
	protected override bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		return m_pStorage;
	}

	//------------------------------------------------------------------------------------------------
	//! Can this entry be performed?
	protected override bool CanBePerformedScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		IEntity item = GetItem();
		return item && m_pStorage.CanUseItem(item);;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryNameScript(out string outName)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryDescriptionScript(out string outDescription)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryIconPathScript(out string outIconPath)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override UIInfo GetUIInfoScript()
	{
		IEntity item = GetItem();
		if (!item)	
			return null;
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return null;
		
		return itemComponent.GetUIInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ItemSelectionMenuEntry(IEntity owner, int quickSlotID)
	{
		if (!owner)
			return;
		
		m_iQuickSlotID = quickSlotID;
		
		m_pStorage = SCR_CharacterInventoryStorageComponent.Cast(owner.FindComponent(SCR_CharacterInventoryStorageComponent));
	}
};