class SCR_HandBrakeInfo: SCR_BaseVehicleInfo
{
	protected CarControllerComponent m_pCarController;
	
	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override bool GetState()
	{
		if (!m_pCarController)
			return false;
		
		 // TODO: Read also the standard handbrake from controller or simulation
		bool handbrake = /* m_pCarController.IsEngineOn() && */ (m_pCarController.GetHandBrake() || m_pCarController.GetPersistentHandBrake());
		
		// BLINKING: trying to drive away while handbrake is enabled
		VehicleWheeledSimulation simulation = m_pCarController.GetSimulation();
		if (handbrake && simulation && simulation.GetGear() != 1 && simulation.GetThrottle() > 0.1 && simulation.GetClutch() > 0.1)
			return true; // BLINKING
		
		// DESTROYED: simply enabled
		return handbrake;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// Terminate if there is no controller
		if (!m_pCarController)
			return false;
		
		return super.DisplayStartDrawInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);
		
		m_pCarController = CarControllerComponent.Cast(owner.FindComponent(CarControllerComponent));
	}
};