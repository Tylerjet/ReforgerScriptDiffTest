class SCR_KeybindSetting : SCR_SettingsSubMenuBase
{
	protected ref SCR_KeyBindingMenuConfig m_KeybindConfig;

	// Resources
	protected static const string ACTIONROW_LAYOUT_PATH = "{75B1F7B766CA8C91}UI/layouts/Menus/SettingsMenu/BindingMenu/KeybindRow.layout";
	protected static const string SEPARATOR_LAYOUT_PATH = "{01D9FD7791700ADA}UI/layouts/Menus/SettingsMenu/BindingMenu/KeybindSeparator.layout";
	protected static const string KEY_BINDING_CONFIG = "{4EE7794C9A3F11EF}Configs/System/keyBindingMenu.conf";

	// Widgets
	protected VerticalLayoutWidget m_wActionsLayout;

	// Bindings
	protected static ref InputBinding s_Binding;
	protected static const string PRIMARY_PRESET_PREFIX = "primary:";

	// Strings (should be localised)
	protected static const string RESET_ALL_DIALOG_TITLE = "#AR-Settings_Keybind_WarningResetAll";
	protected static const string RESET_ALL_DIALOG_MESSAGE = "#AR-Settings_Keybind_MessageResetAll";
	
	protected SCR_KeybindRowComponent m_SelectedRowComponent;
	protected SCR_NavigationButtonComponent m_ResetSingleButtonComponent;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		m_wActionsLayout = VerticalLayoutWidget.Cast(GetRootWidget().FindAnyWidget("ActionRowsContent"));
		if (!m_wActionsLayout)
			return;

		s_Binding = GetGame().GetInputManager().CreateUserBinding();
		SCR_NavigationButtonComponent reset = CreateNavigationButton("MenuRefresh", "#AR-Settings_Keybind_ResetEveryKeybind", true);
		reset.m_OnActivated.Insert(ResetKeybindsToDefault);
		m_ResetSingleButtonComponent = CreateNavigationButton("MenuResetKeybind", "#AR-Settings_Keybind_ResetAllKeybinds", true);
		m_ResetSingleButtonComponent.m_OnActivated.Insert(ResetSingleKeybindToDefault);
		m_ResetSingleButtonComponent.SetEnabled(false);

		//read the categories and actions from KEY_BINDING_CONFIG
		Resource holder = BaseContainerTools.LoadContainer(KEY_BINDING_CONFIG);
		BaseContainer container = holder.GetResource().ToBaseContainer();
		m_KeybindConfig = SCR_KeyBindingMenuConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		InsertCategoriesToComboBox();
		ListActionsFromCurrentCategory();
	}

	//------------------------------------------------------------------------------------------------
	void ListActionsFromCurrentCategory()
	{
		Widget spinBox = GetRootWidget().FindAnyWidget("CategoriesBox");
		bool firstSeparator = true;
		if (!spinBox)
			return;

		SCR_SpinBoxComponent spinBoxComponent = SCR_SpinBoxComponent.Cast(spinBox.GetHandler(0));
		if (!spinBoxComponent)
			return;

		SCR_KeyBindingCategory category = m_KeybindConfig.keyBindingCategories.Get(spinBoxComponent.GetCurrentIndex());
		if (!category)
			return;

		Widget rowToDelete = m_wActionsLayout.GetChildren();
		while (rowToDelete)
		{
			m_wActionsLayout.RemoveChild(rowToDelete);
			rowToDelete = m_wActionsLayout.GetChildren();
		}

		string displayName;
		Widget separator;
		Widget keybindTitle;
		Widget gamepadTitle;
		TextWidget separatorText;
		Widget actionRowWidget;
		SCR_KeybindRowComponent component;
		foreach (SCR_KeyBindingEntry entry : category.keyBindingEntries)
		{
			displayName = entry.displayName;
			if (displayName.Length() == 0)
				displayName = "<#AR-Settings_Keybind_MissingName>" + entry.actionName;

			if (entry.actionName == "separator")
			{
				separator = GetGame().GetWorkspace().CreateWidgets(SEPARATOR_LAYOUT_PATH ,m_wActionsLayout);
				if (!separator)
					continue;
				separatorText = TextWidget.Cast(separator.FindAnyWidget("ActionCategoryName"));
				if (!separatorText)
					continue;
				separatorText.SetVisible(!entry.displayName.IsEmpty());
				separatorText.SetText(entry.displayName);
				if (firstSeparator)
				{
					keybindTitle = separator.FindAnyWidget("Keybind");
					gamepadTitle = separator.FindAnyWidget("Gamepad");
					if (keybindTitle)
						keybindTitle.SetVisible(true);
					if (gamepadTitle)
						gamepadTitle.SetVisible(true);
					firstSeparator = false;
#ifdef PLATFORM_CONSOLE
					keybindTitle.SetOpacity(0);
#endif
				}
			}
			else
			{
				actionRowWidget = GetGame().GetWorkspace().CreateWidgets(ACTIONROW_LAYOUT_PATH ,m_wActionsLayout);
				component = SCR_KeybindRowComponent.Cast(actionRowWidget.FindHandler(SCR_KeybindRowComponent));
				if (component)
					component.Create(actionRowWidget, displayName, entry.actionName, this, entry.preset,  GetRootWidget(), s_Binding);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void InsertCategoriesToComboBox()
	{
		Widget spinBox = GetRootWidget().FindAnyWidget("CategoriesBox");
		if (!spinBox)
			return;

		SCR_SpinBoxComponent spinBoxComponent = SCR_SpinBoxComponent.Cast(spinBox.GetHandler(0));
		if (!spinBoxComponent)
			return;

		spinBoxComponent.ClearAll();
		spinBoxComponent.m_OnChanged.Insert(ListActionsFromCurrentCategory);

		foreach (SCR_KeyBindingCategory category : m_KeybindConfig.keyBindingCategories)
			spinBoxComponent.AddItem(category.displayName);

		spinBoxComponent.SetCurrentItem(0);

	}

	//------------------------------------------------------------------------------------------------
	protected void ResetKeybindsToDefault()
	{
		DialogUI menu = DialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ConfirmationDialog , DialogPriority.CRITICAL, 0, true));
		menu.SetTitle(RESET_ALL_DIALOG_TITLE);
		menu.SetMessage(RESET_ALL_DIALOG_MESSAGE);
		menu.m_OnConfirm.Insert(ResetKeybindsToDefaultConfirm);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetSingleKeybindToDefault()
	{
		if (!m_SelectedRowComponent)
			return;
		m_SelectedRowComponent.ResetAction();
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetKeybindsToDefaultConfirm()
	{
		array<string> contexts = {};
		s_Binding.GetContexts(contexts);
		
		foreach (SCR_KeyBindingCategory category: m_KeybindConfig.keyBindingCategories)
		{
			foreach (SCR_KeyBindingEntry entry: category.keyBindingEntries)
			{
				string finalPreset = entry.preset;
				if (!entry.preset.IsEmpty())
					finalPreset = PRIMARY_PRESET_PREFIX + entry.preset;
				
				s_Binding.ResetDefault(entry.actionName, EInputDeviceType.KEYBOARD, finalPreset);
				s_Binding.ResetDefault(entry.actionName, EInputDeviceType.MOUSE, finalPreset);
			}
		}
		ListActionsFromCurrentCategory();
		s_Binding.Save();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
		ListActionsFromCurrentCategory();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_KeybindRowComponent GetSelectedRowComponent()
	{
		return m_SelectedRowComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSelectedRowComponent(SCR_KeybindRowComponent rowComponent)
	{
		m_SelectedRowComponent = rowComponent;
	}
	
	void SingleResetEnabled(bool isEnabled)
	{
		if (m_ResetSingleButtonComponent)
			m_ResetSingleButtonComponent.SetEnabled(isEnabled);
	}
};
