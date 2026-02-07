enum EPlayMenuContentType
{
	FEATURED,
	RECOMMENDED,
	RECENT
};

class SCR_PlayMenuEntry : Managed
{
	EPlayMenuContentType m_eContentType;
	ref MissionWorkshopItem m_Item;
	SCR_PlayMenuTileComponent m_Tile;
	Widget m_wRoot;
	
	void SCR_PlayMenuEntry(MissionWorkshopItem item, EPlayMenuContentType type)
	{
		m_Item = item;
		m_eContentType = type;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_PlayMenu: ChimeraMenuBase
{
	protected ResourceName m_sConfig = "{6409EA8EA4BFF7E6}Configs/PlayMenu/PlayMenuEntries.conf";
	protected ref Resource m_Config;
	protected BaseContainer m_ConfigEntries;
	
	const string TYPE_FEATURED = "m_aFeaturedScenarios";
	const string TYPE_RECOMMENDED = "m_aRecommendedScenarios";
	
	protected SCR_PlayMenuComponent m_Featured;
	protected SCR_PlayMenuComponent m_Recommended;
	protected SCR_PlayMenuComponent m_Recent;
	
	protected ref array<MissionWorkshopItem> m_aScenariosFeatured = {};
	protected ref array<MissionWorkshopItem> m_aScenariosRecommended = {};
	protected ref array<MissionWorkshopItem> m_aScenariosRecent = {};

	protected ref array<ref SCR_PlayMenuEntry> m_aEntriesFeatured = {};
	protected ref array<ref SCR_PlayMenuEntry> m_aEntriesRecommended = {};
	protected ref array<ref SCR_PlayMenuEntry> m_aEntriesRecent = {};
		
	protected ref MissionWorkshopItem m_ItemTutorial;
	protected WorkshopApi m_WorkshopAPI;
	protected SCR_PlayMenuTileComponent m_CurrentTile;
	protected Widget m_wRoot;
	protected bool m_bTutorialPlayed;
	protected bool m_bShowPlayTutorialDialog;
	protected int m_iPlayTutorialShowCount;
	protected int m_iPlayTutorialShowMax;

	const int THRESHOLD_RECENTLY_PLAYED = 3600 * 24 * 30;	
		
	protected Widget m_wFooter;
	
	protected SCR_NavigationButtonComponent m_Scenarios;
	protected SCR_NavigationButtonComponent m_Play;
	protected SCR_NavigationButtonComponent m_Continue;
	protected SCR_NavigationButtonComponent m_Restart;
	protected SCR_NavigationButtonComponent m_Host;
	protected SCR_NavigationButtonComponent m_FindServer;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_wRoot = GetRootWidget();
		m_Recent = SCR_PlayMenuComponent.GetComponent("Recent", m_wRoot);
		m_Recommended = SCR_PlayMenuComponent.GetComponent("Recommended", m_wRoot);
		m_Featured = SCR_PlayMenuComponent.GetComponent("Featured", m_wRoot);
		m_WorkshopAPI = GetGame().GetBackendApi().GetWorkshop();

		m_wFooter = m_wRoot.FindWidget("MenuBase1.SizeBase.VerticalLayout0.SizeFooter.Footer");

		SCR_NavigationButtonComponent back = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Back", m_wRoot);
		if (back)
			back.m_OnActivated.Insert(OnBack);		
				
		m_Scenarios = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Scenarios", m_wRoot);
		if (m_Scenarios)
			m_Scenarios.m_OnActivated.Insert(OnScenarios);
		
		m_Play = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Play", m_wFooter);
		if (m_Play)
			m_Play.m_OnActivated.Insert(OnPlay);
		
		m_Continue = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Continue", m_wFooter);
		if (m_Continue)
			m_Continue.m_OnActivated.Insert(OnContinue);

		m_Restart = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Restart", m_wFooter);
		if (m_Restart)
			m_Restart.m_OnActivated.Insert(OnRestart);		
		
		m_Host = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Host", m_wFooter);
		if (m_Host)
			m_Host.m_OnActivated.Insert(OnHost);
		
		m_FindServer = SCR_NavigationButtonComponent.GetNavigationButtonComponent("FindServer", m_wFooter);
		if (m_FindServer)
			m_FindServer.m_OnActivated.Insert(OnFindServer);

		// Read the PlayMenu config
		m_Config = BaseContainerTools.LoadContainer(m_sConfig);
		if (!m_Config)
			return;
		
		m_ConfigEntries = m_Config.GetResource().ToBaseContainer();
		if (!m_ConfigEntries)
			return;		

		// Get tutorial
		ResourceName tutorial;
		m_ConfigEntries.Get("m_TutorialScenario", tutorial);
		m_ItemTutorial = m_WorkshopAPI.GetInGameScenario(tutorial);
		
		if (m_ItemTutorial)
			m_bTutorialPlayed = m_ItemTutorial.GetTimeSinceLastPlay() > -1;
		
		// Check how many times "Play tutorial" dialog was shown
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");
		
		if (settings)
		{
			settings.Get("m_iPlayTutorialShowCount", m_iPlayTutorialShowCount);
			settings.Get("m_iPlayTutorialShowMax", m_iPlayTutorialShowMax);
		}
		
		m_bShowPlayTutorialDialog = !m_bTutorialPlayed && 	m_iPlayTutorialShowCount < m_iPlayTutorialShowMax;
		
		//PrintFormat("[OnMenuOpen] m_bShowPlayTutorialDialog: %1 | m_bTutorialPlayed: %2 | m_iPlayTutorialShowCount: %3", m_bShowPlayTutorialDialog, m_bTutorialPlayed, m_iPlayTutorialShowCount);
		
		// Get scenarios
		GetScenarios(m_aScenariosFeatured, EPlayMenuContentType.FEATURED);
		GetScenarios(m_aScenariosRecommended, EPlayMenuContentType.RECOMMENDED);
		GetScenarios(m_aScenariosRecent, EPlayMenuContentType.RECENT);
		
		// Get menu entries
		CreateMenuEntries(m_aEntriesFeatured, m_aScenariosFeatured, EPlayMenuContentType.FEATURED);
		CreateMenuEntries(m_aEntriesRecommended, m_aScenariosRecommended, EPlayMenuContentType.RECOMMENDED);
		CreateMenuEntries(m_aEntriesRecent, m_aScenariosRecent, EPlayMenuContentType.RECENT);
		
		SetupSectionTiles(m_Featured, m_aEntriesFeatured);
		SetupSectionTiles(m_Recommended, m_aEntriesRecommended);
		SetupSectionTiles(m_Recent, m_aEntriesRecent);
						
		// Set starting focused tile
		if (m_aEntriesRecommended.Count() > 0)
			m_Recommended.SetFocusedItem(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupSectionTiles(SCR_PlayMenuComponent section, array<ref SCR_PlayMenuEntry> entries)
	{
		ref array<Widget> widgets = section.GetWidgets();
		
		// Remove entries that do not fit into the layout
		if (entries.Count() > widgets.Count())
			entries.Resize(widgets.Count());
		
		// Setup grid tiles
		foreach (int i, SCR_PlayMenuEntry entry : entries)
		{
			Widget w = widgets.Get(i);
			if (!w)
				continue;

			SCR_PlayMenuTileComponent tile = SCR_PlayMenuTileComponent.Cast(w.FindHandler(SCR_PlayMenuTileComponent));
			if (!tile)
				continue;

			entry.m_Tile = tile;
			entry.m_wRoot = tile.m_wRoot;
			
			tile.m_OnPlay.Insert(OnPlay);
			tile.m_OnContinue.Insert(OnContinue);
			tile.m_OnRestart.Insert(OnRestart);
			tile.m_OnHost.Insert(OnHost);
			tile.m_OnFindServer.Insert(OnFindServer);

			tile.m_OnFocused.Insert(OnTileFocused);
			
			tile.Setup(entry.m_Item, entry.m_eContentType);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateMenuEntries(out array<ref SCR_PlayMenuEntry> entries, array<MissionWorkshopItem> scenarios, EPlayMenuContentType type)
	{
		foreach (MissionWorkshopItem scenario : scenarios)
		{
			entries.Insert(new SCR_PlayMenuEntry(scenario, type));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void GetScenarios(out array<MissionWorkshopItem> scenarios, EPlayMenuContentType type)
	{
		if (type == EPlayMenuContentType.RECENT)
		{
			GetRecentScenarios(scenarios);
			return;
		}
		
		array<ResourceName> aResources = {};
		
		if (type == EPlayMenuContentType.FEATURED)
			m_ConfigEntries.Get(TYPE_FEATURED, aResources);
		else
			m_ConfigEntries.Get(TYPE_RECOMMENDED, aResources);
		
		foreach (ResourceName sResource : aResources)
		{
			MissionWorkshopItem scenario =  m_WorkshopAPI.GetInGameScenario(sResource);
			
			if (scenario && !IsMissingDependency(scenario))
				scenarios.Insert(scenario);
		}		
	}

	//------------------------------------------------------------------------------------------------
	void GetRecentScenarios(out array<MissionWorkshopItem> scenarios)
	{
		// Get missions from Workshop API
		m_WorkshopAPI.GetPageScenarios(scenarios, 0, 10000);
		
		int count = scenarios.Count();
		int elapsed;
		MissionWorkshopItem scenario;
	
		// Remove scenarios from disabled addons
		for (int i = count - 1; i >= 0; i--)
		{
			scenario = scenarios[i];
			elapsed = scenario.GetTimeSinceLastPlay();

			if (elapsed == -1 || elapsed > THRESHOLD_RECENTLY_PLAYED || IsMissingDependency(scenario))
			{
				//PrintFormat("[GetRecentScenarios] removed: %1 | last played: %2 | missing dependency: %3", scenario.Name(), elapsed, IsMissingDependency(scenario));
				scenarios.Remove(i);
				continue;
			}
		};
			
		SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionTimeSinceLastPlay>.HeapSort(scenarios, false);
		/*
		// DEBUG: Print sorted recent scenarios
		count = scenarios.Count();
		for (int i = 0; i < count; i++)
		{
			scenario = scenarios[i];
			PrintFormat("[GetRecentScenarios] found: %1 | last played: %2", scenario.Name(), scenario.GetTimeSinceLastPlay());
		}
		*/
	}	

	//------------------------------------------------------------------------------------------------
	protected bool IsMissingDependency(MissionWorkshopItem scenario)
	{
		WorkshopItem addon = scenario.GetOwner();
		
		// There are no dependencies if scenario is not from an addon
		if (!addon)
			return false;
		
		array<Dependency> dependencies = {};
		addon.GetActiveRevision().GetDependencies(dependencies);
		
		// Check if any dependency is missing
		foreach (Dependency dependency : dependencies)
		{
			if (!dependency.IsOffline())
				return true;
		}
		
		return false;
	}	
			
	//------------------------------------------------------------------------------------------------
	protected bool IsUnique(MissionWorkshopItem item, array<ref SCR_PlayMenuEntry> entries)
	{
		if (!item)
			return false;
		
		foreach (SCR_PlayMenuEntry entry : entries)
		{
			if (!entry || !entry.m_Item)
				continue;
			
			if (item == entry.m_Item)
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBack()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScenarios()
	{
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
		
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ScenarioMenu);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlay()
	{
		if (m_bShowPlayTutorialDialog && m_ItemTutorial && m_ItemTutorial != m_CurrentTile.m_Item)
		{
			BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");
			
			if (settings)
			{
				m_iPlayTutorialShowCount++;
				
				settings.Set("m_iPlayTutorialShowCount", m_iPlayTutorialShowCount);
				GetGame().UserSettingsChanged();
			}
			
			DialogUI dialog = ShowTutorialDialog();
			if (dialog)
				dialog.m_OnCancel.Insert(PlayCurrentScenario);
		}
		else
		{
			PlayCurrentScenario();
		}
		
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnContinue()
	{
		SCR_MissionHeader header = m_CurrentTile.m_Header;
		if (header && !header.GetSaveFileName().IsEmpty())
			SCR_SaveLoadComponent.LoadOnStart(header);
		else
			SCR_SaveLoadComponent.LoadOnStart();
		
		PlayCurrentScenario();
		
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRestart()
	{
		SCR_SaveLoadComponent.LoadOnStart();
		PlayCurrentScenario();
		
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHost()
	{
		// PLAY TUTORIAL pop-up window removed for now
		/*
		if (m_bShowPlayTutorialDialog && m_ItemTutorial && m_ItemTutorial != m_CurrentTile.m_Item)
		{
			BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");
			
			if (settings)
			{
				m_iPlayTutorialShowCount++;
				
				settings.Set("m_iPlayTutorialShowCount", m_iPlayTutorialShowCount);
				GetGame().UserSettingsChanged();
			}
			
			DialogUI dialog = ShowTutorialDialog();
			if (dialog)
				dialog.m_OnCancel.Insert(HostCurrentScenario);
		}
		else
		{
			HostCurrentScenario();
		}
		*/
		
		HostCurrentScenario();	
		
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}	
		
	//------------------------------------------------------------------------------------------------
	protected void OnFindServer()
	{
		// PLAY TUTORIAL pop-up window removed for now
		/*
		if (m_bShowPlayTutorialDialog && m_ItemTutorial && m_ItemTutorial != m_CurrentTile.m_Item)
		{
			BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");
			
			if (settings)
			{
				m_iPlayTutorialShowCount++;
				
				settings.Set("m_iPlayTutorialShowCount", m_iPlayTutorialShowCount);
				GetGame().UserSettingsChanged();
			}
			
			DialogUI dialog = ShowTutorialDialog();
			if (dialog)
				dialog.m_OnCancel.Insert(FindCurrentScenarioServers);
		}
		else
		{
			FindCurrentScenarioServers();
		}
		*/
		
		FindCurrentScenarioServers();

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnTileFocused(SCR_PlayMenuTileComponent tile)
	{
		m_CurrentTile = tile;
		
		SCR_MissionHeader header = m_CurrentTile.m_Header;
		MissionWorkshopItem item = m_CurrentTile.m_Item;

		bool canBeLoaded = header && SCR_SaveLoadComponent.HasSaveFile(header);
		m_Play.SetVisible(!canBeLoaded, false);
		m_Continue.SetVisible(canBeLoaded, false);
		
		m_Restart.SetVisible(canBeLoaded, false);
		m_Host.SetVisible(item.GetPlayerCount() > 1, false);
		m_FindServer.SetVisible(item.GetPlayerCount() > 1, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected DialogUI ShowTutorialDialog()
	{
		DialogUI dialog = DialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.TutorialDialog));
		dialog.m_OnConfirm.Insert(OnPlayTutorial);
		return dialog;
	}
	
	//------------------------------------------------------------------------------------------------
	void TryPlayScenario(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		SCR_WorkshopUiCommon.TryPlayScenario(scenario);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PlayCurrentScenario()
	{
		TryPlayScenario(m_CurrentTile.m_Item);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FindCurrentScenarioServers()
	{
		MissionWorkshopItem scenario = m_CurrentTile.m_Item;
		
		if (!scenario)
			return;
			
		ServerBrowserMenuUI.OpenWithScenarioFilter(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void HostCurrentScenario()
	{
		ServerHostingUI dialog = ServerHostingUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ServerHostingDialog));
		
		if (!dialog)
			return;			
		
		dialog.SelectScenario(m_CurrentTile.m_Item);
	}	
		
	//------------------------------------------------------------------------------------------------
	protected void OnPlayTutorial()
	{
		TryPlayScenario(m_ItemTutorial);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		// Restore focus to the last accessed tile
		if (m_CurrentTile)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_CurrentTile.m_wRoot);
			return;
		}
		
		// Fallback to the 1st item in the *recent items* list
		if (m_Recent)
			m_Recent.SetFocusedItem(0);
	}		
};