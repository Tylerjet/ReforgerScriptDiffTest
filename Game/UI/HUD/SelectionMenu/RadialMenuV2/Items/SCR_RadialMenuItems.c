//------------------------------------------------------------------------------------------------
class SCR_RadialMenuItems : SCR_RadialMenuHandler
{
	// Page name 
	protected const string PAGENAME_ITEMS = "Items";
	protected const int FIRST_ITEM_SLOT = 4;
	protected const int SLOT_COUNT = 10;
	
	// Data templates 
	protected static ResourceName s_sEntryLayout = "{121C45A1F59DC1AF}UI/layouts/Common/RadialMenu/RadialEntryElement.layout";
	
	//------------------------------------------------------------------------------------------------
	override protected void PageSetup()
	{
		super.PageSetup();
		PageSetupSlots();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create page with slots for items 
	protected void PageSetupSlots()
	{
		// Edit name 
		SCR_MenuPage pageItems = m_aMenuPages[ERadialMenuItemsPages.ITEMS];
		pageItems.SetName(PAGENAME_ITEMS);
		
		// Receive items assigned to quick slots
		SCR_ItemSelectionMenuEntry entry;
		for (int i = FIRST_ITEM_SLOT; i < SLOT_COUNT; i++)
		{
			entry = new SCR_ItemSelectionMenuEntry(m_pSource, i);
			entry.SetEntryLayout(s_sEntryLayout);
			pageItems.AddEntry(entry);
		}

		// Set additional info
		pageItems.SetIconName("copy");
		
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OpenMenu(IEntity owner, bool isOpen)
	{
		super.OpenMenu(owner, isOpen);
		
		if (isOpen)
		{
			m_pSource = SCR_PlayerController.GetLocalMainEntity();
			PageSetup();
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void FillEntry(IEntity owner, BaseSelectionMenuEntry entry, int index)
	{
		// Setup of entry
		BaseSelectionMenuEntry itemEntry = new SCR_ItemSelectionMenuEntry(owner, index);
		
		// Will just pass entry so it's at the end
		super.FillEntry(owner, itemEntry, index);
	}
};

//------------------------------------------------------------------------------------------------
//! group split  
enum ERadialMenuItemsPages
{
	ITEMS = 0
};