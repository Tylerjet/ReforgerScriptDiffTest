class SCR_AIDecoTestVehicleIsMoving : DecoratorTestScripted
{
	private const float MIN_SPEED_MPS = 0.5;
	private const float MIN_SPEED_KMH = 1.8;
	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!controlled)
			return false;
		
		// Iterate parents of entity until we find first Vehicle entity
		IEntity parent = controlled;
		BaseVehicle vehicle = BaseVehicle.Cast(controlled);
		
		while (vehicle == null && parent != null)
		{
			parent = parent.GetParent();
			if (parent)
				vehicle = BaseVehicle.Cast(parent);
		}
		
		if (!vehicle)
			return false;
		
		VehicleBaseSimulation simulation = VehicleBaseSimulation.Cast(vehicle.FindComponent(VehicleBaseSimulation));
		if (simulation)
		{
			// TODO: Move GetSpeedKmh to base simulation, it seems to be a very generic feature
			VehicleWheeledSimulation wheeledSimulation = VehicleWheeledSimulation.Cast(simulation);
			if (wheeledSimulation)
				return Math.AbsFloat(wheeledSimulation.GetSpeedKmh()) > MIN_SPEED_KMH;
		}
		
		// In case there is no simulation, try to get value from physics
		Physics ph = controlled.GetPhysics();
		return ph && ph.IsActive() && ph.GetVelocity().Length() > MIN_SPEED_MPS;
	}
};