/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Weapon
* @{
*/

class BaseSightsComponentClass: GameComponentClass
{
};

class BaseSightsComponent: GameComponent
{
	proto external bool IsSightADSActive();
	/*!
	Returns additional sights position used for calculating zeroing
	A vector is calculated from start position to end position and used as direction for calculating zeroing offsets
	Start position should be the one closer to eye position than end position
	\param local Whether returned position should be in local space or world space
	*/
	proto external vector GetSightsRearPosition(bool localSpace = false);
	/*!
	Returns additional sights position used for calculating zeroing
	A vector is calculated from start position to end position and used as direction for calculating zeroing offsets
	End position should be the one further from eye position than start position
	\param local Whether returned position should be in local space or world space
	*/
	proto external vector GetSightsFrontPosition(bool localSpace = false);
	/*!
	Returns the local sights reference point offset.
	This corresponds to the set point info in base sights component.
	Returns zero vector if no point info is present.
	*/
	proto external vector GetSightsOffset();
	/*!
	Returns current value of field of view.
	\return Current field of view value in degrees.
	*/
	proto external float GetFOV();
	/*!
	Returns current range info value where
	x: animation value
	y: distance
	z: unused
	\return Returns sights info values or empty vector if none.
	*/
	proto external vector GetCurrentSightsRange();
	/*!
	Returns currently selected sights FOV info or null if none.
	\return Current SightsFOVInfo or null if none.
	*/
	proto external SightsFOVInfo GetFOVInfo();
	//! Percentage 0...1 of recoil that should be applied to camera when using this sights component.
	proto external float GetCameraRecoilAmount();
	
	// callbacks
	
	//! Positive weapon angle tilts weapon upwards (muzzle goes up, stock goes down)
	//! Negative weapon angle tilts weapon downwards (muzzle goes down, stock goes up)
	//! Called from GameCode, do not remove!
	event protected bool WB_GetZeroingData(IEntity owner, BaseSightsComponent sights, float weaponAngle, out vector offset, out vector angles);
};

/** @}*/
