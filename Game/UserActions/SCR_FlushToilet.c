//------------------------------------------------------------------------------------------------
class SCR_FlushToilet : ScriptedUserAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Sound project (acp)")]
	private ResourceName m_SoundProject;
	
	protected AudioHandle m_AudioHandle;
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{						
		// Get global signals manager
		GameSignalsManager globalSignalsManager = GetGame().GetSignalsManager();
		
		// SetSignals
		ref array<string> signalName = new array<string>;
		ref array<float> signalValue = new array<float>;
		
		signalName.Insert("GInterior");
		signalName.Insert("GIsThirdPersonCam");
		signalName.Insert("GCurrVehicleCoverage");
		
		foreach(string signal : signalName)
		{
			signalValue.Insert(globalSignalsManager.GetSignalValue(globalSignalsManager.AddOrFindSignal(signal)));
		}
			
		// Set sound position
		vector mat[4];		
		mat[3] = pOwnerEntity.GetOrigin();
		
		// Play sound
		m_AudioHandle = AudioSystem.PlayEvent(m_SoundProject, "SOUND_TOILET", mat, signalName, signalValue);
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