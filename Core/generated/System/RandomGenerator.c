/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup System
* @{
*/

class RandomGenerator: Managed
{
	proto external void SetSeed(int seed);
	/*!
	Generates random point in given polygon
	
	\param polygon Consecutive floats give polygon in 2D (2 floats = Vector2)
	\param bbMin Bounding box minimum corner
	\param bbMax Bounding box maximum corner
	\return Vector3 point in polygon
	*/
	proto external vector GenerateRandomPoint(array<float> polygon, vector bbMin, vector bbMax);
	/*!
	Generates a random point around `center` in range min/max radius
	
	\param minRadius All generated points will be at least this far from center
	\param maxRadius All generated points will be at most this far from center
	\param pos Position around which to generate. Vector2 XZ
	\param uniform If false, has a small bias towards the center which may be desireble in some situations
	\return Vector2 XZ set, Y = 0
	*/
	proto external vector GenerateRandomPointInRadius(float minRadius, float maxRadius, vector center, bool uniform = true);
	//! Generates random float in [0, 1] range
	proto external float RandFloat01();
	//! Generates random float in [X, Y] range
	proto external float RandFloatXY(float x, float y);
};

/** @}*/
