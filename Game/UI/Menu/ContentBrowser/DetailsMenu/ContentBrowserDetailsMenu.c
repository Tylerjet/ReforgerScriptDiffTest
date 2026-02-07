
class ContentBrowserDetailsMenu : SCR_SuperMenuBase
{
	protected SCR_ContentBrowser_AddonsSubMenu m_DependencySubMenu;

	protected static SCR_WorkshopItem m_WorkshopItem;
	protected SCR_InputButtonComponent m_NavBack;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		if (!m_WorkshopItem)
		{
			Close();
			GetGame().GetCallqueue().CallLater(SCR_CommonDialogs.CreateRequestErrorDialog);
			return;
		}

		super.OnMenuOpen();

		SCR_ModDetailsSuperMenuComponent detailsSuperMenu = SCR_ModDetailsSuperMenuComponent.Cast(m_SuperMenuComponent);
		if (detailsSuperMenu)
			detailsSuperMenu.SetWorkshopItem(m_WorkshopItem);
		
		// Setup the 'back' nav button
		m_NavBack = m_DynamicFooter.FindButton("Back");
		if (m_NavBack)	
			m_NavBack.m_OnActivated.Insert(OnNavButtonClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavButtonClose()
	{
		// This menu might be not focused, because another details menu is currently open
		if (IsFocused())
			Close();
	}
	
	// Public
	//------------------------------------------------------------------------------------------------
	//! Opens the menu for a given workshop item
	static ContentBrowserDetailsMenu OpenForWorkshopItem(SCR_WorkshopItem item)
	{
		m_WorkshopItem = item;
		
		ContentBrowserDetailsMenu detailsMenu = ContentBrowserDetailsMenu.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ContentBrowserDetailsMenu));
		return detailsMenu;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_WorkshopItem GetWorkshopItem()
	{
		return m_WorkshopItem;
	}
}
