enum SCR_EModDetailsMenuTabs
{
	OVERVIEW = 0,
	DEPENDENCY
}

class SCR_ModDetailsSuperMenuComponent : SCR_SuperMenuComponent
{
	protected SCR_WorkshopItem m_WorkshopItem;
	protected ref array<ref SCR_WorkshopItem> m_aDependencies = {};
	
	//------------------------------------------------------------------------------------------------
	//! Callback passed to tab view, called when a new tab is created
	override SCR_SubMenuBase OnTabCreate(SCR_TabViewComponent comp, Widget w, int index)
	{
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return null;
		
		SCR_ContentBrowser_AddonsSubMenu dependencySubMenu = SCR_ContentBrowser_AddonsSubMenu.Cast(subMenu);
		if (dependencySubMenu)
			dependencySubMenu.SetWorkshopItems(m_aDependencies);
		
		return super.OnTabCreate(comp, w, index);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWorkshopItem(SCR_WorkshopItem item)
	{
		m_WorkshopItem = item;
		
		// Request data from the backend, if this item is present in the backend and we are connected
		if (m_WorkshopItem.GetOnline() && SCR_ServicesStatusHelper.IsBackendReady() && GetGame().GetBackendApi().IsAuthenticated())
		{
			m_WorkshopItem.m_OnGetAsset.Insert(OnDetailsLoaded);
			m_WorkshopItem.m_OnDependenciesLoaded.Insert(OnAskDetailsGetDependencyTree);

			m_WorkshopItem.LoadDetails();
		}
		else
		{
			// If this item is purely local, we call the callbacks ourselves
			string gameEnv = GetGame().GetBackendApi().GetBackendEnv();
			string modEnv = m_WorkshopItem.GetWorkshopItem().GetBackendEnv();
			if (gameEnv == modEnv)
			{
				OnDetailsLoaded();
				OnAskDetailsGetDependencyTree();
			}
		}
	}
	
	// SCR_WorkshopItem requests
	//------------------------------------------------------------------------------------------------
	protected void OnDetailsLoaded()
	{
		#ifdef WORKSHOP_DEBUG
		m_WorkshopItem.LogState();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	// Called when we have received dependencies
	protected void OnAskDetailsGetDependencyTree()
	{
		// Whole list of dependencies is available now
		m_aDependencies = m_WorkshopItem.GetLatestDependencies();

		if (m_aDependencies.IsEmpty())
			return;
		
		SCR_ContentBrowser_AddonsSubMenu dependencySubMenu;
		
		if (m_aSubMenus.IsIndexValid(SCR_EModDetailsMenuTabs.DEPENDENCY))
		 dependencySubMenu = SCR_ContentBrowser_AddonsSubMenu.Cast(m_aSubMenus[SCR_EModDetailsMenuTabs.DEPENDENCY]);
		
		if (dependencySubMenu)
			dependencySubMenu.SetWorkshopItems(m_aDependencies);

		// Load details for all dependencies, so we have data about reported state
		foreach (SCR_WorkshopItem dep : m_aDependencies)
		{
			dep.LoadDetails();
		}

		GetTabView().EnableTab(SCR_EModDetailsMenuTabs.DEPENDENCY, true);
	}
}