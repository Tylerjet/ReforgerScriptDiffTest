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
	// Time passed since target was detected
	proto external float GetTimeSinceDetected();
	// Time passed since the target was seen (direct LoS)
	proto external float GetTimeSinceSeen();
	// Time (from perception manager) when the target was seen (direct LoS)
	proto external float GetTimeLastSeen();
	// Time passed since the target's type was recognized
	proto external float GetTimeSinceTypeRecognized();
	// Time passed since the target's side was recognized
	proto external float GetTimeSinceSideRecognized();
	// Time passed since the target endangered us
	proto external float GetTimeSinceEndangered();
	// Position where the target was seen last
	proto external vector GetLastSeenPosition();
	// Position where the target was detected last
	proto external vector GetLastDetectedPosition();
	// Returns how we percieve whether that target is endangering us or not. The actual value is updated periodically.
	proto external bool IsEndangering();
	// Returns unit type, same as in PerceivableComponent of that target
	proto external EAIUnitType GetUnitType();
	// Returns perceivable component of target
	proto external PerceivableComponent GetPerceivableComponent();
	// Sets the last seen position to provided value if the timestamp is newer than existing last seen time
	proto external void UpdateLastSeenPosition(vector pos, float perceptionMgrTimestamp);
};

/** @}*/
