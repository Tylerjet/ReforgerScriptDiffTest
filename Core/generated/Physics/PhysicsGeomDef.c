/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

class PhysicsGeomDef: Managed
{
	string	Name;
	PhysicsGeom	Geometry;
	vector	Frame[4] = {Vector(1, 0, 0), Vector(0, 1, 0), Vector(0, 0, 1), Vector(0, 0, 0)};
	int			ParentNode = -1;
	string	MaterialName;
	int			LayerMask; //<Bit mask of layers we are in
	//----------------------------------------------------------------------
	void PhysicsGeomDef(string name, PhysicsGeom geom, string materialName, int layerMask)
	{
		Name = name;
		Geometry = geom;
		MaterialName = materialName;
		LayerMask = layerMask;
	}
	
};

/** @}*/
