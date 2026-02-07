/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Character
* @{
*/

//! CharacterCommandSwim - swimming implementation
class CharacterCommandSwim: CharacterCommand
{
	//!
	proto external void StopSwimming();
	/*
	returns water level from entity position
	returns Vector3(totalWaterDepth, currentCharacteDepth, 0);
	*/
	static proto vector WaterLevelCheck(ChimeraCharacter pCharacter, vector pPosition);
};

/** @}*/
