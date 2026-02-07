/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Entities
* @{
*/

class IEntity: Managed
{
	/*!
	Event when touched by other entity
	\note You need to have TouchComponent in entity to receive this event
	
	\param owner The owner entity
	\param other Entity who touched us
	\param extra Bitmask of touch types TODO
	*/
	event protected void EOnTouch(IEntity owner, IEntity other, int touchTypesMask);
	/*!
	Event after entity is allocated and initialized.
	\param owner The owner entity
	*/
	event protected void EOnInit(IEntity owner);
	/*!
	Event when we are visible
	\param owner The owner entity
	\param frameNumber Frame number
	*/
	event protected void EOnVisible(IEntity owner, int frameNumber);
	/*!
	Event every frame
	\param owner The owner entity
	\param timeSlice Time passed since last frame
	*/
	event protected void EOnFrame(IEntity owner, float timeSlice);
	/*!
	Even after physics update
	\param owner The owner entity
	\param timeSlice Time passed since last frame
	*/
	event protected void EOnPostFrame(IEntity owner, float timeSlice);
	/*!
	Event from animation system
	\param owner The owner entity
	\param type
	\param slot
	*/
	event protected void EOnAnimEvent(IEntity owner, int type, int slot);
	/*!
	Event before simulated by physics engine (called from sub-iterations!
	\param owner The owner entity
	\param timeSlice Time slice of simulation step
	*/
	event protected void EOnSimulate(IEntity owner, float timeSlice);
	/*!
	Event after simulated by physics engine (once per frame)
	\param owner The owner entity
	\param timeSlice Time slice of simulation step
	*/
	event protected void EOnPostSimulate(IEntity owner, float timeSlice);
	/*!
	Event when joint attached to RigidBody of this entity is broken
	\param owner The owner entity
	\param other Other Entity attached to the joint
	*/
	event protected void EOnJointBreak(IEntity owner, IEntity other);
	/*!
	Event when physics engine has moved with this Entity
	\param owner The owner entity
	*/
	event protected void EOnPhysicsMove(IEntity owner);
	/*!
	Event when physics engine registered contact with other RigidBody
	\param owner The owner entity
	\param other Other Entity who contacted us
	\param contact Structure describing the contact
	*/
	event protected void EOnContact(IEntity owner, IEntity other, Contact contact);
	/*!
	Event when physics engine (de)activated RigidBody
	\param owner The owner entity
	*/
	event protected void EOnPhysicsActive(IEntity owner, bool activeState);
	/*!
	Event every frame after EOnFrame, when "Entity diag" is enabled from the debug menu
	\param owner The owner entity
	\param timeSlice Time slice of simulation step
	*/
	event protected void EOnDiag(IEntity owner, float timeSlice);
	/*!
	Event every fixed frame
	\param owner The owner entity
	\param timeSlice Fixed time step
	*/
	event protected void EOnFixedFrame(IEntity owner, float timeSlice);
	/*!
	Event after physics update every fixed frame
	\param owner The owner entity
	\param timeSlice Fixed time step
	*/
	event protected void EOnFixedPostFrame(IEntity owner, float timeSlice);
	//!EntityEvent.EV_USER+0
	protected void EOnUser0(IEntity other, int extra);
	//!EntityEvent.EV_USER+1
	protected void EOnUser1(IEntity other, int extra);
	//!EntityEvent.EV_USER+2
	protected void EOnUser2(IEntity other, int extra);
	//!EntityEvent.EV_USER+3
	protected void EOnUser3(IEntity other, int extra);
	//!EntityEvent.EV_USER+4
	protected void EOnUser4(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy018(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy019(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy020(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy021(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy022(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy023(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy024(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy025(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy026(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy027(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy028(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy029(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy030(IEntity other, int extra);
	//!Placeholder
	private void EOnDummy031(IEntity other, int extra);
	//-----------------------------------------------------------------------
	//! protected script Constructor
	protected void IEntity(IEntitySource src, IEntity parent);
	
	/**
	\brief Return unique entity ID
	\return \p int unique entity ID
	@code
	ItemBase apple = g_Game.CreateObject( "FruitApple", String2Vector("0 10 0"), false );
	Print( apple.GetID() );
	
	>> 0
	@endcode
	*/
	proto external EntityID GetID();
	//!Returns pointer to parent Entity in hierarchy
	proto external IEntity GetParent();
	//!Returns pointer to first child Entity in hierarchy
	proto external IEntity GetChildren();
	//!Returns pointer to next child Entity on the same hierarchy
	proto external IEntity GetSibling();
	/**
	\brief Returns visual object set to this Entity. No reference is added
	*/
	proto external VObject GetVObject();
	proto external EntityPrefabData GetPrefabData();
	proto external EntityComponentPrefabData FindComponentData(typename typeName);
	proto external BaseWorld GetWorld();
	//! Set fixed LOD. -1 for non-fixed LOD
	proto external void SetFixedLOD(int lod);
	/**
	\brief Returns world transformation of Entity. It returns only so much vectors as array is big
	\param mat \p vector[1...4] matrix to be get
	@code
	Man player = g_Game.GetPlayer();
	
	vector mat[4];
	player.GetTransform(mat);
	Print( mat );
	
	>> <0.989879,-0,0.141916>,<0,1,0>,<-0.141916,0,0.989879>,<2545.08,15.6754,2867.49>
	@endcode
	*/
	proto external void GetTransform(out vector mat[]);
	//! See IEntity#GetTransform
	proto external void GetWorldTransform(out vector mat[]);
	//! See IEntity#GetTransform
	proto external void GetLocalTransform(out vector mat[]);
	/**
	\brief Transforms local vector to parent(world) space
	\param vec \p vector local space vector to transform
	\return \p vector parent space vector
	@code
	Man player = g_Game.GetPlayer();
	Print( player.VectorToParent("1 2 3") );
	
	>> <2.89791,2,1.26575>
	@endcode
	*/
	proto external vector VectorToParent(vector vec);
	/**
	\brief Transforms local position to parent(world) space
	\param coord \p vector local position to transform
	\return \p vector position in parent space
	@code
	Man player = g_Game.GetPlayer();
	Print( player.CoordToParent("1 2 3") );
	
	>> <2549,17.6478,2857>
	@endcode
	*/
	proto external vector CoordToParent(vector coord);
	/**
	\brief Transforms world space vector to local space
	\param vec \p vector world space vector to transform
	\return \p vector local space vector
	@code
	Man player = g_Game.GetPlayer();
	Print( player.VectorToLocal("2 1 5") );
	
	>> <-0.166849,1,5.38258>
	@endcode
	*/
	proto external vector VectorToLocal(vector vec);
	/**
	\brief Transforms world space position to local space
	\param coord \p vector world space position to transform
	\return \p vector position in local space
	@code
	Man player = g_Game.GetPlayer();
	Print( player.CoordToLocal("500 10 155") );
	
	>> <15254,-54.2004,8745.53>
	@endcode
	*/
	proto external vector CoordToLocal(vector coord);
	/**
	\brief Returns orientation of Entity in world space (Yaw, Pitch, Roll)
	\return \p vector entity orientation
	@code
	Man player = g_Game.GetPlayer();
	Print( player.GetYawPitchRoll() );
	
	>> <180,-76.5987,180>
	@endcode
	*/
	proto external vector GetYawPitchRoll();
	/**
	\brief Sets angles for entity (Yaw, Pitch, Roll)
	\param angles \p vector angles to be set
	@code
	Man player = g_Game.GetPlayer();
	player.SetYawPitchRoll("180 50 180" );
	Print( player.GetYawPitchRoll() );
	
	>> <-180,50,-180>
	@endcode
	*/
	proto external void SetYawPitchRoll(vector angles);
	//! See IEntity#GetTransformAxis
	proto external vector GetWorldTransformAxis(int axis);
	//! See IEntity#GetTransformAxis
	proto external vector GetTransformAxis(int axis);
	//! See IEntity#GetTransformAxis
	proto external vector GetLocalTransformAxis(int axis);
	//! See IEntity#SetTransform. Returns false, if there is not change in transformation
	proto external bool SetLocalTransform(vector mat[4]);
	//! See IEntity#SetTransform Returns false, if there is not change in transformation
	proto external bool SetWorldTransform(vector mat[4]);
	//! Return pivot ID from hierarchy component
	proto external TNodeId GetPivot();
	/**
	\brief Sets entity world transformation
	\param mat \p vector[4] matrix to be set
	@code
	vector mat[4];
	Math3D.MatrixIdentity( mat )
	
	Man player = g_Game.GetPlayer();
	player.SetTransform( mat );
	
	vector outmat[4];
	player.GetTransform(outmat );
	Print( outmat );
	
	>> <1,0,0>,<0,1,0>,<0,0,1>,<0,0,0>
	@endcode
	*/
	proto external bool SetTransform(vector mat[4]);
	/**
	\brief Same as GetLocalYawPitchRoll, but returns rotation vector around X, Y and Z axis.
	*/
	proto external vector GetLocalAngles();
	/**
	\brief Returns local orientation when it's in hierarchy (Yaw, Pitch, Roll)
	\return \p vector local orientation
	@code
	Man player = g_Game.GetPlayer();
	Print( player.GetLocalYawPitchRoll() );
	
	>> <180,-57.2585,180>
	@endcode
	*/
	proto external vector GetLocalYawPitchRoll();
	/**
	\brief Same as GetYawPitchRoll, but returns rotation vector around X, Y and Z axis.
	*/
	proto external vector GetAngles();
	/**
	\brief Same as SetYawPitchRoll, but sets rotation around X, Y and Z axis.
	*/
	proto external void SetAngles(vector angles);
	/**
	\brief Returns origin of Entity
	\return \p vector entity origin
	@code
	Man player = g_Game.GetPlayer();
	Print( player.GetOrigin() );
	
	>> <2577.02,15.6837,2924.27>
	@endcode
	*/
	proto external vector GetOrigin();
	/**
	\brief Sets origin for entity
	\param orig \p vector origin to be set
	@code
	Man player = g_Game.GetPlayer();
	player.SetOrigin("2550 10 2900" );
	Print( player.GetOrigin() );
	
	>> <2550,10,2900>
	@endcode
	*/
	proto external void SetOrigin(vector orig);
	proto external void SetScale(float scale);
	proto external float GetScale();
	/**
	\brief Returns local bounding box of model on Entity
	\param[out] mins \p vector minimum point of bounding box
	\param[out] maxs \p vector maximum point of bounding box
	@code
	Man player = g_Game.GetPlayer();
	
	vector mins, maxs;
	player.GetBounds(mins, maxs );
	
	Print( mins );
	Print( maxs );
	
	>> <0,0,0>
	>> <0,0,0>
	@endcode
	*/
	proto external void GetBounds(out vector mins, out vector maxs);
	/**
	\brief Returns quantized world-bound-box of Entity
	\param[out] mins \p vector minimum point of bounding box
	\param[out] maxs \p vector maximum point of bounding box
	@code
	Man player = g_Game.GetPlayer();
	
	vector mins, maxs;
	player.GetWorldBounds( mins, maxs );
	
	Print( mins );
	Print( maxs );
	
	>> <2547.2,15.5478,2852.85>
	>> <2548.8,17.5478,2855.05>
	@endcode
	*/
	proto external void GetWorldBounds(out vector mins, out vector maxs);
	//!Dynamic event invokation. Parameters are the same as in IEntity::EOnXXXX() methods
	proto external volatile void SendEvent(notnull IEntity actor, EntityEvent e, void extra);
	proto external string GetName();
	//!Sets component flags
	proto external int SetVComponentFlags(VCFlags flags);
	/**
	\brief Sets the visual object to this entity. Reference is added and released upon entity destruction
	\param object handle to object got by GetObject()
	\param options String, dependant on object type.
	//Only supported one for XOB objects:
	//$remap 'original material name' 'new material'; [$remap 'another original material name' 'anothernew material']
	*/
	proto external void SetObject(VObject object, string options);
	/**
	\brief Sets Entity flags. It's OR operation, not rewrite. Returns previous flags
	\param flags \p int flags to be set
	\param recursively flags will be recursively applied to children of hierarchy too
	\return \p int previous flags
	@code
	Man player = g_Game.GetPlayer();
	player.SetFlags(EntityFlags.VISIBLE|EntityFlags.TRACEABLE );
	Print( player.GetFlags() );
	
	>> 1610612747
	@endcode
	*/
	[Restrict(EAccessLevel.LEVEL_1, false)]
	proto external EntityFlags SetFlags(EntityFlags flags, bool recursively);
	/**
	\brief Clear Entity flags. Returns cleared flags
	\param flags \p int flags to be set
	\param recursively flags will be recursively applied to children of hierarchy too
	\return \p int cleared flags
	@code
	Man player = g_Game.GetPlayer();
	player.ClearFlags(EntityFlags.VISIBLE|EntityFlags.TRACEABLE );
	Print( player.GetFlags() );
	
	>> 1610612744
	@endcode
	*/
	proto external EntityFlags ClearFlags(EntityFlags flags, bool recursively);
	/**
	\brief Returns Entity flags
	\return \p EntityFlags entity flags
	@code
	Man player = g_Game.GetPlayer();
	Print( player.GetFlags() );
	
	>> 1610612745
	@endcode
	*/
	proto external EntityFlags GetFlags();
	/**
	\brief Sets event mask
	\param e combined mask of one or more members of EntityEvent enum
	@code
	Man player = g_Game.GetPlayer();
	Print( player.GetEventMask() );
	player.SetEventMask( EntityEvent.VISIBLE );
	Print( player.GetEventMask() );
	
	>> 0
	>> 128
	@endcode
	*/
	proto external EntityEvent SetEventMask(EntityEvent e );
	/**
	\brief Clears event mask
	\param e \p int event mask
	\return \p int event mask
	@code
	Man player = g_Game.GetPlayer();
	player.SetEventMask(EntityEvent.VISIBLE );
	Print( player.GetEventMask() );
	player.ClearEventMask(EntityEvent.ALL );
	Print( player.GetEventMask() );
	
	>> 128
	>> 0
	@endcode
	*/
	proto external EntityEvent ClearEventMask(EntityEvent e);
	/**
	\brief Returns current event mask
	\return \p int current event mask
	@code
	Man player = g_Game.GetPlayer();
	Print( player.GetEventMask() );
	
	>> 0
	@endcode
	*/
	proto external EntityEvent GetEventMask();
	//! \returns true when entity is loaded from map, false when dynamically spawned
	proto external bool IsLoaded();
	//! \returns true if entity is ready to be deleted
	proto external bool IsRemoved();
	//! \returns true if entity was deleted (entity pointer valid until the end of the frame)
	proto external bool IsDeleted();
	/*!
	Updates entity state/position. Should be called when you want to manually commit position changes etc
	before trace methods etc. Entity is updated automatically at the end and the beginning of simulation step,
	when it has EntityFlags.TFL_ACTIVE flag set.
	\returns mask with flags
	//	EntityFlags.UPDATE - hierarchy has been updated
	//	EntityFlags.UPDATE_MDL - model hierarchy has been updated
	*/
	proto external int Update();
	//!Add Entity to hierarchy. Pivot is pivot index, or -1 for center of parent.
	proto external int AddChild(notnull IEntity child, TNodeId pivot, EAddChildFlags flags = EAddChildFlags.AUTO_TRANSFORM);
	//!Remove Entity from hierarchy
	proto external void RemoveChild(notnull IEntity child, bool keepTransform = false);
	proto external void SetName(string name);
	//!Sets visibility mask for cameras, where Entity will be rendered
	proto external int SetCameraMask(int mask);
	proto external Physics GetPhysics();
	proto external Particles GetParticles();
	//!Updates animation (either xob, or particle, whatever)
	proto external int Animate(float speed, int loop);
	//!Updates animation (either xob, or particle, whatever)
	proto external int AnimateEx(float speed, int loop, out vector lin, out vector ang);
	proto external void	SetBone(TNodeId bone, vector angles, vector trans, float scale);
	proto external bool	SetBoneMatrix(TNodeId bone, vector mat[4]);
	proto external void	SetBoneGlobal(TNodeId bone, vector mat[4]);
	proto external bool	GetBoneMatrix(TNodeId bone, vector mat[4]);
	proto external TNodeId	GetBoneIndex(string boneName);
	proto external void	GetBoneNames(out notnull array<string> boneNames);
	proto external bool	GetBoneLocalMatrix(TNodeId bone, vector mat[4]);
	[Obsolete("Use GetParticles().OverridePrevPos() instead")]
	proto external void ResetParticlePosition();
	[Obsolete("Use GetParticles().Restart() instead")]
	proto external void RestartParticle(bool invalidatePrevPos = false);
	[Obsolete("Use GetParticles.GetNumParticles() instead")]
	proto external int GetParticleCount();
	[Obsolete("Use GetParticles().SetParam() instead")]
	proto external void SetParticleParm(int emitter, EmitterParam parameter, void value);
	[Obsolete("Use GetParticles().GetParam() instead")]
	proto external void GetParticleParm(int emitter, EmitterParam parameter, out void value);
	[Obsolete("Use GetParticles().GetParamOrig() instead")]
	proto external void GetParticleParmOriginal(int emitter, EmitterParam parameter, out void value);
	[Obsolete("Use GetParticles().GetEmitterNames() or GetParticles().GetNumEmitters() instead")]
	proto external int GetParticleEmitters(out string emitters[], int max);
	/*!
	Finds first occurance of the coresponding component.
	\param typeName type of the component
	*/
	proto external Managed FindComponent(typename typeName);
	/*!
	Finds all occurances of the coresponding component.
	\param typeName type of the component
	\param outComponents array to fill with selected components
	*/
	proto external int FindComponents(typename typeName, notnull array<Managed> outComponents);
};

/** @}*/
