/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

sealed class Contact
{
	SurfaceProperties	Material1;
	SurfaceProperties	Material2;
	float	Impulse;
	// to match its memory layout and enable fast copy (potentionally very dangerous and fragile)
	vector	Normal;
	vector	Position;
	vector	VelocityBefore1;
	vector	VelocityBefore2;
	vector	VelocityAfter1;
	vector	VelocityAfter2;
	
	proto external float GetRelativeNormalVelocityBefore();
	proto external float GetRelativeNormalVelocityAfter();
};

/** @}*/
