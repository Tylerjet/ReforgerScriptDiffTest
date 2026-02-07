//------------------------------------------------------------------------------------------------
/*!
	Class generated via ScriptWizard.
*/
class SCR_KeybindRowComponent : ScriptedWidgetComponent
{
	protected static Widget s_RootWidget;
	protected static SCR_KeybindSetting s_KeybindMenuComponent;
	protected static ref InputBinding s_Binding;
	protected static const string CHANGE_KEYBIND_DIALOG_MESSAGE = "#AR-Settings_Keybind_keybindPressPrompt";
	protected static const string NO_KEYBIND = "<#AR-Settings_Keybind_NoKeybind>";
	protected static const string DIALOGWINDOW_LAYOUT_PATH = "{1123A3569ACDCDEC}/UI/layouts/Menus/Dialogs/DialogBase.layout";
	protected Widget m_ParentWidget;

	protected string m_sActionName;
	protected string m_sActionDisplayName;
	protected string m_sActionPreset;

	//------------------------------------------------------------------------------------------------
	void Create(Widget parentWidget, string actionDisplayName, string actionName, SCR_KeybindSetting menuComponent, string preset, Widget rootWidget, InputBinding binding)
	{
		//set up globals
		s_RootWidget = rootWidget;
		m_sActionPreset = preset;
		m_sActionName = actionName;
		s_KeybindMenuComponent = menuComponent;
		m_sActionDisplayName = actionDisplayName;
		m_ParentWidget = parentWidget;
		s_Binding = binding;

		RichTextWidget textBox = RichTextWidget.Cast(m_ParentWidget.FindAnyWidget("Name"));
		textBox.SetText(actionDisplayName);
		textBox.ElideText(1, 1.0, "...");

		SetRichTextAction("PCBind", EInputDeviceType.KEYBOARD, true, actionName, preset);
		SetRichTextAction("ControllerBind", EInputDeviceType.GAMEPAD, false, actionName, preset);
#ifdef PLATFORM_CONSOLE
		ButtonWidget PCKeybind = ButtonWidget.Cast(parentWidget.FindAnyWidget("PCBind"));
		if (PCKeybind)
			PCKeybind.SetOpacity(0);
#endif
	}

	//------------------------------------------------------------------------------------------------
	protected static string GetActionMainKeybind(string findActionName, string preset)
	{
		BaseContainer ctx;
		BaseContainerList list;
		array<string> actionNames;
		BaseContainer action;
		array<string> contexts = {};
		s_Binding.GetContexts(contexts);
		string keybind;
		string contextActionName;

		//go through the inputconfig and find the one for this button
		foreach (string ctxName: contexts)
		{
			ctx = s_Binding.FindContext(ctxName);
			actionNames = new array<string>();
			ctx.Get("ActionRefs", actionNames);
			foreach (string actionName: actionNames)
			{
				keybind = GetActionKeybindText(actionName, findActionName, preset);
				if (!keybind.IsEmpty())
					return keybind;
			}

			list = ctx.GetObjectArray("Actions");
			//go through contexts separately, because their structure differs
			for (int j = 0; j < list.Count(); j++)
			{
				action = list.Get(j);
				contextActionName = action.GetName();
				keybind = GetActionKeybindText(contextActionName, findActionName, preset);
				if (!keybind.IsEmpty())
					return keybind;
			}
		}
		return NO_KEYBIND;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonClick()
	{
		//in case of gamepad just do nothing, because we do not support keybind changes on gamepad
		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.GAMEPAD)
			return;
		s_Binding.StartCapture(m_sActionName, EInputDeviceType.KEYBOARD, m_sActionPreset);

		KeybindMenu menu = KeybindMenu.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.KeybindChangeDialog , DialogPriority.CRITICAL, 0, true));
		menu.SetMessage(m_sActionDisplayName);
		menu.SetKeybind(s_Binding, s_KeybindMenuComponent);
	}

	//------------------------------------------------------------------------------------------------
	protected static string GetActionKeybindText(string actionName, string findAction, string preset)
	{
		array<string> presets = {};
		array<string> binding = {};
		s_Binding.GetPresets(actionName, presets);
		int numPresets = presets.Count();
		int filter = -1;
		if (actionName == findAction)
		{
			if (numPresets)
			{
				for (int i = 0; i < numPresets; i++)
				{
					binding.Clear();
					s_Binding.GetBindings(actionName, binding, EInputDeviceType.MOUSE, preset);
					s_Binding.GetBindings(actionName, binding, EInputDeviceType.KEYBOARD, preset);
				}
			}
			else
			{
				s_Binding.GetBindings(actionName, binding, EInputDeviceType.MOUSE);
				s_Binding.GetBindings(actionName, binding, EInputDeviceType.KEYBOARD);
			}
			if (binding.Count() > 0)
				return binding.Get(0); //temporary solution because we dont do alternate keybinds yet
		}
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetRichTextAction(string widgetName, EInputDeviceType device, bool canChangeKeybind, string actionName, string preset)
	{
		string deviceString = "keyboard";
		ButtonWidget keybindButton = ButtonWidget.Cast(m_ParentWidget.FindAnyWidget(widgetName));
		RichTextWidget keybindAction = RichTextWidget.Cast( keybindButton.FindAnyWidget("Text"));
		if (device == EInputDeviceType.GAMEPAD)
			deviceString = "gamepad";

		keybindAction.SetText(string.Format("<action name='%1' preset='%2' device='" + deviceString + "' scale='1.25'/>", actionName, preset));
		if (canChangeKeybind)
		{
			SCR_ButtonTextComponent bindComponent = SCR_ButtonTextComponent.Cast(keybindButton.FindHandler(SCR_ButtonTextComponent));
			bindComponent.m_OnClicked.Insert(OnButtonClick);
		}
	}
};
