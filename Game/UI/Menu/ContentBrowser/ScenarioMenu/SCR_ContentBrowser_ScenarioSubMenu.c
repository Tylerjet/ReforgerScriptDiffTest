enum EScenarioSubMenuMode
{
	MODE_ALL,		// Show all scenarios
	MODE_FAVOURITE,	// Show only favourite scenarios
	MODE_RECENT		// Show only recently played scenarios
};


class SCR_ContentBrowser_ScenarioSubMenu : SCR_SubMenuBase
{
	// Attributes
	
	// This is quite universal and can work in many modes...
	[Attribute("0", UIWidgets.ComboBox, "Mode in which this submenu must work.", "", ParamEnumArray.FromEnum(EScenarioSubMenuMode) )]
	EScenarioSubMenuMode  m_eMode;
	
	// Constants
	
	protected const string MISSIONS_FOLDER = "Missions";
	protected const int RECENTLY_PLAYED_MAX_ENTRIES = 10; // How many recently played missions to show in the recently played tab
	
	// Message tags
	// Those which end with '2' should be used when no content is found due to filters.
	protected const string MESSAGE_TAG_NOTHING_FOUND =		"nothing_found";
	protected const string MESSAGE_TAG_NOTHING_FOUND_2 =	"nothing_found2";
	protected const string MESSAGE_TAG_NOTHING_FAVOURITE =	"nothing_favourite";
	protected const string MESSAGE_TAG_NOTHING_FAVOURITE_2 ="nothing_favourite2";
	protected const string MESSAGE_TAG_NOTHING_RECENT =		"nothing_recent";
	protected const string MESSAGE_TAG_NOTHING_RECENT_2 =	"nothing_recent2";
	
	// Other
	EInputDeviceType m_eLastInputType;
	protected int m_iWidgetNameNextId = 0; // For unique widget name generation for setting explicit navigation rules.
	protected WorkshopApi m_WorkshopApi;
	protected ref SCR_ContentBrowser_ScenarioSubMenuWidgets m_Widgets = new SCR_ContentBrowser_ScenarioSubMenuWidgets;
	protected SCR_ContentBrowser_ScenarioLineComponent m_LastSelectedLine;
	protected Widget m_wBeforeSort;
	protected MissionWorkshopItem m_SelectedScenario;
	protected ref SCR_MissionHeader m_Header;
	
	// Nav buttons
	protected SCR_NavigationButtonComponent m_NavFilter;
	protected SCR_NavigationButtonComponent m_NavSorting;
	
	protected const string FAVORITE_LABEL_ADD = "#AR-Workshop_ButtonAddToFavourites";
	protected const string FAVORITE_LABEL_REMOVE = "#AR-Workshop_ButtonRemoveFavourites";

	ref ScriptInvoker<bool, string> m_OnFavorite = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu) 
	{	
		super.OnMenuOpen(parentMenu);
		
		m_WorkshopApi = GetGame().GetBackendApi().GetWorkshop();
		
		this.InitWidgets();

		// Try to restore filters
		m_Widgets.m_FilterPanelComponent.TryLoad();
				
		m_WorkshopApi = GetGame().GetBackendApi().GetWorkshop();
		this.InitWorkshopApi();
		
		UpdateScenarioList(true);
		
		// Listen for Actions
		SCR_MenuActionsComponent actionsComp = SCR_MenuActionsComponent.FindComponent(GetRootWidget());
		if(actionsComp)
			actionsComp.m_OnAction.Insert(OnActionTriggered);
		
		// Left footer buttons
		//m_NavFilter = CreateNavigationButton("MenuFilter", "#AR-Workshop_Filter", false);
		m_NavSorting = CreateNavigationButton("MenuFilter", "#AR-ScenarioBrowser_ButtonSorting", false);
		
		//m_NavFilter.m_OnActivated.Insert(OnFilterButton);
		m_NavSorting.m_OnActivated.Insert(OnSortingButton);
	}

	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
		
		// Init workshop API
		// We do it on tab show becasue this tab and others persists when all other tabs are closed,
		// But we can switch back to it later, and we must setup the workshop api again
		this.InitWorkshopApi();
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu) 
	{
		super.OnMenuHide(parentMenu);
		
		// Save configuration of filters
		m_Widgets.m_FilterPanelComponent.Save();
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta) 
	{
		
		super.OnMenuUpdate(parentMenu, tDelta);
		
		UpdateNavigationButtons();
		
		//! Set current scenario in the info panel
		m_Widgets.m_ScenarioDetailsPanelComponent.SetScenario(GetSelectedScenario());
		
		//! Update selected scenario
		MissionWorkshopItem selectedMission = GetSelectedScenario();
		if (selectedMission && m_SelectedScenario != selectedMission)
		{
			m_SelectedScenario = selectedMission;
			m_Header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(selectedMission.Id()));
		}
		
		//! Update Tooltip actions
		EInputDeviceType inputDeviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		if (inputDeviceType == m_eLastInputType || !m_LastSelectedLine)
			return;
		
		m_eLastInputType = inputDeviceType;
		
		SCR_BrowserHoverTooltipComponent hoverComp = SCR_BrowserHoverTooltipComponent.FindComponent(m_LastSelectedLine.GetRootWidget());
		if(hoverComp)
			hoverComp.UpdateButtonAction("Play");
		
		//! Needed to update displayed tooltip actions on tick
		m_eLastInputType = GetGame().GetInputManager().GetLastUsedInputDevice();
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
	protected void UpdateNavigationButtons()
	{		
		// Sorting button is hidden if we are using only mouse
		// It makes no sense to show it for mouse user because focus on sorting header is
		// Not visualized when we use mouse. The orange frame is only shown for KB & Gamepad.
		EInputDeviceType deviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		m_NavSorting.SetVisible(deviceType != EInputDeviceType.MOUSE, false);
	}

	
	//------------------------------------------------------------------------------------------------
	protected void InitWidgets()
	{
		// We provide scenarioSubMenuRoot as root because the widgets of the layours were exported starting from scenarioSubMenuRoot
		m_Widgets.Init(m_wRoot.FindWidget("scenarioSubMenuRoot"));
		
		m_Widgets.m_FilterPanelComponent.GetEditBoxSearch().m_OnConfirm.Insert(OnSearchConfirm);
		m_Widgets.m_FilterPanelComponent.m_OnFilterPanelToggled.Insert(OnFilterPanelToggled);
		
		m_Widgets.m_SortingHeaderComponent.m_OnChanged.Insert(OnSortingHeaderChange);
	}
	
	
	//------------------------------------------------------------------------------------------------
	// Inits workshop API according to current mode
	protected void InitWorkshopApi()
	{		
		// Scan offline items if needed
		if (m_WorkshopApi.NeedScan())
			m_WorkshopApi.ScanOfflineItems();
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Requests missions from API and shows them in the list
	protected void UpdateScenarioList(bool setNewFocus)
	{
		// Get missions from Workshop API
		array<MissionWorkshopItem> missionItemsAll = new array<MissionWorkshopItem>;
		m_WorkshopApi.GetPageScenarios(missionItemsAll, 0, 4096); // Get all missions at once
		
		// Remove scenarios from disabled addons
		for (int i = missionItemsAll.Count() - 1; i >= 0; i--)
		{
			MissionWorkshopItem m = missionItemsAll[i];
			WorkshopItem addon = m.GetOwner();
			
			if (!addon)
				continue;
			
			if (!addon.IsEnabled())
				missionItemsAll.Remove(i);
		}
		
		// Filter items according to current tab mode
		array<MissionWorkshopItem> missionItemsTabFiltered = {};
		switch (m_eMode)
		{
			// Select only fav. missions
			case EScenarioSubMenuMode.MODE_FAVOURITE:
			{
				foreach (MissionWorkshopItem m : missionItemsAll)
					if (m.IsFavorite())
						missionItemsTabFiltered.Insert(m);
			
				// Bail if there are no favourite missions
				if (missionItemsTabFiltered.IsEmpty())
				{
					ScenarioList_ClearMissionEntries();
					SetPanelsMode(true, MESSAGE_TAG_NOTHING_FAVOURITE);
					return;
				}
				
				break;
			}
				
			// Select only recently played missions
			case EScenarioSubMenuMode.MODE_RECENT:
			{
				// Sort missions by time since last play, select top latest N entries
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionTimeSinceLastPlay>.HeapSort(missionItemsAll, false);
				
				int i = 0;
				while (missionItemsTabFiltered.Count() < RECENTLY_PLAYED_MAX_ENTRIES && i < missionItemsAll.Count())
				{
					int dt = missionItemsAll[i].GetTimeSinceLastPlay();
					if (dt != -1)
						missionItemsTabFiltered.Insert(missionItemsAll[i]);
					i++;
				}
				
				// Bail if no recent missions
				if (missionItemsTabFiltered.IsEmpty())
				{
					ScenarioList_ClearMissionEntries();
					SetPanelsMode(true, MESSAGE_TAG_NOTHING_RECENT);
					return;
				}
				
				break;
			}
				
			// Select all missions
			default:
			{
				if (missionItemsAll.IsEmpty())
				{
					ScenarioList_ClearMissionEntries();
					SetPanelsMode(true, MESSAGE_TAG_NOTHING_FOUND);
					return;
				}
				
				missionItemsTabFiltered = missionItemsAll;
				break;
			}
		}
		
		
		// Filter scenarios based on search string
		string searchStr = m_Widgets.m_FilterPanelComponent.GetEditBoxSearch().GetValue();
		array<MissionWorkshopItem> missionItemsSearched = SearchScenarios(missionItemsTabFiltered, searchStr);
		
		// Bail if based on search there is no content
		if (missionItemsSearched.IsEmpty())
		{
			ScenarioList_ClearMissionEntries();
			
			// The message depends on current tab
			string messageTag;
			switch (m_eMode)
			{
				case EScenarioSubMenuMode.MODE_FAVOURITE:
					messageTag = MESSAGE_TAG_NOTHING_FAVOURITE_2;
					break;
					
				case EScenarioSubMenuMode.MODE_RECENT:
					messageTag = MESSAGE_TAG_NOTHING_RECENT_2;
					break;
					
				default:
					messageTag = MESSAGE_TAG_NOTHING_FOUND_2;
					break;
			}
			SetPanelsMode(false, messageTag);
			
			return;
		}
		
		
		// At this point we are sure that there are some missions found
		
		
		// Choose sorting based on selected filter
		array<MissionWorkshopItem> missionItems = missionItemsSearched;
		ESortOrder eSortOrder = m_Widgets.m_SortingHeaderComponent.GetSortOrder();
		string currentSortingItem = m_Widgets.m_SortingHeaderComponent.GetSortElementName();
		
		bool sortOrder = false;
		if (eSortOrder == ESortOrder.DESCENDING)
			sortOrder = true;
		
		// Apply sorting of headers if required
		if (currentSortingItem.IsEmpty())
		{
			currentSortingItem = "name";
			sortOrder = false;
		}
		switch (currentSortingItem)
		{
			case "name":
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionName>.HeapSort(missionItems, sortOrder);
				break;
			case "player_count": // sort by player count
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionPlayerCount>.HeapSort(missionItems, sortOrder);
				break;
			case "favourite":
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionFavourite>.HeapSort(missionItems, sortOrder);
				break;
			case "last_played":
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionTimeSinceLastPlay>.HeapSort(missionItems, sortOrder);
				break;
			case "source":
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionAddonName>.HeapSort(missionItems, sortOrder);
				break;
		}
		
		SetPanelsMode(false);
		ScenarioList_CreateMissionEntries(missionItems, setNewFocus);
	}
	
	
	//------------------------------------------------------------------------------------------------
	void SetPanelsMode(bool showEmptyPanel, string messagePresetTag = string.Empty)
	{
		// Show either main+filter panel or empty panel
		m_Widgets.m_EmptyPanel.SetVisible(showEmptyPanel);
		m_Widgets.m_FilterPanel.SetVisible(!showEmptyPanel);
		m_Widgets.m_LeftPanel.SetVisible(!showEmptyPanel);
		
		// Show message or hide it
		m_Widgets.m_EmptyPanelMessage.SetVisible(!messagePresetTag.IsEmpty());
		m_Widgets.m_MainPanelMessage.SetVisible(!messagePresetTag.IsEmpty());
		
		// Set message based on tag
		if (!messagePresetTag.IsEmpty())
		{
			if (showEmptyPanel)
				m_Widgets.m_EmptyPanelMessageComponent.SetContentFromPreset(messagePresetTag);
			else
				m_Widgets.m_MainPanelMessageComponent.SetContentFromPreset(messagePresetTag);
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	array<MissionWorkshopItem> SearchScenarios(array<MissionWorkshopItem> scenarios, string searchStr)
	{
		array<MissionWorkshopItem> scenariosOut = {};
		
		if (!searchStr.IsEmpty())
		{
			string searchStrLower = searchStr;
			searchStrLower.ToLower();
			foreach (MissionWorkshopItem scenario : scenarios)
			{
				// Name
				if (SearchStringLocalized(scenario.Name(), searchStrLower))
				{
					scenariosOut.Insert(scenario);
					continue;
				}
				
				// Description
				if (SearchStringLocalized(scenario.Description(), searchStrLower))
				{
					scenariosOut.Insert(scenario);
					continue;
				}
			}
		}
		else
		{
			scenariosOut.Copy(scenarios);
		}
		
		// todo	
		return scenariosOut;
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected static bool SearchStringLocalized(string str, string searchStrLower)
	{
		if (str.StartsWith("#"))
		{
			// Check translated lowercased string
			string strTranslated = WidgetManager.Translate(str);
			if (!strTranslated.IsEmpty())
			{
				string strTranslatedLower = strTranslated;
				strTranslatedLower.ToLower();
				
				if (strTranslatedLower.Contains(searchStrLower))
					return true;
			}
		}
		else
		{
			// Check lowercased string
			string strLower = str;
			strLower.ToLower();
			
			if (strLower.Contains(searchStrLower))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ScenarioList_ClearMissionEntries()
	{
		// Delete previous entries in the list
		Widget child = m_Widgets.m_ScenarioList.GetChildren();
		while (child) 
		{
			Widget nextChild = child.GetSibling();
			m_Widgets.m_ScenarioList.RemoveChild(child);
			child = nextChild;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates lines for missions in the scroll view
	protected void ScenarioList_CreateMissionEntries(array<MissionWorkshopItem> scenarios, bool setNewFocus)
	{
		ScenarioList_ClearMissionEntries();
		
		// Create new entries
		foreach (MissionWorkshopItem scenario : scenarios)
		{
			Widget widget = GetGame().GetWorkspace().CreateWidgets(SCR_ContentBrowser_ScenarioLineWidgets.s_sLayout, m_Widgets.m_ScenarioList);
			
			if (widget)
			{
				SCR_ContentBrowser_ScenarioLineComponent comp = SCR_ContentBrowser_ScenarioLineComponent.Cast(widget.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));
				if (comp)
				{
					comp.SetScenario(scenario);
					
					comp.m_OnScenarioStateChanged.Insert(Callback_OnScenarioStateChanged);
					
					SCR_ModularButtonComponent buttonComp = SCR_ModularButtonComponent.FindComponent(widget);
					buttonComp.m_OnFocus.Insert(OnLineFocus);
					
					SCR_HoverDetectorComponent hoverComp = SCR_HoverDetectorComponent.FindComponent(widget);
					if(hoverComp)
						hoverComp.m_OnHoverDetected.Insert(OnTooltipShow);
				}
			}
		}
		
		// Set focus on first line
		Widget firstLine = m_Widgets.m_ScenarioList.GetChildren();
		if (firstLine && setNewFocus)
		{
			// If this is called instantly, it doesn't set new focus when menu is first opened.
			// Probably because menu opening is handled in event handler of main menu's tile.
			GetGame().GetCallqueue().CallLater(GetGame().GetWorkspace().SetFocusedWidget, 0, false, firstLine, false);
		}
		
		// Reset scroll position
		m_Widgets.m_ScenarioScroll.SetSliderPos(0, 0, true);
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
	protected SCR_ContentBrowser_ScenarioLineComponent GetLineUnderCursor()
	{
		Widget w = WidgetManager.GetWidgetUnderCursor();
		
		if (!w)
			return null;
		
		return SCR_ContentBrowser_ScenarioLineComponent.Cast(w.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));
	}

	
	// -------------- Button event handlers ------------------------
	//------------------------------------------------------------------------------------------------
	protected void OnLineFocus(SCR_ModularButtonComponent buttonComp)
	{
		SCR_ContentBrowser_ScenarioLineComponent lineComp = SCR_ContentBrowser_ScenarioLineComponent.FindComponent(buttonComp.GetRootWidget());

		m_LastSelectedLine = lineComp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_HoverDetectorComponent baseHoverComp, Widget widget)
	{
		SCR_BrowserHoverTooltipComponent hoverComp = SCR_BrowserHoverTooltipComponent.FindComponent(widget);
		if (!hoverComp)
			return;
		
		hoverComp.ClearSetupButtons();
		
		//! Setup Tooltip buttons
		MissionWorkshopItem selectedMission = m_LastSelectedLine.GetScenario();
		bool canBeLoaded = m_Header && GetGame().GetSaveManager().HasLatestSave(m_Header);
		
		// Favorite
		hoverComp.AddSetupButton("Favorite", "#AR-ServerBrowser_Favorite", "MenuFavourite");
		
		if (selectedMission && selectedMission.GetPlayerCount() > 1 )
		{
			// Join
			hoverComp.AddSetupButton("Join", "#AR-Workshop_ButtonFindServers", "MenuJoin");
	
			// Host
			if (!GetGame().IsPlatformGameConsole())
				hoverComp.AddSetupButton("Host", "#AR-Workshop_ButtonHost", "MenuHost");
		}
		
		//Restart, Continue, Play
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
	protected void OnLineDoubleClick(SCR_ContentBrowser_ScenarioLineComponent lineComp)
	{
		if(!lineComp)
			return;
		
		MissionWorkshopItem scenario = lineComp.GetScenario();
		if (!scenario)
			return;
		
		SCR_BrowserHoverTooltipComponent hoverComp = SCR_BrowserHoverTooltipComponent.FindComponent(lineComp.GetRootWidget());
		if(hoverComp)
			hoverComp.ForceDeleteTooltip();
		
		OnPlayInteraction(lineComp);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineClick(SCR_ContentBrowser_ScenarioLineComponent lineComp)
	{
		if(!lineComp)
			return;
		
		MissionWorkshopItem scenario = lineComp.GetScenario();
		if (!scenario)
			return;
		
		SCR_BrowserHoverTooltipComponent hoverComp = SCR_BrowserHoverTooltipComponent.FindComponent(lineComp.GetRootWidget());
		if(hoverComp)
			hoverComp.ForceDeleteTooltip();
		
		//! If using Mouse single click opens confirmation dialog, double click goes straight to the play interaction
		if(GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
		{
			SCR_ScenarioConfirmationDialogUi scenarioConfirmationDialog = SCR_ScenarioDialogs.CreateScenarioConfirmationDialog(lineComp, m_OnFavorite);
			if(!scenarioConfirmationDialog)
			{
				OnPlayInteraction(lineComp);
				return;
			}
			
			//! Bind dialog delegates
			scenarioConfirmationDialog.m_OnButtonPressed.Insert(OnConfirmationDialogButtonPressed);
			scenarioConfirmationDialog.m_OnFavorite.Insert(SetFavorite);
		}
		//! If using Gamepad or Keyboard there's no confirmation dialog and single click starts the play interaction
		else
			OnPlayInteraction(lineComp);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnConfirmationDialogButtonPressed(SCR_ScenarioConfirmationDialogUi dialog, string tag)
	{
		SCR_ContentBrowser_ScenarioLineComponent line = dialog.GetLine();
		if(!line)
			return;

		switch (tag)
		{
			case "confirm": OnPlayInteraction(line); break;
			case "restart": Restart(line); break;
			case "join": 	Join(line); break;
			case "host": 	Host(line); break;
			default: break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActionTriggered(string action, float multiplier)
	{
		switch(action)
		{
			case "MenuSelectDouble": OnLineClickInteraction(multiplier); break;
			case "MenuRestart": OnRestartButton(); break;
			case "MenuJoin": OnJoinButton(); break;
			case "MenuFavourite": OnFavouriteButton(); break;
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
		if(!line)
			return;
		
		MissionWorkshopItem selectedMission = line.GetScenario();
		if (selectedMission)
		{
			bool canBeLoaded = m_Header && GetGame().GetSaveManager().HasLatestSave(m_Header);
			if(canBeLoaded)
				Continue(selectedMission);
			else
				Play(selectedMission);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnJoinButton()
	{
		Join(GetSelectedLine());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Join(SCR_ContentBrowser_ScenarioLineComponent line)
	{	
		if(!line)
			return;
		
		MissionWorkshopItem scenario = line.GetScenario();
		if (!scenario)
			return;
		
		bool mp = scenario.GetPlayerCount() > 1;
		if(mp)
			ServerBrowserMenuUI.OpenWithScenarioFilter(scenario);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void Play(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;
		
		SCR_WorkshopUiCommon.TryPlayScenario(scenario);
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ScenarioMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Continue(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;
		
		if (m_Header && !m_Header.GetSaveFileName().IsEmpty())
			GetGame().GetSaveManager().SetFileNameToLoad(m_Header);
		else
			GetGame().GetSaveManager().ResetFileNameToLoad();
		
		SCR_WorkshopUiCommon.TryPlayScenario(scenario);
		
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ScenarioMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRestartButton()
	{
		Restart(GetSelectedLine());
	}
	
	//------------------------------------------------------------------------------------------------
	void Restart(SCR_ContentBrowser_ScenarioLineComponent line)
	{
		if(!line)
			return;
		
		MissionWorkshopItem selectedMission = line.GetScenario();
		if (!selectedMission)
			return;

		m_SelectedScenario = selectedMission;
		m_Header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(selectedMission.Id()));
		bool canBeLoaded = m_Header && GetGame().GetSaveManager().HasLatestSave(m_Header);
		
		if(!canBeLoaded)
			return;
		
		GetGame().GetSaveManager().ResetFileNameToLoad();
		SCR_WorkshopUiCommon.TryPlayScenario(m_SelectedScenario);
		
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ScenarioMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHostButton()
	{
		Host(GetSelectedLine());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Host(SCR_ContentBrowser_ScenarioLineComponent line)
	{
		if(!line)
			return;
		
		MissionWorkshopItem scenario = line.GetScenario();
		if (!scenario)
			return;

		bool mp = scenario.GetPlayerCount() > 1;
		if (!mp)
			return;
		
		// Open server hosting dialog 
		ServerHostingUI dialog = ServerHostingUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ServerHostingDialog));
		
		dialog.SelectScenario(scenario);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnFilterButton()
	{
		// Toggle the state of the filter panel
		bool filterShown = m_Widgets.m_FilterPanelComponent.GetFilterListBoxShown();
		bool selectedAnyMission = GetSelectedScenario() != null;
		bool newFilterShown = !filterShown || (filterShown && selectedAnyMission);
		
		m_Widgets.m_FilterPanelComponent.ShowFilterListBox(newFilterShown);
		
		// Set focus on the button of the filter panel
		if (newFilterShown)
			GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_FilterPanelComponent.m_Widgets.m_FilterButton);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnSortingButton()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget focused = workspace.GetFocusedWidget();
		ScriptedWidgetEventHandler sortComp;
		if (focused)
			sortComp = focused.FindHandler(SCR_SortElementComponent);
		
	
		if (sortComp)
		{
			// Focused on a sort element
			// Focus back to previous widget or on first line
			if (m_wBeforeSort)
				GetGame().GetWorkspace().SetFocusedWidget(m_wBeforeSort);
			else
			{
				Widget firstLine = m_Widgets.m_ScenarioList.GetChildren();
				if (firstLine)
					workspace.SetFocusedWidget(firstLine);
			}
		}
		else
		{
			// Not focused on a sort element
			// Focus on a sort button
			if (focused.FindHandler(SCR_ContentBrowser_ScenarioLineComponent))
				m_wBeforeSort = focused;	// Return to this widget only if it's a scenario line
			else
				m_wBeforeSort = null;
			m_Widgets.m_SortingHeaderComponent.SetFocus(1);
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnFavouriteButton()
	{
		SCR_ContentBrowser_ScenarioLineComponent line = GetSelectedLine();
		SetFavorite(line);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetFavorite(SCR_ContentBrowser_ScenarioLineComponent line)
	{
		if (!line)
			return;
		
		MissionWorkshopItem scenario = line.GetScenario();
		
		//Update the scenario
		scenario.SetFavorite(!scenario.IsFavorite());
		
		//Update the widgets
		line.NotifyScenarioUpdate();
		
		//Update the Tooltip
		SCR_BrowserHoverTooltipComponent hoverComp = SCR_BrowserHoverTooltipComponent.FindComponent(line.GetRootWidget());
		SCR_NavigationButtonComponent favButton;
		
		//Delegate
		m_OnFavorite.Invoke(scenario.IsFavorite());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCopyIdButton()
	{
		SCR_ContentBrowser_ScenarioLineComponent line = GetSelectedLine();
		
		if (!line)
			return;
		
		MissionWorkshopItem scenario = line.GetScenario();
		
		if (scenario)
			System.ExportToClipboard(scenario.Id());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when user confirms the value in the seacrh string
	protected void OnSearchConfirm(SCR_EditBoxComponent comp, string newValue)
	{
		UpdateScenarioList(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when sorting header changes
	protected void OnSortingHeaderChange(SCR_SortHeaderComponent sortHeader)
	{
		UpdateScenarioList(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when filter panel is toggled (shown or hidden)
	protected void OnFilterPanelToggled(bool newState)
	{
		// When filter panel is disabled, we restore focus back to last focused line
		if (!newState && GetSelectedLine())
		{
			GetGame().GetWorkspace().SetFocusedWidget(GetSelectedLine().GetRootWidget());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called from scenario line component when scenario state changes
	protected void Callback_OnScenarioStateChanged(SCR_ContentBrowser_ScenarioLineComponent comp)
	{
		UpdateNavigationButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetFavoriteLabel(bool isFavorite)
	{
		if(isFavorite)
			return FAVORITE_LABEL_REMOVE;
		else
			return FAVORITE_LABEL_ADD;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool GetHostingAllowed()
	{
		#ifdef HOST_SCENARIO_ENABLED
		return true;
		#else
		return false;
		#endif
	}
};


// Classes to compare mission headers

// Sort by name
class SCR_CompareMissionName : SCR_SortCompare<MissionWorkshopItem>
{
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		string name1 = ResolveName(left.Name());
		string name2 = ResolveName(right.Name());
		
		if (name1.Compare(name2) == -1)
			return 1;
		else
			return 0;
	}
	
	static string ResolveName(string name)
	{
		if (name.StartsWith("#"))
			return WidgetManager.Translate(name);
		else
			return name;
	}
};

// Sort by name
class SCR_CompareMissionAddonName : SCR_SortCompare<MissionWorkshopItem>
{
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		WorkshopItem ownerItem = left.GetOwner();
		
		
		string name1 = ResolveAddonName(left);
		string name2 = ResolveAddonName(right);
		
		if (name1.Compare(name2) == -1)
			return 1;
		else
			return 0;
	}
	
	static string ResolveAddonName(MissionWorkshopItem scenario)
	{
		
		WorkshopItem sourceAddon = scenario.GetOwner();
		if (!sourceAddon)
			return WidgetManager.Translate("#AR-Editor_Attribute_OverlayLogo_Reforger");
		else
			return sourceAddon.Name();
	}
};

// Sort by player count
class SCR_CompareMissionPlayerCount : SCR_SortCompare<MissionWorkshopItem>
{
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		return left.GetPlayerCount() < right.GetPlayerCount();
	}
};

// Sort by favouriteness
class SCR_CompareMissionFavourite : SCR_SortCompare<MissionWorkshopItem>
{
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		return right.IsFavorite();
	}
};

// Sort by time since last played
class SCR_CompareMissionTimeSinceLastPlay : SCR_SortCompare<MissionWorkshopItem>
{
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		int timeLeft = left.GetTimeSinceLastPlay();
		if (timeLeft < 0)			// Negative value means we have never played this. 
			timeLeft =	0x7FFFFFFF;	// We set it to max positive int for sorting purpuse.
		int timeRight = right.GetTimeSinceLastPlay();
		if (timeRight < 0)
			timeRight =	0x7FFFFFFF;
		
		return timeLeft < timeRight;
	}
};