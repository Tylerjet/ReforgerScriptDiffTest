class SCR_ControllerPresetsSettingsSubmenu: SCR_SettingsSubMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
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
