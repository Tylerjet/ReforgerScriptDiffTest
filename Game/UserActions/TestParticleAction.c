// Script File
class TestParticleAction : ScriptedUserAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab of fire particle used for a fire action.", "ptc")]
	ResourceName m_sParticle;	
	[Attribute("0 0.2 0", UIWidgets.EditBox, "Particle offset in local space from the origin of the entity", "")]
	vector m_vParticleOffset;
	

	private IEntity m_ParticleEntity = null;
	private IEntity m_Owner = null;
	
	private bool m_bIsPlaying = false;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) 
	{
		if (!GetGame().GetWorldEntity())
			return;
		
		m_Owner = pOwnerEntity;
	}
	
	
	//------------------------------------------------------------------------------------------------
	void PlayParticle()
	{
		if (!m_ParticleEntity)
		{
			m_ParticleEntity = SCR_ParticleEmitter.CreateAsChild(m_sParticle, m_Owner, m_vParticleOffset);
			m_bIsPlaying = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void StopParticle()
	{
		if (m_ParticleEntity)
		{
			delete m_ParticleEntity;
			m_ParticleEntity = null;
		}
		
		m_bIsPlaying = false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{	
		if (m_sParticle == string.Empty)
			return;
		
		// Standard behaviour
		if (!m_bIsPlaying)
		{
			PlayParticle();
		}
		else
		{
			StopParticle();
		}
	}
};