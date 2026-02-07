class SCR_VideoSettingsConsoleSubMenu : SCR_SettingsSubMenuBase
{		
	protected const int COMBO_OPTION_QUALITY = 0;
	protected const int COMBO_OPTION_PERFORMANCE = 1;
	
	protected EPlatform m_ePlatform;
	SCR_SettingsManager m_SettingsManager;
		
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		
		m_SettingsManager = GetGame().GetSettingsManager();

		if (!m_SettingsManager)
			return;
		
		m_ePlatform = System.GetPlatform();
		SetupQualityPreset();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnQualityPresetChanged(SCR_ComboBoxComponent combobox, int itemIndex)
	{
		if (m_ePlatform != EPlatform.XBOX_SERIES_X && m_ePlatform != EPlatform.XBOX_SERIES_S)
			return;
		
		ChangePreset(m_ePlatform, itemIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ChangePreset(EPlatform platform, int itemIndex)
	{
		if (platform == EPlatform.XBOX_SERIES_S && itemIndex == COMBO_OPTION_QUALITY)
			m_SettingsManager.SetConsolePreset(EVideoQualityPreset.SERIES_S_PRESET_QUALITY);
		
		if (platform == EPlatform.XBOX_SERIES_S && itemIndex == COMBO_OPTION_PERFORMANCE)
			m_SettingsManager.SetConsolePreset(EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE);
		
		if (platform == EPlatform.XBOX_SERIES_X && itemIndex == COMBO_OPTION_QUALITY)
			m_SettingsManager.SetConsolePreset(EVideoQualityPreset.SERIES_X_PRESET_QUALITY);
		
		if (platform == EPlatform.XBOX_SERIES_X && itemIndex == COMBO_OPTION_PERFORMANCE)
			m_SettingsManager.SetConsolePreset(EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE);
		
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupQualityPreset()
	{
		SCR_ComboBoxComponent qualityPreset = SCR_ComboBoxComponent.GetComboBoxComponent("QualityPreset", m_wRoot);
		if (!qualityPreset)
			return;
		
		int lastPreset;
		
		BaseContainer videoSettings = GetGame().GetGameUserSettings().GetModule("SCR_VideoSettings");
		if (!videoSettings)
			return;
		
		videoSettings.Get("m_iLastUsedPreset", lastPreset);

		if (lastPreset == EVideoQualityPreset.SERIES_X_PRESET_QUALITY || lastPreset == EVideoQualityPreset.SERIES_S_PRESET_QUALITY)
			qualityPreset.SetCurrentItem(COMBO_OPTION_QUALITY);
		
		if (lastPreset == EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE || lastPreset == EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE)
			qualityPreset.SetCurrentItem(COMBO_OPTION_PERFORMANCE);
		
		qualityPreset.m_OnChanged.Insert(OnQualityPresetChanged);
	}
};