class SCR_ModReportDialogComponent : SCR_ScriptedWidgetComponent
{
	// Const localized strings 
	protected const string MSG_SELECT = "#AR-Workshop_SelectMsg";
	protected const string LINE_DOWNLOADED = "#AR-Workshop_TabName_Downloaded";
	
	protected const int AUTHOR_MOD_LIMIT = 50;
	
	// Attributes
	[Attribute("{ADEA32EB841E8629}Configs/ContentBrowser/ContentBrowserReportDialogs.conf", UIWidgets.ResourceNamePicker, "Layout of the navigation button", params: "layout")]
	protected ResourceName m_sDialogsConfig;
	
	[Attribute("select_report")]
	protected string m_sTagSelectReport;
	
	[Attribute("report_author")]
	protected string m_sTagReportAuthor;
	
	// Variables 
	protected ref SCR_ConfigurableDialogUi m_CurrentDialog;
	protected ReportDialogUI m_ReportDialog;
	
	protected ref SCR_WorkshopItem m_Item;
	protected WorkshopAuthor m_Author;
	
	protected ref SCR_WorkshopApiCallback_RequestPage m_CallbackPage;
	protected ref SCR_ContentBrowser_GetAssetListParams m_Params;
	
	protected ref array<ref SCR_ConfigurableDialogUi> m_aDialogs = {};
	protected SCR_LoadingOverlayDialog m_LoadingOverlayDlg;

	protected ref array <ref SCR_WorkshopItem> m_aAuthorModsList = {};
	
	//------------------------------------------------------------------------------------------------
	void OpenSelectReport(notnull SCR_WorkshopItem item)
	{
		m_CurrentDialog = SCR_ConfigurableDialogUi.CreateFromPreset(m_sDialogsConfig, m_sTagSelectReport);
		if (!m_CurrentDialog)
			return;
		
		m_aDialogs.Insert(m_CurrentDialog);
		
		m_Item = item;
		m_Author = item.GetWorkshopItem().Author();
		
		// Message
		string author = m_Item.GetAuthorName();
		m_CurrentDialog.GetMessageWidget().SetTextFormat(MSG_SELECT, author);
		m_CurrentDialog.GetMessageWidget().SetVisible(true);
		
		// Actions 
		m_CurrentDialog.m_OnConfirm.Insert(OnSelectReportConfirm);
		m_CurrentDialog.m_OnCancel.Insert(OnSelectReportCancel);
		
		// Author report action
		SCR_NavigationButtonComponent butAuthor = m_CurrentDialog.FindButton("report_author");
		if (butAuthor)
			butAuthor.m_OnActivated.Insert(OnSelectReportAuthor);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSelectReportConfirm(SCR_ConfigurableDialogUi dialog)
	{
		OpenReportThis();
		m_CurrentDialog.ClearButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSelectReportCancel(SCR_ConfigurableDialogUi dialog)
	{
		m_CurrentDialog.ClearButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSelectReportAuthor()
	{
		// Callback 
		m_CallbackPage = new ref SCR_WorkshopApiCallback_RequestPage(0);
		
		if (!m_Author.IsBlocked())
			m_CallbackPage.m_OnSuccess.Insert(OpenReportAuthorModList);
		else 
			m_CallbackPage.m_OnSuccess.Insert(OpenRemoveAuthorBlockModList);
		
		m_LoadingOverlayDlg = SCR_LoadingOverlayDialog.Create();
		
		// Params 
		ContentBrowserUI cb = ContentBrowserUI.Cast(
			GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.ContentBrowser)
		);
		
		SCR_ContentBrowser_AddonsSubMenu addonsSubMenu = cb.GetOnlineSubMenu();
		m_Params = new SCR_ContentBrowser_GetAssetListParams(addonsSubMenu);
		m_Params.limit = AUTHOR_MOD_LIMIT;
		m_Params.offset = 0;
		
		m_Author.RequestPage(m_CallbackPage, m_Params, false);
				
		///OpenReportAuthorModList();
		if (m_CurrentDialog)
			m_CurrentDialog.ClearButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Dialog for selecting type, typing message and confirming this mod report
	void OpenReportThis()
	{
		m_ReportDialog = ReportDialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ReportItemDialog));
		if (!m_ReportDialog)
			return;
		
		// Init 
		ContentBrowserDetailsMenu detailsMenu = ContentBrowserDetailsMenu.Cast(GetGame().GetMenuManager().GetTopMenu());
		if (detailsMenu)
			m_ReportDialog.Init(detailsMenu);
		
		// Invoker actions 
		m_ReportDialog.m_OnCancel.Insert(OnCancelThisReport);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelThisReport()
	{
		if (m_ReportDialog)
			m_ReportDialog.m_OnCancel.Remove(OnCancelThisReport);
		
		OpenSelectReport(m_Item);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Dialog displaying all author mmods and explaing what will happen after report confirm
	protected void OpenReportAuthorModList(SCR_WorkshopApiCallback_RequestPage callback)
	{		
		m_CallbackPage.m_OnSuccess.Remove(OpenReportAuthorModList);
		
		string author = m_Item.GetAuthorName();
		
		OpenAuthorModsDialog();
		m_CurrentDialog.GetMessageWidget().SetTextFormat("#AR-Workshop_ReportAuthorMsg" + "\n\n" + "#AR-Workshop_AffectedMods", author);
		
		// Actions 
		m_CurrentDialog.m_OnConfirm.Insert(OnConfirmReportAuthorModList);
		m_CurrentDialog.m_OnCancel.Insert(OnCancelReportAuthorModList);
		
		m_LoadingOverlayDlg.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Dialog displaying all author mmods and explaing what will happen after report confirm
	protected void OpenRemoveAuthorBlockModList(SCR_WorkshopApiCallback_RequestPage callback)
	{		
		m_CallbackPage.m_OnSuccess.Remove(OpenRemoveAuthorBlockModList);
		
		string author = m_Item.GetAuthorName();
		
		OpenAuthorModsDialog(false);
		m_CurrentDialog.GetMessageWidget().SetTextFormat("#AR-Workshop_CancelAuthorReport" + "\n\n" + "#AR-Workshop_AffectedMods", author);
		
		// Actions 
		m_CurrentDialog.m_OnConfirm.Insert(OnConfirmRemoveAuthorBlock);
		m_CurrentDialog.m_OnCancel.Insert(OnCancelRemoveAuthorReport);
		
		m_LoadingOverlayDlg.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpenAuthorModsDialog(bool showRerpoted = true)
	{
		// Create dialog list
		array<WorkshopItem> toReport = {};
		m_Author.GetPageItems(toReport);
		
		m_aAuthorModsList.Clear();
	
		foreach (WorkshopItem item : toReport)
		{
			SCR_WorkshopItem scrItem = SCR_AddonManager.GetInstance().Register(item);
			m_aAuthorModsList.Insert(scrItem);
		}
		
		SCR_AddonListDialog dialog = SCR_AddonListDialog.CreateItemsList(m_aAuthorModsList, m_sTagReportAuthor, m_sDialogsConfig);
		m_CurrentDialog = dialog;
		if (!dialog)
			return;
		
		m_aDialogs.Insert(m_CurrentDialog);
		
		// Show message
		m_CurrentDialog.GetMessageWidget().SetVisible(true);
		
		// Mark mod entries 
		array<SCR_DownloadManager_AddonDownloadLine> lines = dialog.GetDonwloadLines();
	
		foreach (SCR_DownloadManager_AddonDownloadLine line : lines)
		{
			SCR_WorkshopItem item = line.GetItem();
			
			// Owned 
			if (item.GetOffline())
			{
				line.DisplayError(LINE_DOWNLOADED);
				continue;
			}
		}
	}
	
	protected ref SCR_WorkshopItemActionAddAuthorBlock m_ActionAddAuthorBlock;
	protected ref SCR_WorkshopItemActionRemoveAuthorBlock m_ActionRemoveAuthorBlock;
	
	//------------------------------------------------------------------------------------------------
	protected void OnConfirmReportAuthorModList()
	{
		// Block 
		m_ActionAddAuthorBlock = m_Item.AddAuthorBlock();
		
		m_ActionAddAuthorBlock.m_OnCompleted.Insert(OnAuthorReportSuccess);
		
		m_ActionAddAuthorBlock.Activate();
		
		OnCancelReportAuthorModList();
		m_LoadingOverlayDlg = SCR_LoadingOverlayDialog.Create();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnConfirmRemoveAuthorBlock()
	{
		m_ActionRemoveAuthorBlock = m_Item.RemoveAuthorBlock();
		
		m_ActionRemoveAuthorBlock.m_OnCompleted.Insert(OnRemoveAuthorBlockSuccess);
		
		m_ActionRemoveAuthorBlock.Activate();
		
		OnCancelReportAuthorModList();
		m_LoadingOverlayDlg = SCR_LoadingOverlayDialog.Create();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAuthorReportSuccess()
	{
		ContentBrowserDetailsMenu detailsMenu = ContentBrowserDetailsMenu.Cast(GetGame().GetMenuManager().GetTopMenu());
		
		detailsMenu.OnItemReportedSuccessfully();
		
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		
		// Clear up all author items loading and remove from local storage
		for (int i = 0, count = m_aAuthorModsList.Count(); i < count; i++)
		{
			m_aAuthorModsList[i].DeleteLocally();
			m_aAuthorModsList[i].SetSubscribed(false);
			
			// Cancel download 
			if (!mgr)
				continue;
			
			SCR_WorkshopItemActionDownload action = mgr.GetActionOfItem(m_aAuthorModsList[i]);
			if (action)
				action.Cancel();
		}
		
		//m_CurrentDialog.Close();
		m_CurrentDialog.m_OnClose.Insert(CloseDialogs);
		m_LoadingOverlayDlg.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CloseDialogs()
	{
		for (int i = 0, count = m_aDialogs.Count(); i < count; i++)
		{
			m_aDialogs[i].Close();
		}
		
		m_aDialogs.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelReportAuthorModList()
	{
		m_CurrentDialog.m_OnConfirm.Remove(OnConfirmReportAuthorModList);
		m_CurrentDialog.m_OnCancel.Remove(OnCancelReportAuthorModList);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRemoveAuthorBlockSuccess()
	{
		//m_CurrentDialog.Close();
		m_LoadingOverlayDlg.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelRemoveAuthorReport()
	{
		m_CurrentDialog.m_OnConfirm.Remove(OnRemoveAuthorBlockSuccess);
		m_CurrentDialog.m_OnCancel.Remove(OnCancelRemoveAuthorReport);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Dialog for selecting type, typing message and confirming author report
	/*void OpenReportAuthor()
	{
		m_ReportDialog = ReportDialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ReportItemDialog));
		if (!m_ReportDialog)
			return;
		
		// Add script 
		ReportDialogUI reportDialogComp = new ReportDialogUI();
		m_ReportDialog.GetRootWidget().AddHandler(reportDialogComp);
		
		ContentBrowserDetailsMenu detailsMenu = ContentBrowserDetailsMenu.Cast(GetGame().GetMenuManager().GetTopMenu());
		if (detailsMenu)
		{
			//m_ReportDialog.Init(detailsMenu);
			m_ReportDialog.Init(detailsMenu, true);
		}
		
		m_ReportDialog.SetTitle("#AR-Workshop_ReportAuthor");
		
		m_ReportDialog.m_OnCancel.Insert(OnCancelAuthorReport);
	}*/
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelAuthorReport()
	{
		m_ReportDialog.m_OnCancel.Remove(OnCancelAuthorReport);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetItem(SCR_WorkshopItem item)
	{
		m_Item = item;
		m_Author = item.GetWorkshopItem().Author();
	}
};
