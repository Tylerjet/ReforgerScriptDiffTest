class SCR_PlayMenuTileComponent : SCR_TileBaseComponent
{
	[Attribute("0", UIWidgets.CheckBox, "Is this a big vertical tile? E.g. will use hi-res thumb picture..")]
	protected bool m_bBigTile;

	[Attribute("480", UIWidgets.Slider, "Estimated width of grid thumb picture, in a reference resolution.", "320 1920 1")]
	protected float m_fThumbnailWidth;

	MissionWorkshopItem m_Item;
	ref SCR_MissionHeader m_Header;
	string m_sScenarioPath;

	Widget m_wContentGroup;
	Widget m_wFeatured;
	TextWidget m_wName;
	TextWidget m_wDescription;
	Widget m_wRecentlyPlayed;
	TextWidget m_wRecentlyPlayedText;

	// Mouse interact buttons
	Widget m_wMouseInteractButtons;
	Widget m_wPlay;
	Widget m_wContinue;
	Widget m_wRestart;
	Widget m_wHost;
	Widget m_wFindServer;

	bool m_bIsMouseInteraction;
	
	protected ref array<SCR_ModularButtonComponent> m_aMouseButtonsError = {};

	bool m_bFocused;
	
	// Button components
	SCR_ModularButtonComponent m_Play;
	SCR_ModularButtonComponent m_Continue;
	SCR_ModularButtonComponent m_FindServer;
	SCR_ModularButtonComponent m_Host;
	SCR_ModularButtonComponent m_Restart;
	
	// Warning
	SCR_SimpleWarningOverlayComponent m_WarningOverlay;
	
	// Tooltip
	protected SCR_ScriptedWidgetTooltip m_CurrentTooltip;

	// Script invokers for the mouse interact buttons
	protected ref ScriptInvokerString m_OnMouseInteractionButtonClicked;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		m_wName = TextWidget.Cast(w.FindAnyWidget("Name"));
		m_wDescription = TextWidget.Cast(w.FindAnyWidget("Description"));
		m_wFeatured = w.FindAnyWidget("Featured");
		m_wContentGroup = w.FindAnyWidget("ContentGroup");

		m_wRecentlyPlayed = w.FindAnyWidget("RecentlyPlayed");
		m_wRecentlyPlayedText = TextWidget.Cast(m_wRecentlyPlayed.FindAnyWidget("Label"));

		m_wMouseInteractButtons = w.FindAnyWidget("MouseInteractButtons");
		m_wPlay = w.FindAnyWidget(SCR_ScenarioEntryHelper.BUTTON_PLAY);
		m_wContinue = w.FindAnyWidget(SCR_ScenarioEntryHelper.BUTTON_CONTINUE);
		m_wRestart = w.FindAnyWidget(SCR_ScenarioEntryHelper.BUTTON_RESTART);
		m_wHost = w.FindAnyWidget(SCR_ScenarioEntryHelper.BUTTON_HOST);
		m_wFindServer = w.FindAnyWidget(SCR_ScenarioEntryHelper.BUTTON_FIND_SERVERS);

		m_Play = SCR_ModularButtonComponent.FindComponent(m_wPlay);
		if (m_Play)
			m_Play.m_OnClicked.Insert(OnPlay);

		m_Continue = SCR_ModularButtonComponent.FindComponent(m_wContinue);
		if (m_Continue)
			m_Continue.m_OnClicked.Insert(OnContinue);

		m_Restart = SCR_ModularButtonComponent.FindComponent(m_wRestart);
		if (m_Restart)
			m_Restart.m_OnClicked.Insert(OnRestart);

		m_Host = SCR_ModularButtonComponent.FindComponent(m_wHost);
		if (m_Host)
		{
			m_aMouseButtonsError.Insert(m_Host);
			m_Host.m_OnClicked.Insert(OnHost);
		}

		m_FindServer = SCR_ModularButtonComponent.FindComponent(m_wFindServer);
		if (m_FindServer)
		{
			m_aMouseButtonsError.Insert(m_FindServer);
			m_FindServer.m_OnClicked.Insert(OnFindServers);
		}
		
		if (GetGame().InPlayMode())
			Enable(false);

		InputManager inputManager = GetGame().GetInputManager();

		if (inputManager)
			m_bIsMouseInteraction = (inputManager.GetLastUsedInputDevice() == EInputDeviceType.MOUSE);

		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceUserChanged);
		
		m_WarningOverlay = SCR_SimpleWarningOverlayComponent.Cast(SCR_SimpleWarningOverlayComponent.FindComponentInHierarchy(m_wRoot));
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{	
		super.OnFocus(w, x, y);
		
		m_bFocused = true;
		UpdateModularButtons();

		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		
		m_bFocused = false;
		UpdateModularButtons();
	
		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] item
	//! \param[in] contentType
	void Setup(notnull MissionWorkshopItem item, EPlayMenuContentType contentType)
	{
		m_Item = item;

		// Init header only if scenario is NOT coming from an addon
		if (!item.GetOwner())
			m_Header = SCR_MissionHeader.GetMissionHeader(item);

		m_wName.SetText(item.Name());
		m_wDescription.SetText(item.Description());

		bool canContinue = m_Header && SCR_ScenarioEntryHelper.HasSave(item);
		bool mp = SCR_ScenarioEntryHelper.IsMultiplayer(item);

		m_wPlay.SetVisible(!canContinue);
		m_wContinue.SetVisible(canContinue);
		m_wRestart.SetVisible(canContinue);
		m_wHost.SetVisible(mp && !GetGame().IsPlatformGameConsole());
		m_wFindServer.SetVisible(mp);

		UpdateModularButtons();
		UpdateWarning();
		
		// Set image through SCR_ButtonImageComponent
		SCR_ButtonImageComponent comp = SCR_ButtonImageComponent.Cast(m_wRoot.FindHandler(SCR_ButtonImageComponent));
		if (comp)
		{
			ResourceName texture = GetTexture();

			//PrintFormat("%1 | texture: %2", item.Name(), texture);

			if (!texture.IsEmpty())
				comp.SetImage(texture, item.GetOwner() != null);
		}

		//DEBUG
//		string savefile_item = SCR_SaveLoadComponent.GetSaveFileName(item);
//		Print(savefile_item, LogLevel.NORMAL);
//		string savefile_header = SCR_SaveLoadComponent.GetSaveFileName(m_Header);
//		Print(savefile_header, LogLevel.NORMAL);

		m_wFeatured.SetVisible(contentType == EPlayMenuContentType.FEATURED);
		m_wRecentlyPlayed.SetVisible(contentType == EPlayMenuContentType.RECENT);

		if (contentType == EPlayMenuContentType.RECENT)
		{
			int timeSinceLastPlayedSeconds = m_Item.GetTimeSinceLastPlay();
			string sLastPlayed = SCR_FormatHelper.GetTimeSinceEventImprecise(timeSinceLastPlayedSeconds);

			m_wRecentlyPlayedText.SetText(sLastPlayed);
		}
		
		if (m_wMouseInteractButtons)
			m_wMouseInteractButtons.SetVisible(false);

		Enable();
	}

	//------------------------------------------------------------------------------------------------
	protected void Enable(bool enable = true)
	{
		m_wRoot.SetEnabled(enable);
		m_wContentGroup.SetVisible(enable);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetTexture()
	{
		if (m_bBigTile && m_Header && !m_Header.m_sPreviewImage.IsEmpty())
			return m_Header.m_sPreviewImage;

		if (m_Header)
			return m_Header.m_sLoadingScreen;

		if (m_Item)
		{
			BackendImage image = m_Item.Thumbnail();

			// DEBUG problems with scenario addon thumbs
//			array<ImageScale> scales = {};
//			int i = image.GetScales(scales);

			// Get optimal width for the thumb
			float width = g_Game.GetWorkspace().DPIScale(m_fThumbnailWidth);
			ImageScale scale = image.GetLocalScale((int)width);
			if (scale)
				return scale.Path();
		}

		return string.Empty;
	}

	// React on switching between input methods
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceUserChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		//PrintFormat("OnInputDeviceUserChanged | %1 -> %2", oldDevice, newDevice);

		if (newDevice == EInputDeviceType.TRACK_IR || newDevice == EInputDeviceType.JOYSTICK)
			return;

		m_bIsMouseInteraction = (newDevice == EInputDeviceType.MOUSE);
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
		if (SCR_ScenarioEntryHelper.IsInErrorState(m_Item))
			return;
		
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioEntryHelper.BUTTON_HOST);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFindServers()
	{
		if (SCR_ScenarioEntryHelper.IsInErrorState(m_Item))
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
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltipClass, Widget tooltipWidget, Widget hoverWidget, SCR_ScriptedWidgetTooltipPreset preset, string tag)
	{
		m_CurrentTooltip = tooltipClass;
		SCR_ScenarioEntryHelper.UpdateErrorMouseButtonsTooltip(tooltipClass, m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateModularButtons()
	{
		SCR_ScenarioEntryHelper.UpdateErrorMouseButtonsTooltip(m_CurrentTooltip, m_Item);
		
		if (m_wMouseInteractButtons)
			m_wMouseInteractButtons.SetVisible(m_bFocused);

		array<SCR_ModularButtonComponent> mouseButtons = {};
		if (m_Restart)
			mouseButtons.Insert(m_Restart);
		
		SCR_ScenarioEntryHelper.UpdateMouseButtons(mouseButtons, m_aMouseButtonsError, m_Item, m_bFocused);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateWarning()
	{
		if (!m_WarningOverlay)
			return;
		
		m_bIsInErrorState = SCR_ScenarioEntryHelper.IsModInErrorState(m_Item);
		
		m_WarningOverlay.SetWarningVisible(m_bIsInErrorState, false);
		m_WarningOverlay.SetWarning(SCR_ScenarioEntryHelper.GetErrorMessage(m_Item), SCR_ScenarioEntryHelper.GetErrorTexture(m_Item));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerString GetOnMouseInteractionButtonClicked()
	{
		if (!m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked = new ScriptInvokerString();

		return m_OnMouseInteractionButtonClicked;
	}
}
