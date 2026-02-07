class SCR_AudioSource
{	 
	//! Stores valid Audio handle
	AudioHandle m_AudioHandle = AudioHandle.Invalid;
	//! Paren entity audio source is linked to
	IEntity m_Owner;
	//!  Transformation where sound was triggered
	protected vector m_aMat[4];
	//! Audio source configuration
	ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;	
	//! Signal names
	protected ref array<string> m_aSignalName;
	//! Signal values
	protected ref array<float> m_aSignalValue;
	//! Interior callback
	ref SCR_InteriorRequestCallback m_InteriorRequestCallback;
	//! Sound is terminated
	bool m_bTerminated = false;
	
	//! Local signal names
	static const string INTERIOR_SIGNAL_NAME = "Interior";
	static const string SURFACE_SIGNAL_NAME = "Surface";
	static const string ENTITY_SIZE_SIGNAL_NAME = "EntitySize";
	static const string PHASES_TO_DESTROYED_PHASE_SIGNAL_NAME = "PhasesToDestroyed";
	static const string COLLISION_D_V_SIGNAL_NAME = "CollisionDV";
	static const string DISTANCE_SINAL_NAME = "Distance";
	static const string ROOM_SIZE_SIGNAL_NAME = "RoomSize";
	static const string FOREST_SIGNAL_NAME = "Forest";
	static const string HOUSES_SIGNAL_NAME = "Houses";
	static const string MEADOWS_SIGNAL_NAME = "Meadows";
	static const string SEA_SIGNAL_NAME = "Sea";
	
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
	void SetGlobalOcclusionSignals()
	{	
		GameSignalsManager gameSignalsManager = GetGame().GetSignalsManager();
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
			
		SetSignalValue(SCR_SoundManagerEntity.G_INTERIOR_SIGNAL_NAME, gameSignalsManager.GetSignalValue(soundManagerEntity.GetGInteriorSignalIdx()));
		SetSignalValue(SCR_SoundManagerEntity.G_CURR_VEHICLE_COVERAGE_SIGNAL_NAME, gameSignalsManager.GetSignalValue(soundManagerEntity.GetGCurrVehicleCoverageSignalIdx()));
		SetSignalValue(SCR_SoundManagerEntity.G_IS_THIRD_PERSON_CAM_SIGNAL_NAME, gameSignalsManager.GetSignalValue(soundManagerEntity.GetGIsThirdPersonCamSignalIdx()));
		SetSignalValue(SCR_SoundManagerEntity.G_ROOM_SIZE, gameSignalsManager.GetSignalValue(soundManagerEntity.GetRoomSizeIdx()));
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Sets environmental signals based on SoundMap
	*/
	void SetEnvironmentalSignals(SoundWorld soundWorld, vector pos)
	{
		if (!soundWorld || !SCR_Enum.HasFlag(m_AudioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.EnvironmentSignals))
			return;
		
		float sea, forest, city, meadow;	
		soundWorld.GetMapValuesAtPos(pos, sea, forest, city, meadow);
		
		SetSignalValue(SEA_SIGNAL_NAME, sea);
		SetSignalValue(FOREST_SIGNAL_NAME, forest);
		SetSignalValue(HOUSES_SIGNAL_NAME, city);	
		SetSignalValue(MEADOWS_SIGNAL_NAME, meadow);
	}
	
	//------------------------------------------------------------------------------------------------
	bool Play()
	{		
		// Play event
		m_AudioHandle = AudioSystem.PlayEvent(m_AudioSourceConfiguration.m_sSoundProject, m_AudioSourceConfiguration.m_sSoundEventName, m_aMat, m_aSignalName, m_aSignalValue);

		// Check if AudioHandle is valid
		if (m_AudioHandle == AudioHandle.Invalid)
			return false;
		
		// Set bounding volume size
		if (m_Owner && SCR_Enum.HasFlag(m_AudioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.BoundingVolume))
		{
			// Get world bounding box
			vector mins, maxs;			
			m_Owner.GetWorldBounds(mins, maxs);													
			AudioSystem.SetBoundingVolumeParams(m_AudioHandle, AudioSystem.BV_Box, maxs[0] - mins[0], maxs[1] - mins[1], maxs[2] - mins[2]);
		}	
						
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTransformation(vector mat[4])
	{
		m_aMat = mat;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateTransformation()
	{
		m_Owner.GetTransform(m_aMat);
		AudioSystem.SetSoundTransformation(m_AudioHandle, m_aMat);	
	}
	
	//------------------------------------------------------------------------------------------------
	void Terminate()
	{
		AudioSystem.TerminateSound(m_AudioHandle);
		m_bTerminated = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AudioSource(IEntity owner, SCR_AudioSourceConfiguration audioSourceConfiguration, float distance)
	{
		m_AudioSourceConfiguration = audioSourceConfiguration;
		m_Owner = owner;
		
		// Set distance signal
		SetSignalValue(DISTANCE_SINAL_NAME, distance);
	}
}