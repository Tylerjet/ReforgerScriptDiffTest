//------------------------------------------------------------------------------------------------
class SCR_SettingsManagerKeybindModule : SCR_SettingsManagerModuleBase
{
	protected ref InputBinding m_Binding;
	
	protected ref SCR_KeyBindingMenuConfig m_KeybindConfig;
	
	protected static const string KEY_BINDING_CONFIG = "{4EE7794C9A3F11EF}Configs/System/keyBindingMenu.conf";
	
	//------------------------------------------------------------------------------------------------
	void SCR_SettingsManagerKeybindModule()
	{
		//load keybinding confis
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
		
		SetModuleType(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING);
		m_Binding = GetGame().GetInputManager().CreateUserBinding();
		if (!m_Binding)
		{
			Print("SCR_SettingsManagerKeybindModule: InputBindings were not created!", LogLevel.WARNING);
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
	void SetFilterForActionByIndex(string actionName, string actionPreset, int filterIndex, int keybindIndex, EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{
		if (!m_Binding)
			return;
		
		array<ref SCR_KeyBindingFilter> filters = GetFilters();
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
		
		m_Binding.SetFilter(actionName, device, actionPreset, keybindIndex, filter.filterString);
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
		
		m_Binding.AddBinding(actionName, preset, bind.bindString, filterName);
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_KeyBindingFilter> GetFilters()
	{
		if (!m_KeybindConfig)
			return null;
		
		return m_KeybindConfig.inputFilters;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_KeyBindingBind> GetCustomBinds()
	{
		if (!m_KeybindConfig)
			return null;
		
		return m_KeybindConfig.inputBinds;
	}
	
	//------------------------------------------------------------------------------------------------
	//! returns false if the action has no conflicts (no other action in keybind settings with same bind), otherwise returns those actions in confirmedConflicts
	bool IsActionConflicted(string actionName, notnull out array<SCR_KeyBindingEntry> confirmedConflicts, int bindIndex, string preset = string.Empty)
	{
		if (!m_Binding || actionName.IsEmpty())
			return false;
		
		array<int> indices = {};
		array<string> conflicts = {};
		array<string> presets = {};
		m_Binding.GetConflicts(actionName, indices, conflicts, presets, EInputDeviceType.KEYBOARD, preset);
		if (conflicts.IsEmpty())
			return false;
		
		array<string> actionBinds = {};
		m_Binding.GetBindings(actionName, actionBinds, EInputDeviceType.KEYBOARD, preset);
		
		//TEMPORARY solution that will be solved as soon as we have updated GetConflicts from enfusion. Right now this prevents nulls because GetBindings do not return mouse binds
		if (bindIndex > actionBinds.Count() - 1)
			return false;
		
		string searchedBind = actionBinds.Get(bindIndex);
		
		array<SCR_KeyBindingEntry> foundActions = {};
		
		//compare the actions against those configures in keybinding config, and save only those, to avoid actions for UI etc
		foreach (string action : conflicts)
		{
			foreach (SCR_KeyBindingCategory category : m_KeybindConfig.keyBindingCategories)
			{
				foreach (SCR_KeyBindingEntry entry : category.keyBindingEntries)
				{
						if (entry.actionName == "separator" || entry.actionName != action)
						continue;
					
					foundActions.Insert(entry);
				}
			}
		}
		
		//compare the found actions conflict back against the tested action, to eliminate duplicate actions like CharacterForward that have single action for two keybinds
		conflicts.Clear();
		foreach (SCR_KeyBindingEntry confirmedEntry : foundActions)
		{
			m_Binding.GetConflicts(confirmedEntry.actionName, indices, conflicts, presets, EInputDeviceType.KEYBOARD, confirmedEntry.preset);
			
			foreach (string conflictedAction : conflicts)
			{
				if (conflictedAction == actionName)
				{
					actionBinds.Clear();
					m_Binding.GetBindings(confirmedEntry.actionName, actionBinds, EInputDeviceType.KEYBOARD, confirmedEntry.preset);
					
					//check if the action conflicts on the specific keybind we are checking
					if (actionBinds.Find(searchedBind) != -1)
						confirmedConflicts.Insert(confirmedEntry);
				}
			}
			
		}
		
		return !confirmedConflicts.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	void UnbindAction(string actionName, string preset = "", EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{		
		m_Binding.StartCapture(actionName, EInputDeviceType.KEYBOARD, preset, false);
		m_Binding.SaveCapture();
		m_Binding.Save();
	}
}