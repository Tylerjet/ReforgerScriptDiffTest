[ComponentEditorProps(icon: HYBRID_COMPONENT_ICON)]
class SCR_HelicopterExhaustEffectComponentClass: MotorExhaustEffectComponentClass
{
};

class SCR_HelicopterExhaustEffectComponent : MotorExhaustEffectComponent
{
	IEntity m_Effect;
	
	[Attribute( "", UIWidgets.EditBox, "Speed in m/s which tops the effect's interpolation from 100 to 0% lifetime value" )]
	float m_ExhaustEndSpeedInM;
	
	override void OnFrame(IEntity owner, float timeSlice)
	{
		Physics physics = owner.GetPhysics();
		
		if (!m_Effect)
			m_Effect = GetParticleEntity();
		
		if (physics  &&  m_Effect  &&  m_ExhaustEndSpeedInM > 0)
		{
			vector velocity_vector = physics.GetVelocity();
			float speed_m_per_s = velocity_vector.Length();
			
			float lifetime_scale = Math.Clamp( 1 - (speed_m_per_s / m_ExhaustEndSpeedInM) , 0 , 1 );
			
			SCR_ParticleAPI.LerpAllEmitters(m_Effect,  lifetime_scale, EmitterParam.LIFETIME );
			SCR_ParticleAPI.LerpAllEmitters(m_Effect,  lifetime_scale, EmitterParam.LIFETIME_RND );
		}
	}
};