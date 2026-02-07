[ComponentEditorProps(icon: HYBRID_COMPONENT_ICON)]
class SCR_HelicopterExhaustEffectComponentClass: MotorExhaustEffectComponentClass
{
};

class SCR_HelicopterExhaustEffectComponent : MotorExhaustEffectComponent
{
	Particles m_Effect;
	
	[Attribute( "", UIWidgets.EditBox, "Speed in m/s which tops the effect's interpolation from 100 to 0% lifetime value" )]
	float m_ExhaustEndSpeedInM;
	
	override void OnFrame(IEntity owner, float timeSlice)
	{
		Physics physics = owner.GetPhysics();
		
		if (!m_Effect)
			m_Effect = GetParticleEntity().GetParticles();
		
		if (physics  &&  m_Effect  &&  m_ExhaustEndSpeedInM > 0)
		{
			vector velocity_vector = physics.GetVelocity();
			float speed_m_per_s = velocity_vector.Length();
			
			float lifetime_scale = Math.Clamp( 1 - (speed_m_per_s / m_ExhaustEndSpeedInM) , 0 , 1 );
			
			m_Effect.MultParam(-1, EmitterParam.LIFETIME,     lifetime_scale);
			m_Effect.MultParam(-1, EmitterParam.LIFETIME_RND, lifetime_scale);
		}
	}
};