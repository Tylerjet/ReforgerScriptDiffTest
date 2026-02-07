class SCR_PlayEntry : Managed
{
	bool m_bIsFeatured;
	bool m_bIsRecent;
	bool m_bIsRecommended;
	ref MissionWorkshopItem m_Header;
	SCR_PlayMenuTileComponent m_Tile;
	Widget m_wRoot;
	
	void SCR_PlayEntry(MissionWorkshopItem h, bool featured, bool recent, bool recommended)
	{
		m_Header = h;
		m_bIsFeatured = featured;
		m_bIsRecent = recent;
		m_bIsRecommended = recommended;
	}
	
	void SetTile(SCR_PlayMenuTileComponent tile, Widget widget)
	{
		m_Tile = tile;
		m_wRoot = widget;
	}
};

//------------------------------------------------------------------------------------------------
class PlayMenuUI: ChimeraMenuBase
{
	protected ResourceName m_Layout = "{02155A85F2DC521F}UI/layouts/Menus/PlayMenu/PlayMenuTile.layout";
	protected ResourceName m_Config = "{6409EA8EA4BFF7E6}Configs/PlayMenu/PlayMenuEntries.conf";
	protected SCR_GalleryComponent m_Gallery;
	protected ref MissionWorkshopItem m_TutorialHeader;
	protected WorkshopApi m_WorkshopAPI;
	protected SCR_PlayMenuTileComponent m_CurrentTile;
	protected Widget m_wRoot;
	protected ref array<ref SCR_PlayEntry> m_aEntries = {};

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_wRoot = GetRootWidget();
		m_Gallery = SCR_GalleryComponent.GetGalleryComponent("Gallery", m_wRoot);
		m_WorkshopAPI = GetGame().GetBackendApi().GetWorkshop();

		SCR_NavigationButtonComponent back = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Back", m_wRoot);
		if (back)
			back.m_OnActivated.Insert(OnBack);

		PrepareHeaders();
		PrepareWidgets();
		
		if (m_aEntries.Count() > 1)
			m_Gallery.SetFocusedItem(1);
	}

	//------------------------------------------------------------------------------------------------
	protected void PrepareHeaders()
	{
		Resource resource = BaseContainerTools.LoadContainer(m_Config);
		if (!resource)
			return;
		
		BaseContainer entries = resource.GetResource().ToBaseContainer();
		if (!entries)
			return;
		
		ResourceName featured;
		ResourceName tutorial;
		array<MissionWorkshopItem> recent = {};
		array<ResourceName> recommended = {};
		bool tutorialPlayed;
		int countRecentMissions;
		
		entries.Get("m_FeaturedMission", featured);
		entries.Get("m_TutorialMission", tutorial);
		entries.Get("m_aRecommendedMissions", recommended);
		entries.Get("m_iShowRecentMissions", countRecentMissions);
		BaseContainer cont = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");

		if (cont)
			cont.Get("m_bTutorialPlayed",tutorialPlayed);

		GetRecentScenarios(recent, countRecentMissions);
		
		// If tutorial was not played, mark it as featured and hide the actually featured mission
		// If tutorial was played, hide it (might be in recent missions)
		
		ResourceName actuallyFeatured;
		
		if (tutorialPlayed || tutorial.IsEmpty())
		{
			actuallyFeatured = featured;
		}
		else
		{
			actuallyFeatured = tutorial;
			if (tutorial != string.Empty)
				m_TutorialHeader = GetMission(tutorial);
		}
		
		MissionWorkshopItem header;
		if (actuallyFeatured != string.Empty)
			header = GetMission(actuallyFeatured);
		
		if (header)
			m_aEntries.Insert(new SCR_PlayEntry(header, true, false, false));
		
		if (recent)
		{
			for (int i = recent.Count() - 1; i >= 0; i--)
			{
				if (IsUnique(recent[i], m_aEntries))
					m_aEntries.Insert(new SCR_PlayEntry(recent[i], false, true, false));
			}
		}
		
		if (recommended)
		{
			foreach (ResourceName str : recommended)
			{
				MissionWorkshopItem h = GetMission(str);
				if (!h || !IsUnique(h, m_aEntries))
					continue;
				
				m_aEntries.Insert(new SCR_PlayEntry(h, false, false, true));
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	MissionWorkshopItem GetMission(ResourceName rsc)
	{
		return m_WorkshopAPI.GetInGameScenario(rsc);
	}
	
	//------------------------------------------------------------------------------------------------
	void GetRecentScenarios(array<MissionWorkshopItem> recent, int showCount)
	{
		if (showCount <= 0)
			return;
		
		// Get missions from Workshop API
		array<MissionWorkshopItem> missionItemsAll = new array<MissionWorkshopItem>;
		m_WorkshopAPI.GetPageScenarios(missionItemsAll, 0, 1000); // Get all missions at once
		
		// Remove scenarios from disabled addons
		for (int i = missionItemsAll.Count() - 1; i >= 0; i--)
		{
			MissionWorkshopItem m = missionItemsAll[i];
			WorkshopItem addon = m.GetOwner();
			
			if (!addon)
				continue;
			
			// Play menu will not show any entries from addons for now
			//if (!addon.IsEnabled())
				missionItemsAll.Remove(i);
		}
		
		int count = missionItemsAll.Count();
		SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionTimeSinceLastPlay>.HeapSort(missionItemsAll, false);
		showCount = Math.Min(showCount, count);
		
		// Get entries from the last valid index
		for (int i = 0; i < showCount; i++)
		{
			recent.Insert(missionItemsAll[count - 1 - i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsUnique(MissionWorkshopItem item, array<ref SCR_PlayEntry> entries)
	{
		if (!item)
			return false;
		
		foreach (SCR_PlayEntry entry : entries)
		{
			if (!entry || !entry.m_Header)
				continue;
			
			if (item == entry.m_Header)
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PrepareWidgets()
	{
		foreach (SCR_PlayEntry entry : m_aEntries)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_Layout, m_wRoot);
			if (!w)
				continue;

			SCR_PlayMenuTileComponent tile = SCR_PlayMenuTileComponent.Cast(w.FindHandler(SCR_PlayMenuTileComponent));
			if (!tile)
				continue;

			entry.SetTile(tile, tile.m_wRoot);
			tile.m_OnPlay.Insert(OnPlay);
			tile.m_OnContinue.Insert(OnContinue);
			tile.m_OnRestart.Insert(OnRestart);
			tile.m_OnFindServer.Insert(OnFindServer);
			tile.m_OnFocused.Insert(OnTileFocused);
			tile.ShowMission(entry.m_Header, entry.m_bIsFeatured, entry.m_bIsRecent, entry.m_bIsRecommended);
			
			m_Gallery.AddItem(w);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBack()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlay(SCR_PlayMenuTileComponent tile)
	{
		if (m_TutorialHeader && m_TutorialHeader != tile.m_Item)
		{
			DialogUI dialog = ShowTutorialDialog();
			if (dialog)
				dialog.m_OnCancel.Insert(PlayCurrentScenario);
		}
		else
		{
			SCR_SaveLoadComponent.LoadOnStart();
			TryPlayScenario(tile.m_Item);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnContinue(SCR_PlayMenuTileComponent tile)
	{
		SCR_MissionHeader header = tile.m_Header;
		if (header && !header.GetSaveFileName().IsEmpty())
			SCR_SaveLoadComponent.LoadOnStart(header);
		else
			SCR_SaveLoadComponent.LoadOnStart();
		
		TryPlayScenario(tile.m_Item);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRestart(SCR_PlayMenuTileComponent tile)
	{
		// TODO: Implement
		Print("[PlayMenuUI] Restarting scenario");
		SCR_SaveLoadComponent.LoadOnStart();
		SCR_WorkshopUiCommon.TryPlayScenario(tile.m_Item);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTileFocused(SCR_PlayMenuTileComponent tile)
	{
		m_CurrentTile = tile;
	}
	
	//------------------------------------------------------------------------------------------------
	protected DialogUI ShowTutorialDialog()
	{
		// Show tutorial dialog
		Print("[PlayMenuUI] Showing tutorial dialog");
		DialogUI dialog = DialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.TutorialDialog));
		dialog.m_OnConfirm.Insert(OnPlayTutorial);
		SetTutorialPlayed();
		return dialog;
	}
	
	//------------------------------------------------------------------------------------------------
	void TryPlayScenario(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;
		
		if (m_TutorialHeader == scenario)
			SetTutorialPlayed();
		
		Print("[PlayMenuUI] Playing scenario");
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
			MissionWorkshopItem scenario = GetGame().GetBackendApi().GetWorkshop().GetInGameScenario(m_CurrentTile.m_Header.GetHeaderResourceName());
			if (!scenario)
				return;
			
			ServerBrowserMenuUI.OpenWithScenarioFilter(scenario);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayTutorial()
	{
		TryPlayScenario(m_TutorialHeader);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayMission()
	{
		// If possible, load the save
		if (!m_CurrentTile || !m_CurrentTile.m_Header)
			return;
		
		SCR_SaveLoadComponent.LoadOnStart(m_CurrentTile.m_Header);
		TryPlayScenario(m_CurrentTile.m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFindServer(SCR_PlayMenuTileComponent tile)
	{
		if (m_TutorialHeader != null && m_TutorialHeader != tile.m_Header)
		{
			DialogUI dialog = ShowTutorialDialog();
			if (dialog)
				dialog.m_OnCancel.Insert(FindCurrentScenarioServers);
		}
		else
		{
			FindCurrentScenarioServers();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void SetTutorialPlayed()
	{
		// Warning was shown already, player knows about the tutorial
		BaseContainer cont = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");
		if (cont)
			cont.Set("m_bTutorialPlayed", true);
		GetGame().UserSettingsChanged();
	}
};