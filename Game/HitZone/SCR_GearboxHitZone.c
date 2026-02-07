class SCR_GearboxHitZone: ScriptedHitZone
{
	[Attribute("0.25", UIWidgets.EditBox, desc: "Minimum gearbox efficiency ratio",  params: "0 1 0.01")]
	private float m_fMinimumGearboxEfficiencyRatio;
	
	// Audio features
	private int m_iGearboxSignalIdx;
	
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		// Get Signals manager component + GearboxDamage signal index
		SignalsManagerComponent signalsManager = SignalsManagerComponent.Cast(pOwnerEntity.FindComponent(SignalsManagerComponent));
		if (signalsManager)
			m_iGearboxSignalIdx = signalsManager.AddOrFindSignal("GearboxDamage");
		
		UpdateGearboxState();
	}
	
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();
		
		UpdateGearboxState();
	}
	
	void UpdateGearboxState()
	{
		if(GetGame().GetIsClientAuthority())
		{
			VehicleWheeledSimulation simulation = VehicleWheeledSimulation.Cast(GetOwner().FindComponent(VehicleWheeledSimulation));
			if (!simulation)
				return;
			
			if (!simulation.IsValid())
				return;
			
			EDamageState state = GetDamageState();
	 		float health = GetDamageStateThreshold(state);
			
			float maximumEfficiency = simulation.GearboxGetEfficiency();
			float minimumEfficiency = m_fMinimumGearboxEfficiencyRatio * maximumEfficiency;
			float efficiency = Math.Lerp(minimumEfficiency, maximumEfficiency, health);
			simulation.GearboxSetEfficiencyState(efficiency);
			
			// Set GearboxDamage signal value
			if (m_iGearboxSignalIdx == -1)
				return;
			
			SignalsManagerComponent signalsManager = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));
			if (signalsManager)
				signalsManager.SetSignalValue(m_iGearboxSignalIdx, 1 - health);
		}
		else
		{
			VehicleWheeledSimulation_SA simulation = VehicleWheeledSimulation_SA.Cast(GetOwner().FindComponent(VehicleWheeledSimulation_SA));
			if (!simulation)
				return;
			
			if (!simulation.IsValid())
				return;
			
			EDamageState state = GetDamageState();
	 		float health = GetDamageStateThreshold(state);
			
			float maximumEfficiency = simulation.GearboxGetEfficiency();
			float minimumEfficiency = m_fMinimumGearboxEfficiencyRatio * maximumEfficiency;
			float efficiency = Math.Lerp(minimumEfficiency, maximumEfficiency, health);
			simulation.GearboxSetEfficiencyState(efficiency);
			
			// Set GearboxDamage signal value
			if (m_iGearboxSignalIdx == -1)
				return;
			
			SignalsManagerComponent signalsManager = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));
			if (signalsManager)
				signalsManager.SetSignalValue(m_iGearboxSignalIdx, 1 - health);
		}
	}
};