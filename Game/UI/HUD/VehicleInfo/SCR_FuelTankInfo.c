class SCR_FuelTankInfo: SCR_BaseVehicleInfo
{
	// TODO: Specific fuel tank IDs, -1 for total fuel - can track fuel leaks when IDs are provided.
	protected FuelManagerComponent m_pFuelManager;
	
	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override EVehicleInfoState GetState()
	{
		if (!m_pFuelManager)
			return EVehicleInfoState.DISABLED;
		
		// BLINKING: currently leaking
		// register to an event. If leaking, return blinking
		
		// ERROR: < reserve fuel
		// WARNING: < low fuel - should be configured in fuel manager
		float totalFuel = m_pFuelManager.GetTotalFuel();
		if (totalFuel <= 0.05)
			return EVehicleInfoState.ERROR;
		else if (totalFuel <= 0.15)
			return EVehicleInfoState.WARNING;
		
		return EVehicleInfoState.DISABLED;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// Terminate if there is no fuel manager
		if (!m_pFuelManager)
			return false;
		
		return super.DisplayStartDrawInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);
		
		m_pFuelManager = FuelManagerComponent.Cast(owner.FindComponent(FuelManagerComponent));
	}
};