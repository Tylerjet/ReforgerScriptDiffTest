class SCR_AIDecoTestIsVehicle : DecoratorTestScripted
{
	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		return ( Vehicle.Cast( controlled ) == null );
	}
};