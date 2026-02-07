/*
Component for a line in scenario browser.
*/
class SCR_ContentBrowser_ScenarioLineComponent : SCR_ListMenuEntryComponent
{
	protected ref SCR_ContentBrowser_ScenarioLineWidgets m_Widgets = new SCR_ContentBrowser_ScenarioLineWidgets;

	protected MissionWorkshopItem m_Mission;
	
	protected const string MOD_ICON = "modIcon";
	
	protected Widget m_wLineBackground;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_Widgets.Init(w);

		if (!GetGame().InPlayMode())
			return;

		// Mouse interaction buttons
		m_FavComponent = m_Widgets.m_FavouriteButtonComponent;

		m_aMouseButtons.Insert(m_FavComponent);
		m_aMouseButtons.Insert(m_Widgets.m_PlayComponent);
		m_aMouseButtons.Insert(m_Widgets.m_ContinueComponent);
		m_aMouseButtons.Insert(m_Widgets.m_RestartComponent);
		m_aMouseButtons.Insert(m_Widgets.m_HostComponent);
		m_aMouseButtons.Insert(m_Widgets.m_FindServersComponent);
		
		m_aMouseButtonsError.Insert(m_Widgets.m_HostComponent);
		m_aMouseButtonsError.Insert(m_Widgets.m_FindServersComponent);

		m_Widgets.m_PlayComponent.m_OnClicked.Insert(OnPlay);
		m_Widgets.m_ContinueComponent.m_OnClicked.Insert(OnContinue);
		m_Widgets.m_RestartComponent.m_OnClicked.Insert(OnRestart);
		m_Widgets.m_HostComponent.m_OnClicked.Insert(OnHost);
		m_Widgets.m_FindServersComponent.m_OnClicked.Insert(OnFindServers);

		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		m_wLineBackground = w.FindAnyWidget("LineBackground");
		
		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateModularButtons()
	{
		if (!m_Mission)
			return;

		// Visibility
		bool mp = SCR_ScenarioEntryHelper.IsMultiplayer(m_Mission);
		bool canContinue = SCR_ScenarioEntryHelper.HasSave(m_Mission);
		bool show = m_bMouseButtonsEnabled && m_bFocused && SCR_ScenarioEntryHelper.IsReady(m_Mission);

		m_Widgets.m_Play.SetVisible(show && !canContinue);
		m_Widgets.m_Continue.SetVisible(show && canContinue);
		m_Widgets.m_Restart.SetVisible(canContinue);
		
		m_Widgets.m_Host.SetVisible(show && !GetGame().IsPlatformGameConsole() && mp);

		m_Widgets.m_FindServers.SetVisible(show && mp);

		m_bIsInErrorState = SCR_ScenarioEntryHelper.IsInErrorState(m_Mission);
		SCR_ScenarioEntryHelper.UpdateErrorMouseButtonsTooltip(m_CurrentTooltip, m_Mission);
		
		bool inError = SCR_ScenarioEntryHelper.IsModInErrorState(m_Mission);
		
		if (m_Widgets.m_SourceImageCommunity.IsVisible())
		{
			string icon = MOD_ICON;
			Color color = Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt());
			
			if (inError)
			{
				icon = SCR_ScenarioEntryHelper.GetErrorTexture(m_Mission);
				if (m_bFocused)
					color = Color.FromInt(UIColors.WARNING.PackToInt());
				else
					color = Color.FromInt(UIColors.WARNING_DISABLED.PackToInt());
			}
			
			m_Widgets.m_SourceImageCommunity.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, icon);
			m_Widgets.m_SourceImageCommunity.SetIsColorInherited(!inError);
			m_Widgets.m_SourceImageCommunity.SetColor(color);
		}

		super.UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	override bool SetFavorite(bool favorite)
	{
		if (!m_Mission || !super.SetFavorite(favorite))
			return false;

		m_Mission.SetFavorite(favorite);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltipClass, Widget tooltipWidget, Widget hoverWidget, SCR_ScriptedWidgetTooltipPreset preset, string tag)
	{
		SCR_ScenarioEntryHelper.UpdateErrorMouseButtonsTooltip(tooltipClass, m_Mission);
		
		super.OnTooltipShow(tooltipClass, tooltipWidget, hoverWidget, preset, tag);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		if (!m_Mission)
			return;

		// Fav button
		if (m_Mission)
			SetFavorite(m_Mission.IsFavorite());

		//! Name
		m_Widgets.m_NameText.SetText(m_Mission.Name());

		//! Game mode
		/*
		// Not supported yet - todo
		*/

		//! Type/player count
		int playerCount = m_Mission.GetPlayerCount();
		bool mp = SCR_ScenarioEntryHelper.IsMultiplayer(m_Mission);
	
		m_Widgets.m_SinglePlayerImage.SetVisible(!m_bMouseButtonsEnabled && !mp);
		m_Widgets.m_MultiPlayerImage.SetVisible(!m_bMouseButtonsEnabled && mp);
		m_Widgets.m_PlayerCountText.SetText(playerCount.ToString());

		//! Last played time text
		int timeSinceLastPlayedSeconds = m_Mission.GetTimeSinceLastPlay();
		string timeSinceLastPlayText;

		if (timeSinceLastPlayedSeconds > 0)
			timeSinceLastPlayText = SCR_FormatHelper.GetTimeSinceEventImprecise(timeSinceLastPlayedSeconds);
		else
			timeSinceLastPlayText = "-";

		m_Widgets.m_LastPlayedText.SetText(timeSinceLastPlayText);

		//! Source addon
		WorkshopItem sourceAddon = m_Mission.GetOwner();
		m_Widgets.m_SourceImageOfficial.SetVisible(sourceAddon == null);
		m_Widgets.m_SourceImageCommunity.SetVisible(sourceAddon != null);
		m_Widgets.m_SourceNameTextOfficial.SetVisible(sourceAddon == null);
		m_Widgets.m_SourceNameTextCommunity.SetVisible(sourceAddon != null);
		string sourceAddonText = string.Empty;
		if (sourceAddon)
			m_Widgets.m_SourceNameTextCommunity.SetText(sourceAddon.Name());
		else
			m_Widgets.m_SourceNameTextOfficial.SetText("#AR-Editor_Attribute_OverlayLogo_Reforger");

		//! Modular buttons
		UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlay()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioEntryHelper.BUTTON_PLAY);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnContinue()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioEntryHelper.BUTTON_CONTINUE);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestart()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioEntryHelper.BUTTON_RESTART);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHost()
	{
		if (m_bIsInErrorState)
			return;
		
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioEntryHelper.BUTTON_HOST);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFindServers()
	{
		if (m_bIsInErrorState)
			return;
		
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioEntryHelper.BUTTON_FIND_SERVERS);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateModularButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Must be called when the scenario properties have been updated (set/clear favourite or similar)
	void NotifyScenarioUpdate()
	{
		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	void SetScenario(MissionWorkshopItem mission)
	{
		m_Mission = mission;

		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	MissionWorkshopItem GetScenario()
	{
		return m_Mission;
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowFavouriteButton(bool show)
	{
		m_Widgets.m_FavouriteButtonOverlay.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	void ShowName(bool show)
	{
		m_Widgets.m_NameText.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	void ShowPlayerCount(bool show)
	{
		m_Widgets.m_PlayerCount.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	void ShowLastPlayedText(bool show)
	{
		m_Widgets.m_LastPlayedTextSize.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	void ShowSource(bool show)
	{
		m_Widgets.m_Source.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_ContentBrowser_ScenarioLineComponent FindComponent(Widget w)
	{
		return SCR_ContentBrowser_ScenarioLineComponent.Cast(w.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));
	}
}