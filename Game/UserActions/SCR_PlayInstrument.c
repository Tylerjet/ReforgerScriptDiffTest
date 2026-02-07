//------------------------------------------------------------------------------------------------
class SCR_PlayInstrument : ScriptedUserAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Sound project (acp)")]
	private ResourceName m_SoundProject;
	
	protected AudioHandle m_AudioHandle = AudioHandle.Invalid;
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
		SoundComponent soundComponent = SoundComponent.Cast(pOwnerEntity.FindComponent(SoundComponent));
		
		if (!soundComponent)
			return;
		
		if (!soundComponent.IsFinishedPlaying(m_AudioHandle))
			soundComponent.Terminate(m_AudioHandle);
						
		m_AudioHandle = soundComponent.SoundEvent(SCR_SoundEvent.SOUND_PLAY_INSTRUMENT);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "#AR-UserAction_Play";
		return true;
	}
	
	void ~SCR_PlayInstrument()
	{
		// Terminate sound
		AudioSystem.TerminateSound(m_AudioHandle);
	}
};