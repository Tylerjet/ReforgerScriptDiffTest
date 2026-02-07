class SCR_AIDecoTestIsDangerousVehicle : DecoratorTestScripted
{
	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!controlled)
			return false;
		
		// TODO: should generalize also for group
		SCR_ChimeraAIAgent chimera = SCR_ChimeraAIAgent.Cast(agent);
		if (!chimera)
		{
			Debug.Error("SCR_AIDecoTestIsDangerousVehicle currently can only be run on chimera.");
			return false;
		}
		
		return chimera.IsEnemy(controlled);
	}
};