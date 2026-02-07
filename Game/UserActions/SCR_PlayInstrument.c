//------------------------------------------------------------------------------------------------
class SCR_PlayInstrument : ScriptedUserAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Sound project (acp)")]
	private ResourceName m_SoundProject;
	
	protected AudioHandle m_AudioHandle;
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
		if (!AudioSystem.IsSoundPlayed(m_AudioHandle))
			AudioSystem.TerminateSound(m_AudioHandle);
				
		// Play sound
		ref array<string> signalName = new array<string>;
		ref array<float> signalValue = new array<float>;
		vector mat[4];
		
		mat[3] = pOwnerEntity.GetOrigin();
		
		m_AudioHandle = AudioSystem.PlayEvent(m_SoundProject, "SOUND_PLAY_INSTRUMENT", mat, signalName, signalValue);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "#AR-UserAction_Play";
		return true;
	}
	
	void ~SCR_PlayInstrument()
	{
		// Tesminate sound
		AudioSystem.TerminateSound(m_AudioHandle);
	}
};