//------------------------------------------------------------------------------------------------
class SCR_PlayInstrument : ScriptedUserAction
{
	[Attribute("", UIWidgets.Auto)]
	ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
	
	protected AudioHandle m_AudioHandle = AudioHandle.Invalid;
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;
				
		if (!m_AudioSourceConfiguration || !m_AudioSourceConfiguration.IsValid())
			return;
		
		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(pOwnerEntity, m_AudioSourceConfiguration);
		if (!audioSource)
			return;
			
		AudioSystem.TerminateSound(m_AudioHandle);
		soundManagerEntity.PlayAudioSource(audioSource);			
		m_AudioHandle = audioSource.m_AudioHandle;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "#AR-UserAction_Play";
		return true;
	}
	
	void ~SCR_PlayInstrument()
	{
		AudioSystem.TerminateSound(m_AudioHandle);
	}
};