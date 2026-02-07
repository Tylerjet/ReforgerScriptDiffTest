//------------------------------------------------------------------------------------------------
class SCR_FlushToilet : ScriptedUserAction
{	
	[Attribute("", UIWidgets.Auto)]
	ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
	
	[Attribute("", UIWidgets.Coords)]
	private vector m_vSoundOffset;
	
	private AudioHandle m_AudioHandle = AudioHandle.Invalid;
	
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
		
		vector mat[4];
		mat[3] = pOwnerEntity.CoordToParent(m_vSoundOffset);
		
		soundManagerEntity.PlayAudioSource(audioSource, mat);
	
		m_AudioHandle = audioSource.m_AudioHandle;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "#AR-UserAction_FlushToilet";
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{		
		return AudioSystem.IsSoundPlayed(m_AudioHandle);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_FlushToilet()
	{
		AudioSystem.TerminateSound(m_AudioHandle);
	}
};