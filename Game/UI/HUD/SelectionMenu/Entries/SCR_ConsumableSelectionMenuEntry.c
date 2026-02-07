class SCR_ConsumableSelectionMenuEntry : SCR_BaseGroupEntry
{
	protected IEntity m_Consumable;
	
	protected IEntity m_Owner;
	protected IEntity m_wPlayer
	protected SCR_InventoryStorageManagerComponent m_inventoryManagerComp;
	
	protected int m_iCount; 
	
	//------------------------------------------------------------------------------------------------
	override void UpdateVisuals()
	{
		m_EntryComponent.SetIconFaded(true);
		
		if (!m_EntryComponent)
			return;
		
		m_iCount = GetItemCount();
		
		if (!m_Consumable || m_iCount < 1)
		{
			// Default empty visuals
			m_EntryComponent.SetLabelText(string.Empty);
			m_EntryComponent.SetPreviewItem(null);
			return;
		}
		
		// Widget setup 
		m_EntryComponent.SetPreviewItem(m_Consumable);
		m_EntryComponent.SetIcon(string.Empty);
		m_EntryComponent.SetLabelText(m_iCount.ToString() + "x");
		m_EntryComponent.SetIconVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback for when this entry is supposed to be performed
	protected override event void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
		if (user != m_wPlayer)
			return;
		
		if (!m_inventoryManagerComp)
			m_inventoryManagerComp = SCR_InventoryStorageManagerComponent.Cast(m_wPlayer.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!m_inventoryManagerComp)
			return;
		
		CharacterControllerComponent controller = CharacterControllerComponent.Cast(m_wPlayer.FindComponent(CharacterControllerComponent));
		if (!controller)
			return;
		
		auto bandage = m_inventoryManagerComp.GetBandageItem();
		if (bandage)
			controller.TakeGadgetInLeftHand(bandage, 1);
		
		super.OnPerform(user, sourceMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! Can this entry be shown?
	protected override bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		if (user != m_Owner)
			return false;
		
		// check bandage in inventory 
		if (!m_inventoryManagerComp)
			m_inventoryManagerComp = SCR_InventoryStorageManagerComponent.Cast(m_Owner.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!m_inventoryManagerComp)
			return false;
		
		auto bandage = m_inventoryManagerComp.GetBandageItem();
		if (bandage)
			return true;
				
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Can this entry be performed?
	protected override bool CanBePerformedScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		if (user != m_Owner)
			return false;
		
		if (!m_Consumable || GetItemCount() < 1)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryNameScript(out string outName)
	{
		if (!m_Consumable || GetItemCount() < 1)
		{
			outName = "#AR-Error_NoBandages";
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryDescriptionScript(out string outDescription)
	{
		if (!m_Consumable || GetItemCount() < 1)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryIconPathScript(out string outIconPath)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override UIInfo GetUIInfoScript()
	{
		if (!m_Consumable)
			return null; 
		
		InventoryItemComponent item = InventoryItemComponent.Cast(m_Consumable.FindComponent(InventoryItemComponent));
		if (!item)
			return null;
		
		SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(item.GetAttributes());
		if (!attributes)
			return null;
		
		UIInfo uiInfo = attributes.GetUIInfo();
		if (!uiInfo)
			return null;
		
		return uiInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetItemCount()
	{
		if (!m_wPlayer)
			return 0;
		
		// Inventory check 
		if (!m_inventoryManagerComp)
			m_inventoryManagerComp = SCR_InventoryStorageManagerComponent.Cast(m_wPlayer.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!m_inventoryManagerComp)
			return 0;
				
		return m_inventoryManagerComp.GetHealthComponentCount();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_ConsumableSelectionMenuEntry(IEntity item, IEntity owner, IEntity player)
	{
		m_Consumable = item;
		m_Owner = owner;
		m_wPlayer = player;
	}
};

