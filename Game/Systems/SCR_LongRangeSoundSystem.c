class SCR_LongRangeSoundSystem : LongRangeSoundSystem
{
	override protected void SoundEventTriggered(vector position, ResourceName soundProject, string soundEventName)
	{
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;
		
		SCR_AudioSourceConfiguration sourceConfig = new SCR_AudioSourceConfiguration();
		sourceConfig.m_sSoundProject = soundProject;
		sourceConfig.m_sSoundEventName = soundEventName;
		sourceConfig.m_eFlags |= EAudioSourceConfigurationFlag.FinishWhenEntityDestroyed | EAudioSourceConfigurationFlag.Static | EAudioSourceConfigurationFlag.EnvironmentSignals;
		
		if (!sourceConfig.IsValid())
			return;
		
		float distance = AudioSystem.IsAudible(sourceConfig.m_sSoundProject, sourceConfig.m_sSoundEventName, position);
		if (distance < 0)
			return;
		
		SCR_AudioSource audioSource = new SCR_AudioSource(null, sourceConfig, distance);
		
		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		mat[3] = position;
		
		soundManagerEntity.PlayAudioSource(audioSource, mat);
	}
}
