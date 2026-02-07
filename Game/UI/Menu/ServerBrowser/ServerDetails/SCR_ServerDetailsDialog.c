/*!
Details with mod list to display all server mods.
There should be displayed missing server.
Extra information about server can be added. 
*/

class SCR_ServerDetailsDialog : SCR_AddonListDialog
{
	protected const string WIDGET_SCROLl = "ScrollLayout";
	protected const string WIDGET_ADDON_LIST = "AddonList";
	protected const string WIDGET_MISSING_ADDON_WRAP = "MissingAddonWrap"; // Wrap holding text + list
	protected const string WIDGET_MISSING_ADDON_LIST = "MissingAddonList"; // Just vertical list for filling with entries
	protected const string WIDGET_LOADING = "Loading";
	
	protected const string BTN_CONFIRM = "confirm";
	
	protected Widget m_wScroll;
	protected Widget m_wAddonList;
	protected Widget m_wMissingAddonWrap;
	protected Widget m_wMissingAddonList;
	protected Widget m_wLoading;
	
	protected SCR_NavigationButtonComponent m_NavConfirm;

	//----------------------------------------------------------------------------------------
	// Override
	//----------------------------------------------------------------------------------------
		
	//----------------------------------------------------------------------------------------
	override protected void InitWidgets()
	{
		super.InitWidgets();
		
		m_wScroll = m_wRoot.FindAnyWidget(WIDGET_SCROLl);
		m_wAddonList = m_wRoot.FindAnyWidget(WIDGET_ADDON_LIST);
		m_wMissingAddonWrap = m_wRoot.FindAnyWidget(WIDGET_MISSING_ADDON_WRAP);
		m_wMissingAddonList = m_wRoot.FindAnyWidget(WIDGET_MISSING_ADDON_LIST);
		m_wLoading = m_wRoot.FindAnyWidget(WIDGET_LOADING);
	}
	
	override protected void Init(Widget root, SCR_ConfigurableDialogUiPreset preset, SCR_ConfigurableDialogUiProxy proxyMenu)
	{
		super.Init(root, preset, proxyMenu);
		
		m_NavConfirm = FindButton(BTN_CONFIRM);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		// Set visibility 
		m_wScroll.SetVisible(false);
		m_wLoading.SetVisible(true);
	}
	
	//----------------------------------------------------------------------------------------
	// Public
	//----------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	static SCR_ServerDetailsDialog CreateServerDetails(Room room, array<ref SCR_WorkshopItem> items, string preset, ResourceName dialogsConfig = "")
	{
		if (dialogsConfig == "")
			dialogsConfig = SCR_WorkshopUiCommon.DIALOGS_CONFIG;
						
		SCR_ServerDetailsDialog dialog = new SCR_ServerDetailsDialog(items, "");
		SCR_ConfigurableDialogUi.CreateFromPreset(dialogsConfig, preset, dialog);
		return dialog;
	}
	
	//------------------------------------------------------------------------------------------------
	void FillModList()
	{
		Widget layout = GetRootWidget().FindAnyWidget("AddonList");
		m_wMissingAddonWrap.SetVisible(false);
		
		// Setup downloaded
		foreach (SCR_WorkshopItem item : m_aItems)
		{
			// Change layout if need to be downloaded 
			if (!item.GetOffline() || item.GetCurrentLocalVersion() != item.GetDependency().GetVersion())
			{
				layout = m_wMissingAddonList;
				
				m_wMissingAddonWrap.SetVisible(true);
			}
			else
			{
				layout = m_wAddonList;
			}
			
			Widget w = GetGame().GetWorkspace().CreateWidgets(ADDON_LINE_LAYOUT, layout);
			
			SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
			comp.InitForWorkshopItem(item, string.Empty, false);
			
			m_aDownloadLines.Insert(comp);
		}
		
		// Set visibility
		m_wScroll.SetVisible(true);
		m_wLoading.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCanJoin(bool canJoin)
	{
		if (!m_NavConfirm)
			return;
		
		m_NavConfirm.SetEnabled(canJoin, false);
	}
};