/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Input
* @{
*/

sealed class InputManager: ActionManager
{
	private void InputManager();
	private void ~InputManager();
	
	//! Sets system cursor position
	proto external void SetCursorPosition(int x, int y);
	//! Returns true when mouse/keyboard is preferred input method
	proto external bool IsUsingMouseAndKeyboard();
	/**
	\brief Returns last dominant input device used by user.
	EInputDeviceType.MOUSE for standard mouse & keyboard combination
	EInputDeviceType.KEYBOARD for keyboard only (activated when using keyboard navigation in UI)
	EInputDeviceType.* for other devices
	*/
	proto external EInputDeviceType GetLastUsedInputDevice();
	proto external bool SetLastUsedInputDevice(EInputDeviceType type);
	//! Tells input manager, that is loading. IM than show hide cursor even without per frame updating
	proto external void SetLoading(bool isLoading);
	/**
	\brief Start to rumble a gamepad for specific user (if available). Intensty for low/high frequency is in range from 0 to 1
	*/
	proto external void SetGamepadRumble(int userIdx, float fLeftMotorSpeed, float fRightMotorSpeed, float fLeftTriggerSpeed, float fRightTriggerSpeed, int iDurationMs = -1, int iFadeInMs = 0, int iFadeOutMs = 0);
	//! use userIdx = -1 to stop rubmle for all controllers
	proto external void StopRumble(int userIdx = -1);
	//! Resets internal state of action
	proto external void ResetAction(string actionName);
	proto external bool RegisterActionManager(ActionManager pManager);
	proto external bool UnregisterActionManager(ActionManager pManager);
	proto external ref InputBinding CreateUserBinding();
};

/** @}*/
