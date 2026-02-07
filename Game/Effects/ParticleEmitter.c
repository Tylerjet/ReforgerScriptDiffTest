[EntityEditorProps(category: "GameScripted", description: "Particle emitter", color: "32 94 200 255")]
class SCR_ParticleEmitterClass: GenericEntityClass
{
	const int PLAYSTATE_STOPPED = 0;
	const int PLAYSTATE_PLAYING = 1;
	const int PLAYSTATE_PAUSED = 2;
	const int PLAYSTATE_FINISHED = 3;
};

class SCR_ParticleEmitter : GenericEntity
{
	static const int EMITTERS_MAX = SCR_ParticleAPI.EMITTERS_MAX;//This magic trick is necesarry to avoid compilation errors related to the usage of EMITTERS_MAX in this script
	
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
	
	private int m_PlayState = SCR_ParticleEmitterClass.PLAYSTATE_STOPPED;
	
	void SetPathToPTC(ResourceName path)
	{
		m_EffectPath = path;
	}
	
	ResourceName GetPathToPTC()
	{
		return m_EffectPath;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns whether the particle effect is playing
	bool GetIsPlaying()
	{
		if (m_PlayState == SCR_ParticleEmitterClass.PLAYSTATE_PLAYING)
			return true;
		else
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns whether the particle effect is paused
	bool GetIsPaused()
	{
		if (m_PlayState == SCR_ParticleEmitterClass.PLAYSTATE_PAUSED)
			return true;
		else
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays the particle effect
	void Play()
	{
		if (m_EffectPath == "")
			return;
		
		if (m_PlayState == SCR_ParticleEmitterClass.PLAYSTATE_PAUSED)
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
			m_PlayState = SCR_ParticleEmitterClass.PLAYSTATE_PLAYING;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Pauses the playing particle effect
	void Pause()
	{
		if (m_PlayState != SCR_ParticleEmitterClass.PLAYSTATE_PLAYING)
			return;
		
		SCR_ParticleAPI.LerpAllEmitters(this, 0, EmitterParam.BIRTH_RATE);
		SCR_ParticleAPI.LerpAllEmitters(this, 0, EmitterParam.BIRTH_RATE_RND);
		
		m_PlayState = SCR_ParticleEmitterClass.PLAYSTATE_PAUSED;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unpauses the paused particle effect
	void UnPause()
	{
		if (m_PlayState != SCR_ParticleEmitterClass.PLAYSTATE_PAUSED)
			return;
		
		SCR_ParticleAPI.LerpAllEmitters(this, 1, EmitterParam.BIRTH_RATE);
		SCR_ParticleAPI.LerpAllEmitters(this, 1, EmitterParam.BIRTH_RATE_RND);
		
		m_PlayState = SCR_ParticleEmitterClass.PLAYSTATE_PLAYING;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stops the playing particle effect
	void Stop()
	{
		SetObject(null, "");
		m_PlayState = SCR_ParticleEmitterClass.PLAYSTATE_STOPPED;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the particle should be updated
	//! \param timeSlice The amount of time that has passed since the last update
	private void Update(float timeSlice)
	{
		bool finished = false;
		if (m_PlayState == SCR_ParticleEmitterClass.PLAYSTATE_PLAYING  ||  m_PlayState == SCR_ParticleEmitterClass.PLAYSTATE_PAUSED)
			finished = Animate(timeSlice, 0);
		
		if (finished)
		{
			#ifdef WORKBENCH
				if (GetGame().GetWorldEntity()) // True if the game has started
				{
					m_PlayState = SCR_ParticleEmitterClass.PLAYSTATE_FINISHED;
					if (m_DeleteOnFinish)
						delete this;
				}
				else
					RestartParticle();
			#else
				m_PlayState = SCR_ParticleEmitterClass.PLAYSTATE_FINISHED;
				if (m_DeleteOnFinish)
					delete this;
			#endif
		}
	}
	
	//------------------------------------------------------------------------------------------------
	#ifdef WORKBENCH
		//------------------------------------------------------------------------------------------------
		override void _WB_AfterWorldUpdate(float timeSlice)
		{
			if( m_EffectPath.Length() > 0 )
			{	
				if (  m_PlayInEditor  &&  !GetIsPlaying())
					Play();
			
				if ( !m_PlayInEditor  &&  GetIsPlaying())
					Stop();
			}		
			
			Update(timeSlice);
			Update(); // Update entity (it does not happen automatically in edit mode)
		}
	#endif
	
	//------------------------------------------------------------------------------------------------
	//! Called every frame to update the particle effect
	override void EOnFrame(IEntity owner, float timeSlice) //!EntityEvent.FRAME
	{
		Update(timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the particle emitter entity is initialized
	override event void EOnInit(IEntity owner) //!EntityEvent.INIT
	{
		if (m_PlayOnSpawn)
			Play();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_ParticleEmitter(IEntitySource src, IEntity parent)
	{
		SetFlags(EntityFlags.ACTIVE | EntityFlags.VISIBLE, false);
		SetEventMask(EntityEvent.FRAME | EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ParticleEmitter()
	{
		SetObject(null, "");
	}
};