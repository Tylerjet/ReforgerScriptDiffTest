/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Input
* @{
*/

class InputBinding
{
	private void InputBinding();
	
	proto external void ResetDefault(string actionName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	proto external bool IsDefault(string actionName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	proto external void StartCapture(string actionName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty, bool bAppend = false);
	proto external void CancelCapture();
	//! Additional key bindings can be added using additionalKeyBindings array.
	proto external void SaveCapture(array<string> additionalKeyBindings = null);
	proto external string GetActionFilter(string actionName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	proto external void SetActionFilter(string actionName, string filterClassName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	proto external EInputBindingCaptureState GetCaptureState();
	proto external BaseContainer FindContext(string contextName);
	proto external BaseContainer FindAction(string actionName);
	proto external void Save();
	proto external bool GetBindings(string actionName, out notnull array<string> bindings, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	proto external void GetContexts(out array<string> contextNames);
	proto external void GetPresets(string actionName, out array<string> presetNames);
};

/** @}*/
