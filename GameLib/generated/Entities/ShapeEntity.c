/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Entities
* @{
*/

class ShapeEntityClass: GenericEntityClass
{
};

class ShapeEntity: GenericEntity
{
	proto external void GenerateTesselatedShape(notnull array<vector> outPoints);
	/*!
	Returns bboxes for all segments of this shape
	Similar to GeneratorBaseEntity events, ith bbox is given by mins[i] and maxes[i].
	Since the bboxes are done by parts their sum may be smaller than the bbox of the whole entity.
	The bboxes returned may overlap.
	*/
	proto void GetAllInfluenceBBoxes(IEntitySource shapeEntitySrc, out notnull array<vector> mins, out notnull array<vector> maxes);
	//! Copies positions of points to given array
	proto external void GetPointsPositions(notnull array<vector> outPoints);
};

/** @}*/
