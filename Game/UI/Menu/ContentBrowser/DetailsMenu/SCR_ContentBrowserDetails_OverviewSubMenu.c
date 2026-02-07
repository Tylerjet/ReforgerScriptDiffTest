/*
The overview tab of the details view
*/

class SCR_ContentBrowserDetails_OverviewSubMenu : SCR_ContentBrowserDetails_SubMenuBase
{
	[Attribute()]
	protected ref SCR_ContentBrowser_ColorScheme m_ColorScheme;
	
	protected ref SCR_ContentBrowserDetails_OverviewSubMenuWidgets widgets;
	
	protected ref SCR_WorkshopUiCommon_DownloadSequence m_DownloadRequest;
	
	protected SCR_LoadingOverlayDialog m_LoadingOverlayDlg;
	
	protected ref array<SCR_ContentBrowser_ScenarioLineComponent> m_aScenarioLines = {};

	protected bool m_bGalleryUpdated = false; // Set true when we first time show images in gallery.
	
	// Addon-related navigation buttons
	protected SCR_NavigationButtonComponent m_NavPrimary;
	protected SCR_NavigationButtonComponent m_NavEnable;
	protected SCR_NavigationButtonComponent m_NavFavourite;
	protected SCR_NavigationButtonComponent m_NavUnsubscribe;
	
	// Scenario-related buttons
	protected SCR_NavigationButtonComponent m_NavPlay;
	protected SCR_NavigationButtonComponent m_NavJoin;
	protected SCR_NavigationButtonComponent m_NavHost;
	
	protected SCR_ContentBrowser_ScenarioLineComponent m_LastSelectedLine;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu) 
	{
		super.OnMenuOpen(parentMenu);
		
		this.InitWidgets();
		
		// Hide scenario and gallery section
		widgets.m_Gallery.SetVisible(false);
		widgets.m_ScenarioSection.SetVisible(false);

		// Init details panel
		widgets.m_AddonDetailsPanelComponent.SetForceShowVersion(true); // Show version regardless of addon being downloaded or not
				
		if (m_WorkshopItem)
		{
			if (SCR_WorkshopUiCommon.GetConnectionState())
			{
				m_WorkshopItem.m_OnGetAsset.Insert(Callback_OnGetAsset);
				m_WorkshopItem.m_OnChanged.Insert(Callback_OnItemChanged);
				m_WorkshopItem.m_OnScenariosLoaded.Insert(Callback_OnDownloadScenarios);
				m_WorkshopItem.m_OnTimeout.Insert(Callback_OnTimeout);
				m_WorkshopItem.LoadDetails();
			}
			else
			{
				Callback_OnDownloadScenarios();
			}
		}
		
		// Init event handlers last of all.
		// We don't want event handlers to get called while we are still initializing UI.
		InitWidgetEventHandlers();
		
		// Init the addon details panel
		widgets.m_AddonDetailsPanelComponent.SetWorkshopItem(m_WorkshopItem);
		
		// Add event to the huge background button so when we click on nothing, we lose focus.
		ContentBrowserDetailsMenu parent = ContentBrowserDetailsMenu.Cast(parentMenu);
		if (parent)
			parent.widgets.m_BackgroundButtonComponent.GetOnClick().Insert(OnBackgroundButtonClick);
		widgets.m_MainContentScrollComponent1.GetOnClick().Insert(OnBackgroundButtonClick);	// Scroll layout intercepts clicks too
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
		
		UpdateAllWidgets();
		
		// Set the default focused widget
		Widget defaultFocus = widgets.m_SubscribeButton;
		if (m_WorkshopItem.GetOffline())
			defaultFocus = widgets.m_EnableButton;
		
		GetGame().GetCallqueue().CallLater(GetGame().GetWorkspace().SetFocusedWidget, 0, false, defaultFocus, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		
		if (m_WorkshopItem)
		{
			m_WorkshopItem.m_OnGetAsset.Remove(Callback_OnGetAsset);
			m_WorkshopItem.m_OnChanged.Remove(Callback_OnItemChanged);
			m_WorkshopItem.m_OnScenariosLoaded.Remove(Callback_OnDownloadScenarios);
			m_WorkshopItem.m_OnTimeout.Remove(Callback_OnTimeout);
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void InitWidgets()
	{
		widgets = new SCR_ContentBrowserDetails_OverviewSubMenuWidgets;
		widgets.Init(m_wRoot);
		
		// Create nav buttons
		m_NavUnsubscribe = CreateNavigationButton("WorkshopUnsubscribe", "#AR-Workshop_ButtonDelete", true);
		m_NavFavourite = CreateNavigationButton("MenuFavourite", "#AR-Workshop_ButtonAddToFavourites", true);
		m_NavPrimary = CreateNavigationButton("WorkshopPrimary", "#AR-WorkshopButtonSubscribe", true);
		m_NavEnable = CreateNavigationButton("MenuEnable", "#AR-Workshop_ButtonEnable", true);
		
		m_NavPlay = CreateNavigationButton("MenuSelect", "#AR-Workshop_ButtonPlay", true);
		m_NavJoin = CreateNavigationButton("MenuJoin", "#AR-Workshop_ButtonJoin", true);
		if (SCR_ContentBrowser_ScenarioSubMenu.GetHostingAllowed())
			m_NavHost = CreateNavigationButton("MenuHost", "#AR-Workshop_ButtonHost", true);
		
		m_NavPlay.m_OnActivated.Insert(OnPlayButton);
		m_NavJoin.m_OnActivated.Insert(OnJoinButton);
		if (m_NavHost)
			m_NavHost.m_OnActivated.Insert(OnHostButton);
		
		// Hide verrsion selection in release
		#ifndef WORKSHOP_DEBUG
		widgets.m_VersionComboBoxComponent.SetVisible(false, false);
		#endif
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	protected void InitWidgetEventHandlers()
	{
		// Init callbacks
		widgets.m_VoteUpButtonComponent.m_OnToggled.Insert(OnVoteUpButton);			// Toggleable!
		widgets.m_VoteDownButtonComponent.m_OnToggled.Insert(OnVoteDownButton);		// Toggleable!
		widgets.m_EnableButtonComponent.m_OnToggled.Insert(OnEnableButtonToggled);	// Toggleable!
		widgets.m_ReportButtonComponent.m_OnClicked.Insert(OnReportButton);
		widgets.m_SubscribeButtonComponent.m_OnClicked.Insert(OnSubscribeButton);
		widgets.m_SolveIssuesButtonComponent.m_OnClicked.Insert(OnSolveIssuesButton);
		
		m_NavPrimary.m_OnActivated.Insert(OnPrimaryButton);
		m_NavEnable.m_OnActivated.Insert(OnNavEnableButton);
		m_NavFavourite.m_OnActivated.Insert(OnFavouriteButton);
		m_NavUnsubscribe.m_OnActivated.Insert(OnUnsubscribeNavButton);
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	
	///////////////////////////////////////////////////////////////////
	// Callbacks
	///////////////////////////////////////////////////////////////////
	
	
	//------------------------------------------------------------------------------------------------
	void Callback_OnGetAsset()
	{
		UpdateGallery();
	}
	
	
	//------------------------------------------------------------------------------------------------
	void Callback_OnItemChanged()
	{
		this.UpdateAllWidgets();
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Called when we receive scenarios from backend
	void Callback_OnDownloadScenarios()
	{
		array<MissionWorkshopItem> scenarios = new array<MissionWorkshopItem>;
		m_WorkshopItem.GetScenarios(scenarios);
		
		#ifdef WORKSHOP_DEBUG
		ContentBrowserUI._print(string.Format("Downloaded %1 scenarios", scenarios.Count()));
		#endif
		
		widgets.m_ScenarioSection.SetVisible(!scenarios.IsEmpty());
		
		// Delete old entries
		while(widgets.m_ScenariosList.GetChildren())
			widgets.m_ScenariosList.RemoveChild(widgets.m_ScenariosList.GetChildren());
		
		// Sort missions by name
		SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionName>.HeapSort(scenarios, false);
		
		// Create lines for scenarios
		if (!scenarios.IsEmpty())
		{
			foreach (MissionWorkshopItem scenario : scenarios)
			{
				Widget w = GetGame().GetWorkspace().CreateWidgets(SCR_ContentBrowser_ScenarioLineWidgets.s_sLayout, widgets.m_ScenariosList);
				SCR_ContentBrowser_ScenarioLineComponent comp = SCR_ContentBrowser_ScenarioLineComponent.Cast(w.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));
				comp.SetScenario(scenario);
				m_aScenarioLines.Insert(comp);
				
				comp.m_OnScenarioStateChanged.Insert(Callback_OnScenarioStateChanged);
				
				SCR_ModularButtonComponent buttonComp = SCR_ModularButtonComponent.Cast(w.FindHandler(SCR_ModularButtonComponent));
				
				buttonComp.m_OnFocus.Insert(OnLineFocus);
				buttonComp.m_OnFocusLost.Insert(OnLineFocusLost);
				buttonComp.m_OnMouseEnter.Insert(OnLineMouseEnter);
				buttonComp.m_OnMouseLeave.Insert(OnLineMouseLeave);
				buttonComp.m_OnDoubleClicked.Insert(OnLineDoubleClick);
			}
			
			UpdateScenarioLines();
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	void Callback_OnTimeout()
	{
		// Ignore if we are not connected to backend at all
		if (!SCR_WorkshopUiCommon.GetConnectionState())
			return;
		
		SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateTimeoutOkDialog();
		dlg.m_OnClose.Insert(Callback_OnTimeOut_OnCloseDialog);
	}
	void Callback_OnTimeOut_OnCloseDialog()
	{
		m_ParentMenu.CloseMenu();
	}
	
	
	///////////////////////////////////////////////////////////////////
	// Widget callbacks
	///////////////////////////////////////////////////////////////////
	
	//------------------------------------------------------------------------------------------------
	protected void OnSubscribeButton()
	{
		// Here we either subscribe or unsubscribe
		if (m_WorkshopItem.GetOffline())
		{
			SCR_WorkshopUiCommon.OnUnsubscribeAddonButton(m_WorkshopItem);
		}
		else
		{
			string specificVersion;
			
			#ifdef WORKSHOP_DEBUG
			specificVersion = widgets.m_VersionComboBoxComponent.GetCurrentItem();
			#endif
			
			if (!specificVersion.IsEmpty())
			{
				auto dlAction = m_WorkshopItem.Download(specificVersion);
				dlAction.Activate();
			}
			else
				m_DownloadRequest = SCR_WorkshopUiCommon_DownloadSequence.TryCreate(m_WorkshopItem, true, m_DownloadRequest);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSolveIssuesButton()
	{
		// This button does something only if there is a problem
		EWorkshopItemProblem problem = m_WorkshopItem.GetHighestPriorityProblem();
		
		if (problem == EWorkshopItemProblem.NO_PROBLEM)
			return;
		
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_WorkshopItem, m_DownloadRequest);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnUnsubscribeNavButton()
	{
		SCR_WorkshopUiCommon.OnUnsubscribeAddonButton(m_WorkshopItem);
	}

	
	//------------------------------------------------------------------------------------------------
	protected void OnPrimaryButton()
	{
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_WorkshopItem, m_DownloadRequest);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! By the time when this is called, the button has already been
	//! toggled by the widget library code
	protected void OnVoteUpButton()
	{
		bool newState = widgets.m_VoteUpButtonComponent.GetToggled();
		this.OnVoteButtons(newState, !newState);
	}
		
	
	//------------------------------------------------------------------------------------------------
	protected void OnVoteDownButton()
	{
		bool newState = widgets.m_VoteDownButtonComponent.GetToggled();
		this.OnVoteButtons(!newState, !newState);
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	// resetAll - resets the rating
	// voteUp - if not reset rating, vote up or down
	protected void OnVoteButtons(bool voteUp, bool resetAll)
	{
		SCR_ModularButtonComponent bup = widgets.m_VoteUpButtonComponent;
		SCR_ModularButtonComponent bdwn = widgets.m_VoteDownButtonComponent;
		
		bup.SetToggled(!resetAll && voteUp, false); // Don't invoke the OnToggled again!
		bdwn.SetToggled(!resetAll && !voteUp, false);
		
		if (resetAll)
		{
			m_WorkshopItem.ResetMyRating();
		}
		else
		{
			m_WorkshopItem.SetMyRating(voteUp);
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnEnableButtonToggled(SCR_ModularButtonComponent comp)
	{
		SCR_WorkshopUiCommon.OnEnableAddonToggleButton(m_WorkshopItem, comp);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnNavEnableButton()
	{
		SCR_WorkshopUiCommon.OnEnableAddonButton(m_WorkshopItem);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnFavouriteButton()
	{
		SCR_WorkshopItem item = m_WorkshopItem;
		MissionWorkshopItem scenario = GetSelectedScenario();
		
		if (scenario)
		{
			if (item.GetOffline())
			{
				scenario.SetFavorite(!scenario.IsFavorite());
				SCR_ContentBrowser_ScenarioLineComponent comp = GetSelectedLine();
				comp.NotifyScenarioUpdate();
				UpdateButtons();
			}
				
		}
		else
		{
			if (item.GetRestricted())
				return;
			
			bool fav = item.GetFavourite();
			item.SetFavourite(!fav);
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! This button used in reported and not reported state. It either sends a report or cancels it.
	protected void OnReportButton()
	{
		// Bail if not in backend
		if (!m_WorkshopItem.GetOnline())
			return;
	
		if (m_WorkshopItem.GetReportedByMe())
		{	
			if (m_LoadingOverlayDlg)
				m_LoadingOverlayDlg.Close();
			
			m_LoadingOverlayDlg = SCR_LoadingOverlayDialog.Create();
			m_LoadingOverlayDlg.m_OnCloseStarted.Insert(Callback_OnLoadMyReportCancel); // Called when user decides to cancel and not wait
			
			m_WorkshopItem.m_OnMyReportLoaded.Insert(Callback_OnLoadMyReportSuccess);
			m_WorkshopItem.m_OnTimeout.Insert(Callback_OnLoadMyReportTimeout);
			m_WorkshopItem.LoadReport();
		}
		else
		{
			// Not reported yet
			ReportDialogUI dialog = ReportDialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ReportItemDialog));
			if (dialog)
			{
				dialog.Init(m_DetailsMenu);
			}
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Called from scenario line component when scenario state changes
	protected void Callback_OnScenarioStateChanged(SCR_ContentBrowser_ScenarioLineComponent comp)
	{
		UpdateButtons();
		UpdateSidePanel();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnJoinButton()
	{
		MissionWorkshopItem scenario = GetSelectedScenario();
		
		if (!scenario || !m_WorkshopItem.GetOffline())
			return;
		
		ServerBrowserMenuUI.OpenWithScenarioFilter(scenario);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayButton()
	{
		MissionWorkshopItem scenario = GetSelectedScenario();
		
		if (!scenario || !m_WorkshopItem.GetOffline())
			return;
		
		SCR_WorkshopUiCommon.TryPlayScenario(scenario);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnHostButton()
	{
		MissionWorkshopItem scenario = GetSelectedScenario();
		
		if (!scenario || !m_WorkshopItem.GetOffline())
			return;

		SCR_WorkshopUiCommon.TryHostScenario(scenario);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBackgroundButtonClick()
	{
		GetGame().GetWorkspace().SetFocusedWidget(null);
	}
	
	
	//------------------------------------------------------------------------------------------------
	// Unreporting, loading report
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnLoadMyReportCancel()
	{
		m_WorkshopItem.m_OnMyReportLoaded.Remove(Callback_OnLoadMyReportSuccess);	// todo improve error handling :(
		m_WorkshopItem.m_OnTimeout.Remove(Callback_OnLoadMyReportTimeout);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnLoadMyReportTimeout()
	{
		m_WorkshopItem.m_OnMyReportLoaded.Remove(Callback_OnLoadMyReportSuccess);
		m_WorkshopItem.m_OnTimeout.Remove(Callback_OnLoadMyReportTimeout);
		
		m_LoadingOverlayDlg.CloseAnimated();
		SCR_CommonDialogs.CreateTimeoutOkDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnLoadMyReportSuccess()
	{
		m_WorkshopItem.m_OnMyReportLoaded.Remove(Callback_OnLoadMyReportSuccess);
		m_WorkshopItem.m_OnTimeout.Remove(Callback_OnLoadMyReportTimeout);
		
		m_LoadingOverlayDlg.CloseAnimated();
		
		new SCR_CancelMyReportDialog(m_WorkshopItem);
	}
	
	
	
	
	///////////////////////////////////////////////////////////////////
	// Methods for updating state of individual components
	///////////////////////////////////////////////////////////////////
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		UpdateVersionComboBox();
		UpdateButtons();
		UpdateAddonStaticWidgets();
		UpdateScenarioLines();
	}
	
	//------------------------------------------------------------------------------------------------
	//! updated widgets which depend on state of the mod (so everything except for description, gallery)
	protected void UpdateButtons()
	{
		if (!GetShown())
			return;
		
		SCR_WorkshopItem item = m_WorkshopItem;
		
		auto actionThisItem = item.GetDownloadAction();
		auto actionDependencies = item.GetDependencyCompositeAction();
		bool downloading = actionThisItem || actionDependencies;
		EWorkshopItemProblem problem = item.GetHighestPriorityProblem();
		
		// Label of primary action
		string primaryActionText = SCR_WorkshopUiCommon.GetPrimaryActionName(item);
		
		// Enable button
		bool enableButtonEnabled = item.GetOffline() && !SCR_AddonManager.GetAddonsEnabledExternally() && !downloading;
		widgets.m_EnableButton.SetEnabled(enableButtonEnabled);
		widgets.m_EnableButtonComponent.SetToggled(item.GetEnabled(), false);
		
		// NAV Enable button
		// Enable button - enabled only for downloaded addons
		m_NavEnable.SetEnabled(enableButtonEnabled);
		if (item.GetOffline())
		{
			string enableLabel;
			if (item.GetEnabled())
				enableLabel = "#AR-Workshop_ButtonDisable";
			else
				enableLabel = "#AR-Workshop_ButtonEnable";
			m_NavEnable.SetLabel(enableLabel);
		}
		
		// Report button
		widgets.m_ReportButtonComponent.SetEnabled(item.GetOnline());
		string reportButtonMode = "not_reported";
		if (item.GetReportedByMe())
			reportButtonMode = "reported";
		widgets.m_ReportButtonComponent.SetEffectsWithAnyTagEnabled({"all", reportButtonMode});
		
		
		// NAV Primary action button
		if (!primaryActionText.IsEmpty())
			m_NavPrimary.SetLabel(primaryActionText);
		m_NavPrimary.SetEnabled(!primaryActionText.IsEmpty());	
		
		
		// NAV Unsubscribe button
		// Enabled only for online addons
		m_NavUnsubscribe.SetEnabled(m_WorkshopItem.GetOnline() && m_WorkshopItem.GetOffline());		
		
		
		
		// Subscribe button
		bool subscribeButtonEnabled = false;
		string subscribeButtonMode = "not_subscribed";
		if (!downloading && item.GetOnline() && item.GetOffline())
		{
			// We are in a state when we want to unsubscribe this
			subscribeButtonEnabled = true;
			subscribeButtonMode = "subscribed";
		}
		else if (!downloading && item.GetOnline())
		{
			// We are in a state when we want to subscribe to this
			subscribeButtonEnabled = true;
		}
		
		widgets.m_SubscribeButtonComponent.SetEnabled(subscribeButtonEnabled);
		widgets.m_SubscribeButtonComponent.SetEffectsWithAnyTagEnabled({"all_modes", subscribeButtonMode}); // Enable only effects used in this mode or used in all modes
		
		// Solve issues button
		// Visible only when there are any issues
		bool solveIssuesVisible = problem != EWorkshopItemProblem.NO_PROBLEM;
		widgets.m_SolveIssuesButton.SetVisible(solveIssuesVisible);
		if (solveIssuesVisible)
		{
			SCR_ButtonEffectText textEffect = SCR_ButtonEffectText.Cast(widgets.m_SolveIssuesButtonComponent.FindEffect("text"));
			textEffect.m_sDefault = primaryActionText;
			textEffect.m_sHovered = primaryActionText;
			textEffect.PropertiesChanged();
		}
		
		// Vote up/down
		bool enabled = item.GetOnline();
		widgets.m_VoteUpButtonComponent.SetEnabled(enabled);
		widgets.m_VoteDownButtonComponent.SetEnabled(enabled);

		bool rating, ratingSet;
		item.GetMyRating(ratingSet, rating);
		
		bool voteUpToggled = ratingSet && rating;
		bool voteDownToggled = ratingSet && !rating;
		
		widgets.m_VoteUpButtonComponent.SetToggled(voteUpToggled, false);
		widgets.m_VoteDownButtonComponent.SetToggled(voteDownToggled, false);
		
		
		
		
		// Nav button visibility depends on what we have selected: scenario line or smth else.
		auto scenarioLine = GetSelectedLine();
		MissionWorkshopItem scenario = null;
		if (scenarioLine)
			scenario = scenarioLine.GetScenario();
		bool scenarioSelected = scenario != null;
		
		m_NavPlay.SetVisible(scenarioSelected);				// Scenario-related buttons
		if (m_NavHost)
			m_NavHost.SetVisible(scenarioSelected);
		m_NavJoin.SetVisible(scenarioSelected);
		
		m_NavPrimary.SetVisible(!scenarioSelected);			// Mod-related buttons
		m_NavEnable.SetVisible(!scenarioSelected);
		m_NavUnsubscribe.SetVisible(!scenarioSelected);
		
		// Update UI related to scenario
		if (scenario)
		{
			bool addonDownloaded = m_WorkshopItem.GetOffline();
			bool mp = scenario.GetPlayerCount() > 1;
			m_NavJoin.SetEnabled(addonDownloaded && mp);
			if (m_NavHost)
				m_NavHost.SetEnabled(addonDownloaded && mp);
			m_NavPlay.SetEnabled(addonDownloaded);
		}
		
		
		// Favourite button
		// We check favourite state either of scenario or of workshop item
		string favLabel;
		bool favourite = false; 
		if (scenarioSelected)
		{
			favourite = scenario.IsFavorite();
			m_NavFavourite.SetEnabled(m_WorkshopItem.GetOffline()); // We can only favourite a scenario which is downloaded
		}
		else
		{
			favourite = m_WorkshopItem.GetFavourite();
			m_NavFavourite.SetEnabled(true); // We can always set favourite an addon
		}
			
		if (!favourite)
			favLabel = "#AR-Workshop_ButtonAddToFavourites";
		else
			favLabel = "#AR-Workshop_ButtonRemoveFavourites";
		m_NavFavourite.SetLabel(favLabel);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Widgets for addon properties like description, name, which can not change
	protected void UpdateAddonStaticWidgets()
	{
		if (!m_WorkshopItem)
			return;
		
		// Name
		widgets.m_NameText.SetText(m_WorkshopItem.GetName());
		
		// Description
		string description = m_WorkshopItem.GetDescription();
		widgets.m_DescriptionText.SetText(description);
		
		// Author
		widgets.m_AuthorNameText.SetText(m_WorkshopItem.GetAuthorName());
		
		// Rating
		int ratingCount = m_WorkshopItem.GetRatingCount();
		float avgRating = m_WorkshopItem.GetAverageRating();
		avgRating = Math.Round(avgRating * 100.0);
		widgets.m_RatingText.SetText(string.Format("%1%% (%2x)", avgRating, ratingCount));
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateVersionComboBox()
	{
		#ifdef WORKSHOP_DEBUG
		// Fill version combo box - only for testing!
		if (widgets.m_VersionComboBoxComponent.GetNumItems() == 0)
		{
			array<string> versions = m_WorkshopItem.GetVersions();
			if (!versions.IsEmpty())
			{
				widgets.m_VersionComboBoxComponent.ClearAll();
				foreach (string version : versions)
				{
					widgets.m_VersionComboBoxComponent.AddItem(version);
				}
			}
		}
		#endif
	}
	

	
	//------------------------------------------------------------------------------------------------
	protected void UpdateGallery()
	{
		if (m_bGalleryUpdated)
			return;
		
		array<BackendImage> galleryImages = m_WorkshopItem.GetGallery();
		
		widgets.m_Gallery.SetVisible(!galleryImages.IsEmpty());
		
		if (!galleryImages.IsEmpty())
		{
			widgets.m_GalleryComponent.SetImages(galleryImages);
			m_bGalleryUpdated = true;
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateScenarioLines()
	{
		bool modDownloaded = m_WorkshopItem.GetOffline();
		foreach (SCR_ContentBrowser_ScenarioLineComponent scenarioLine : m_aScenarioLines)
		{
			if (scenarioLine)
			{
				scenarioLine.ShowFavouriteButton(modDownloaded); // Show fav button only when downloaded
				scenarioLine.ShowLastPlayedText(false);
				scenarioLine.NotifyScenarioUpdate();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSidePanel()
	{
		SCR_ContentBrowser_ScenarioLineComponent lineComp = GetSelectedLine();
		if (lineComp)
		{
			auto scenario = lineComp.GetScenario();
			if (scenario)
				widgets.m_ScenarioDetailsPanelComponent.SetScenario(scenario);
		}
		
		widgets.m_AddonDetailsPanel.SetVisible(!lineComp);
		widgets.m_ScenarioDetailsPanel.SetVisible(lineComp != null);
	}
	
	
	
	
	
	//------------------------------------------------------------------------------------------------
	// Code related to handling of scenario lines
	
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ContentBrowser_ScenarioLineComponent GetSelectedLine()
	{
		// We are not over a line, use currently focused line
		Widget wfocused = GetGame().GetWorkspace().GetFocusedWidget();
		SCR_ContentBrowser_ScenarioLineComponent comp;
		if (wfocused)
			comp = SCR_ContentBrowser_ScenarioLineComponent.Cast(wfocused.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));
		
		EInputDeviceType inputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
		
		if (inputDevice == EInputDeviceType.MOUSE)
		{
			if (GetLineUnderCursor())
			{
				// We are over a line, use either last focused line or
				// Last mouse-entered line
				return m_LastSelectedLine;
			}
			else
			{
				return comp;
			}
		}
		else
		{
			// If mouse is not used, ignore what is under cursor,
			// Return component of the focused widget
			return comp;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected MissionWorkshopItem GetSelectedScenario()
	{
		SCR_ContentBrowser_ScenarioLineComponent comp = GetSelectedLine();
		
		if (!comp)
			return null;
		
		return comp.GetScenario();
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ContentBrowser_ScenarioLineComponent GetLineUnderCursor()
	{
		Widget w = WidgetManager.GetWidgetUnderCursor();
		
		if (!w)
			return null;
		
		return SCR_ContentBrowser_ScenarioLineComponent.Cast(w.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineFocus(SCR_ModularButtonComponent buttonComp)
	{
		auto lineComp = SCR_ContentBrowser_ScenarioLineComponent.FindComponent(buttonComp.GetRootWidget());
		m_LastSelectedLine = lineComp;
		
		UpdateSidePanel();
		UpdateButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineFocusLost(SCR_ModularButtonComponent buttonComp)
	{
		UpdateSidePanel();
		UpdateButtons();
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineMouseEnter(SCR_ModularButtonComponent buttonComp, bool mouseInput)
	{
		// Bail if last input wasn't mouse
		if (!mouseInput)
			return;
		
		auto lineComp = SCR_ContentBrowser_ScenarioLineComponent.FindComponent(buttonComp.GetRootWidget());
		
		m_LastSelectedLine = lineComp;
		
		UpdateSidePanel();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineMouseLeave(SCR_ModularButtonComponent buttonComp, bool mouseInput)
	{
		// Bail if last input wasn't mouse
		if (!mouseInput)
			return;
		
		GetGame().GetCallqueue().CallLater(UpdateSidePanel, 0, false); // Hack because otherwise right now mouse is still over the previous widget for some reason
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineDoubleClick(SCR_ModularButtonComponent buttonComp)
	{
		// Is downloaded 
		if (!m_WorkshopItem || !m_WorkshopItem.GetOffline())
			return;
		
		// Get scenario mission
		auto lineComp = SCR_ContentBrowser_ScenarioLineComponent.FindComponent(buttonComp.GetRootWidget());
		
		MissionWorkshopItem scenario = lineComp.GetScenario();
		
		if (!scenario)
			return;
		
		SCR_WorkshopUiCommon.TryPlayScenario(scenario);
	}
};