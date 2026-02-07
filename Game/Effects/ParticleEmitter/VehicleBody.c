class VehicleBodyEffectBaseClass: SCR_ParticleEmitterClass
{
};

class VehicleBodyEffectBase : SCR_ParticleEmitter
{
	ScriptComponent m_ComponentOwner;
	
	void VehicleBodyEffectBase(IEntitySource src, IEntity parent)
	{}
	
	void UpdateVehicleDustEffect(float speed, float start_speed, float end_speed)
	{
		float size_coef = Math.AbsFloat(  0.2 + ( (speed - start_speed)*0.8)  / end_speed );
		float speed_coef = Math.AbsFloat( 1 + ( (speed - start_speed)*0.5)  / end_speed );
		float gravity_coef = Math.AbsFloat( 0.8 + ( (speed - start_speed)*0.2)  / end_speed );
		
		SCR_ParticleAPI.LerpAllEmitters(this, size_coef, EmitterParam.SIZE);
		SCR_ParticleAPI.LerpAllEmitters(this, gravity_coef, EmitterParam.GRAVITY_SCALE_RND);
		SCR_ParticleAPI.LerpAllEmitters(this, speed_coef, EmitterParam.VELOCITY);
		SCR_ParticleAPI.LerpAllEmitters(this, speed_coef, EmitterParam.VELOCITY_RND);
	}
};