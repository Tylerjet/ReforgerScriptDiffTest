//------------------------------------------------------------------------------------------------
// THIS FILE CONTAINS COMMON FUNCTIONS USED BY THE VARIOUS DESTRUCTION SYSTEMS
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//! Class containing common functions between destructible classes
class SCR_DestructionCommon
{
	static const float PARTICLE_IMPULSE_DEFAULT = 0.5;
	static const float PARTICLE_IMPULSE_INCENDIARY = 1;
	static const float PARTICLE_IMPULSE_EXPLOSION = 1;
	static const float PARTICLE_IMPULSE_COLLISION = 0.5;
	
	//------------------------------------------------------------------------------------------------
	//! Returns particle velocity impulse multiplier for the given damage type
	static float GetPTCImpulseScale(EDamageType type)
	{
		float impulseScale = PARTICLE_IMPULSE_DEFAULT;
		
		switch (type)
		{
			case EDamageType.INCENDIARY:
			{
				impulseScale = PARTICLE_IMPULSE_INCENDIARY;
				break;
			}
			case EDamageType.EXPLOSIVE:
			{
				impulseScale = PARTICLE_IMPULSE_EXPLOSION;
				break;
			}
			case EDamageType.COLLISION:
			{
				impulseScale = PARTICLE_IMPULSE_COLLISION;
				break;
			}
		}
		
		return impulseScale;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays a particle effect to represent destruction of a fraction (shard/splinter) of an object
	static SCR_ParticleEmitter PlayParticleEffect_FractionDestruction(IEntity entity, ResourceName particlePath, EDamageType type, vector hitPos, vector hitDir)
	{
		if (!entity)
			return null;
		
		if (particlePath == ResourceName.Empty)
			return null;
		
		SCR_ParticleEmitter ptc =SCR_ParticleEmitter.Create(particlePath, hitPos);
		if (!ptc)
			return null;
		
		vector fw = hitDir * 0.5 + entity.GetWorldTransformAxis(2);
		fw.Normalize();
		vector rt = vector.Up * fw;
		rt.Normalize();
		vector up = fw * rt;
		up.Normalize();
		
		vector ptcMat[4];
		ptcMat[0] = rt;
		ptcMat[1] = up;
		ptcMat[2] = fw;
		ptcMat[3] = hitPos;
		
		ptc.SetTransform(ptcMat);
		
		float velocityScale = GetPTCImpulseScale(type);
		Particles particles = ptc.GetParticles();
		particles.MultParam(-1, EmitterParam.VELOCITY,     velocityScale);
		particles.MultParam(-1, EmitterParam.VELOCITY_RND, velocityScale);
		
		return ptc;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays a particle effect to represent entire object destruction
	static SCR_ParticleEmitter PlayParticleEffect_CompleteDestruction(IEntity entity, ResourceName particlePath, EDamageType damageType, bool atBoundBoxCenter = true)
	{
		if (!entity)
			return null;
		
		if (particlePath == ResourceName.Empty)
			return null;
		
		SCR_ParticleEmitter ptc;
		if (atBoundBoxCenter)
		{
			vector mins, maxs;
			entity.GetBounds(mins, maxs);
			vector center = (maxs - mins) * 0.5 + mins;
			ptc = SCR_ParticleEmitter.Create(particlePath, entity.CoordToParent(center), entity.GetAngles());
		}
		else
			ptc = SCR_ParticleEmitter.Create(particlePath, entity.GetOrigin(), entity.GetAngles());
		if (!ptc)
			return null;
		
		float velocityScale = GetPTCImpulseScale(damageType);
		Particles particles = ptc.GetParticles();
		particles.MultParam(-1, EmitterParam.VELOCITY,     velocityScale);
		particles.MultParam(-1, EmitterParam.VELOCITY_RND, velocityScale);
		
		return ptc;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays a particle effect as child of entity
	static SCR_ParticleEmitter PlayParticleEffect_Child(ResourceName particlePath, EDamageType damageType, notnull IEntity parent, vector mat[4])
	{
		if (particlePath == ResourceName.Empty)
			return null;
		
		vector angles = Math3D.MatrixToAngles(mat);
		
		SCR_ParticleEmitter ptc = SCR_ParticleEmitter.CreateAsChild(particlePath, parent, parent.CoordToLocal(mat[3]), angles);
		if (!ptc)
			return null;
		
		float velocityScale = GetPTCImpulseScale(damageType);
		Particles particles = ptc.GetParticles();
		particles.MultParam(-1, EmitterParam.VELOCITY,     velocityScale);
		particles.MultParam(-1, EmitterParam.VELOCITY_RND, velocityScale);
		
		return ptc;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays a particle effect at input transformation matrix
	static SCR_ParticleEmitter PlayParticleEffect_Transform(ResourceName particlePath, EDamageType damageType, vector mat[4])
	{
		if (particlePath == ResourceName.Empty)
			return null;
		
		SCR_ParticleEmitter ptc = SCR_ParticleEmitter.CreateWithTransform(particlePath, mat);
		if (!ptc)
			return null;
		
		float velocityScale = GetPTCImpulseScale(damageType);
		Particles particles = ptc.GetParticles();
		if (particles)
		{
			particles.MultParam(-1, EmitterParam.VELOCITY,     velocityScale);
			particles.MultParam(-1, EmitterParam.VELOCITY_RND, velocityScale);
		}
		
		return ptc;
	}
};
