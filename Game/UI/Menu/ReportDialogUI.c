//------------------------------------------------------------------------------------------------
class ReportDialogUI: DialogUI
{
	ContentBrowserDetailsMenu m_DetailsMenu;
		
	SCR_ComboBoxComponent m_ReasonCombo;
	SCR_EditBoxComponent m_InputField;
	
	Widget m_wDialogWindow;
	
	protected bool m_bAcceptInput = true;
	
	protected ref SCR_WorkshopItemAction m_ReportAction;
	
	protected bool m_bShouldEnableConfirmButton;
	
	//! Available categories of feedback
	
	
	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm()
	{
		if (!m_bAcceptInput)
			return;
		
		m_OnConfirm.Invoke();
		//super.OnConfirm(); // we don't want to close the dialog
		
		// Fetch feedback data

		int category;
		int reason;
		string content = "";
		
		if (m_ReasonCombo)
		{
			category = m_ReasonCombo.GetCurrentIndex();
			switch (category)
			{
				case 0:
					reason = EWorkshopReportType.EWREPORT_INAPPROPRIATE_CONTENT;
					break;
				
				case 1:
					reason = EWorkshopReportType.EWREPORT_OFFENSIVE_LANGUAGE;
					break;
				
				case 2:
					reason = EWorkshopReportType.EWREPORT_MISLEADING;
					break;
				
				case 3:
					reason = EWorkshopReportType.EWREPORT_OTHER;
					break;
			}
		}
		
		if (m_InputField)
			content = m_InputField.GetValue();
		
		// Show loading screen
		SCR_LoadingOverlay overlay = SCR_LoadingOverlay.ShowForWidget(m_wDialogWindow, string.Empty);
		
		// Disable writing mode so that user can press escape again
		if(overlay)
			overlay.SetFocus();
		
		SetAcceptInput(false);
		
		// Report
		m_ReportAction = m_DetailsMenu.m_WorkshopItem.Report(reason, content);
		
		m_ReportAction.m_OnCompleted.Insert(OnReportSuccess);
		
		m_ReportAction.Activate();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnReportSuccess()
	{
		m_DetailsMenu.OnItemReportedSuccessfully();
		m_DetailsMenu.m_WorkshopItem.DeleteLocally();
		m_DetailsMenu.m_WorkshopItem.SetSubscribed(false);
		
		// Cancel download 
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr)
		{
			SCR_WorkshopItemActionDownload action = mgr.GetActionOfItem(m_DetailsMenu.m_WorkshopItem);
			if (action)
				action.Cancel();
		}
		
		this.Close();
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	protected void SetAcceptInput(bool accept)
	{
		m_bAcceptInput = accept;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if content (textbox) is empty
	protected bool IsContentEmpty()
	{
		if (m_InputField)
		{
			string txt = m_InputField.GetValue();
			return txt.Length() <= 0;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		if (m_InputField)
			GetGame().GetWorkspace().SetFocusedWidget(m_InputField.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		Widget root = GetRootWidget();
		
		m_InputField = SCR_EditBoxComponent.GetEditBoxComponent("InputField", root);
		if (m_InputField)
		{
			m_InputField.m_OnChanged.Insert(OnFieldChanged);
		}
		
		m_ReasonCombo = SCR_ComboBoxComponent.GetComboBoxComponent("ReasonCombo", root);
		if (m_ReasonCombo)
		{
			m_ReasonCombo.ClearAll();
			string reportTypeStr = SCR_WorkshopUiCommon.GetReportTypeString(0);
			int i = 1;
			while (!reportTypeStr.IsEmpty())
			{
				m_ReasonCombo.AddItem(reportTypeStr);
				reportTypeStr = SCR_WorkshopUiCommon.GetReportTypeString(i);
				i++;
			}
			m_ReasonCombo.SetCurrentItem(0);
		}
		
		m_wDialogWindow = root.FindAnyWidget("DialogBase0");
		
		// Disable the confirm button initially because text is empty
		m_Confirm.SetEnabled(false);
		
		if(m_InputField)
		{
			m_InputField.m_OnWriteModeLeave.Insert(OnWriteModeLeave);
			m_InputField.m_OnTextChange.Insert(OnTextChange);
		}
		
		SCR_NavigationButtonComponent tos = SCR_NavigationButtonComponent.GetNavigationButtonComponent("ToS", GetRootWidget());
		if (tos)
			tos.m_OnActivated.Insert(OnTos);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWriteModeLeave()
	{
		OnTextChange();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTextChange()
	{
		if (!m_Confirm || !m_InputField)
			return;
		
		m_Confirm.SetEnabled(!m_InputField.GetValue().IsEmpty());
	}
	
	
	//------------------------------------------------------------------------------------------------
	void OnFieldChanged(SCR_EditBoxComponent comp, string text)
	{
		if (m_Confirm)
			m_Confirm.SetEnabled(text.Length() > 0);
	}
	
	//------------------------------------------------------------------------------------------------
	void Init(ContentBrowserDetailsMenu parentMenu)
	{
		m_DetailsMenu = parentMenu;
	}

	//------------------------------------------------------------------------------------------------
	void OnTos()
	{
		GetGame().GetPlatformService().OpenBrowser(GetGame().GetBackendApi().GetLinkItem("Link_PrivacyPolicy"));
	}
};