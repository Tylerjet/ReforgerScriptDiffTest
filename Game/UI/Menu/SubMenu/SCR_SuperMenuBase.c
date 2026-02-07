//------------------------------------------------------------------------------------------------
class SCR_SuperMenuBase : MenuRootBase //ChimeraMenuBase
{
	protected ResourceName m_sNavigationButtonLayout = "{08CF3B69CB1ACBC4}UI/layouts/WidgetLibrary/WLib_NavigationButton.layout";
	protected Widget m_wFooterLeft;
	protected Widget m_wFooterRight;
	protected const string TAB_VIEW_NAME = "TabViewRoot0";
	protected const string FOOTER_LEFT_NAME = "Footer";
	protected const string FOOTER_RIGHT_NAME = "FooterRight";
	protected const string TITLE_NAME = "Title";
	protected int m_iActiveTab = -1;
	protected SCR_TabViewComponent m_TabViewComponent;
	protected SCR_SubMenuBase m_OpenedSubmenu;
	protected ref array<SCR_SubMenuBase> m_aSubMenus = new ref array<SCR_SubMenuBase>;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		if (m_OpenedSubmenu && m_OpenedSubmenu.GetRootWidget().IsVisible())
			m_OpenedSubmenu.OnMenuUpdate(this, tDelta);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		// Find tabViewComponent, listen to OnChange
		Widget w = GetRootWidget();
		Widget tab = w.FindAnyWidget(TAB_VIEW_NAME);
		if (!tab)
			return;
		
		m_wFooterLeft = w.FindAnyWidget(FOOTER_LEFT_NAME);
		m_wFooterRight = w.FindAnyWidget(FOOTER_RIGHT_NAME);

		m_TabViewComponent = SCR_TabViewComponent.Cast(tab.FindHandler(SCR_TabViewComponent));
		if (!m_TabViewComponent)
			return;

		m_TabViewComponent.m_OnContentCreate.Insert(OnTabCreate);
		m_TabViewComponent.m_OnContentShow.Insert(OnTabShow);
		m_TabViewComponent.m_OnContentHide.Insert(OnTabHide);
		m_TabViewComponent.m_OnContentRemove.Insert(OnTabRemove);
		
		m_TabViewComponent.Init();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		if (m_OpenedSubmenu)
			m_OpenedSubmenu.OnMenuShow(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		if (m_OpenedSubmenu)
			m_OpenedSubmenu.OnMenuHide(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		foreach (SCR_SubMenuBase subMenu : m_aSubMenus)
		{
			if (subMenu)
				subMenu.OnMenuClose(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTabCreate(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return;
		
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;
		
		subMenu.OnMenuOpen(this);
		m_aSubMenus.Insert(subMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTabShow(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return;
		
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;
		
		subMenu.OnMenuShow(this);
		m_OpenedSubmenu = subMenu;
	}

	//------------------------------------------------------------------------------------------------
	void OnTabHide(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return;
		
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;
		
		subMenu.OnMenuHide(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTabRemove(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return;
		
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;
		
		subMenu.OnMenuClose(this);
		m_aSubMenus.RemoveItem(subMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! rightFooter - when true, the button will be added to the right footer.
	SCR_NavigationButtonComponent AddNavigationButton(string action, string buttonText, bool rightFooter = false)
	{
		if (!m_wFooterLeft || !m_wFooterRight || m_sNavigationButtonLayout == string.Empty)
			return null;
		
		Widget footer = m_wFooterLeft;
		if (rightFooter)
		{
			footer = m_wFooterRight;
		}
			
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sNavigationButtonLayout, footer);
		if (!w)
			return null;
		
		//Handle padding
		float padding, paddingLeft, paddingRight;
		float a, b, c;
		AlignableSlot.GetPadding(w, a, b, padding, c);
		if(rightFooter)
			paddingLeft = padding;
		else 
			paddingRight = padding;
		
		AlignableSlot.SetPadding(w, paddingLeft, 0.0, paddingRight, 0.0);
		
		
		SCR_NavigationButtonComponent comp = SCR_NavigationButtonComponent.Cast(w.FindHandler(SCR_NavigationButtonComponent));
		if (!comp)
			return null;
		
		comp.SetLabel(buttonText);
		comp.SetAction(action);
		return comp;
	}
	
	//------------------------------------------------------------------------------------------------
	void CloseMenu(bool animate = true)
	{
		if (!animate)
		{
			Close();
			return;
		}
		
		AnimateWidget.Opacity(GetRootWidget(), 0, UIConstants.FADE_RATE_FAST);
		float time = 1 / UIConstants.FADE_RATE_FAST;
		GetGame().GetCallqueue().CallLater(Close, time);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTitle(string title)
	{
		TextWidget wTitle = TextWidget.Cast(GetRootWidget().FindAnyWidget(TITLE_NAME));
		if (wTitle)
			wTitle.SetText(title);
	}
	
	//------------------------------------------------------------------------------------------------
	void OpenSubMenu(int index)
	{
		if (m_TabViewComponent)
			m_TabViewComponent.ShowTab(index);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SubMenuBase GetOpenedSubMenu()
	{
		return m_OpenedSubmenu;
	}
};