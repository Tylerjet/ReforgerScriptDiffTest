class SCR_AIDecoTestIsOutOfVehicle : DecoratorTestScripted
{
	CompartmentAccessComponent compAccComp;
	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		bool result;
		
		AIGroup gr = AIGroup.Cast(agent);
		if (gr)
		{
			array<AIAgent> agents = new array<AIAgent>();
			gr.GetAgents(agents);
		
			int success = 0;
			int agentsCount = agents.Count();
	
			foreach (AIAgent ag : agents)
			{
				auto chart = ag.GetControlledEntity();
				ChimeraCharacter char = ChimeraCharacter.Cast(chart);
				if (char)
				{
					if (!char.IsInVehicle())
						success = success + 1;
				}			
			}
			result = (success == agentsCount);
		}
		else
		{		
			if (!controlled)
				return false;
			
			if (!compAccComp)
		 		compAccComp = CompartmentAccessComponent.Cast(controlled.FindComponent(CompartmentAccessComponent));
			result = !compAccComp.IsInCompartment();
		}		
		return result;
	}
};