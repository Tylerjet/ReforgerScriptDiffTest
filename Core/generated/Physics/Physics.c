/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

sealed class Physics: NativeComponent
{
	static const float STANDARD_GRAVITY = 9.81;
	static const vector VGravity = "0 -9.81 0";
	
	//!Destroys physics body
	proto external void Destroy();
	proto external void SetInteractionLayer(int mask);
	proto external void GetWorldTransform(out vector mat[4]);
	proto external void GetDirectWorldTransform(out vector mat[4]);
	proto external void SetGeomInteractionLayer(int index, int mask);
	proto external int GetInteractionLayer();
	proto external int GetGeomInteractionLayer(int index);
	/*!
	If both maxMotion and shapeCastRadius is >=0 then the continuous collision detection is enabled.
	If you want to disable it, use -1
	\param maxMotion max motion threshold when sphere-cast is performed, to find time of impact. It should be
	little bit less than size of the geometry to catch the situation when tunelling can happen
	\param sphereCastRadius The radius of the largest possible sphere, that is completelly inside the body geometry.
	*/
	proto external void EnableCCD(float maxMotion, float sphereCastRadius);
	proto external void ChangeSimulationState(SimulationState simState);
	proto external SimulationState GetSimulationState();
	//!returns center of mass offset
	proto external vector GetCenterOfMass();
	proto external void SetActive(int act);
	proto external bool IsActive();
	proto external bool IsDynamic();
	proto external bool EnableGravity(bool enable);
	proto external void SetDamping(float linearDamping, float angularDamping);
	proto external float GetMass();
	proto external void SetMass(float mass);
	proto external void SetInertiaTensorV(vector v);
	proto external void SetInertiaTensorM(vector m[3]);
	/**
	\brief Gets angular velocity for a rigidbody
	*/
	proto external vector GetAngularVelocity();
	proto external vector GetVelocityAt(vector pos);
	proto external void SetSleepingTreshold(float linearTreshold, float angularTreshold);
	/*!
	Sets scale factor for all impulses/velocities/forces. Default is <1,1,1>. Zero any axis if you want to do 2D physics
	*/
	proto external void SetLinearFactor(vector linearFactor);
	/**
	\brief Returns linear velocity
	\return \p vector linear velocity
	@code
	Physics physics = g_Game.GetPlayer().GetPhysics();
	if (physics)
	{
	Print( physics.GetVelocity() );
	}
	>> <0,0,0>
	@endcode
	*/
	proto external vector GetVelocity();
	/**
	\brief Sets linear velocity (for Rigid bodies)
	\param velocity \p velocity vector to be set
	*/
	proto external void SetVelocity(vector velocity);
	/**
	\brief Changed an angular velocity
	\param velocity \p Angular velocity, rotation around x, y and z axis (not yaw/pitch/roll)
	*/
	proto external void SetAngularVelocity(vector velocity);
	/**
	\brief Sets target transformation. If timeslice == dt (simulation step delta time), it will happen in next step, otherwise in time = timeslice
	*/
	proto external void SetTargetMatrix(vector matrix[4], float timeslice);
	/**
	\brief Applies impuls on a rigidbody (origin)
	*/
	proto external void ApplyImpulse(vector impulse);
	/**
	\brief Applies impuls on rigidbody at a position (world coordinates)
	*/
	proto external void ApplyImpulseAt(vector pos, vector impulse);
	/**
	\brief Applies constant force on a rigidbody (origin)
	*/
	proto external void ApplyForce(vector force);
	/**
	\brief Applies constant force on a rigidbody at a position (world coordinates)
	*/
	proto external void ApplyForceAt(vector pos, vector force);
	/**
	\brief Apply constant torque on a rigidbody
	*/
	proto external void ApplyTorque(vector torque);
	/*!
	\brief Clear forces and torques on a rigidbody
	*/
	proto external void ClearForces();
	/*!
	\brief Returns total force currently applied to a rigidbody.
	*/
	proto external vector GetTotalForce();
	/*!
	\brief Returns total torque currently applied to a rigidbody
	*/
	proto external vector GetTotalTorque();
	/*!
	Changes index to response matrix, see project settings/physics/interactions/response matrix
	Usually this index is set by physics component property, but can be override in runtime
	*/
	proto external void SetResponseIndex(int responseIndex);
	//!Returns actual index to response matrix
	proto external int GetResponseIndex();
	//! adds new geometry and returns an index of the geometry or -1 when an error occurred
	proto external int AddGeom(string name, PhysicsGeom geom, vector frame[4], string material, int interactionLayer);
	//! find a geometry by its name and returns its index or -1 if the geometry wasn't found
	proto external int GetGeom(string name);
	//! returns number of geometries of the entity
	proto external int GetNumGeoms();
	//! returns world space transformation of a geometry element
	proto external void GetGeomWorldTransform(int index, out vector mat[4]);
	//! returns entity space transformation of a geometry element
	proto external void GetGeomTransform(int index, out vector mat[4]);
	//! returns internal physics space transformation of a geometry element
	proto external void GetGeomDirectTransform(int index, out vector mat[4]);
	//! returns world space position of a geometry element
	proto external vector GetGeomWorldPosition(int index);
	//! returns entity space position of a geometry element
	proto external vector GetGeomPosition(int index);
	//! returns internal physics space position of a geometry element
	proto external vector GetGeomDirectPosition(int index);
	//! returns world bounds of a geometry element
	proto external void GetGeomWorldBounds(int index, out vector min, out vector max);
	//! returns internal physics bounds of a geometry element (at scale 1)
	proto external void GetGeomDirectBounds(int index, out vector min, out vector max);
	proto external void GetGeomSurfaces(int index, notnull out array<SurfaceProperties> surfaces);
	/*!
	Creates CollisionObject from geometry embedded in attached VObject. If there is not any, false is returned
	\param ent Entity that will be associated with physics body
	\param layerMask Bit mask of layers that is ANDed with layers in object geometry. Use 0xffffffff if you want to keep it unmodified
	*/
	static proto Physics CreateStatic(notnull IEntity ent, int layerMask = 0xffffffff);
	/*!
	Creates RigidBody from custom made geometries. The geometries are deleted when rigid body is destroyed
	\param ent Entity that will be associated with physics body
	\param geoms array of custom made geometries
	@code
	autoptr PhysicsGeomDef geoms[] = {PhysicsGeomDef("", dGeomCreateBox(size), "material/default", 0xffffffff)};
	Physics.CreateStaticEx(this, geoms);
	@endcode
	*/
	static proto Physics CreateStaticEx(notnull IEntity ent, PhysicsGeomDef geoms[]);
	/*!
	Creates RigidBody from geometry embedded in attached VObject. If there is not any, false is returned
	\param ent Entity that will be associated with physics body
	\param mass Body mass
	\param layerMask Bit mask of layers that is ANDed with layers in object geometry. Use 0xffffffff if you want to keep it unmodified
	*/
	static proto Physics CreateDynamic(notnull IEntity ent, float mass, int layerMask = 0xffffffff);
	/*!
	Creates RigidBody from custom made geometries. The geometries are deleted when rigid body is destroyed
	\param ent Entity that will be associated with physics body
	\param centerOfMass Offset from object-pivot to center of mass
	\param mass Body mass
	\param geoms array of custom made geometries
	@code
	autoptr PhysicsGeomDef geoms[] = {PhysicsGeomDef("", dGeomCreateBox(size), "material/default", 0xffffffff)};
	Physics.CreateDynamicEx(this, center, 1.0, geoms);
	@endcode
	*/
	static proto Physics CreateDynamicEx(notnull IEntity ent, vector centerOfMass, float mass, PhysicsGeomDef geoms[]);
	static proto Physics CreateGhostEx(notnull IEntity ent, PhysicsGeomDef geoms[]);
};

/** @}*/
