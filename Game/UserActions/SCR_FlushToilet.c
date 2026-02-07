//------------------------------------------------------------------------------------------------
class SCR_FlushToilet : ScriptedUserAction
{	
	protected AudioHandle m_AudioHandle = AudioHandle.Invalid;
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{						
		SoundComponent soundComponent = SoundComponent.Cast(pOwnerEntity.FindComponent(SoundComponent));
		
		if (!soundComponent)
			return;
		
		// Play sound
		m_AudioHandle = soundComponent.SoundEvent(SCR_SoundEvent.SOUND_TOILET);
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
		if (!AudioSystem.IsSoundPlayed(m_AudioHandle))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_FlushToilet()
	{
		// Tesminate sound
		AudioSystem.TerminateSound(m_AudioHandle);
	}
};