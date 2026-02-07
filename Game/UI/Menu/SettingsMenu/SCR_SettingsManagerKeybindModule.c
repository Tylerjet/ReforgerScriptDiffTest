//------------------------------------------------------------------------------------------------
class SCR_SettingsManagerKeybindModule : SCR_SettingsManagerModuleBase
{
	protected const string GAMEPAD_PRESET_PREFIX = "gamepad:";
	protected const string PRIMARY_PRESET_PREFIX = "";
	
	protected ref InputBinding m_Binding;
	
	protected ref SCR_KeyBindingMenuConfig m_KeybindConfig;
	protected ref SCR_ControllerPresetsConfig m_ControllerPresetsConfig;
	
	protected static const string KEY_BINDING_CONFIG = "{4EE7794C9A3F11EF}Configs/System/keyBindingMenu.conf";
	
	protected static const string CONTROLLER_PRESETS_CONFIG = "{27780DD27C5E97CF}Configs/System/ControlSchemes/Gamepad/ControllerPresets.conf";
	
	//------------------------------------------------------------------------------------------------
	void SCR_SettingsManagerKeybindModule()
	{
		LoadKeybindConfig();
		LoadControllerPresetsConfig();
		
		SetModuleType(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING);
		m_Binding = GetGame().GetInputManager().CreateUserBinding();
		if (!m_Binding)
		{
			Print("SCR_SettingsManagerKeybindModule: InputBindings were not created!", LogLevel.WARNING);
			return;
		}
	}
	
	void LoadKeybindConfig()
	{
		Resource holder = BaseContainerTools.LoadContainer(KEY_BINDING_CONFIG);
		if (!holder)
		{
			Print("SCR_SettingsManagerKeybindModule: Loading of keybinding config failed!", LogLevel.WARNING);
			return;		
		}
		
		BaseContainer container = holder.GetResource().ToBaseContainer();
		if (!container)
		{
			Print("SCR_SettingsManagerKeybindModule: Loading of keybinding config failed!", LogLevel.WARNING);
			return;		
		}
		
		m_KeybindConfig = SCR_KeyBindingMenuConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!m_KeybindConfig)
		{
			Print("SCR_SettingsManagerKeybindModule: Loading of keybinding config failed!", LogLevel.WARNING);
			return;		
		}
	}
	
	void LoadControllerPresetsConfig()
	{
		Resource holder = BaseContainerTools.LoadContainer(CONTROLLER_PRESETS_CONFIG);
		if (!holder || !holder.IsValid())
		{
			Print("SCR_SettingsManagerKeybindModule: Loading of controller presets config failed!", LogLevel.WARNING);
			return;		
		}
		
		BaseContainer container = holder.GetResource().ToBaseContainer();
		if (!container)
		{
			Print("SCR_SettingsManagerKeybindModule: Loading of controller presets config failed!", LogLevel.WARNING);
			return;		
		}
		
		//if we only had pointers :(
		m_ControllerPresetsConfig = SCR_ControllerPresetsConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!m_ControllerPresetsConfig)
		{
			Print("SCR_SettingsManagerKeybindModule: Loading of controller presets config failed!", LogLevel.WARNING);
			return;		
		}
	}
	
	//------------------------------------------------------------------------------------------------
	int GetActionBindCount(string actionName, string actionPreset = string.Empty, EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{
		if (!m_Binding)
			return 0;
	
		return m_Binding.GetBindingsCount(actionName, device, actionPreset);
	}
	
	//------------------------------------------------------------------------------------------------
	InputBinding GetInputBindings()
	{
		return m_Binding;
	}
	
	//------------------------------------------------------------------------------------------------
	void DeleteActionBindByIndex(string actionName, int keybindIndex, string actionPreset = "", EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{
		if (!m_Binding)
			return;
		
		if (m_Binding.IsDefault(actionName, device, actionPreset))
		{
			// if it is default binding first create user binding
			m_Binding.CreateUserBinding(actionName, device, actionPreset);
			m_Binding.Save();
		}
		
		m_Binding.RemoveBinding(actionName, device, actionPreset, keybindIndex);
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	void StartCaptureForAction(string actionName, string actionPreset, EInputDeviceType device = EInputDeviceType.KEYBOARD, bool append = false)
	{
		if (!m_Binding)
			return;

		m_Binding.StartCapture(actionName, device, actionPreset, append);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFilterForActionByIndex(string actionName, string actionPreset, int filterIndex, int keybindIndex, SCR_EActionPrefixType prefixType, EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{
		if (!m_Binding)
			return;
		
		array<ref SCR_KeyBindingFilter> filters = GetFilters(prefixType);
		if (!filters)
			return;
		
		SCR_KeyBindingFilter filter = filters.Get(filterIndex);
		if (!filter)
			return;
		
		if (m_Binding.IsDefault(actionName, device, actionPreset))
		{
			// if it is default binding first create user binding
			m_Binding.CreateUserBinding(actionName, device, actionPreset);
			m_Binding.Save();
		}
		
		m_Binding.SetFilter(actionName, device, actionPreset, keybindIndex, filter.m_sFilterString);
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	void AddKeybindToActionByIndex(string actionName, string preset, int bindIndex, string filterName = string.Empty)
	{
		if (!m_Binding)
			return;
		
		array<ref SCR_KeyBindingBind> binds = GetCustomBinds();
		if (!binds)
			return;
		
		SCR_KeyBindingBind bind = binds.Get(bindIndex);
		if (!bind)
			return;
		
		m_Binding.AddBinding(actionName, preset, bind.m_sBindString, filterName);
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_KeyBindingFilter> GetFilters(SCR_EActionPrefixType filterType)
	{
		if (!m_KeybindConfig)
			return null;
		
		array <ref SCR_KeyBindingFilter> foundFilters = {};
		foreach (SCR_KeyBindingFilter filter : m_KeybindConfig.m_aInputFilters)
		{
			if (filterType == SCR_EActionPrefixType.AMBIGUOUS || filter.GetFilterType() == filterType)
				foundFilters.Insert(filter);
		}
		
		return foundFilters;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_KeyBindingBind> GetCustomBinds()
	{
		if (!m_KeybindConfig)
			return null;
		
		return m_KeybindConfig.m_aInputBinds;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_KeyBindingCombo> GetCustomComboKeys()
	{
		if (!m_KeybindConfig)
			return null;
		
		return m_KeybindConfig.m_aComboKeys;
	}
	
	//------------------------------------------------------------------------------------------------
	//! returns false if the action has no conflicts (no other action in keybind settings with same bind), otherwise returns those actions in confirmedConflicts
	bool IsActionConflicted(string actionName, notnull out array<SCR_KeyBindingEntry> confirmedConflicts, int bindIndex, string preset = string.Empty)
	{
		if (!m_Binding || actionName.IsEmpty())
			return false;
		
		confirmedConflicts.Clear();
		
		array<int> indices = {};
		array<string> conflicts = {};
		array<string> presets = {};
		m_Binding.GetConflicts(actionName, indices, conflicts, presets, EInputDeviceType.KEYBOARD, preset);
		
		if (conflicts.IsEmpty())
			return false;
		
		array<SCR_KeyBindingEntry> foundActions = {};
		
		//compare the actions against those configures in keybinding config, and save only those, to avoid actions for UI etc
		foreach (int index, string action : conflicts)
		{
			if (indices[index] != bindIndex)
				continue;
			
			foreach (SCR_KeyBindingCategory category : m_KeybindConfig.m_KeyBindingCategories)
			{
				foreach (SCR_KeyBindingEntry entry : category.m_KeyBindingEntries)
				{
					if (entry.m_sActionName == "separator" || entry.m_sActionName != action || entry.m_sPreset != presets[index]) 
						continue;
					
					foundActions.Insert(entry);
				}
			}
		}
				
		confirmedConflicts.Copy(foundActions);
		
		return !foundActions.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	void UnbindAction(string actionName, string preset = "", EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{	
		m_Binding.StartCapture(actionName, device, preset, false);
		m_Binding.SaveCapture();
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	void AddComboToActionByIndex(string actionName, string preset, int comboIndex, int keybindIndex, string filterName = string.Empty, int comboPosition = 0)
	{
		if (!m_Binding)
			return;
		
		array<ref SCR_KeyBindingCombo> combos = GetCustomComboKeys();
		if (!combos)
			return;
		
		SCR_KeyBindingCombo combo = combos.Get(comboIndex);
		if (!combo)
			return;
		
		m_Binding.InsertCombo(actionName, preset, combo.m_sComboString, filterName, keybindIndex, comboPosition);
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetAction(string actionName, string preset = "", EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{
		if (!m_Binding)
			return;
		
		bool reset;
		switch (device)
		{
			case EInputDeviceType.KEYBOARD:
			case EInputDeviceType.MOUSE:
			{
				if (!m_Binding.IsDefault(actionName, EInputDeviceType.KEYBOARD, preset))
				{
					m_Binding.ResetDefault(actionName, EInputDeviceType.KEYBOARD, preset);
					reset = true;
				}

				if (!m_Binding.IsDefault(actionName, EInputDeviceType.MOUSE, preset))
				{
					m_Binding.ResetDefault(actionName, EInputDeviceType.MOUSE, preset);
					reset = true;
				}

				break;
			}
			case EInputDeviceType.GAMEPAD:
			{
				if (!m_Binding.IsDefault(actionName, EInputDeviceType.GAMEPAD, preset))
				{
					m_Binding.ResetDefault(actionName, EInputDeviceType.GAMEPAD, preset);
					reset = true;
				}

				break;
			}
		}
		
		if (!reset)
			return;

		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetAllActions(EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{
		array<string> contexts = {};
		m_Binding.GetContexts(contexts);
		string finalPreset;
		string devicePrefix;
		string actionName;
		
		foreach (SCR_KeyBindingCategory category: m_KeybindConfig.m_KeyBindingCategories)
		{
			foreach (SCR_KeyBindingEntry entry: category.m_KeyBindingEntries)
			{
				if (device == EInputDeviceType.GAMEPAD)
					devicePrefix = GetGamepadPresetPrefix();
				else 
					devicePrefix = GetPrimaryPresetPrefix();
				
				if (device == EInputDeviceType.GAMEPAD && !entry.m_sPresetGamepadOptional.IsEmpty())
					finalPreset = devicePrefix + entry.m_sPresetGamepadOptional;
				else if (!entry.m_sPreset.IsEmpty())
					finalPreset = devicePrefix + entry.m_sPreset;
				
				if (device == EInputDeviceType.GAMEPAD && !entry.m_sActionNameGamepadOptional.IsEmpty())
					actionName = entry.m_sActionNameGamepadOptional;
				else
					actionName = entry.m_sActionName;

				ResetAction(actionName, finalPreset, device);
			}
		}
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	string GetGamepadPresetPrefix()
	{
		return GAMEPAD_PRESET_PREFIX;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetPrimaryPresetPrefix()
	{
		return PRIMARY_PRESET_PREFIX;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ControllerPreset> GetControllerPresets()
	{
		if (m_ControllerPresetsConfig)
			return m_ControllerPresetsConfig.GetPresets();
		
		Print("SCR_SettingsManagerKeybindModule: Controller preset not present, check init process!", LogLevel.ERROR);
		return null;	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetControllerPreset(int presetIndex)
	{
		if (!m_Binding)
			return;
		
		array<ref SCR_ControllerPreset> presets = GetControllerPresets();
		if (!presets || presets.IsEmpty() || presets.Count() <= presetIndex)
			return;
		
		array<ResourceName> presetToSet = {};
		
		//empty preset config means we set it to default
		if (!presets.Get(presetIndex).GetResourceName().IsEmpty())
			presetToSet.Insert(presets.Get(presetIndex).GetResourceName());
		
		m_Binding.SetCustomConfigs(presetToSet);
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetActivePresetIndex()
	{
		if (!m_Binding)
			return -1;
		
		array<ResourceName> setPresets = {};
		m_Binding.GetCustomConfigs(setPresets);
		if (setPresets.IsEmpty())
			return 0;
		
		array<ref SCR_ControllerPreset> presets = GetControllerPresets();
		
		foreach (int i, SCR_ControllerPreset preset : presets)
		{
			if (preset.GetResourceName() == setPresets.Get(0))
				return i;
		}
		
		return -1;
	}
}