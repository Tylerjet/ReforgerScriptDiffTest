/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

sealed class PhysicsGeom: pointer
{
	//! Destroys geometry
	proto external void Destroy();
	//! Creates box geometry
	static proto PhysicsGeom CreateBox(vector size);
	//! Creates sphere geometry
	static proto PhysicsGeom CreateSphere(float radius);
	//! Creates capsule geometry
	static proto PhysicsGeom CreateCapsule(float radius, float height);
	//! Creates cylinder geometry
	static proto PhysicsGeom CreateCylinder(float radius, float height);
	//! Creates tri-mesh geometry
	static proto PhysicsGeom CreateTriMesh(vector vertices[], int indices[], int numVertices, int numIndices);
};

/** @}*/
