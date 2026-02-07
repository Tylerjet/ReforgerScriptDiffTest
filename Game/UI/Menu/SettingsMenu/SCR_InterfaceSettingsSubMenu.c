class SCR_InterfaceSettingsSubMenu : SCR_SettingsSubMenuBase
{
	protected const string CHAT_SPINBOX_WIDGET_NAME = "Chat";
	protected const string NOTIFICATIONS_SPINBOX_WIDGET_NAME = "Notifications";
	protected const string VON_SPINBOX_WIDGET_NAME = "VoN";
	protected const string CONTROLHINTS_SPINBOX_WIDGET_NAME = "ControlHints";
	protected const string HINTS_SPINBOX_WIDGET_NAME = "Hints";
	protected const string NEARBYINTERACTION_SPINBOX_WIDGET_NAME = "NearbyInteractions";
	protected const string NAMETAGS_SPINBOX_WIDGET_NAME = "NameTags";
	protected const string FPS_SPINBOX_WIDGET_NAME = "FPS";
	protected const string SERVER_FPS_SPINBOX_WIDGET_NAME = "ServerFPS";
	protected const string PACKETLOSS_SPINBOX_WIDGET_NAME = "PacketLoss";
	protected const string LATENCY_SPINBOX_WIDGET_NAME = "Latency";
	protected const string GAMEVERSION_SPINBOX_WIDGET_NAME = "GameVersion";
	protected const string WEAPONINFO_SPINBOX_WIDGET_NAME = "WeaponInfo";
	protected const string VEHICLEINFO_SPINBOX_WIDGET_NAME = "VehicleInfo";

	protected BaseContainer m_InterfaceSettings;
	protected SCR_SpinBoxComponent m_SpinnerBoxComp;
	
	//------------------------------------------------------------------------------------------------
	//! Get the correct Spinnerbox widget for the setting and set it's value based on the stored setting
	//! \param[in] name of the Spinnerbox widget
	//! \param[in] variable name of the setting from SCR_InterfaceSettings
	protected void SetupSpinboxes(string widgetName, string settingName)
	{
		m_SpinnerBoxComp = SCR_SpinBoxComponent.GetSpinBoxComponent(widgetName, m_wRoot);
		if (m_SpinnerBoxComp)
		{
			bool state;
			m_InterfaceSettings.Get(settingName, state);
			m_SpinnerBoxComp.SetCurrentItem(state);
		}
		else
		{
			PrintFormat("Interface setting widget '%1' not found", widgetName, level: LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		m_InterfaceSettings = GetGame().GetGameUserSettings().GetModule("SCR_InterfaceSettings");

		if (!m_InterfaceSettings)
			return;

		// Setup Chat
		SetupSpinboxes(CHAT_SPINBOX_WIDGET_NAME, "m_bShowChat");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnChatChange);

		// Setup Notifications
		SetupSpinboxes(NOTIFICATIONS_SPINBOX_WIDGET_NAME, "m_bShowNotifications");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnNotificationsChange);

		// Setup VoN
		SetupSpinboxes(VON_SPINBOX_WIDGET_NAME, "m_bShowVoN");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnVoNChange);

		// Setup Control hints
		SetupSpinboxes(CONTROLHINTS_SPINBOX_WIDGET_NAME, "m_bShowControlHints");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnControlHintsChange);

		// Setup hints
		SetupSpinboxes(HINTS_SPINBOX_WIDGET_NAME, "m_bShowHints");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnHintsChange);
		
		// Setup nearby interactions
		SetupSpinboxes(NEARBYINTERACTION_SPINBOX_WIDGET_NAME, "m_bShowNearbyInteractions");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnNearbyInteractionsChange);

		// Setup Nametags
		SetupSpinboxes(NAMETAGS_SPINBOX_WIDGET_NAME, "m_bShowNameTags");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnNameTagsChange);

		// Setup FPS
		SetupSpinboxes(FPS_SPINBOX_WIDGET_NAME, "m_bShowFPS");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnFPSChange);

		// Setup ServerFPS
		SetupSpinboxes(SERVER_FPS_SPINBOX_WIDGET_NAME, "m_bShowServerFPS");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnServerFPSChange);

		// Setup PacketLoss
		SetupSpinboxes(PACKETLOSS_SPINBOX_WIDGET_NAME, "m_bShowPacketLoss");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnPacketLossChange);

		// Setup Latency
		SetupSpinboxes(LATENCY_SPINBOX_WIDGET_NAME, "m_bShowLatency");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnLatencyChange);

		// Setup GameVersion
		SetupSpinboxes(GAMEVERSION_SPINBOX_WIDGET_NAME, "m_bShowGameVersion");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnGameVersionChange);

		// Setup WeaponInfo
		SetupSpinboxes(WEAPONINFO_SPINBOX_WIDGET_NAME, "m_bShowWeaponInfo");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnWeaponInfoChange);

		// Setup VehicleInfo
		SetupSpinboxes(VEHICLEINFO_SPINBOX_WIDGET_NAME, "m_bShowVehicleInfo");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnVehicleInfoChange);
	}

	//------------------------------------------------------------------------------------------------
	void OnChatChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowChat", currentItem);

		BaseContainer hintSetings = GetGame().GetGameUserSettings().GetModule("SCR_HintSettings");
		if (hintSetings)
			hintSetings.Set("m_bHintsEnabled", currentItem);

		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnNotificationsChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowNotifications", currentItem);

		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnVoNChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowVoN", currentItem);

		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnControlHintsChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowControlHints", currentItem);

		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnHintsChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowHints", currentItem);

		GetGame().UserSettingsChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnNearbyInteractionsChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowNearbyInteractions", currentItem);

		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnNameTagsChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowNameTags", currentItem);

		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnFPSChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowFPS", currentItem);

		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnServerFPSChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowServerFPS", currentItem);

		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnPacketLossChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowPacketLoss", currentItem);

		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnLatencyChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowLatency", currentItem);

		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnGameVersionChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowGameVersion", currentItem);

		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnWeaponInfoChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowWeaponInfo", currentItem);

		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnVehicleInfoChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		m_InterfaceSettings.Set("m_bShowVehicleInfo", currentItem);

		GetGame().UserSettingsChanged();
	}
}
