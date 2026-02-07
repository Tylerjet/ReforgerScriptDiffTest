/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Input
\{
*/

sealed class InputManager: ActionManager
{
	private void InputManager();
	private void ~InputManager();

	//! Sets system cursor position
	proto external void SetCursorPosition(int x, int y);
	//! Returns true when mouse/keyboard is preferred input method
	proto external bool IsUsingMouseAndKeyboard();
	/*!
	\brief Returns last dominant input device used by user.
	EInputDeviceType.MOUSE for standard mouse & keyboard combination
	EInputDeviceType.KEYBOARD for keyboard only (activated when using keyboard navigation in UI)
	EInputDeviceType.* for other devices
	*/
	proto external EInputDeviceType GetLastUsedInputDevice();
	proto external bool SetLastUsedInputDevice(EInputDeviceType type);
	//! Tells input manager, that is loading. IM than show hide cursor even without per frame updating
	proto external void SetLoading(bool isLoading);
	/*!
	\brief Start to rumble a gamepad for specific user (if available). Intensty for low/high frequency is in range from 0 to 1
	*/
	proto external void SetGamepadRumble(int userIdx, float fLeftMotorSpeed, float fRightMotorSpeed, float fLeftTriggerSpeed, float fRightTriggerSpeed, int iDurationMs = -1, int iFadeInMs = 0, int iFadeOutMs = 0);
	//! use userIdx = -1 to stop rubmle for all controllers
	proto external void StopRumble(int userIdx = -1);
	//! Resets internal state of action
	proto external void ResetAction(string actionName);
	//! Resets internal state of all actions in context
	proto external void ResetContext(string contextName);
	proto external bool RegisterActionManager(ActionManager pManager);
	proto external bool UnregisterActionManager(ActionManager pManager);
	proto external ref InputBinding CreateUserBinding();
	/*
	Return config for given key. Config is set in project settings (.gproj file) InputManagerSettings.UiMappings
	\code
	BaseContainer container = GetGame().GetInputManager().GetKeyUIMapping("keyboard:KC_G");
	if (container)
	{
		string label;
		container.Get("Label", label);
		Print("Key label for UI: " + label);
	}
	\endcode
	*/
	proto external BaseContainer GetKeyUIMapping(string keyName);
	/*
	Return key binding for given action. Key binding is a tree of combos and alternatives. Method returned key binding tree encoded as binary expression tree (https://en.wikipedia.org/wiki/Binary_expression_tree).
	Process from right to left.
	Example: for key binding "P or (CTRL + G)" is keyStack looks like {'KC_G', 'KC_CTRL', '+', 'KC_P', '|'}

	\param actionName name of action
	\param keyStack stack for keys. Can be: '+' for input combo (binary node followed by two child nodes) or '|' for input sum/alternative (binary node followed by two child nodes) otherwise contains key code (leaf node)
	\param filterStack contains pointer to InputFilter container for key with the same index on keyStack (size of filterStack is always same as keyStack)
	\param deviceType filter just key binds for specific device. Leave EInputDeviceType.INVALID to return key binding for last used device
	\param preset filter just key binds for specific preset. Leave empty to any preset
	\param keyBindIndex filter just key bind with given index. Leave -1 to all key binds

	\code
	// Iterate trough key bind stack
	string ProcessKeybindStack(inout int index, notnull array<string> keyStack)
	{
		if (index < 0)
			return string.Empty;

		// pop back
		string key = keyStack[index];
		index--;

		switch(key)
		{
			case "+": // combo
				return ProcessKeybindStack(index, keyStack) + " + " + ProcessKeybindStack(index, keyStack);

			case "|": // sum/alternative
				return ProcessKeybindStack(index, keyStack) + " | " + ProcessKeybindStack(index, keyStack);

			default: // key value
				return key;
		}

		return string.Empty;
	}

	void DoKeybindTest()
	{
		array<string> keyStack = {};
		array<BaseContainer> filterStack ={};
		if (GetGame().GetInputManager().GetActionKeybinding("ShowGroupMenu", keyStack, filterStack))
		{
			int index = keyStack.Count() - 1;
			Print("Key bind: " + ProcessKeybindStack(index, keyStack))
		}
	}
	\endcode
	*/
	proto external bool GetActionKeybinding(string actionName, notnull array<string> keyStack, notnull array<BaseContainer> filterStack, EInputDeviceType deviceType = EInputDeviceType.INVALID, string preset = string.Empty, int keyBindIndex = -1);
}

/*!
\}
*/
