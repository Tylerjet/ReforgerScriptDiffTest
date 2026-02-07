class SCR_EngineHitZone : ScriptedHitZone
{
	private float							m_fInitialStartupChance = 100;
	private int								m_iEngineDamageSignalIdx = -1;

	[Attribute("25", UIWidgets.Auto, desc: "Minimum engine power scale (% of initial)",  params: "0 100 0.1")]
	private float m_fMinimumEnginePowerScale;

	[Attribute("20", UIWidgets.Auto, desc: "Minimum engine startup chance (% of initial)",  params: "0 100 0.1")]
	private float m_fMinimumEngineStartupChance;

	//! Called when hit zone is initialized
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		SCR_CarControllerComponent controller;
		BaseVehicleNodeComponent node = BaseVehicleNodeComponent.Cast(pOwnerEntity.FindComponent(BaseVehicleNodeComponent));
		if (node)
			controller = SCR_CarControllerComponent.Cast(node.FindComponent(SCR_CarControllerComponent));
		
		if (controller)
		{
			controller.SetEngineHitZone(this);
			m_fInitialStartupChance = controller.GetEngineStartupChance();
		}
		
		SignalsManagerComponent signalsManager = SignalsManagerComponent.Cast(pOwnerEntity.FindComponent(SignalsManagerComponent));
		if (signalsManager)
			m_iEngineDamageSignalIdx = signalsManager.AddOrFindSignal("EngineDamage");
		
		SCR_PowerComponent powerComponent = SCR_PowerComponent.Cast(pOwnerEntity.FindComponent(SCR_PowerComponent));
		if (powerComponent)
			powerComponent.GetEventBatteryStateChanged().Insert(UpdateEngineState);
		
		UpdateEngineState();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called after damage multipliers and thresholds are applied to received impacts and damage is applied to hitzone.
	This is also called when transmitting the damage to parent hitzones!
	\param type Type of damage
	\param damage Amount of damage received
	\param pOriginalHitzone Original hitzone that got dealt damage, as this might be transmitted damage.
	\param instigator Damage source parent entity (soldier, vehicle, ...)
	\param hitTransform [hitPosition, hitDirection, hitNormal]
	\param speed Projectile speed in time of impact
	\param colliderID ID of the collider receiving damage
	\param nodeID ID of the node of the collider receiving damage
	*/
	override void OnDamage(EDamageType type, float damage, HitZone pOriginalHitzone, IEntity instigator, inout vector hitTransform[3], float speed, int colliderID, int nodeID)
	{
		super.OnDamage(type, damage, pOriginalHitzone, instigator, hitTransform, speed, colliderID, nodeID);
		
		// Powerful impact should always stop engine
		if (damage < GetCriticalDamageThreshold()*GetMaxHealth())
			return;
		
		VehicleControllerComponent controller;
		BaseVehicleNodeComponent node = BaseVehicleNodeComponent.Cast(GetOwner().FindComponent(BaseVehicleNodeComponent));
		if (node)
			controller = VehicleControllerComponent.Cast(node.FindComponent(VehicleControllerComponent));
		
		if (controller && controller.IsEngineOn())
			controller.StopEngine(false);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when the damage state changes.
	*/
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();
		
		UpdateEngineState();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Update engine power and stop destroyed egnine according to vehicle features present.
	*/
	void UpdateEngineState()
	{
		IEntity owner = GetOwner();
		EDamageState state = GetDamageState();
 		float healthEngine = GetDamageStateThreshold(state);
		
		if (m_iEngineDamageSignalIdx != -1)
		{
			SignalsManagerComponent signalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
			if (signalsManager)
				signalsManager.SetSignalValue(m_iEngineDamageSignalIdx, 1 - healthEngine);
		}
		
		VehicleControllerComponent controller;
		BaseVehicleNodeComponent node = BaseVehicleNodeComponent.Cast(owner.FindComponent(BaseVehicleNodeComponent));
		if (node)
			controller = VehicleControllerComponent.Cast(node.FindComponent(VehicleControllerComponent));
		
		if (!controller)
			return;
		
		float healthStarter = healthEngine;
		
		// Battery influence on starter and engine power output
		SCR_PowerComponent powerComponent = SCR_PowerComponent.Cast(owner.FindComponent(SCR_PowerComponent));
		if (powerComponent)
		{
			bool hasPower = powerComponent.HasPower();
			
			if (!hasPower)
				healthStarter = 0;
			
			if (!hasPower && powerComponent.ShouldApplyNoPowerPenalty())
				healthEngine *= powerComponent.GetNoPowerMultiplier();
		}
		
		// Reduce starter reliability
		float startupChance = 100;
		if (healthStarter < 1)
			startupChance = m_fInitialStartupChance * 0.01 * Math.Lerp(m_fMinimumEngineStartupChance, 100, healthStarter);
		
		controller.SetEngineStartupChance(startupChance);
		
		// Stop destroyed engine
		if (state == EDamageState.DESTROYED && controller.IsEngineOn())
			controller.StopEngine(false);
		
		// Reduce power output of damaged engine
		VehicleWheeledSimulation simulation = VehicleWheeledSimulation.Cast(owner.FindComponent(VehicleWheeledSimulation));
		if (!simulation || !simulation.IsValid())
			return;
		
		float peakTorque = simulation.EngineGetPeakTorque();
		float peakPower  = simulation.EngineGetPeakPower();
		float powerRatio = 0.01 * Math.Lerp(m_fMinimumEnginePowerScale, 100, healthEngine);
		simulation.EngineSetPeakTorqueState(peakTorque * powerRatio);
		simulation.EngineSetPeakPowerState(peakPower * powerRatio);
	}
};