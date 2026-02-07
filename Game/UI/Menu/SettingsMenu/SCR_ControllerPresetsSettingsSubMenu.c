class SCR_ControllerPresetsSettingsSubmenu: SCR_SettingsSubMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();

		SCR_HeadTrackingSettings.SetHeadTrackingSettings();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_aSettingsBindings.Clear();

		// HEAD TRACKING
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_bHeadTrackingFreelook", "HeadTrackingFreelook"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_bHeadTrackingADS", "HeadTrackingADS"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingSensitivity", "HeadTrackingSensitivity"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingADSDeadzone", "HeadTrackingADSDeadzone"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_eHeadTrackingLean", "HeadTrackingLean"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingLeanSensitivity", "HeadTrackingLeanSensitivity"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingLeanDeadzone", "HeadTrackingLeanDeadzone"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingLeanYawLimit", "HeadTrackingLeanYawLimit"));

		LoadSettings();

		// PRESET
		BindControllerPresets();

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_UI_SHOW_ALL_SETTINGS, false))
			return;

		// Show only if there is TrackIR connected
		if (!GetGame().GetInputManager().IsTrackIRConnected())
		{
			HideMenuItem("TitleHeadTracking");

			HideMenuItem("HeadTrackingFreelook");
			HideMenuItem("HeadTrackingADS");
			HideMenuItem("HeadTrackingSensitivity");
			HideMenuItem("HeadTrackingADSDeadzone");

			HideMenuItem("HeadTrackingLean");
			HideMenuItem("HeadTrackingLeanSensitivity");
			HideMenuItem("HeadTrackingLeanDeadzone");
			HideMenuItem("HeadTrackingLeanYawLimit");
		}
	}

	//------------------------------------------------------------------------------------------------
	void BindControllerPresets()
	{
		Widget presetsWidget = m_wRoot.FindAnyWidget("Presets");
		if (!presetsWidget)
			return;
		
		SCR_ComboBoxComponent comboComp = SCR_ComboBoxComponent.Cast(presetsWidget.FindHandler(SCR_ComboBoxComponent));
		if (!comboComp)
			return;
		
		SCR_SettingsManagerKeybindModule keybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!keybindModule)
			return;
		
		array<ref SCR_ControllerPreset> presets = keybindModule.GetControllerPresets();
		
		foreach(SCR_ControllerPreset preset : presets)
		{
			comboComp.AddItem(preset.GetDisplayName());
		}
		
		comboComp.SetCurrentItem(keybindModule.GetActivePresetIndex());
		comboComp.m_OnChanged.Insert(SetControllerPreset);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetControllerPreset(SCR_ComboBoxComponent comp, int index)
	{
		SCR_SettingsManagerKeybindModule keybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!keybindModule)
			return;
		
		keybindModule.SetControllerPreset(index);
	}
}
