/*!
Classes for Scenario dialogs
*/

class SCR_ScenarioDialogs
{
	static protected const ResourceName DIALOGS_CONFIG = "{F020A20CC93DB3C7}Configs/ContentBrowser/ScenarioDialogs.conf";

	//------------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateDialog(string presetName)
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, presetName);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_ScenarioConfirmationDialogUi CreateScenarioConfirmationDialog(SCR_ContentBrowser_ScenarioLineComponent line, ScriptInvoker onFavoritesResponse = null)
	{
		SCR_ScenarioConfirmationDialogUi dialogUI = new SCR_ScenarioConfirmationDialogUi(line, onFavoritesResponse);
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "SCENARIO_CONFIRMATION", dialogUI);

		return dialogUI;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioConfirmationDialogUi : SCR_ConfigurableDialogUi
{
	protected SCR_ContentBrowser_ScenarioLineComponent m_Line;

	protected Widget m_wFavoriteImage;
	protected SCR_NavigationButtonComponent m_Favorite;
	
	//This should probably be a setting in SCR_HorizontalScrollAnimationComponent, as this is a bandaid solution to the title flickering
	protected const int MAX_TITLE_LENGTH = 55;

	ref ScriptInvoker<SCR_ContentBrowser_ScenarioLineComponent> m_OnFavorite = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioConfirmationDialogUi(SCR_ContentBrowser_ScenarioLineComponent line, ScriptInvoker onFavoritesResponse = null)
	{
		m_Line = line;

		if (onFavoritesResponse)
			onFavoritesResponse.Insert(UpdateFavoriteWidgets);
	}

	//! OVERRIDES
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);

		if (!m_Line)
			return;

		MissionWorkshopItem scenario = m_Line.GetScenario();
		if (!scenario)
			return;

		//! Update visuals
		SetTitle(scenario.Name());

		Widget backgroundImageBackend = GetRootWidget().FindAnyWidget("BackgroundImageBackend");
		if (backgroundImageBackend)
		{
			SCR_ScenarioBackendImageComponent backendImageComp = SCR_ScenarioBackendImageComponent.Cast(backgroundImageBackend.FindHandler(SCR_ScenarioBackendImageComponent));
			if (backendImageComp)
				backendImageComp.SetScenarioAndImage(scenario, scenario.Thumbnail());
		}

		//! Content layout
		Widget contentLayoutRoot = GetContentLayoutRoot(GetRootWidget());
		if (!contentLayoutRoot)
			return;

		Widget singlePlayerImage = contentLayoutRoot.FindAnyWidget("SinglePlayerImageOverlay");
		Widget multiPlayerImage = contentLayoutRoot.FindAnyWidget("MultiPlayerImageOverlay");
		TextWidget playerCountText = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("PlayerCountText"));
		TextWidget playerCountLabelText = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("PlayerCountLabelText"));

		Widget sourceImageOfficial = contentLayoutRoot.FindAnyWidget("SourceImageOfficialOverlay");
		Widget sourceImageCommunity = contentLayoutRoot.FindAnyWidget("SourceImageCommunityOverlay");
		TextWidget sourceNameTextOfficial = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("SourceNameTextOfficial"));
		TextWidget sourceNameTextCommunity = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("SourceNameTextCommunity"));

		//! Type and player count
		int playerCount = scenario.GetPlayerCount();
		bool mp = playerCount > 1;
		singlePlayerImage.SetVisible(!mp);
		multiPlayerImage.SetVisible(mp);
		if (mp)
		{
			playerCountText.SetText(playerCount.ToString());
			playerCountLabelText.SetText("#AR-ServerBrowser_ServerPlayers");
		}

		//! Source addon
		bool isSourceAddonValid;
		WorkshopItem sourceAddon = scenario.GetOwner();

		if (sourceAddon)
		{
			isSourceAddonValid = true;
			sourceNameTextCommunity.SetText(sourceAddon.Name());
		}
		else
		{
			isSourceAddonValid = false;
		}

		sourceImageOfficial.SetVisible(!isSourceAddonValid);
		sourceImageCommunity.SetVisible(isSourceAddonValid);
		sourceNameTextOfficial.SetVisible(!isSourceAddonValid);
		sourceNameTextCommunity.SetVisible(isSourceAddonValid);

		//! Buttons
		SCR_MissionHeader header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(scenario.Id()));
		bool canBeLoaded = header && GetGame().GetSaveManager().HasLatestSave(header);
		string playLabel = "#AR-Workshop_ButtonPlay";
		if (canBeLoaded)
			playLabel = "#AR-PauseMenu_Continue";

		SCR_NavigationButtonComponent confirm = FindButton("confirm");
		if (confirm)
			confirm.SetLabel(playLabel);

		SCR_NavigationButtonComponent restart = FindButton("restart");
		if (restart)
			restart.SetVisible(canBeLoaded, false);

		SCR_NavigationButtonComponent join = FindButton("join");
		if (join)
			join.SetVisible(mp, false);

		SCR_NavigationButtonComponent host = FindButton("host");
		if (host)
			host.SetVisible(mp && !GetGame().IsPlatformGameConsole() /*&& SCR_ContentBrowser_ScenarioSubMenu.GetHostingAllowed()*/, false);

		m_Favorite = FindButton("favorite");
		if (m_Favorite)
			m_Favorite.m_OnActivated.Insert(OnFavoritesButton);

		//! Star button
		Widget favButton = m_wRoot.FindAnyWidget("FavoriteButton");
		if (favButton)
		{
			SCR_ButtonComponent favButtonComp = SCR_ButtonComponent.Cast(favButton.FindHandler(SCR_ButtonComponent));
			if (favButtonComp)
				favButtonComp.m_OnClicked.Insert(OnFavoritesButton);
		}

		m_wFavoriteImage = m_wRoot.FindAnyWidget("FavoriteImage");

		//! Favorites widgets update
		UpdateFavoriteWidgets(scenario.IsFavorite());
	}


	//------------------------------------------------------------------------------------------------
	override void OnButtonPressed(SCR_NavigationButtonComponent button)
	{
		super.OnButtonPressed(button);

		if (m_sLastPressedButtonTag != "favorite")
			Close();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetTitle(string text)
	{
		super.SetTitle(text);
		
		Widget titleFrame = m_wRoot.FindAnyWidget("TitleFrame");
		if (!titleFrame)
			return;
		
		SCR_HorizontalScrollAnimationComponent scrollComp = SCR_HorizontalScrollAnimationComponent.Cast(titleFrame.FindHandler(SCR_HorizontalScrollAnimationComponent));
		if (!scrollComp)
			return;
		
		if (text.Length() < MAX_TITLE_LENGTH)
			scrollComp.AnimationStop();
		else
			scrollComp.AnimationStart();
	}

	//! PROTECTED
	//------------------------------------------------------------------------------------------------
	protected void OnFavoritesButton()
	{
		m_OnFavorite.Invoke(m_Line);
	}


	//------------------------------------------------------------------------------------------------
	protected void UpdateFavoriteWidgets(bool isFavorite)
	{
		// Footer Button
		string label = "#AR-Workshop_ButtonAddToFavourites";
		if (isFavorite)
			label = "#AR-Workshop_ButtonRemoveFavourites";

		m_Favorite.SetLabel(label);

		// Star Button
		if (m_wFavoriteImage)
		{
			if (isFavorite)
				AnimateWidget.Color(m_wFavoriteImage, UIColors.CONTRAST_COLOR, UIConstants.FADE_RATE_FAST);
			else
				AnimateWidget.Color(m_wFavoriteImage, UIColors.LIGHT_GREY, UIConstants.FADE_RATE_FAST);
		}
	}


	//! PUBLIC
	//------------------------------------------------------------------------------------------------
	SCR_ContentBrowser_ScenarioLineComponent GetLine()
	{ 
		return m_Line; 
	}
};
