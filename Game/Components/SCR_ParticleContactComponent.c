class SCR_ParticleContactComponentClass : ScriptComponentClass
{	
	[Attribute("", UIWidgets.Auto, "", category: "Sound")]
	ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
	
	[Attribute("true", desc: "Set surface signal", category: "Sound")]
	bool m_bSurfaceSignal;
		
	[Attribute("true", desc: "Disable OnContact after the first contact")]
	bool m_bFirstContactOnly;
	
	[Attribute("false", desc: "Particle oriented to surface", category: "VFX")]
	bool m_bParticleOriented;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, desc: "Particle effect", params: "ptc", category: "VFX")]
	ResourceName m_Particle;
}

//------------------------------------------------------------------------------------------------
class SCR_ParticleContactComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	private void PlaySound(IEntity owner, SCR_ParticleContactComponentClass prefabData, Contact contact)
	{
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;
				
		if (!prefabData.m_AudioSourceConfiguration || !prefabData.m_AudioSourceConfiguration.IsValid())
			return;
				
		// Create audio source
		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(owner, prefabData.m_AudioSourceConfiguration);	
		if (!audioSource)
			return;

		// Set surface signal
		if (prefabData.m_bSurfaceSignal)
		{
			GameMaterial material = contact.Material2;
			if (material)
				audioSource.SetSignalValue(SCR_AudioSource.SURFACE_SIGNAL_NAME, material.GetSoundInfo().GetSignalValue());
		}
		
		// Play sound
		if (SCR_Enum.HasFlag(prefabData.m_AudioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.Static))
		{
			vector mat[4];
			mat[3] = contact.Position;
			
			soundManagerEntity.PlayAudioSource(audioSource, mat);
		}
		else		
			soundManagerEntity.PlayAudioSource(audioSource);		
	}
	
	//------------------------------------------------------------------------------------------------			
	override void OnPostInit(IEntity owner)
	{
		SCR_ParticleContactComponentClass prefabData = SCR_ParticleContactComponentClass.Cast(GetComponentData(owner));
		if (!prefabData)
			return;
				
		SetEventMask(owner, EntityEvent.CONTACT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		// Get prefab data
		SCR_ParticleContactComponentClass prefabData = SCR_ParticleContactComponentClass.Cast(GetComponentData(owner));
		if (!prefabData)
		{
			ClearEventMask(owner, EntityEvent.CONTACT);
			return;
		}
				
		// Play sound
		PlaySound(owner, prefabData, contact);
		
		// Play VFX
		if (prefabData.m_Particle != string.Empty)
		{
			if (prefabData.m_bParticleOriented)	
				SCR_ParticleEmitter.Create(prefabData.m_Particle, contact.Position, contact.Normal);
			else
				SCR_ParticleEmitter.Create(prefabData.m_Particle, contact.Position);
		}
				
		// Disable OnContact after the first contact
		if (prefabData.m_bFirstContactOnly)
			ClearEventMask(owner, EntityEvent.CONTACT);
	}
}