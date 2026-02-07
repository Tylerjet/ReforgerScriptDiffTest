/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Perception
* @{
*/

class BaseTarget: ScriptAndConfig
{
	/**
	Returns target entity
	*/
	proto external IEntity GetTargetEntity();
	proto external ETargetCategory GetTargetCategory();
	// Last time the target was detected
	proto external float GetTimeSinceDetected();
	// Last time the target has been seen (direct LoS)
	proto external float GetTimeSinceSeen();
	// Last time the target's type was recognized
	proto external float GetTimeSinceTypeRecognized();
	// Last time the target's side was recognized
	proto external float GetTimeSinceSideRecognized();
	// Position where the target was seen last
	proto external vector GetLastSeenPosition();
	// Position where the target was detected last
	proto external vector GetLastDetectedPosition();
};

/** @}*/
