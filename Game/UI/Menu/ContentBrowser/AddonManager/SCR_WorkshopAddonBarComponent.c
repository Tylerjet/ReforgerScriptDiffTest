class SCR_WorkshopAddonBarComponent : ScriptedWidgetComponent
{
	protected const ResourceName ADDON_BAR_ICON_LAYOUT = "{9B80BD4A534C651C}UI/layouts/Menus/ContentBrowser/AddonManager/AddonBar/AddonBarIcon.layout";
	
	protected ref SCR_WorkshopAddonBarWidgets m_Widgets = new SCR_WorkshopAddonBarWidgets();
	protected Widget m_wRoot;
	
	[Attribute("", UIWidgets.EditBox, desc: "Widget that should be focused if is not possible to escape from bar")]
	protected string m_sLeaveWidget;
	
	[Attribute()]
	protected ref array<string> m_aButtonNames;
	
	[Attribute("20", UIWidgets.EditBox, desc: "After how many character should be display preset name cut")]
	protected int m_iPresetNameCap;
	
	protected Widget m_wLeaveWidget;
	
	protected ref array<Widget> m_aButtons = {};
	protected static SCR_ConfigurableDialogUi m_FailDialog;
	
	//---------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
			return;
		
		m_wRoot = w;
		m_Widgets.Init(w);
		
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		
		if (!mgr)
			return;
		
		mgr.m_OnAddonsEnabledChanged.Insert(Callback_OnAddonsEnabledChanged);
		mgr.GetPresetStorage().GetEventOnUsedPresetChanged().Insert(OnUsedPresetChanged);
		
		OnUsedPresetChanged(mgr.GetPresetStorage().GetUsedPreset());
		
		UpdateAllWidgets();
		
		m_Widgets.m_PresetsButtonComponent.m_OnClicked.Insert(Callback_OnPresetsButton);
		m_Widgets.m_UpdateButtonComponent.m_OnClicked.Insert(Callback_OnUpdateButton);
		
		GetGame().GetCallqueue().CallLater(OnFrame, 0, true);
		
		// Find buttons 
		for (int i = 0, count = m_aButtonNames.Count(); i < count; i++)
		{
			Widget btn = w.FindAnyWidget(m_aButtonNames[i]);
			if (btn)
				m_aButtons.Insert(btn);
		}
		
		// Check menu actions 
		GetGame().GetInputManager().AddActionListener("MenuUp", EActionTrigger.PRESSED, CheckTopMove);
		GetGame().GetInputManager().AddActionListener("MenuDown", EActionTrigger.PRESSED, CheckDownMove);
		
		GetGame().GetCallqueue().CallLater(InitialFocus);
		
		// Listen to download manager 
		SCR_DownloadManager.GetInstance().GetEventOnDownloadFail().Remove(DisplayFailDialog);
		SCR_DownloadManager.GetInstance().GetEventOnDownloadFail().Insert(DisplayFailDialog);
	}
	
	
	//---------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		
		if (mgr)
			SCR_AddonManager.GetInstance().m_OnAddonsEnabledChanged.Remove(Callback_OnAddonsEnabledChanged);
		
		GetGame().GetCallqueue().Remove(OnFrame);
		
		GetGame().GetInputManager().RemoveActionListener("MenuUp", EActionTrigger.PRESSED, CheckTopMove);
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected void Callback_OnAddonsEnabledChanged()
	{
		UpdateAllWidgets();
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Show actual preset name on preset name change
	protected void OnUsedPresetChanged(string name)
	{
		m_Widgets.m_SizePresetName.SetVisible(!name.IsEmpty());
		
		// Cap 
		if (!name.IsEmpty() && name.Length() > m_iPresetNameCap)
			name = name.Substring(0, m_iPresetNameCap) + "...";
		
		m_Widgets.m_TxtPresetName.SetText(name);
	}
	
	
	//---------------------------------------------------------------------------------------------------
	void UpdateAllWidgets()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		
		if (!mgr)
			return;
		
		// Mod count text
		int nAddonsEnabled = SCR_AddonManager.CountItemsBasic(SCR_AddonManager.GetInstance().GetOfflineAddons(), EWorkshopItemQuery.ENABLED);
		m_Widgets.m_ModsCountFrame.SetVisible(nAddonsEnabled > 0);
		m_Widgets.m_ModsCountText.SetText(nAddonsEnabled.ToString());
	}
	
	
	//---------------------------------------------------------------------------------------------------
	void Callback_OnPresetsButton()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.AddonsToolsMenu);
	}
	
	//---------------------------------------------------------------------------------------------------
	void Callback_OnUpdateButton()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		array<ref SCR_WorkshopItem> addonsOutdated = SCR_AddonManager.SelectItemsBasic(mgr.GetOfflineAddons(), EWorkshopItemQuery.UPDATE_AVAILABLE);
		
		// Open download confirmation dialog
		array<ref Tuple2<SCR_WorkshopItem, ref Revision>> addonsAndVersions = {};
		foreach (SCR_WorkshopItem item : addonsOutdated)
			addonsAndVersions.Insert(new Tuple2<SCR_WorkshopItem, ref Revision>(item, null));
		
		SCR_DownloadConfirmationDialog.CreateForAddons(addonsAndVersions, false);
	}
	
	
	//---------------------------------------------------------------------------------------------------
	void OnFrame()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		if (!mgr)
			return;
		
		int nOutdated = mgr.GetCountAddonsOutdated();
		
		m_Widgets.m_UpdateButton.SetVisible(nOutdated > 0);
		
		if (nOutdated > 0)
		{
			m_Widgets.m_OutdatedAddonsCountText.SetText(nOutdated.ToString());
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void CheckTopMove()
	{
		if (GetGame().GetWorkspace().GetFocusedWidget() != m_wRoot)
			GetGame().GetCallqueue().CallLater(FocusAddonBar, 100, false, GetGame().GetWorkspace().GetFocusedWidget());
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void FocusAddonBar(Widget lastFocused)
	{
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();	
		
		if (GetGame().GetWorkspace().GetFocusedWidget() == lastFocused)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_PresetsButton);
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void CheckDownMove()
	{
		GetGame().GetCallqueue().CallLater(LoseFocusAddonBar, 0, false, GetGame().GetWorkspace().GetFocusedWidget());
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void LoseFocusAddonBar(Widget lastFocused)
	{
		// Find leave widget 
		if (!m_wLeaveWidget && !m_sLeaveWidget.IsEmpty())
			m_wLeaveWidget = GetGame().GetWorkspace().FindAnyWidget(m_sLeaveWidget);
		
		if (!m_wLeaveWidget)
			return;
		
		FocusDown(lastFocused);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void FocusDown(Widget lastFocused)
	{
		for (int i = 0, count = m_aButtons.Count(); i < count; i++)
		{
			if (lastFocused == m_aButtons[i])
			{
				GetGame().GetWorkspace().SetFocusedWidget(m_wLeaveWidget);
				break;
			}
		}
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected void InitialFocus()
	{
		FocusDown(GetGame().GetWorkspace().GetFocusedWidget());
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void DisplayFailDialog()
	{
		// Check if dialog is not already opened
		if (m_FailDialog)
			return;
		
		array<ref Tuple2<SCR_WorkshopItem, ref Revision>> failed = {};
		array<ref SCR_WorkshopItemActionDownload> failedActions = SCR_DownloadManager.GetInstance().GetFailedDownloads();
		
		foreach (SCR_WorkshopItemActionDownload action : failedActions)
		{
			SCR_WorkshopItem item = action.m_Wrapper;
			Revision version = item.GetDependency().GetRevision();
			
			failed.Insert(new Tuple2<SCR_WorkshopItem, ref Revision>>(item, version));
		}
		
		// Setup dialog
		SCR_DownloadFailDialog dialog = SCR_DownloadFailDialog.CreateFailedAddonsDialog(failed, false);
		if (!dialog)
			return;
		
		m_FailDialog = dialog;
		m_FailDialog.m_OnConfirm.Insert(Callback_OnFailDialogConfirm);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnFailDialogConfirm(SCR_ConfigurableDialogUi dialog)
	{
		// Clear dialog
		m_FailDialog.m_OnConfirm.Clear();
		m_FailDialog.Close();
		m_FailDialog = null;
		SCR_DownloadManager.GetInstance().ClearFailedDownloads();
	}
}
