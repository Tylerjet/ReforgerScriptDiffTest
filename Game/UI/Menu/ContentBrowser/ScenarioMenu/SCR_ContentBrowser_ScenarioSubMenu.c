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
	
	// Nav buttons
	protected SCR_NavigationButtonComponent m_NavPlay;
	protected SCR_NavigationButtonComponent m_NavJoin;
	protected SCR_NavigationButtonComponent m_NavHost;
	protected SCR_NavigationButtonComponent m_NavFilter;
	protected SCR_NavigationButtonComponent m_NavSorting;
	protected SCR_NavigationButtonComponent m_NavFavourite;
	
	
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
		
		// Create nav buttons
		SCR_ScenarioMenu scenarioMenu = SCR_ScenarioMenu.Cast(parentMenu);
		m_NavPlay = CreateNavigationButton("MenuSelect", "#AR-Workshop_ButtonPlay", true);
		m_NavJoin = CreateNavigationButton("MenuJoin", "#AR-Workshop_ButtonJoin", true);
		if (GetHostingAllowed())
			m_NavHost = CreateNavigationButton("MenuHost", "#AR-Workshop_ButtonHost", true);
		//m_NavFilter = CreateNavigationButton("MenuFilter", "#AR-Workshop_Filter", false);
		m_NavSorting = CreateNavigationButton("MenuFilter", "#AR-ScenarioBrowser_ButtonSorting", false);
		m_NavFavourite = CreateNavigationButton("MenuFavourite", "#AR-Workshop_ButtonAddToFavourites", true);
		
		m_NavPlay.m_OnActivated.Insert(OnPlayButton);
		m_NavJoin.m_OnActivated.Insert(OnJoinButton);
		if (m_NavHost)
			m_NavHost.m_OnActivated.Insert(OnHostButton);
		//m_NavFilter.m_OnActivated.Insert(OnFilterButton);
		m_NavSorting.m_OnActivated.Insert(OnSortingButton);
		m_NavFavourite.m_OnActivated.Insert(OnFavouriteButton);
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
		
		// When we switch from non-mouse input to mouse, set focused widget again
		// This is needed for the case when we scroll with keys, stop scrolling,
		// Then the mouse is staying over some new line, but it is not being focused
		// because mouse enter event doesn't get called
		
		// Disabled for now because we no longer focus on mouse enter
		/*
		EInputDeviceType inputType = GetGame().GetInputManager().GetLastUsedInputDevice();
		if (m_eLastInputType != EInputDeviceType.MOUSE && inputType == EInputDeviceType.MOUSE)
		{
			GetGame().GetWorkspace().SetFocusedWidget(WidgetManager.GetWidgetUnderCursor());
		}
		m_eLastInputType = inputType;
		*/
		
		
		// Set current scenario in the info panel
		m_Widgets.m_ScenarioDetailsPanelComponent.SetScenario(GetSelectedScenario());
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateNavigationButtons()
	{
		auto selectedMission = GetSelectedScenario();
		bool anythingSelected = selectedMission != null;
		bool mp;
		
		// If line is selected...
		if (selectedMission)
			mp = selectedMission.GetPlayerCount() > 1;
		
		m_NavJoin.SetEnabled(mp && anythingSelected);
		if (m_NavHost)
			m_NavHost.SetEnabled(mp && anythingSelected);
		m_NavFavourite.SetEnabled(anythingSelected);
		m_NavPlay.SetEnabled(anythingSelected);
		
		// Sorting button is hidden if we are using only mouse
		// It makes no sense to show it for mouse user because focus on sorting header is
		// Not visualized when we use mouse. The orange frame is only shown for KB & Gamepad.
		EInputDeviceType deviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		m_NavSorting.SetVisible(deviceType != EInputDeviceType.MOUSE);
		
		if (selectedMission)
		{
			string favLabel;
			if (!selectedMission.IsFavorite())
				favLabel = "#AR-Workshop_ButtonAddToFavourites";
			else
				favLabel = "#AR-Workshop_ButtonRemoveFavourites";
			m_NavFavourite.SetLabel(favLabel);
		}
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
				foreach (auto m : missionItemsAll)
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
					
					auto buttonComp = SCR_ModularButtonComponent.FindComponent(widget);
					buttonComp.m_OnFocus.Insert(OnLineFocus);
					buttonComp.m_OnDoubleClicked.Insert(OnLineDoubleClick);
					buttonComp.m_OnMouseEnter.Insert(OnLineMouseEnter);
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
	//! Called from OnMenuOpen
	protected array<ref SCR_MissionHeader> LoadMissionHeadersFromFolder(string folderPath)
	{
		array<ref SCR_MissionHeader> missionHeaders = new array<ref SCR_MissionHeader>;
		HeaderFileCallback callback = new HeaderFileCallback;
		callback.m_aHeaders = missionHeaders;
		callback.api = GetGame().GetBackendApi().GetWorkshop();
		System.FindFiles(callback.FindFilesCallback, folderPath, ".conf");
		
		return callback.m_aHeaders;
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
	
	//------------------------------------------------------------------------------------------------
	protected void TryFocusLineUnderCursor()
	{
		SCR_ContentBrowser_ScenarioLineComponent comp = GetLineUnderCursor();
		
		if (!comp)
			return;
		
		if (comp.GetRootWidget() != GetGame().GetWorkspace().GetFocusedWidget())
			GetGame().GetWorkspace().SetFocusedWidget(comp.GetRootWidget());
	}
	
	// -------------- Button event handlers ------------------------
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineFocus(SCR_ModularButtonComponent buttonComp)
	{
		auto lineComp = SCR_ContentBrowser_ScenarioLineComponent.FindComponent(buttonComp.GetRootWidget());
		
		m_LastSelectedLine = lineComp;
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineMouseEnter(SCR_ModularButtonComponent buttonComp, bool mouseInput)
	{
		// Bail if last input wasn't mouse
		if (!mouseInput)
			return;
		
		auto lineComp = SCR_ContentBrowser_ScenarioLineComponent.FindComponent(buttonComp.GetRootWidget());
		
		m_LastSelectedLine = lineComp;
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineDoubleClick(SCR_ModularButtonComponent buttonComp)
	{
		auto lineComp = SCR_ContentBrowser_ScenarioLineComponent.FindComponent(buttonComp.GetRootWidget());
		
		MissionWorkshopItem scenario = lineComp.GetScenario();
		
		if (!scenario)
			return;
		
		SCR_WorkshopUiCommon.TryPlayScenario(scenario);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnJoinButton()
	{
		MissionWorkshopItem scenario = GetSelectedScenario();
		
		if (!scenario)
			return;
		
		TryFocusLineUnderCursor();
		
		ServerBrowserMenuUI.OpenWithScenarioFilter(scenario);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayButton()
	{
		MissionWorkshopItem scenario = GetSelectedScenario();
		
		if (!scenario)
			return;
		
		TryFocusLineUnderCursor();
		
		SCR_WorkshopUiCommon.TryPlayScenario(scenario);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnHostButton()
	{
		MissionWorkshopItem scenario = GetSelectedScenario();
		
		if (!scenario)
			return;
		
		TryFocusLineUnderCursor();
		
		SCR_WorkshopUiCommon.TryHostScenario(scenario);
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
		
		if (!line)
			return;
		
		TryFocusLineUnderCursor();
		
		MissionWorkshopItem scenario = line.GetScenario();
		
		bool fav = scenario.IsFavorite();
		scenario.SetFavorite(!fav);
		line.NotifyScenarioUpdate(); // Update the widgets
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
		return left.GetPlayerCount() < right.GetPlayerCount())
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