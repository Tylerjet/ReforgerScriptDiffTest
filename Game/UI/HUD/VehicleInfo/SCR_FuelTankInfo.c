class SCR_FuelTankInfo: SCR_BaseVehicleInfo
{
	// TODO: Specific fuel tank IDs, -1 for total fuel - can track fuel leaks when IDs are provided.
	protected FuelManagerComponent m_pFuelManager;
	
	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override bool GetState()
	{
		if (!m_pFuelManager)
			return false;
		
		// BLINKING: currently leaking
		// register to an event. If leaking, return blinking
		
		// DESTROYED: < reserve fuel
		float totalFuel = m_pFuelManager.GetTotalFuel();
		if (totalFuel < 0.15)
			return true;
		
		// WARNING: < low fuel
		return totalFuel < 0.05;
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