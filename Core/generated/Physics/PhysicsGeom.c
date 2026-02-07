/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Physics
\{
*/

/*!
Geometric shapes relevant for physics simulation
*/
sealed class PhysicsGeom: pointer
{
	//! Destroys geometry
	proto external void Destroy();
	/*!
	Creates box geometry
	\param size Dimensions of the box
	@code
		PhysicsGeom geom = PhysicsGeom.CreateBox("1 1 1");
	@endcode
	*/
	static proto PhysicsGeom CreateBox(vector size);
	/*!
	Creates sphere geometry
	\param radius Radius of the sphere
	@code
		PhysicsGeom geom = PhysicsGeom.CreateSphere(0.5);
	@endcode
	*/
	static proto PhysicsGeom CreateSphere(float radius);
	/*!
	Creates capsule geometry
	\param radius Radius of the cylindrical/hemispherical part
	\param height Height of the cylindrical part
	@code
		PhysicsGeom geom = PhysicsGeom.CreateCapsule(0.5, 1);
	@endcode
	*/
	static proto PhysicsGeom CreateCapsule(float radius, float height);
	/*!
	Creates cylinder geometry
	\param radius Radius of the cylinder
	\param height Height of the cylinder
	@code
		PhysicsGeom geom = PhysicsGeom.CreateCylinder(0.5, 1);
	@endcode
	*/
	static proto PhysicsGeom CreateCylinder(float radius, float height);
	/*!
	Creates tri-mesh geometry
	\param vertices Set of vertices representing the geometry
	\param indices Set of indices representing the geometry
	\param numVertices Number of vertices
	\param numIndices Number of indices
	@code
		const int numVerts = 4;
		const int numInds = 12;
		vector verts[numVerts] = { "0 0 0", "1 0 0", "0 0 1", "0 1 0" };
		int inds[numInds] = { 0, 1, 2, 0, 3, 1, 3, 2, 1, 0, 2, 3 };
		PhysicsGeom geom = PhysicsGeom.CreateTriMesh(verts, inds, numVerts, numInds);
	@endcode
	*/
	static proto PhysicsGeom CreateTriMesh(vector vertices[], int indices[], int numVertices, int numIndices);
}

/*!
\}
*/
