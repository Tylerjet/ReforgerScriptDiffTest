class SCR_CarControllerComponentClass: CarControllerComponentClass
{
};

/*!
	Class responsible for game car.
	It connects all car components together and handles all comunication between them.
 */
class SCR_CarControllerComponent : CarControllerComponent
{
	//----------------------------------------------------------------------------
	// simulation
	
	//! needed to check if engine start is even possible
	private SCR_PowerComponent m_pPowerComponent;
	private HitZone m_pEngineHitZone;
	private bool m_bFirstRun = true;
	private bool m_bIsStarting;
	
	//sound
	private AudioHandle m_EngineStarterHandle = AudioHandle.Invalid;
	private AudioHandle m_EngineStartHandle = AudioHandle.Invalid;
	private AudioHandle m_EngineStopHandle = AudioHandle.Invalid;
	
	//------------------------------------------------------------------------------------------------
	//! Get engine hitzone
	HitZone GetEngineHitZone()
	{
		return m_pEngineHitZone;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set engine hitzone
	void SetEngineHitZone(HitZone hitZone)
	{
		m_pEngineHitZone = hitZone;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return damage state of the engine hitzone
	EDamageState GetEngineDamageState()
	{
		if (m_pEngineHitZone)
			return m_pEngineHitZone.GetDamageState();
		
		return EDamageState.UNDAMAGED;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if engine is damaged to an extent that reduces its performance
	bool IsEngineDefective()
	{
		if (!m_pEngineHitZone)
			return false;
		
		EDamageState state = m_pEngineHitZone.GetDamageState();
		return m_pEngineHitZone.GetDamageStateThreshold(state) < m_pEngineHitZone.GetDamageStateThreshold(EDamageState.INTERMEDIARY);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return if engine starter is functional
	bool IsStarterFunctional()
	{
		if (GetEngineDamageState() == EDamageState.DESTROYED)
			return false;
		
		if (m_bFirstRun)
		{
			m_bFirstRun = false;
			m_pPowerComponent = SCR_PowerComponent.Cast(GetOwner().FindComponent(SCR_PowerComponent));
		}
		
		// TODO: Option to start engine despite lack of power if vehicle is moving
		if (m_pPowerComponent && !m_pPowerComponent.HasPower())
			return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets called when the engine start routine begins (animation event).
	override void OnEngineStartBegin()
	{
		if (GetEngineDrowned())
			return;
		
		if (!IsStarterFunctional())
			return;
		
		m_bIsStarting = true;
		
		SCR_MotorExhaustEffectGeneralComponent motorExhaust = SCR_MotorExhaustEffectGeneralComponent.Cast(GetOwner().FindComponent(SCR_MotorExhaustEffectGeneralComponent));
		if (motorExhaust)
			motorExhaust.OnEngineStart(false);
		
		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (soundComponent)
		{
			soundComponent.Terminate(m_EngineStopHandle);
			m_EngineStopHandle = AudioHandle.Invalid;
			
			m_EngineStarterHandle = soundComponent.SoundEvent(SCR_SoundEvent.SOUND_ENGINE_STARTER_LP);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get called while engine starter is active.
	override void OnEngineStartProgress()
	{
		 if (!m_bIsStarting)
			return;
		
		if (GetEngineDamageState() == EDamageState.DESTROYED)
			OnEngineStartFail(EVehicleEngineStartFailedReason.DESTROYED);
		else if (!IsStarterFunctional())
			OnEngineStartFail(EVehicleEngineStartFailedReason.MAX_ATTEMPTS);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets called when the engine start routine was interrupted.
	override void OnEngineStartInterrupt()
	{
		m_bIsStarting = false;
		
		SCR_MotorExhaustEffectGeneralComponent motorExhaust = SCR_MotorExhaustEffectGeneralComponent.Cast(GetOwner().FindComponent(SCR_MotorExhaustEffectGeneralComponent));
		if (motorExhaust)
			motorExhaust.OnEngineStop();
		
		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (soundComponent)
		{
			soundComponent.Terminate(m_EngineStarterHandle);
			m_EngineStarterHandle = AudioHandle.Invalid;
			soundComponent.Terminate(m_EngineStartHandle);
			m_EngineStartHandle = AudioHandle.Invalid;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gets called when the engine start routine has successfully completed.
	override void OnEngineStartSuccess()
	{
		m_bIsStarting = false;
		
		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (soundComponent)
		{
			soundComponent.Terminate(m_EngineStarterHandle);
			m_EngineStarterHandle = AudioHandle.Invalid;
			soundComponent.Terminate(m_EngineStartHandle);
			m_EngineStartHandle = soundComponent.SoundEvent(SCR_SoundEvent.SOUND_ENGINE_START);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gets called when the engine start routine has failed.
	override void OnEngineStartFail(EVehicleEngineStartFailedReason reason)
	{
		if (!m_bIsStarting)
			return;
		
		m_bIsStarting = false;
		
		SCR_MotorExhaustEffectGeneralComponent motorExhaust = SCR_MotorExhaustEffectGeneralComponent.Cast(GetOwner().FindComponent(SCR_MotorExhaustEffectGeneralComponent));
		if (motorExhaust)
			motorExhaust.OnEngineStop();
		
		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (soundComponent)
		{
			soundComponent.Terminate(m_EngineStarterHandle);			
			m_EngineStarterHandle = AudioHandle.Invalid;
			soundComponent.Terminate(m_EngineStartHandle);
			m_EngineStartHandle = AudioHandle.Invalid;
			
			if (reason != EVehicleEngineStartFailedReason.NO_FUEL)
				return;
			
			soundComponent.SoundEvent(SCR_SoundEvent.SOUND_ENGINE_START_FAILED);
		}
	}

	//------------------------------------------------------------------------------------------------
 	/*!
		Is called every time the controller wants to start the engine.

		\return true if the engine can start, false otherwise.
	*/
	override bool OnBeforeEngineStart()
	{
		if (!IsStarterFunctional())
			return false;
		
		// engine can start by default
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Is called every time the engine starts.
	override void OnEngineStart()
	{
		SCR_FuelConsumptionComponent fuelConsumption = SCR_FuelConsumptionComponent.Cast(GetOwner().FindComponent(SCR_FuelConsumptionComponent));
		if (fuelConsumption)
			fuelConsumption.SetEnabled(true);

		SCR_MotorExhaustEffectGeneralComponent motorExhaust = SCR_MotorExhaustEffectGeneralComponent.Cast(GetOwner().FindComponent(SCR_MotorExhaustEffectGeneralComponent));
		if (motorExhaust)
			motorExhaust.OnEngineStart(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Is called every time the engine stops.
	override void OnEngineStop()
	{
		SCR_FuelConsumptionComponent fuelConsumption = SCR_FuelConsumptionComponent.Cast(GetOwner().FindComponent(SCR_FuelConsumptionComponent));
		if (fuelConsumption)
			fuelConsumption.SetEnabled(false);

		SCR_MotorExhaustEffectGeneralComponent motorExhaust = SCR_MotorExhaustEffectGeneralComponent.Cast(GetOwner().FindComponent(SCR_MotorExhaustEffectGeneralComponent));
		if (motorExhaust)
			motorExhaust.OnEngineStop();
		
		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (soundComponent)
		{
			soundComponent.Terminate(m_EngineStarterHandle);
			m_EngineStarterHandle = AudioHandle.Invalid;
			soundComponent.Terminate(m_EngineStartHandle);
			m_EngineStartHandle = AudioHandle.Invalid;
			
			m_EngineStopHandle = soundComponent.SoundEvent(SCR_SoundEvent.SOUND_ENGINE_STOP);
		}
	}
	
	protected int m_iOccupants;
	
	protected void OnDestroyed(IEntity ent)
	{
		GarbageManager garbageMan = GetGame().GetGarbageManager();
		if (garbageMan)
			garbageMan.Insert(Vehicle.Cast(ent));
	}
	
	protected void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (++m_iOccupants > 0)
		{
			GarbageManager garbageMan = GetGame().GetGarbageManager();
			if (garbageMan)
				garbageMan.Withdraw(Vehicle.Cast(vehicle));
		}
	}
	
	protected void OnCompartmentLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (--m_iOccupants == 0)
		{
			GarbageManager garbageMan = GetGame().GetGarbageManager();
			if (garbageMan)
				garbageMan.Insert(Vehicle.Cast(vehicle));
		}
	}
	
	protected override void OnPostInit(IEntity owner)
	{
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
		if (ev)
		{
			ev.RegisterScriptHandler("OnDestroyed", this, OnDestroyed);
			ev.RegisterScriptHandler("OnCompartmentEntered", this, OnCompartmentEntered);
			ev.RegisterScriptHandler("OnCompartmentLeft", this, OnCompartmentLeft);
		}	
	}
	protected override void OnDelete(IEntity owner)
	{
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
		if (ev)
		{
			ev.RemoveScriptHandler("OnDestroyed", this, OnDestroyed);
			ev.RemoveScriptHandler("OnCompartmentEntered", this, OnCompartmentEntered);
			ev.RemoveScriptHandler("OnCompartmentLeft", this, OnCompartmentLeft);
		}	
	}
};
