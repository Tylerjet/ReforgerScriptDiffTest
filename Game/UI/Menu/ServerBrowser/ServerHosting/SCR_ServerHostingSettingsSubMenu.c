/*!
Wrapper sub menu clas for server hosting setting tab
*/
class SCR_ServerHostingSettingsSubMenu : SCR_SubMenuBase
{
	protected SCR_NavigationButtonComponent m_NavHost;
	protected SCR_NavigationButtonComponent m_NavSave;
	
	//------------------------------------------------------------------------------------------------
	void Init(notnull SCR_SuperMenuComponent superMenu)
	{
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(GetRootWidget().FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;
		
		subMenu.SetParentMenuComponent(superMenu);
		
		m_NavHost = CreateNavigationButton("DialogConfirm", "#AR-ServerHosting_HostLocally", true);
		
		// Tempory disabled saving
		if (!GetGame().IsPlatformGameConsole())
			m_NavSave = CreateNavigationButton("MenuFilter", "#AR-PauseMenu_Save", true);
	}
	
	//-------------------------------------------------------------------------------------------
	// API 
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	SCR_NavigationButtonComponent GetNavHostButton()
	{
		return m_NavHost;
	}
	
	//-------------------------------------------------------------------------------------------
	SCR_NavigationButtonComponent GetNavSaveButton()
	{
		return m_NavSave;
	}
}