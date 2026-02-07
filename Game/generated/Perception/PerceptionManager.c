/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Perception
\{
*/

class PerceptionManagerClass: GenericEntityClass
{
}

class PerceptionManager: GenericEntity
{
	proto external float GetTime();
	// Returns ambient light factor, from 0 to 1.0
	proto external float GetAmbientLightFactor();
	//! Iterates all registered PerceptionComponents and requests update of faction friendliness.
	//! DoCheckIfFactionFriendly will be called for faction of all targets at the next update of all PerceptionComponents.
	//! This is a heavy operation and should not be performed often.
	proto external void RequestUpdateAllTargetsFactions();
}

/*!
\}
*/
