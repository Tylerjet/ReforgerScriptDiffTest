[ComponentEditorProps(category: "GameScripted/Sound", description: "THIS IS THE SCRIPT DESCRIPTION.", color: "0 0 255 255")]
class SCR_HelicopterSoundComponentClass: VehicleSoundComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_HelicopterSoundComponent : VehicleSoundComponent
{
	const float ENGINEON_TRESHOLD = 0.01;
	
	// Signals Idxs
	protected int m_iSignalSpeedIdx;
	protected int m_iSignalPitchIdx;
	protected int m_iSignalRollIdx;
	protected int m_iSignalRotorAngleYPrimaryIdx;
	protected int m_iSignalRotorRPMScaledprimaryIdx;
	
	// Components
	protected SignalsManagerComponent m_SignalManagerComp;	
	
	// Signal values
	protected float m_fRotorAngleYPrimaryLast;
	protected float m_fEngineRPMSignalValLast;
	
	// Sound Handles
	protected AudioHandle m_RotorHi = AudioHandle.Invalid;
	protected AudioHandle m_EngineStartHandle = AudioHandle.Invalid;
	protected AudioHandle m_EngineStopHandle = AudioHandle.Invalid;
	
	//----------------------------------------------------------------------------------------------
	
	private void HandleEngineStartStopSound(float engineRPMSignalVal)
	{	
		if (engineRPMSignalVal > ENGINEON_TRESHOLD && m_fEngineRPMSignalValLast <= ENGINEON_TRESHOLD)
		{			
			if (!IsFinishedPlaying(m_EngineStopHandle))
			{
				Terminate(m_EngineStopHandle);
			}
				
			m_EngineStartHandle = SoundEvent("SOUND_ENGINE_START");
		}
							
		if (engineRPMSignalVal < m_fEngineRPMSignalValLast)
		{					
			if (!IsFinishedPlaying(m_EngineStartHandle))
			{
				Terminate(m_EngineStartHandle);
			}
		
			if (IsFinishedPlaying(m_EngineStopHandle))
			{
				m_EngineStopHandle = SoundEvent("SOUND_ENGINE_STOP");
			}		
		}		
	}
	
	//----------------------------------------------------------------------------------------------
	
	override void OnFrame(IEntity owner, float timeSlice)
	{
		super.OnFrame(owner, timeSlice);
		
		Physics physics = owner.GetPhysics();
					
		if (physics)
		{
			
			// Start + Stop engine ------------------------------------------------------------------
		
			float engineRPMSignalVal = m_SignalManagerComp.GetSignalValue(m_iSignalRotorRPMScaledprimaryIdx);
			
			HandleEngineStartStopSound(engineRPMSignalVal);
			
			m_fEngineRPMSignalValLast = engineRPMSignalVal;
			
			// Signals ------------------------------------------------------------------------------
			
			vector velocity_vector = physics.GetVelocity();
			float speed_m_per_s = velocity_vector.Length();
						
			m_SignalManagerComp.SetSignalValue(m_iSignalSpeedIdx, speed_m_per_s);
						
			m_SignalManagerComp.SetSignalValue(m_iSignalPitchIdx, owner.GetYawPitchRoll()[1]);
			m_SignalManagerComp.SetSignalValue(m_iSignalRollIdx, owner.GetYawPitchRoll()[2]);
			
			// Rotor Hi trigger --------------------------------------------------------------------
			
			// Value 0 to -360
			float fRotorAngleYPrimary = m_SignalManagerComp.GetSignalValue(m_iSignalRotorAngleYPrimaryIdx);
					
			if (fRotorAngleYPrimary < -330 && m_fRotorAngleYPrimaryLast >= -330)
			{
				if (!IsFinishedPlaying(m_RotorHi))
				{
					Terminate(m_RotorHi);
				}

				m_RotorHi = SoundEvent("SOUND_ROTOR_HI");
			}
			
			m_fRotorAngleYPrimaryLast = fRotorAngleYPrimary;
				
			// --------------------------------------------------------------------------------------
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
					
		GenericEntity genEnt = GenericEntity.Cast(owner);
		m_SignalManagerComp = SignalsManagerComponent.Cast(genEnt.FindComponent(SignalsManagerComponent));
		
		if (!m_SignalManagerComp)
			return;

		m_iSignalSpeedIdx = m_SignalManagerComp.AddOrFindSignal("speed");
		m_iSignalPitchIdx = m_SignalManagerComp.AddOrFindSignal("pitch");
		m_iSignalRollIdx = m_SignalManagerComp.AddOrFindSignal("roll");
		m_iSignalRotorRPMScaledprimaryIdx = m_SignalManagerComp.AddOrFindSignal("RotorRPMScaledprimary");
		m_iSignalRotorAngleYPrimaryIdx = m_SignalManagerComp.AddOrFindSignal("RotorAngleYPrimary");
		
		SetEventMask(owner, EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_HelicopterSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		// Disable parameter
		#ifndef DISABLE_SCRIPTVEHICLESOUND
			SetScriptedMethodsCall(true);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_HelicopterSoundComponent()
	{
	}

};
