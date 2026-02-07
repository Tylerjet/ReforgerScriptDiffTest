enum EWorkshopTabId
{
	ONLINE = 0,
	OFFLINE,
	MOD_MANAGER
}

//! The super menu class for WORKSHOP content browser
class ContentBrowserUI : SCR_SuperMenuBase
{
	protected SCR_InputButtonComponent m_NavBack;
	static const float SMALL_DOWNLOAD_THRESHOLD = 50 * 1024 * 1024; // 50 Megabytes
	
	float m_fTimerLowFreqUpdate;
	
	//------------------------------------------------------------------------------------------------
	static ContentBrowserUI Create()
	{
		MenuBase menuBase = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ContentBrowser);
		ContentBrowserUI browser = ContentBrowserUI.Cast(menuBase);

		return browser;
	}
	
	//------------------------------------------------------------------------------------------------
	static void _print(string str, LogLevel logLevel = LogLevel.DEBUG)
	{
		Print(string.Format("[Content Browser] %1", str), logLevel);
	}
	
	// --- Overrides ---
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		// Setup the 'back' nav button
		m_NavBack = m_DynamicFooter.FindButton("Back");
		if (m_NavBack)
			m_NavBack.m_OnActivated.Insert(Close);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		// Restore server hosting 
		if (!GetGame().IsPlatformGameConsole())
		{		
			if (ServerHostingUI.GetTemporaryConfig())
			{
				GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu);
				SCR_CommonDialogs.CreateServerHostingDialog();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		m_fTimerLowFreqUpdate += tDelta;
		if (m_fTimerLowFreqUpdate > 1)
		{
			LowFreqUpdate();
			m_fTimerLowFreqUpdate -= 1;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update non important UI elements
	protected void LowFreqUpdate()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		
		if(!mgr)
			return;
		
		// Warning sign
		// Show warning icon if there is any offline mod which has any issue
		array<ref SCR_WorkshopItem> allAddons = mgr.GetAllAddons();
		array<ref SCR_WorkshopItem> offlineAddons = SCR_AddonManager.SelectItemsBasic(allAddons, EWorkshopItemQuery.OFFLINE);
		
		bool anyIssue = false;
		
		foreach (SCR_WorkshopItem item : offlineAddons)
		{
			bool dependenciesMissing = item.GetAnyDependencyMissing();
			bool dependenciesOutdated = item.GetAnyDependencyUpdateAvailable();
			bool dependenciesDisabled = item.GetEnabledAndAnyDependencyDisabled();
			bool updateAvailable = item.GetUpdateAvailable();
			
			if (dependenciesMissing || dependenciesOutdated || dependenciesDisabled || updateAvailable)
			{
				anyIssue = true;
				break;
			}
		}
		
		m_SuperMenuComponent.GetTabView().ShowIcon(EWorkshopTabId.OFFLINE, anyIssue);		
	}
	
	//------------------------------------------------------------------------------------------------
	protected static string GetWorkshopStatus()
	{
		// Find WS status item
		ServiceStatusItem wsStatus = FindStatusItemByName(SCR_ServicesStatusHelper.SERVICE_WORKSHOP);

		if (!wsStatus)
			return string.Empty;
		
		return wsStatus.Status();
	}
	
	//------------------------------------------------------------------------------------------------
	// TODO: Move into generic backend script api
	protected static ServiceStatusItem FindStatusItemByName(string name)
	{
		BackendApi backend = GetGame().GetBackendApi();
		int statusesCount = backend.GetStatusCount();
		
		for (int i = 0; i < statusesCount; i++)
		{
			if (backend.GetStatusItem(i).Name() == name)
				return backend.GetStatusItem(i);
		}
		
		return null;
	}
	
	// --- Public ---
	//-----------------------------------------------------------------------------------------------
	SCR_SubMenuBase GetOpenedSubMenu()
	{
		return m_SuperMenuComponent.GetOpenedSubMenu();
	}
	
	//-----------------------------------------------------------------------------------------------
	void OpenModManager()
	{
		m_SuperMenuComponent.GetTabView().ShowTab(EWorkshopTabId.MOD_MANAGER);
	}
}