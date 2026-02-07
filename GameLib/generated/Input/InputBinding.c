/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Input
\{
*/

class InputBinding
{
	private void InputBinding();

	proto external void ResetDefault(string actionName, EInputDeviceType deviceType = EInputDeviceType.INVALID, string preset = string.Empty);
	proto external bool IsDefault(string actionName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	proto external bool CreateUserBinding(string actionName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	proto external int GetBindingsCount(string actionName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	proto external void RemoveBinding(string actionName, EInputDeviceType deviceType, string preset, int keyBindIndex);
	proto external void AddBinding(string actionName, string preset, string keyBinding, string filterName = string.Empty);
	proto external void InsertCombo(string actionName, string preset, string keyBinding, string filterName, int keyBindIndex, int comboIndex = -1);
	proto external bool SetFilter(string actionName, EInputDeviceType deviceType, string preset, int keyBindIndex, string filterName);
	proto external string GetFilter(string actionName, EInputDeviceType deviceType, string preset, int keyBindIndex);
	proto external void StartCapture(string actionName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty, bool bAppend = false);
	proto external void CancelCapture();
	//! Additional key bindings can be added using additionalKeyBindings array.
	proto external void SaveCapture(array<string> additionalKeyBindings = null);
	proto external EInputBindingCaptureState GetCaptureState();
	proto external BaseContainer FindContext(string contextName);
	proto external BaseContainer FindAction(string actionName);
	proto external void Save();
	proto external bool GetConflicts(string actionName, out notnull array<int> keyBindIndices, out notnull array<string> conflictedActions, out notnull array<string> conflictedActionPresets, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	proto external bool GetBindings(string actionName, out notnull array<string> bindings, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	proto external void GetContexts(out array<string> contextNames);
	proto external void GetPresets(string actionName, out array<string> presetNames);
}

/*!
\}
*/
