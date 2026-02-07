/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Core
* @{
*/

//! ValidityCheck is connected to an entity of a particle effect and tracks its current user.
class ValidityCheck
{
	//! Use this function to get info if you still own a particle effect.
	/*!
	\param timeStamp identifier of the owner
	\return true if timeStamp matches current owner
	*/
	proto external bool IsValid(int timeStamp);
	//! Update the ownership
	/*!
	\return id for the current owner
	*/
	proto external int Update();
};

/** @}*/
