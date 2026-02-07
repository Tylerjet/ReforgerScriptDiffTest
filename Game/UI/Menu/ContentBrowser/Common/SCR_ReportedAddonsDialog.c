//------------------------------------------------------------------------------------------------
//! Show list of reported mods and provide option to cancel reports
class SCR_ReportedAddonsDialog : SCR_AddonListDialog
{
	protected const ResourceName ADDON_LINE_INTERACTIVE_LAYOUT = "{0C4FAA01F4F56A90}UI/layouts/Menus/ContentBrowser/DownloadManager/DonwloadManager_AddonDownloadLineReport.layout";
	
	protected string WIDGET_DETAIL = "TxtDetailType";
	protected string WIDGET_MESSAGE = "TxtDetailMessage";
	
	protected const string BUTTON_CANCEL_REPORT = "cancelReport";
	
	protected ref ScriptInvoker<SCR_ReportedAddonsDialog> Event_OnAllReportsCanceled;
	
	// Widgets 
	protected TextWidget m_wTxtType;
	protected TextWidget m_wTxtMessage;
	protected SCR_NavigationButtonComponent m_NavCancelReport;
	
	//protected SCR_WorkshopItem m_ItemFocused;
	protected SCR_DownloadManager_AddonDownloadLine m_LineFocused;
	
	//------------------------------------------------------------------------------------------------
	// Override API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		VerticalLayoutWidget layout = VerticalLayoutWidget.Cast(GetRootWidget().FindAnyWidget("AddonList"));
		
		// Find widgets 
		m_wTxtType = TextWidget.Cast(GetRootWidget().FindAnyWidget(WIDGET_DETAIL));
		m_wTxtMessage = TextWidget.Cast(GetRootWidget().FindAnyWidget(WIDGET_MESSAGE));
		
		// Create widgets
		foreach (SCR_WorkshopItem item : m_aItems)
		{
			Widget w;
			Print(ADDON_LINE_INTERACTIVE_LAYOUT);
			
			// Interactive line for reported
			if (item.GetReportedByMe())
				w = GetGame().GetWorkspace().CreateWidgets(ADDON_LINE_INTERACTIVE_LAYOUT, layout);
			else
				w = GetGame().GetWorkspace().CreateWidgets(ADDON_LINE_LAYOUT, layout);
			
			SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
			comp.InitForWorkshopItem(item, string.Empty, false);
			
			m_aDownloadLines.Insert(comp);
			
			// Find button comp
			SCR_ModularButtonComponent button = SCR_ModularButtonComponent.Cast(w.FindHandler(SCR_ModularButtonComponent));
			if (!button)
				continue;
			
			// Invoker actions
			button.m_OnFocus.Insert(OnAddonLineFocus);
			button.m_OnDoubleClicked.Insert(OnReportedModDoubleClick);
			
			// Load reports
			item.LoadReport();
		}
		
		// Cancel report action 
		m_NavCancelReport = FindButton(BUTTON_CANCEL_REPORT);
		if (m_NavCancelReport)
			m_NavCancelReport.m_OnActivated.Insert(OnCancelReportActived);
		
		// Focus first 
		if (m_aDownloadLines.IsEmpty())
			return;
		
		Widget line = m_aDownloadLines[0].GetRootWidget();
		if (line)
			GetGame().GetWorkspace().SetFocusedWidget(line);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Close()
	{
		super.Close();
		
		if (m_NavCancelReport)
			m_NavCancelReport.m_OnActivated.Remove(OnCancelReportActived);
	} 
	
	//------------------------------------------------------------------------------------------------
	// Protected API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Show details of addon line report when focused
	protected void OnAddonLineFocus(SCR_ModularButtonComponent button)
	{
		Widget w = button.GetRootWidget();
		SCR_DownloadManager_AddonDownloadLine line = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
		
		if (line)
			m_LineFocused = line;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Start to canceling report of currently focused mod
	protected void OnCancelReportActived(SCR_NavigationButtonComponent button, string action)
	{
		if (m_LineFocused)
		{
			LineCancelReport(m_LineFocused);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnReportedModDoubleClick(SCR_ModularButtonComponent button)
	{
		if (!button)
			return;
		
		Widget w = button.GetRootWidget();
		if (!w)
			return;
		
		SCR_DownloadManager_AddonDownloadLine line = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
		if (!line)
			return;
		
		LineCancelReport(line);
	}
	
	protected ref SCR_CancelMyReportDialog m_DialogCancelReport;
	
	//------------------------------------------------------------------------------------------------
	//! Open report of given mod entry 
	protected SCR_CancelMyReportDialog LineCancelReport(SCR_DownloadManager_AddonDownloadLine line)
	{
		m_DialogCancelReport = new SCR_CancelMyReportDialog(line.GetItem());
		
		// Invoker actions
		if (m_DialogCancelReport)
		{
			/*cancelReport.GetOnSucces().Insert(OnCancelReportSuccess);
			cancelReport.GetOnFail().Insert(OnCancelReportFail);*/
			
			/*cancelReport.GetWorkshopItemAction().m_OnCompleted.Insert(OnCancelReportSuccess);
			cancelReport.GetWorkshopItemAction().m_OnFailed.Insert(OnCancelReportFail);*/
			
			m_DialogCancelReport.m_OnConfirm.Insert(OnCancelReportConfirm);
			m_DialogCancelReport.m_OnClose.Insert(OnCancelReportDialog);
		}
		
		return m_DialogCancelReport;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelReportConfirm(SCR_ConfigurableDialogUi dialog)
	{
		//SCR_CancelMyReportDialog cancelReport = SCR_CancelMyReportDialog.Cast(dialog);
		
		m_DialogCancelReport.GetWorkshopItemAction().m_OnCompleted.Insert(OnCancelReportSuccess);
		m_DialogCancelReport.GetWorkshopItemAction().m_OnFailed.Insert(OnCancelReportFail);
		
		// Clear confirm 
		m_DialogCancelReport.m_OnConfirm.Remove(OnCancelReportConfirm);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelReportSuccess(SCR_WorkshopItemActionCancelReport action)
	{		
		// Remove item 
		m_aItems.RemoveItem(action.m_Wrapper);
		
		if (m_aItems.IsEmpty())
		{
			Close();
			InvokeOnAllReportsCanceled();
		}
		else
		{
			// Mark successful report cancel 
			if (m_LineFocused)
			{
				// TODO: add "report canceled" string
				m_LineFocused.DisplayError("#AR-Workshop_Dialog_Success", true);
				
				// Clear interactions
				m_LineFocused.GetRootWidget().SetFlags(WidgetFlags.NOFOCUS);
				SCR_ModularButtonComponent button = SCR_ModularButtonComponent.Cast(m_LineFocused.GetRootWidget().FindHandler(SCR_ModularButtonComponent));
				if (button)
				{
					button.m_OnFocus.Remove(OnAddonLineFocus);
					button.m_OnDoubleClicked.Remove(OnReportedModDoubleClick);
				}
			}	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelReportFail(SCR_WorkshopItemActionCancelReport action)
	{
		// Remove invoker actions 
		action.m_OnCompleted.Remove(OnCancelReportSuccess);
		action.m_OnFailed.Remove(OnCancelReportFail);
		
		// Open dialog again with warning
		SCR_CancelMyReportDialog nextDialog = LineCancelReport(m_LineFocused);
		
		nextDialog.SetMessage("#AR-Workshop_ErrorUnknown");
		if (nextDialog.GetMessageWidget())
			nextDialog.GetMessageWidget().SetColor(UIColors.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelReportDialog(SCR_ConfigurableDialogUi dialog)
	{
		//SCR_CancelMyReportDialog cancelReport = SCR_CancelMyReportDialog.Cast(dialog);
		
		// Focus first 
		GetGame().GetCallqueue().CallLater(FocusTopLine, 1);
		
		// Clear invokers 
		m_DialogCancelReport.m_OnConfirm.Remove(OnCancelReportConfirm);
		m_DialogCancelReport.m_OnCancel.Remove(OnCancelReportDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FocusTopLine()
	{
		// Find first focusable 
		for (int i = 0, count = m_aDownloadLines.Count(); i < count; i++)
		{
			if (m_aDownloadLines[i].GetRootWidget().IsFocusable())
			{
				GetGame().GetWorkspace().SetFocusedWidget(m_aDownloadLines[i].GetRootWidget());
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Invokers 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeOnAllReportsCanceled()
	{
		if (!Event_OnAllReportsCanceled)
			Event_OnAllReportsCanceled = new ScriptInvoker();
		
		Event_OnAllReportsCanceled.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAllReportsCanceled()
	{
		if (!Event_OnAllReportsCanceled)
			Event_OnAllReportsCanceled = new ScriptInvoker();
		
		return Event_OnAllReportsCanceled;
	}
	
};