class SCR_ParticleContactComponentClass : ScriptComponentClass
{	
	[Attribute("", UIWidgets.EditBox, desc: "Sound event name", category: "Sound")]
	string m_sSoundEventName;
	
	[Attribute("true", desc: "Disable OnContact after the first contact")]
	bool m_bFirstContactOnly;
		
	[Attribute("false", desc: "Particle oriented to surface", category: "VFX")]
	bool m_bParticleOriented;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, desc: "Particle effect", params: "ptc", category: "VFX")]
	ResourceName m_Particle;
}

class SCR_ParticleContactComponent : ScriptComponent
{	
	private static const string SURFACE_SIGNAL_NAME = "Surface";
		
	override void OnPostInit(IEntity owner)
	{
		SCR_ParticleContactComponentClass prefabData = SCR_ParticleContactComponentClass.Cast(GetComponentData(owner));
		if (!prefabData)
			return;
				
		SetEventMask(owner, EntityEvent.CONTACT);
	}

	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		// Get prefab data
		SCR_ParticleContactComponentClass prefabData = SCR_ParticleContactComponentClass.Cast(GetComponentData(owner));
		if (!prefabData)
		{
			ClearEventMask(owner, EntityEvent.CONTACT);
			return;
		}
		
		// Set Surface signal
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		if (signalsManagerComponent)
		{
			GameMaterial material = contact.Material2;
			if (material)
				signalsManagerComponent.SetSignalValue(signalsManagerComponent.AddOrFindSignal(SURFACE_SIGNAL_NAME), material.GetSoundInfo().GetSignalValue());
		}
		
		// Play sound
		SoundComponent soundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
		if (soundComponent)
		{		
			// Get sound position
			vector mat[4];		
			mat[3] = owner.GetOrigin();
			
			soundComponent.SetTransformation(mat);
			
			// Play sound		
			soundComponent.SoundEvent(prefabData.m_sSoundEventName);
		}
		
		// Play VFX
		if (prefabData.m_Particle != string.Empty)
		{
			if (prefabData.m_bParticleOriented)	
			{
				SCR_ParticleEmitter.CreateOriented(prefabData.m_Particle, contact.Position, contact.Normal);
			}
			else {
				
				SCR_ParticleEmitter.Create(prefabData.m_Particle, contact.Position);
			}
		}
				
		// Disable OnContact after the first contact
		if (prefabData.m_bFirstContactOnly)
			ClearEventMask(owner, EntityEvent.CONTACT);
	}
}