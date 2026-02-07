/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup World
* @{
*/

//! collision and tracing
sealed class TraceContact
{
	private void TraceContact();
	private void ~TraceContact();
	
	float	Fraction;
	int	Content;
	int	MaterialFlags;
	int	Triangle;
	int	SurfaceID;
	int	V0,V1,V2;
	Material	MaterialPtr;
	Material	OriginalMaterialPtr;
	SurfaceProperties	SurfaceProps;
	float	Plane[4];
	vector	Point;
	
};

/** @}*/
