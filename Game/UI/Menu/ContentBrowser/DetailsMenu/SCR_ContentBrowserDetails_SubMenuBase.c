/*
Base class for all tabs in content browser details.
*/

class SCR_ContentBrowserDetails_SubMenuBase : SCR_SubMenuBase
{
	ContentBrowserDetailsMenu m_DetailsMenu;
	ref SCR_WorkshopItem m_WorkshopItem; // Strong ref!
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		
		m_DetailsMenu = ContentBrowserDetailsMenu.Cast(parentMenu);
		m_WorkshopItem = m_DetailsMenu.m_WorkshopItem;
		
		PrintFormat("OnMenuOpen: %1", this);
		
		//SCR_NavigationButtonComponent comp = this.CreateNavigationButton("MenuBack", "#AR-Menu_Back");
		//comp.m_OnActivated.Insert(OnMenuBack);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuClose(parentMenu);
		
		PrintFormat("OnMenuClose: %1", this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
		
		PrintFormat("OnMenuShow: %1", this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		PrintFormat("HandlerAttached: %1", this);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMenuBack()
	{
		this.CloseParent();
	}
};