/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Math
* @{
*/

sealed class Math2D
{
	private void Math2D();
	private void ~Math2D();
	
	/*!
	Check if a point is inside polygon
	Takes polygon and point only in the XZ plane
	*/
	static proto bool IsPointInPolygonXZ(notnull array<vector> polygon, vector point);
	/**
	\brief Check if point is inside triangle
	*/
	static proto bool IsPointInTriangleXZ(vector p1, vector p2, vector p3, vector point);
	/*!
	Check if a point is inside polygon
	\param polygon Must be an array of Vector2, each two consecutive floats represent one point in polygon
	\param x X coordinate of point to test
	\param y Y coordinate of point to test
	*/
	static proto bool IsPointInPolygon(notnull array<float> polygon, float x, float y);
	/*!
	Determines winding order of triangle given by points a, b, c
	If the triangles form a line WindingOrder.INVALID is returned
	*/
	static proto WindingOrder TriangleWindingXZ(vector a, vector b, vector c);
};

/** @}*/
