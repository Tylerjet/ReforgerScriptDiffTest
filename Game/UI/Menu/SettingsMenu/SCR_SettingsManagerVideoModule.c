class SCR_SettingsManagerVideoModule : SCR_SettingsManagerModuleBase
{
	protected const int COMBO_OPTION_QUALITY = 0;
	protected const int COMBO_OPTION_PERFORMANCE = 1;
	
	//------------------------------------------------------------------------------------------------
	void SetConsolePreset(int presetIndex)
	{
		// TODO: marian: split into methods and use switch
		ArmaReforgerScripted game = GetGame();
		UserSettings engineUserSettings = game.GetEngineUserSettings();
		BaseContainer displayUserSettings = engineUserSettings.GetModule("DisplayUserSettings");
		UserSettings videoUserSettings = engineUserSettings.GetModule("VideoUserSettings");
		BaseContainer videoSettings = game.GetGameUserSettings().GetModule("SCR_VideoSettings");

		if (!displayUserSettings || !videoUserSettings || !videoSettings)
			return;

		if (presetIndex == EVideoQualityPreset.SERIES_S_PRESET_QUALITY)
		{
			displayUserSettings.Set("OverallQuality", EVideoQualityPreset.SERIES_S_PRESET_QUALITY);
			videoUserSettings.Set("ResolutionScale", 0.7);
			videoUserSettings.Set("Fsaa", 2);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			videoUserSettings.Set("MaxFps", 30);
			videoUserSettings.Set("Vsynch", true);
			videoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.SERIES_S_PRESET_QUALITY);
		}		
		else if (presetIndex == EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE)
		{
			displayUserSettings.Set("OverallQuality", EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE);
			videoUserSettings.Set("ResolutionScale", 0.60);
			videoUserSettings.Set("Fsaa", 0);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			videoUserSettings.Set("MaxFps", 60);
			videoUserSettings.Set("Vsynch", true);
			videoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE);
		}
		else if (presetIndex == EVideoQualityPreset.SERIES_X_PRESET_QUALITY)
		{
			displayUserSettings.Set("OverallQuality", EVideoQualityPreset.SERIES_X_PRESET_QUALITY);
			videoUserSettings.Set("ResolutionScale", 0.75);
			videoUserSettings.Set("Fsaa", 2);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.BOKEH);
			videoUserSettings.Set("MaxFps", 30);
			videoUserSettings.Set("Vsynch", true);
			videoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.SERIES_X_PRESET_QUALITY);
		}
		else if (presetIndex == EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE)
		{
			displayUserSettings.Set("OverallQuality", EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE);
			videoUserSettings.Set("ResolutionScale", 0.50);
			videoUserSettings.Set("Fsaa", 2);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			videoUserSettings.Set("MaxFps", 60);
			videoUserSettings.Set("Vsynch", true);
			videoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE);
		}
		
		GetGame().ApplySettingsPreset();
		GetGame().UserSettingsChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_SettingsManagerVideoModule()
	{
		SetModuleType(ESettingManagerModuleType.SETTINGS_MANAGER_VIDEO);
	}
}

enum EVideoQualityPreset
{
	SERIES_X_PRESET_QUALITY = 4,
	SERIES_S_PRESET_QUALITY = 5,
	SERIES_X_PRESET_PERFORMANCE = 6,
	SERIES_S_PRESET_PERFORMANCE = 7,
}
