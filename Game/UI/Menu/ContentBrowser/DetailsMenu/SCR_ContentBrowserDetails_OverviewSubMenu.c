/*
The overview tab of the details view
*/

class SCR_ContentBrowserDetails_OverviewSubMenu : SCR_ContentBrowserDetails_SubMenuBase
{
	[Attribute()]
	protected ref SCR_ContentBrowser_ColorScheme m_ColorScheme;

	protected ref SCR_ContentBrowserDetails_OverviewSubMenuWidgets widgets;

	protected ref SCR_WorkshopDownloadSequence m_DownloadRequest;

	protected SCR_LoadingOverlayDialog m_LoadingOverlayDlg;

	protected ref array<SCR_ContentBrowser_ScenarioLineComponent> m_aScenarioLines = {};

	protected bool m_bGalleryUpdated = false; // Set true when we first time show images in gallery.

	protected SCR_ContentBrowser_ScenarioLineComponent m_LastSelectedLine;

	// Other
	protected EInputDeviceType m_eLastInputType;

	protected MissionWorkshopItem m_SelectedScenario;
	protected ref SCR_MissionHeader m_Header;

	protected const string FAVORITE_LABEL_ADD = "#AR-Workshop_ButtonAddToFavourites";
	protected const string FAVORITE_LABEL_REMOVE = "#AR-Workshop_ButtonRemoveFavourites";
	protected const ResourceName TOOLTIP_MESSAGE_IMAGESET = "{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset";
	
	ref ScriptInvoker<bool, string> m_OnLineFavorite = new ScriptInvoker();

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
				m_WorkshopItem.m_OnDownloadComplete.Insert(Callback_OnDownloadComplete);
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
		
		// Listen for Actions
		SCR_MenuActionsComponent actionsComp = SCR_MenuActionsComponent.FindComponent(GetRootWidget());
		if(actionsComp)
			actionsComp.m_OnAction.Insert(OnActionTriggered);
	}


	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta)
	{
		super.OnMenuUpdate(parentMenu, tDelta);

		//! Update Tooltip actions
		EInputDeviceType inputDeviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		if (inputDeviceType == m_eLastInputType || !m_LastSelectedLine || !m_WorkshopItem)
			return;

		m_eLastInputType = inputDeviceType;

		SCR_BrowserHoverTooltipComponent hoverComp = SCR_BrowserHoverTooltipComponent.FindComponent(m_LastSelectedLine.GetRootWidget());
		if (hoverComp)
		{
			hoverComp.UpdateButtonAction("Play");
			hoverComp.UpdateButtonAction("Download");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadWorkshopDetails()
	{
		if (!m_WorkshopItem)
			return;

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
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();
		
		SCR_MenuActionsComponent actionsComp = SCR_MenuActionsComponent.FindComponent(GetRootWidget());
		if (actionsComp)
			actionsComp.ActivateActions();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();
		
		SCR_MenuActionsComponent actionsComp = SCR_MenuActionsComponent.FindComponent(GetRootWidget());
		if (actionsComp)
			actionsComp.DeactivateActions();
	}

	//------------------------------------------------------------------------------------------------
	protected void InitWidgets()
	{
		widgets = new SCR_ContentBrowserDetails_OverviewSubMenuWidgets;
		widgets.Init(m_wRoot);

		// Hide verrsion selection in release
		#ifndef WORKSHOP_DEBUG
		widgets.m_VersionComboBoxComponent.SetVisible(false, false);
		#endif
	}



	//------------------------------------------------------------------------------------------------
	protected void InitWidgetEventHandlers()
	{
		// Init callbacks
		widgets.m_VoteUpButtonComponent.m_OnToggled.Insert(OnVoteUpButton);						// Toggleable!
		widgets.m_VoteDownButtonComponent.m_OnToggled.Insert(OnVoteDownButton);					// Toggleable!
		widgets.m_EnableButtonComponent.m_OnToggled.Insert(OnEnableButtonToggled);				// Toggleable!
		widgets.m_FavoriteButtonComponent.m_OnToggled.Insert(OnAddonFavouriteButtonToggled);	// Toggleable!
		widgets.m_ReportButtonComponent.m_OnClicked.Insert(OnReportButton);
		widgets.m_SubscribeButtonComponent.m_OnClicked.Insert(OnSubscribeButton);
		widgets.m_SolveIssuesButtonComponent.m_OnClicked.Insert(OnSolveIssuesButton);
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
	void Callback_OnDownloadComplete()
	{
		//! TOOLTIP
		//! If a tooltip is visible, recreate it when the download completes. The new tooltip will contain the available line actions
		SCR_ContentBrowser_ScenarioLineComponent line = GetSelectedLine();

		if (!line)
			return;

		Widget lineRoot = line.GetRootWidget();
		SCR_BrowserHoverTooltipComponent hoverComp = SCR_BrowserHoverTooltipComponent.FindComponent(lineRoot);

		if (!hoverComp || !hoverComp.GetTooltip())
			return;

		OnTooltipShow(SCR_HoverDetectorComponent.FindComponent(lineRoot), lineRoot);
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
		while (widgets.m_ScenariosList.GetChildren())
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

				SCR_HoverDetectorComponent hoverComp = SCR_HoverDetectorComponent.FindComponent(w);
				hoverComp.m_OnHoverDetected.Insert(OnTooltipShow);
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
			Revision specificVersion;

			#ifdef WORKSHOP_DEBUG
			string strSpecificVersion = widgets.m_VersionComboBoxComponent.GetCurrentItem();
			specificVersion = m_WorkshopItem.FindRevision(strSpecificVersion);
			#endif

			if (specificVersion)
			{
				//SCR_WorkshopItemActionDownload dlAction = m_WorkshopItem.Download(specificVersion);
				//dlAction.Activate();
				
				m_DownloadRequest = SCR_WorkshopDownloadSequence.Create(m_WorkshopItem, specificVersion, m_DownloadRequest);
			}
			else
				m_DownloadRequest = SCR_WorkshopDownloadSequence.Create(m_WorkshopItem, m_WorkshopItem.GetLatestRevision(), m_DownloadRequest);
		}

		//! TOOLTIP
		// Starting the download from a scenario line causes a Tooltip to appear showing a status message
		if (!m_LastSelectedLine)
			return;
		
		Widget lineRoot = m_LastSelectedLine.GetRootWidget();
		OnTooltipShow(SCR_HoverDetectorComponent.FindComponent(lineRoot), lineRoot);
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
	//! Sets favorite state of the Addon
	protected void OnAddonFavouriteButtonToggled()
	{
		SCR_WorkshopItem item = m_WorkshopItem;
		bool fav;

		if (item.GetRestricted())
			return;

		fav = !item.GetFavourite();
		item.SetFavourite(fav);
	}


	protected SCR_ModReportDialogComponent m_ModReportDialog;

	//------------------------------------------------------------------------------------------------
	//! This button used in reported and not reported state. It either sends a report or cancels it.
	protected void OnReportButton()
	{
		// Bail if not in backend
		if (!m_WorkshopItem.GetOnline())
			return;

		// Individually reported mod - reported by me
		if (m_WorkshopItem.GetReportedByMe())
		{
			if (m_LoadingOverlayDlg)
				m_LoadingOverlayDlg.Close();

			m_LoadingOverlayDlg = SCR_LoadingOverlayDialog.Create();
			m_LoadingOverlayDlg.m_OnCloseStarted.Insert(Callback_OnLoadMyReportCancel); // Called when user decides to cancel and not wait

			m_WorkshopItem.m_OnMyReportLoaded.Insert(Callback_OnLoadMyReportSuccess);
			m_WorkshopItem.m_OnTimeout.Insert(Callback_OnLoadMyReportTimeout);
			m_WorkshopItem.LoadReport();

			return;
		}

		m_ModReportDialog = SCR_ModReportDialogComponent.Cast(m_wRoot.FindHandler(SCR_ModReportDialogComponent));

		// Mod with reported author
		if (m_WorkshopItem.GetModAuthorReportedByMe())
		{
			if (m_ModReportDialog)
			{
				m_ModReportDialog.SetItem(m_WorkshopItem);
				m_ModReportDialog.OnSelectReportAuthor();
				m_WorkshopItem.m_OnReportStateChanged.Insert(OnAuthorBlockStateChanged);
			}

			return;
		}

		// Not reported
		if (m_ModReportDialog)
		{
			m_ModReportDialog.OpenSelectReport(m_WorkshopItem);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAuthorBlockStateChanged()
	{
		LoadWorkshopDetails();
		m_WorkshopItem.m_OnReportStateChanged.Remove(OnAuthorBlockStateChanged);
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
		LineJoin(GetSelectedLine());
	}

	//------------------------------------------------------------------------------------------------
	protected void LineJoin(SCR_ContentBrowser_ScenarioLineComponent line)
	{
		if (!line)
			return;

		MissionWorkshopItem scenario = line.GetScenario();

		if (!scenario || !m_WorkshopItem || !m_WorkshopItem.GetOffline() || scenario.GetPlayerCount() <= 1)
			return;

		// Interrupt when server config is being edited
		// just temporary solution to prevent endless opening of menus
		if (ServerHostingUI.GetTemporaryConfig())
		{
			SCR_MenuToolsLib.GetEventOnAllMenuClosed().Insert(OnAllMenuCloseJoin);
			SCR_MenuToolsLib.CloseAllMenus({MainMenuUI, ServerBrowserMenuUI});
			return;
		}

		ServerBrowserMenuUI.OpenWithScenarioFilter(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAllMenuCloseJoin()
	{
		SCR_MenuToolsLib.GetEventOnAllMenuClosed().Remove(OnAllMenuCloseJoin);

		//GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu);
		if (!GetGame().IsPlatformGameConsole())
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ServerHostingDialog);
	}

	//------------------------------------------------------------------------------------------------
	void OnActionTriggered(string action, float multiplier)
	{
		switch(action)
		{
			case "MenuSelectDouble": OnLineClickInteraction(multiplier); break;
			case "MenuRestart": OnRestartButton(); break;
			case "MenuJoin": OnJoinButton(); break;
			case "MenuFavourite": OnLineFavouriteButton(); break;
			case "MenuHost":
			{
				if (!GetGame().IsPlatformGameConsole()) 
					OnHostButton(); 
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayInteraction(SCR_ContentBrowser_ScenarioLineComponent line)
	{
		if (!line)
			return;

		MissionWorkshopItem selectedMission = line.GetScenario();
		if (!selectedMission || !m_WorkshopItem || !m_WorkshopItem.GetOffline())
			return;

		bool canBeLoaded = m_Header && GetGame().GetSaveManager().HasLatestSave(m_Header);
		if (canBeLoaded)
			LineContinue(selectedMission);
		else
			LinePlay(selectedMission);
	}


	//------------------------------------------------------------------------------------------------
	protected void LinePlay(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		SCR_WorkshopUiCommon.TryPlayScenario(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void LineContinue(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		if (m_Header && !m_Header.GetSaveFileName().IsEmpty())
			GetGame().GetSaveManager().SetFileNameToLoad(m_Header);
		else
			GetGame().GetSaveManager().ResetFileNameToLoad();

		SCR_WorkshopUiCommon.TryPlayScenario(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHostButton()
	{
		LineHost(GetSelectedLine());
	}


	//------------------------------------------------------------------------------------------------
	protected void LineHost(SCR_ContentBrowser_ScenarioLineComponent line)
	{
		if (!line)
			return;

		MissionWorkshopItem scenario = line.GetScenario();
		if (!scenario || !m_WorkshopItem || !m_WorkshopItem.GetOffline())
			return;

		if (scenario.GetPlayerCount() < 2)
			return;

		// Open server hosting dialog
		ServerHostingUI dialog = ServerHostingUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ServerHostingDialog));

		dialog.SelectScenario(scenario);
	}


	//------------------------------------------------------------------------------------------------
	protected void OnRestartButton()
	{
		LineRestart(GetSelectedLine());
	}


	//------------------------------------------------------------------------------------------------
	void LineRestart(SCR_ContentBrowser_ScenarioLineComponent line)
	{
		if (!line)
			return;

		MissionWorkshopItem selectedMission = line.GetScenario();
		if (!selectedMission)
			return;

		m_SelectedScenario = selectedMission;
		m_Header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(selectedMission.Id()));
		bool canBeLoaded = m_Header && GetGame().GetSaveManager().HasLatestSave(m_Header);

		if (!canBeLoaded)
			return;

		GetGame().GetSaveManager().ResetFileNameToLoad();
		SCR_WorkshopUiCommon.TryPlayScenario(m_SelectedScenario);
	}


	//------------------------------------------------------------------------------------------------
	// Unreporting, loading report
	//------------------------------------------------------------------------------------------------
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



	//------------------------------------------------------------------------------------------------
	protected void OnAuthorModUnreport()
	{

	}



	//------------------------------------------------------------------------------------------------
	// Methods for updating state of individual components
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		UpdateVersionComboBox();
		UpdateButtons();
		UpdateAddonStaticWidgets();
		UpdateScenarioLines();

		//! TOOTLIP
		if (m_LastSelectedLine)
		{
			SCR_BrowserHoverTooltipComponent hoverComp = SCR_BrowserHoverTooltipComponent.FindComponent(m_LastSelectedLine.GetRootWidget());
			UpdateTooltipMessage(hoverComp);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! updated widgets which depend on state of the mod (so everything except for description, gallery)
	protected void UpdateButtons()
	{
		if (!GetShown())
			return;

		m_WorkshopItem = m_DetailsMenu.m_WorkshopItem;
		SCR_WorkshopItem item = m_WorkshopItem;

		SCR_WorkshopItemActionDownload actionThisItem = item.GetDownloadAction();
		SCR_WorkshopItemActionComposite actionDependencies = item.GetDependencyCompositeAction();
		bool downloading = actionThisItem || actionDependencies;
		EWorkshopItemProblem problem = item.GetHighestPriorityProblem();

		// Label of primary action
		string primaryActionText = SCR_WorkshopUiCommon.GetPrimaryActionName(item);

		// Enable button
		bool enableButtonEnabled = item.GetOffline() && !SCR_AddonManager.GetAddonsEnabledExternally() && !downloading;
		widgets.m_EnableButton.SetEnabled(enableButtonEnabled);
		widgets.m_EnableButtonComponent.SetToggled(item.GetEnabled(), false);

		bool reportIndividual = item.GetReportedByMe();
		bool reportAuthor = item.GetModAuthorReportedByMe();

		// Report button
		widgets.m_ReportButtonComponent.SetEnabled(item.GetOnline());
		string reportButtonMode = "not_reported";
		if (reportIndividual || reportAuthor)
			reportButtonMode = "reported";
		widgets.m_ReportButtonComponent.SetEffectsWithAnyTagEnabled({"all", reportButtonMode});

		//! Subscribe button
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

		//! Favourite button
		widgets.m_FavoriteButtonComponent.SetToggled(m_WorkshopItem.GetFavourite(), false);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetFavoriteLabel(bool isFavorite)
	{
		if (isFavorite)
			return FAVORITE_LABEL_REMOVE;
		else
			return FAVORITE_LABEL_ADD;
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
			MissionWorkshopItem scenario = lineComp.GetScenario();
			if (scenario)
				widgets.m_ScenarioDetailsPanelComponent.SetScenario(scenario);
		}

		widgets.m_AddonDetailsPanel.SetVisible(!lineComp);
		widgets.m_ScenarioDetailsPanel.SetVisible(lineComp != null);
	}


	//------------------------------------------------------------------------------------------------
	//! SCENARIO LINES
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
		SCR_ContentBrowser_ScenarioLineComponent lineComp = SCR_ContentBrowser_ScenarioLineComponent.FindComponent(buttonComp.GetRootWidget());
		m_LastSelectedLine = lineComp;

		UpdateSidePanel();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_HoverDetectorComponent baseHoverComp, Widget widget)
	{
		SCR_BrowserHoverTooltipComponent hoverComp = SCR_BrowserHoverTooltipComponent.FindComponent(widget);
		if (!hoverComp)
			return;
		
		//! Needed to update displayed tooltip actions on tick
		m_eLastInputType = GetGame().GetInputManager().GetLastUsedInputDevice();

		//! Clear Tooltip setup
		hoverComp.ClearSetupButtons();
		hoverComp.SetMessage("");

		//! Check if the addon has been downloaded
		if (!m_WorkshopItem || !m_WorkshopItem.GetOffline())
		{
			//! If not, check download state
			bool inProgress, paused;
			float progress;
			Revision revision;
			m_WorkshopItem.GetDownloadState(inProgress, paused, progress, revision);
			string size = SCR_ByteFormat.GetReadableSize(m_WorkshopItem.GetSizeBytes());

			//! ADDON DOWNLOADING IN PROGRESS
			if (inProgress)
			{
				UpdateTooltipMessage(hoverComp);
				hoverComp.CreateTooltip();
				return;
			}

			//! ADDON YET TO BE DOWNLOADED
			//Download button hint
			hoverComp.AddSetupButton("Download", "#AR-Workshop_Dialog_ConfirmDownload_ButtonDownload", "MenuSelect", "MenuEntryDoubleClickMouse");
			UpdateTooltipMessage(hoverComp);
			hoverComp.CreateTooltip();
			return;
		}

		//! ADDON DOWNLOADED
		//! Show scenario button hints if the addon has been downloaded
		//! Update selected scenario
		if(!m_LastSelectedLine)
			return;
		
		MissionWorkshopItem selectedMission = m_LastSelectedLine.GetScenario();
		if (selectedMission && m_SelectedScenario != selectedMission)
		{
			m_SelectedScenario = selectedMission;
			m_Header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(selectedMission.Id()));
		}

		bool canBeLoaded = m_Header && GetGame().GetSaveManager().HasLatestSave(m_Header);

		//! Favorites button label & multiplayer check
		bool mp;
		string favLabel = "#AR-ServerBrowser_Favorite";
		if (selectedMission)
		{
			mp = selectedMission.GetPlayerCount() > 1;
			//favLabel = GetFavoriteLabel(selectedMission.IsFavorite());
		}

		//! Favorite
		hoverComp.AddSetupButton("Favorite", favLabel, "MenuFavourite");

		//! Join
		if (mp)
			hoverComp.AddSetupButton("Join", "#AR-Workshop_ButtonFindServers", "MenuJoin");

		//! Host
		if (!GetGame().IsPlatformGameConsole() && mp)
			hoverComp.AddSetupButton("Host", "#AR-Workshop_ButtonHost", "MenuHost");

		//! Restart, Continue, Play
		string playLabel = "#AR-Workshop_ButtonPlay";
		if (canBeLoaded)
		{
			hoverComp.AddSetupButton("Restart", "#AR-PauseMenu_Restart", "MenuRestart");
			playLabel = "#AR-PauseMenu_Continue";
		}

		hoverComp.AddSetupButton("Play", playLabel, "MenuSelect", "MenuEntryDoubleClickMouse");
		
		hoverComp.CreateTooltip();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineFocusLost(SCR_ModularButtonComponent buttonComp)
	{
		UpdateSidePanel();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineClickInteraction(float multiplier)
	{
		//! multiplier value in the action is used to differentiate between single and double click

		SCR_ContentBrowser_ScenarioLineComponent lineComp = GetSelectedLine();
		if (!lineComp)
			return;

		EInputDeviceType lastInputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
		if(lastInputDevice == EInputDeviceType.MOUSE && lineComp != GetLineUnderCursor())
			return;
		
		switch (Math.Floor(multiplier))
		{
			case 1: OnLineClick(lineComp); break;
			case 2: OnLineDoubleClick(lineComp); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineClick(SCR_ContentBrowser_ScenarioLineComponent lineComp)
	{
		if (!lineComp)
			return;

		EInputDeviceType lastUsedInputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();

		//! Handle interaction with addon not downloaded
		if (!m_WorkshopItem || !m_WorkshopItem.GetOffline())
		{
			//! Single click does nothing
			if (lastUsedInputDevice == EInputDeviceType.MOUSE)
				return;
			else
				MissingAddonLineInteraction(lineComp);

			return;
		}

		//! Handle interaction with addon dowloaded
		//! CONFIRMATION DIALOG
		MissionWorkshopItem scenario = lineComp.GetScenario();
		if (!scenario || !lineComp)
			return;

		//! If using Mouse single click opens confirmation dialog, double click goes straight to the play interaction
		if (lastUsedInputDevice == EInputDeviceType.MOUSE)
		{
			SCR_ScenarioConfirmationDialogUi scenarioConfirmationDialog = SCR_ScenarioDialogs.CreateScenarioConfirmationDialog(lineComp, m_OnLineFavorite);
			if (!scenarioConfirmationDialog)
			{
				OnPlayInteraction(lineComp);
				return;
			}

			//! Bind dialog delegates
			scenarioConfirmationDialog.m_OnButtonPressed.Insert(OnConfirmationDialogButtonPressed);
			scenarioConfirmationDialog.m_OnFavorite.Insert(LineSetFavourite);
		}
		else //! If using Gamepad or Keyboard there's no confirmation dialog and single click starts the play interaction
		{
			OnPlayInteraction(lineComp);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnConfirmationDialogButtonPressed(SCR_ScenarioConfirmationDialogUi dialog, string tag)
	{
		SCR_ContentBrowser_ScenarioLineComponent line = dialog.GetLine();
		if (!line)
			return;

		switch (tag)
		{
			case "confirm": OnPlayInteraction(line); break;
			case "restart": LineRestart(line); break;
			case "join": 	LineJoin(line); break;
			case "host": 	LineHost(line); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineDoubleClick(SCR_ContentBrowser_ScenarioLineComponent lineComp)
	{
		if (!lineComp)
			return;

		//! Handle interaction with addon not downloaded
		if (!m_WorkshopItem || !m_WorkshopItem.GetOffline())
		{
			MissingAddonLineInteraction(lineComp);
			return;
		}

		//! Handle interaction with addon dowloaded
		OnPlayInteraction(lineComp);
	}


	//------------------------------------------------------------------------------------------------
	void MissingAddonLineInteraction(SCR_ContentBrowser_ScenarioLineComponent line)
	{
		// Check if the addon is downloaded
		if (!line || (m_WorkshopItem && m_WorkshopItem.GetOffline()))
			return;

		//! START DOWNLOAD
		// Clicking a line without the addon installed starts the download
		bool inProgress, paused;
		float progress;
		Revision revision;
		m_WorkshopItem.GetDownloadState(inProgress, paused, progress, revision);

		if (!inProgress && !paused)
			OnSubscribeButton();
	}


	//------------------------------------------------------------------------------------------------
	protected void OnLineFavouriteButton()
	{
		SCR_ContentBrowser_ScenarioLineComponent line = GetSelectedLine();
		LineSetFavourite(line);
	}


	//------------------------------------------------------------------------------------------------
	//! Sets favorite state of the Scenario line
	protected void LineSetFavourite(SCR_ContentBrowser_ScenarioLineComponent line)
	{
		if (!line)
			return;

		SCR_WorkshopItem item = m_WorkshopItem;
		MissionWorkshopItem scenario = line.GetScenario();

		if (!scenario || !item.GetOffline())
			return;

		bool isFavorite = !scenario.IsFavorite();
		scenario.SetFavorite(isFavorite);
		line.NotifyScenarioUpdate();

		m_OnLineFavorite.Invoke(isFavorite, GetFavoriteLabel(isFavorite));
	}


	//------------------------------------------------------------------------------------------------
	void UpdateTooltipMessage(SCR_BrowserHoverTooltipComponent hoverComp)
	{	
		if (!m_WorkshopItem || !hoverComp)
			return;

		// If not, check download state
		bool inProgress, paused;
		float progress;
		Revision revision;
		m_WorkshopItem.GetDownloadState(inProgress, paused, progress, revision);
		string size = SCR_ByteFormat.GetReadableSize(m_WorkshopItem.GetSizeBytes());
		string progressText = WidgetManager.Translate("#AR-ValueUnit_Percentage", Math.Ceil(progress * 100.0));
		
		string message, bracketsMessage, icon, formattedIcon;

		if (inProgress)
		{
			message = "#AR-Workshop_ButtonDownloading ";
			bracketsMessage = progressText;
			icon = "downloading";
		}

		if (paused)
		{
			message = "#AR-DownloadManager_State_AllDownloadsPaused ";
			bracketsMessage = progressText;
			icon = "download-pause";
		}

		if (!inProgress && !paused)
		{
			//! Addon needs to be downloaded
			if(!m_WorkshopItem.GetOffline())
			{
				bracketsMessage = size;
				icon = "download";
			}
		}

		//! Addon is downloaded and ready, no need for message
		if (message.IsEmpty() && icon.IsEmpty() && bracketsMessage.IsEmpty())
		{
			hoverComp.SetMessage("");
			return;
		}
		
		//! Compose message
		if (!icon.IsEmpty())
			formattedIcon = string.Format("<image set='%1' name='%2' scale='1.75'/>", TOOLTIP_MESSAGE_IMAGESET, icon);
		
		hoverComp.SetMessage(string.Format("%1 [%2%3 ]", message, formattedIcon, bracketsMessage));
	}
};
