class SCR_AudioSource
{	 
	//! Stores valid Audio handle
	AudioHandle m_AudioHandle = AudioHandle.Invalid;
	//! Paren entity audio source is linked to
	IEntity m_Owner;
	//! Audio source configuration
	ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;	
	//! Signal names
	ref array<string> m_aSignalName;
	//! Signal values
	ref array<float> m_aSignalValue;
	
	//! Local signal names
	static const string INTERIOR_SIGNAL_NAME = "Interior";
	static const string SURFACE_SIGNAL_NAME = "Surface";
	static const string ENTITY_SIZE_SIGNAL_NAME = "EntitySize";
	static const string PHASES_TO_DESTROYED_PHASE_SIGNAL_NAME = "PhasesToDestroyed";
	static const string COLLISION_D_V_SIGNAL_NAME = "CollisionDV";
	static const string DISTANCE_SINAL_NAME = "Distance";
	
	//------------------------------------------------------------------------------------------------	
	/*!
	Sets value for given signal name 
	\name Signal name
	\value Signal value
	*/
	void SetSignalValue(string name, float value)
	{
		if (!m_aSignalName)
			m_aSignalName = {};
		
		if (!m_aSignalValue)
			m_aSignalValue = {};
		
		m_aSignalName.Insert(name);
		m_aSignalValue.Insert(value);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Sets all occlusion signals
	*/
	void SetOcclusionSignals()
	{	
		GameSignalsManager gameSignalsManager = GetGame().GetSignalsManager();
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
			
		SetSignalValue(SCR_SoundManagerEntity.G_INTERIOR_SIGNAL_NAME, gameSignalsManager.GetSignalValue(soundManagerEntity.GetGInteriorSignalIdx()));
		SetSignalValue(SCR_SoundManagerEntity.G_CURR_VEHICLE_COVERAGE_SIGNAL_NAME, gameSignalsManager.GetSignalValue(soundManagerEntity.GetGCurrVehicleCoverageSignalIdx()));
		SetSignalValue(SCR_SoundManagerEntity.G_IS_THIRD_PERSON_CAM_SIGNAL_NAME, gameSignalsManager.GetSignalValue(soundManagerEntity.GetGIsThirdPersonCamSignalIdx()));
	
		// TODO: Set Interior signal	
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AudioSource(IEntity owner, SCR_AudioSourceConfiguration audioSourceConfiguration, float distance)
	{
		m_AudioSourceConfiguration = audioSourceConfiguration;
		m_Owner = owner;
		
		// Set distance signal
		if (SCR_Enum.HasFlag(audioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.DistanceSignal))
			SetSignalValue(DISTANCE_SINAL_NAME, distance);
	}
}