/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Gamepad
\{
*/

class GamepadIOHandlerComponentClass: ScriptComponentClass
{
}

class GamepadIOHandlerComponent: ScriptComponent
{
	proto external GamepadTriggerEffect GetActiveLeftTriggerEffect();
	proto external GamepadTriggerEffect GetActiveRightTriggerEffect();
	proto external GamepadLightEffect GetActiveLightEffect();
	proto external void Reset();
	proto external void RegisterEffectsManager(GamepadEffectsManagerComponent manager);
	proto external void UnregisterEffectsManager(GamepadEffectsManagerComponent manager);
	proto external void ApplyGamepadEffectImmediate(GamepadEffect effect);
}

/*!
\}
*/
