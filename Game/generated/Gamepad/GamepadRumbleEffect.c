/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Gamepad
\{
*/

//! Effect used to rumble the gamepad.
class GamepadRumbleEffect: GamepadEffect
{
	//! Get impact type
	proto external ERumbleImpact GetImpact();
	//! Get rumble interval type
	proto external ERumbleInterval GetInterval();
	//! How many times the effect will loop
	proto external int GetLoopTimes();
	//! Map ERumbleImpact to range [0.0, 1.0]
	static proto float GetImpactValue(ERumbleImpact eImpact);
	//! Map ERumbleInterval to Milliseconds
	static proto int GetIntervalMS(ERumbleInterval eInterval);
}

/*!
\}
*/
