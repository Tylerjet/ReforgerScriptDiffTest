/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Character
* @{
*/

class CharacterCommandLadder: CharacterCommand
{
	//! returns true if ladder can be exited
	proto external bool CanExit();
	//!
	proto external void Exit();
	static proto LadderComponent DetectLadder(ChimeraCharacter pCharacter, int pLadderPhxLayer);
};

/** @}*/
