/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class BaseFireMode: ScriptAndConfig
{
	proto external int GetBurstSize();
	proto external EBurstType GetBurstType();
	proto external int GetSalvoSize();
	proto external float GetShotSpan();
	proto external owned string GetUIName();
	proto external EWeaponFiremodeType GetFiremodeType();
	proto external void SetTriggerEffect(bool bEnable, WeaponGamepadEffectsManagerComponent pEffectsManager, bool isCharged = true);
	proto external void SetFiringTriggerEffectEnabled(bool bEnable, WeaponGamepadEffectsManagerComponent pEffectsManager);
}

/*!
\}
*/
