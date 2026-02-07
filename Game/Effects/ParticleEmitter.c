[EntityEditorProps(category: "GameScripted", description: "Particle emitter", color: "32 94 200 255")]
class SCR_ParticleEmitterClass: GenericEntityClass
{
};

[Obsolete("Use ParticleEffectEntity instead")]
class SCR_ParticleEmitter : GenericEntity
{	

	//! Path to the particle effect file
	[Attribute("", UIWidgets.ResourceNamePicker, "Path to the particle effect file", "ptc")]
	private ResourceName m_EffectPath;
	
	//! Whether to play particle effect on spawn
	[Attribute("1", UIWidgets.CheckBox, "Whether to play particle effect on spawn")]
	bool m_PlayOnSpawn;
	
	//! Whether to delete the particle effect once it finishes playing (inapplicable for looping particles)
	[Attribute("1", UIWidgets.CheckBox, "Whether to delete the particle effect once it finishes playing (inapplicable for looping particles)")]
	bool m_DeleteOnFinish;
	
	//! Whether to play particle effect on spawn
	[Attribute("1", UIWidgets.CheckBox, "Play / stop the particle in Editor")]
	bool m_PlayInEditor;
	
	protected EParticleEmitterState m_ePlayState = EParticleEmitterState.PLAYSTATE_STOPPED;
	
	
	//-----------------------------------------------------------------------------------------------------------------
	//
	//                                   Static methods for particle FX spawning
	//
	//-----------------------------------------------------------------------------------------------------------------
	
	/*!
	Spawns an SCR_ParticleEmitter entity instance with a given particle effect at a given position.
	
	\param name     ResourceName of a particle effect (ptc file)
	\param pos      Position of the entity to spawn (relative to the given parent, world position when parent is null)
	\param rot      Optional euler angles of the entity to spawn
	                (relative to the given parent, i.e. world position when parent is null)
	\param parent   Optional parent entity, may be null.
	                If not null, the new entity will be spawned in its hiearchy with auto-transform enabled.
	\param boneID   Optional ID of a bone to attach the entity to. -1 for no bone. Only used when parent is not null.
	\param play     Optional - true (default) to play the effect immediately after creation.
	
	\return The spawned entity, null if something went wrong.
	*/
	[Obsolete("Use ParticleEffectEntity.SpawnParticleEffect() instead")]
	static SCR_ParticleEmitter Create(
		ResourceName name, vector pos,
		vector rot = "0 0 0", IEntity parent = null, int boneID = -1, bool play = true)
	{
		return CreateEx(SCR_ParticleEmitter, name, pos, rot, parent, boneID, play);
	}
	
	/*!
	Spawns an SCR_ParticleEmitter entity instance with a given particle effect as a child of given parent entity.
	Convenience alternative for Create().
	
	\param name     ResourceName of a particle effect (ptc file)
	\param parent   Parent entity
	\param localPos Optional position of the entity to spawn (relative to the parent)
	\param localRot Optional euler angles of the entity to spawn (relative to the given parent)
	\param boneID   Optional ID of a bone to attach the entity to. -1 for no bone.
	\param play     Optional - true (default) to play the effect immediately after creation.
	
	\return The spawned entity, null if something went wrong.
	*/
	[Obsolete("Use ParticleEffectEntity.SpawnParticleEffect() instead")]
	static SCR_ParticleEmitter CreateAsChild(
		ResourceName name, IEntity parent,
		vector localPos = "0 0 0", vector localRot = "0 0 0", int boneID = -1, bool play = true)
	{
		return CreateEx(SCR_ParticleEmitter, name, localPos, localRot, parent, boneID, play);
	}
		
	/*!
	Spawns an SCR_ParticleEmitter entity instance with a given particle effect and transformation matrix.
	The same as Create() but with transform param instead of position and rotation.
	
	\param name         ResourceName of a particle effect (ptc file)
	\param transform    Entity transformation
	                    (relative to the given parent, i.e. it's a world transform when parent is null)
	\param parent       Optional parent entity, may be null.
	                    If not null, the new entity will be spawned in its hiearchy with auto-transform enabled.
	\param boneID       Optional ID of a bone to attach the entity to. -1 for none. Only used when parent is not null.
	\param play         Optional - true (default) to play the effect immediately after creation.
	
	\return The spawned entity, null if something went wrong.
	*/
	[Obsolete("Use ParticleEffectEntity.SpawnParticleEffect() instead")]
	static SCR_ParticleEmitter CreateWithTransform(
		ResourceName name, vector transform[4],
		IEntity parent = null, int boneID = -1, bool play = true)
	{
		return CreateWithTransformEx(SCR_ParticleEmitter, name, transform, parent, boneID, play);
	}
		
	/*!
	Spawns an SCR_ParticleEmitter entity instance with a given particle effect 
	at a given position with its y-axis aligned to a given up-vector.
	The effect is expected to be axisymetric around its y-axis.
	The same as Create() but with upVec param instead of rotation.
	
	\param name         ResourceName of a particle effect (ptc file)
	\param pos          Position of the entity to spawn (relative to the given parent, world position when parent is null)
	\param upVec        Vector to which the effect's y-axis will be aliged (world space)
	\param parent       Optional parent entity, may be null.
	                    If not null, the new entity will be spawned in its hiearchy with auto-transform enabled.
	\param boneID       Optional ID of a bone to attach the entity to. -1 for none. Only used when parent is not null.
	\param play         Optional - true (default) to play the effect immediately after creation.
	
	\return The spawned entity, null if something went wrong.
	*/
	[Obsolete("Use ParticleEffectEntity.SpawnParticleEffect() instead")]
	static SCR_ParticleEmitter CreateOriented(
		ResourceName name, vector pos, vector upVec,
		IEntity parent = null, int boneID = -1, bool play = true)
	{
		return CreateOrientedEx(SCR_ParticleEmitter, name, pos, upVec, parent, boneID, play);
	}
	
	/*!
	Spawns an SCR_ParticleEmitter entity instance as a child of given parent entity
	with its y-axis aligned to a given up-vector.
	The effect is expected to be axisymetric around its y-axis.
	Convenience alternative for CreateOriented().
	
	\param name         ResourceName of a particle effect (ptc file)
	\param parent       Parent entity
	\param upVec        Vector to which the effect's y-axis will be aliged (world space)
	\param localPos     Optional position of the entity to spawn (relative to the parent)
	\param boneID       Optional ID of a bone to attach the entity to. -1 for none. Only used when parent is not null.
	\param play         Optional - true (default) to play the effect immediately after creation.
	
	\return The spawned entity, null if something went wrong.
	*/
	[Obsolete("Use ParticleEffectEntity.SpawnParticleEffect() instead")]
	static SCR_ParticleEmitter CreateAsChildOriented(
		ResourceName name, IEntity parent, vector upVec,
		vector localPos = "0 0 0", int boneID = -1, bool play = true)
	{
		return CreateOrientedEx(SCR_ParticleEmitter, name, localPos, upVec, parent, boneID, play);
	}
	
	/*!
	The same as Create() but this one can spawn entities of different types.
	\param type Type of the entity to spawn. Must be inherited from SCR_ParticleEmitter.
	*/
	[Obsolete("Use ParticleEffectEntity.SpawnParticleEffect() instead")]
	static SCR_ParticleEmitter CreateEx(
		typename type, ResourceName name, vector pos,
		vector rot = "0 0 0", IEntity parent = null, int boneID = -1, bool play = true)
	{
		SCR_ParticleEmitter ent = SpawnParticleEmitter(ent, parent, type);
		
		ent.m_DeleteOnFinish = true;
		ent.SetPathToPTC(name);
		ent.SetOrigin(pos);
		ent.SetAngles(rot);
		
		if (parent)
			parent.AddChild(ent, boneID);
		
		if (play)
			ent.Play();
		
		return ent;
	}
	
	/*!
	The same as CreateWithTransform() but this one can spawn entities of different types.
	\param type Type of the entity to spawn. Must be inherited from SCR_ParticleEmitter.
	*/
	[Obsolete("Use ParticleEffectEntity.SpawnParticleEffect() instead")]
	static SCR_ParticleEmitter CreateWithTransformEx(
		typename type, ResourceName name, vector transform[4],
		IEntity parent = null, int boneID = -1, bool play = true)
	{
		SCR_ParticleEmitter ent =SpawnParticleEmitter(ent, parent, type);
		
		if (!ent)
		{
			Debug.Error("Unable to spawn a particle effect. " + type + " does not inherit SCR_ParticleEmitter.");
			return null;
		}
		
		ent.m_DeleteOnFinish = true;
		ent.SetPathToPTC(name);
		ent.SetTransform(transform);
		
		if (parent)
			parent.AddChild(ent, boneID);
		
		if (play)
			ent.Play();
		
		return ent;
	}
	
	//-----------------------------------------------------------------------------------------------------------------
	[Obsolete("Use ParticleEffectEntity.SpawnParticleEffect() instead")]
	static SCR_ParticleEmitter SpawnParticleEmitter(SCR_ParticleEmitter ent, IEntity parent, typename type)
	{
		if (!parent)
			ent = SCR_ParticleEmitter.Cast(GetGame().SpawnEntity(type));
		else
		{
			EntitySpawnParams spawnParams = new EntitySpawnParams;
			spawnParams.TransformMode = ETransformMode.WORLD;
			parent.GetWorldTransform(spawnParams.Transform);
			
			ent = SCR_ParticleEmitter.Cast(GetGame().SpawnEntity(type, parent.GetWorld(), spawnParams));
		}
		
		return ent;
	}
	
	/*!
	The same as CreateOriented() but this one can spawn entities of different types.
	\param type Type of the entity to spawn. Must be inherited from SCR_ParticleEmitter.
	*/
	[Obsolete("Use ParticleEffectEntity.SpawnParticleEffect() instead")]
	static SCR_ParticleEmitter CreateOrientedEx(
		typename type, ResourceName name, vector pos, vector upVec,
		IEntity parent = null, int boneID = -1, bool play = true)
	{
		vector transform[4];
		transform[3] = pos;
		if (parent == null)
		{
			// no parent - we are setting world transform
			Math3D.MatrixFromUpVec(upVec, transform);
			return CreateWithTransformEx(type, name, transform, parent, boneID, play);
		}
		else
		{
			// we have parent - let's create it without orientation first
			SCR_ParticleEmitter ent = CreateWithTransformEx(type, name, transform, parent, boneID, play);
			
			// now get the world transform (updated by the parent in the CreateWithTransformEx) and modify its orientation
			ent.GetWorldTransform(transform);
			Math3D.MatrixFromUpVec(upVec, transform);
			ent.SetWorldTransform(transform);
			return ent;
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------------
	
	
	
	
	
	//-----------------------------------------------------------------------------------------------------------------	
	void SetPathToPTC(ResourceName path)
	{
		m_EffectPath = path;
	}
	
	ResourceName GetPathToPTC()
	{
		return m_EffectPath;
	}
	
	//-----------------------------------------------------------------------------------------------------------------
	//! Returns whether the particle effect is playing
	bool GetIsPlaying()
	{
		return m_ePlayState == EParticleEmitterState.PLAYSTATE_PLAYING;
	}
	
	//-----------------------------------------------------------------------------------------------------------------
	//! Returns whether the particle effect is paused
	bool GetIsPaused()
	{
		return m_ePlayState == EParticleEmitterState.PLAYSTATE_PAUSED || m_ePlayState == EParticleEmitterState.PLAYSTATE_PAUSING || m_ePlayState == EParticleEmitterState.PLAYSTATE_PAUSING_AND_DELETE;
	}
	
	//-----------------------------------------------------------------------------------------------------------------
	//! Plays the particle effect
	void Play()
	{		
		if (m_EffectPath == "")
			return;
		
		if (GetIsPaused())
		{
			UnPause();
			return;
		}
		
		Resource resource = Resource.Load(m_EffectPath);
		if (!resource)
			return;
		
		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
			return;
		
		VObject particle_effect = resourceObject.ToVObject();
		if (particle_effect)
		{
			SetObject(particle_effect, "");
			SetPlayState(EParticleEmitterState.PLAYSTATE_PLAYING);
		}
	}
	
	//----------------------------------------------------------------------------------------------------------------
	//! Pauses the playing particle effect
	void Pause()
	{
		if (m_ePlayState != EParticleEmitterState.PLAYSTATE_PLAYING)
			return;
		
		Particles particles = GetParticles();
		particles.SetParam(-1, EmitterParam.BIRTH_RATE, 0);
		particles.SetParam(-1, EmitterParam.BIRTH_RATE_RND, 0);
		
		SetPlayState(EParticleEmitterState.PLAYSTATE_PAUSING);
	}
	
	//----------------------------------------------------------------------------------------------------------------
	//! Unpauses the paused particle effect
	void UnPause()
	{
		if (!GetIsPaused())
			return;
		
		Particles particles = GetParticles();
		particles.MultParam(-1, EmitterParam.BIRTH_RATE, 1);
		particles.MultParam(-1, EmitterParam.BIRTH_RATE_RND, 1);
		
		SetPlayState(EParticleEmitterState.PLAYSTATE_PLAYING);
	}
	
	//----------------------------------------------------------------------------------------------------------------
	/*!
	Pauses the particle and when all particles are gone will delete the particle Emitter
	This is to naturally delete the emitter after all particles disapear instand of instantly deleting it
	Note if the State is already stopped, paused or finished than it is instantly deleted
	Cancelled if Play() is called again
	*/
	void PauseAndDelete()
	{
		EParticleEmitterState state = GetPlayState();
		
		//~ Already in a paused state so instant delete
		if (state == EParticleEmitterState.PLAYSTATE_STOPPED || state == EParticleEmitterState.PLAYSTATE_PAUSED || state == EParticleEmitterState.PLAYSTATE_FINISHED)
		{
			delete this;
			return;
		}
		
		//~ Execute pause logics
		Pause();
		
		//~ Set state to pausing and delete
		SetPlayState(EParticleEmitterState.PLAYSTATE_PAUSING_AND_DELETE);
	}
	
	//----------------------------------------------------------------------------------------------------------------
	//! Stops the playing particle effect
	void Stop()
	{
		SetObject(null, "");
		SetPlayState(EParticleEmitterState.PLAYSTATE_STOPPED);
	}
	
	//----------------------------------------------------------------------------------------------------------------
	/*!
	Get current PlayState
	\return Current PlayState
	*/ 
	EParticleEmitterState GetPlayState()
	{
		return m_ePlayState;
	}
	
	//----------------------------------------------------------------------------------------------------------------
	//~ Set playstate
	protected void SetPlayState(EParticleEmitterState playState)
	{
		//~ State not changed
		if (m_ePlayState == playState)
			return;
		
		//~ Set new state
		m_ePlayState = playState;
		
		//~ Set or Clear OnFrame event mask
		switch (m_ePlayState)
		{
			//~ Playing Enable OnFrame
			case EParticleEmitterState.PLAYSTATE_PLAYING :
			{
				SetEventMask(EntityEvent.FRAME);
				return;
			}
			//~ Finished Disable OnFrame
			case EParticleEmitterState.PLAYSTATE_FINISHED :
			{
				ClearEventMask(EntityEvent.FRAME);
				return;
			}
			//~ Pausing Enable OnFrame
			case EParticleEmitterState.PLAYSTATE_PAUSING :
			{
				SetEventMask(EntityEvent.FRAME);
				return;
			}
			//~ Pausing and delete Enable OnFrame
			case EParticleEmitterState.PLAYSTATE_PAUSING_AND_DELETE :
			{
				SetEventMask(EntityEvent.FRAME);
				return;
			}
			//~ Paused Disable OnFrame
			case EParticleEmitterState.PLAYSTATE_PAUSED :
			{
				ClearEventMask(EntityEvent.FRAME);
				return;
			}
			//~ Stopped Disable OnFrame
			case EParticleEmitterState.PLAYSTATE_STOPPED :
			{
				ClearEventMask(EntityEvent.FRAME);
				return;
			}
		};
	}
	
	//----------------------------------------------------------------------------------------------------------------
	protected void RestartEmitter()
	{
		GetParticles().Restart();
	}
	
	//----------------------------------------------------------------------------------------------------------------
	//! Called when the particle should be updated
	//! \param timeSlice The amount of time that has passed since the last update
	protected void Update(float timeSlice)
	{
		bool finished = GetParticles().Simulate(timeSlice);
		
		//~ Animate but stop when all Particles are done
		if (m_ePlayState == EParticleEmitterState.PLAYSTATE_PLAYING)
		{
			if (!finished)
				return;

			#ifdef WORKBENCH
			if (GetGame().GetWorldEntity()) // True if the game has started
			{
				SetPlayState(EParticleEmitterState.PLAYSTATE_FINISHED);
				if (m_DeleteOnFinish)
					delete this;
			}
			else
				RestartEmitter();
			#else
			SetPlayState(EParticleEmitterState.PLAYSTATE_FINISHED);
			if (m_DeleteOnFinish)
				delete this;
			#endif
		}
		//~ Not in play state but OnFrame is still called so check if there are any particles left alive
		else if (GetParticles().GetNumParticles() <= 0)
		{
			//~ Set paused state as no particles alive
			if (m_ePlayState == EParticleEmitterState.PLAYSTATE_PAUSING)
				SetPlayState(EParticleEmitterState.PLAYSTATE_PAUSED);
			//~ Delete as no particles alive
			else if (m_ePlayState == EParticleEmitterState.PLAYSTATE_PAUSING_AND_DELETE)
				delete this;
		}
	}
	
	//----------------------------------------------------------------------------------------------------------------
	#ifdef WORKBENCH
		//------------------------------------------------------------------------------------------------------------
		override void _WB_AfterWorldUpdate(float timeSlice)
		{
			if( m_EffectPath.Length() > 0 )
			{	
				if (  m_PlayInEditor  &&  !GetIsPlaying())
					Play();
			
				if ( !m_PlayInEditor  &&  GetIsPlaying())
					Stop();
			}		
			
			if (GetEventMask() & EntityEvent.FRAME)
			{
				Update(timeSlice);
				Update(); // Update entity (it does not happen automatically in edit mode)
			}
		}
	#endif
	
	//----------------------------------------------------------------------------------------------------------------
	//! Called every frame to update the particle effect
	override void EOnFrame(IEntity owner, float timeSlice) //!EntityEvent.FRAME
	{
		Update(timeSlice);
	}
	
	//----------------------------------------------------------------------------------------------------------------
	//! Called when the particle emitter entity is initialized
	override event void EOnInit(IEntity owner) //!EntityEvent.INIT
	{
		if (m_PlayOnSpawn)
			Play();
	}

	//----------------------------------------------------------------------------------------------------------------
	void SCR_ParticleEmitter(IEntitySource src, IEntity parent)
	{
		SetFlags(EntityFlags.VISIBLE);
		SetEventMask(EntityEvent.INIT);
	}
	
	//----------------------------------------------------------------------------------------------------------------
	void ~SCR_ParticleEmitter()
	{
		SetObject(null, "");
	}
	
	
	
	//----------------------------------------------------------------------------------------------------------------
	// OBSOLETE METHODS
	//----------------------------------------------------------------------------------------------------------------
	
	[Obsolete("Use CreateWithTransform()")]
	static SCR_ParticleEmitter CreateWithTransformAsChild(
		ResourceName name, IEntity parent, vector transform[4],
		int boneID = -1, bool play = true)
	{
		return CreateWithTransformAsChildEx(SCR_ParticleEmitter, name, parent, transform, boneID, play);
	}
		
	[Obsolete("Use CreateWithTransformEx()")]
	static SCR_ParticleEmitter CreateWithTransformAsChildEx(
		typename type, ResourceName name, IEntity parent, vector localTransform[4],
		int boneID = -1, bool play = true)
	{
		return CreateWithTransformEx(type, name, localTransform, parent, boneID, play);
	}
		
	[Obsolete("Use CreateEx()")]
	static SCR_ParticleEmitter CreateAsChildEx(
		typename type, ResourceName name, IEntity parent,
		vector localPos = "0 0 0", vector localRot = "0 0 0", int boneID = -1, bool play = true)
	{
		return CreateEx(type, name, localPos, localRot, parent, boneID, play);
	}
	
};

/**
Particle Emitter state enum
*/
enum EParticleEmitterState
{
	PLAYSTATE_STOPPED 				= 10,
	PLAYSTATE_PLAYING 				= 20,
	PLAYSTATE_PAUSING 				= 30,
	PLAYSTATE_PAUSING_AND_DELETE 	= 40,
	PLAYSTATE_PAUSED 				= 50,
	PLAYSTATE_FINISHED 				= 60,
};
