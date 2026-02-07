class SCR_KeybindDialogs
{
	protected static const string DIALOGS_CONFIG = "{A24DA5B329261CB7}Configs/Dialogs/KeybindDialogs.conf";

	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateKeybindsResetDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "reset_keybinds");
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_SimpleKeybindDialogUI CreateSimpleKeybindDialog(SCR_KeyBindingEntry entry, string displayName, string actionName, InputBinding binding, EInputDeviceType device, bool showOverrideWarning = true)
	{
		string tag = "simple_keybind";
		if (device == EInputDeviceType.GAMEPAD)
			tag = "simple_keybind_gamepad";
		
		SCR_SimpleKeybindDialogUI dialog = new SCR_SimpleKeybindDialogUI(entry, displayName, actionName, binding, device, showOverrideWarning);
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, tag, dialog);

		return dialog;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_AdvancedKeybindDialogUI CreateAdvancedKeybindDialog(SCR_KeyBindingEntry entry, string displayName, string actionName, string actionPreset, SCR_EActionPrefixType prefixType)
	{
		SCR_AdvancedKeybindDialogUI dialog = new SCR_AdvancedKeybindDialogUI(entry, displayName, actionName, actionPreset, prefixType);
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "advanced_keybind", dialog);
		
		return dialog;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_KeybindDialogBase : SCR_ConfigurableDialogUi
{
	protected SCR_KeyBindingEntry m_Entry;
	protected string m_sActionName;
	protected string m_sDisplayName;
	
	protected EInputDeviceType m_eDevice;
	
	protected SCR_SettingsManagerKeybindModule m_SettingsKeybindModule;

	//-----------------------------------------------------------------------------------------------
	protected void Setup(SCR_KeyBindingEntry entry, string displayName, string actionName, EInputDeviceType device)
	{
		m_Entry = entry;
		
		m_sDisplayName = displayName;
		m_sActionName = actionName;
		m_eDevice = device;
		
		m_SettingsKeybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!m_SettingsKeybindModule)
		{
			Print("SCR_KeybindDialogBase::Keybind module for settings manager not found!", LogLevel.WARNING);
			return;
		}
	}
}