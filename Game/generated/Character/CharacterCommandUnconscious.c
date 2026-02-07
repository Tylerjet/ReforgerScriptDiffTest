/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Character
* @{
*/

class CharacterCommandUnconscious: CharacterCommand
{
	proto external bool CanStartReviving();
	//! Should start/stop reviving command
	proto external void Reviving(bool shouldStart);
	/*
	Starts/Stops unconscious Command
	Lying on your back - 1,
	On belly rolling over to the left - 2, (with arms up - 6)
	On belly rolling over to the right - 3, (with arms up - 7)
	Lying on left side transition - 4, (with arms up - 8)
	Lying on right side transition - 5, (with arms up - 9)
	Exit uncsocious animation state - 0
	*/
	proto external void StartCommandUnconscious(int commandI);
	/*
	
	General Injury - BodyPart == 1
	Head Injury - BodyPart == 2
	Chest Injury - BodyPart == 3
	Left Leg Injury - BodyPart == 4
	Right Leg Injury - BodyPart == 5
	Left Arm Injury - BodyPart == 6
	Right Arm Injury - BodyPart == 7
	
	*/
	proto external void SetBodyPartVar(int bodyPart);
};

/** @}*/
