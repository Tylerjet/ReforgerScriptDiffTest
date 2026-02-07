/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Entities
* @{
*/

class SplineShapeEntityClass: ShapeEntityClass
{
};

class SplineShapeEntity: ShapeEntity
{
	/*!
	Returns true if tangents at given anchor point were explicitly set by user,
	false if they are computed automatically.
	*/
	proto external bool HasPointExplicitTangents(int pointIdx);
	/*!
	Returns tangents at given anchor point
	For point `i` the inTangent is a tangent between points `i - 1` and `i` at point `i`.
	The outTangent is a tangent between points `i` and `i + 1` at point `i`.
	*/
	proto void GetTangents(int pointIdx, out vector inTangent, out vector outTangent);
};

/** @}*/
