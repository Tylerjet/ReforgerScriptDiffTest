class SCR_AIGetUsedVehicle : AITaskScripted
{
	static const string PORT_VEHICLE = "VehicleOut";
		
	//----------------------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group) 
			group = SCR_AIGroup.Cast(owner.GetParentGroup());
		if (!group)
			return ENodeResult.FAIL;
		
		array<AIAgent> agents = new array<AIAgent>();
		group.GetAgents(agents);
		
		IEntity usedVehicle;
		
		foreach (AIAgent agent : agents)
		{
			ChimeraCharacter chartacter = ChimeraCharacter.Cast(agent.GetControlledEntity());
			if (!chartacter || !chartacter.IsInVehicle())
				continue;
				
			CompartmentAccessComponent access = chartacter.GetCompartmentAccessComponent();
			if (!access || !access.IsInCompartment())
				continue;
			
			BaseCompartmentSlot comp = access.GetCompartment();
			if (!comp)
				continue;
			
			usedVehicle = comp.GetVehicle();
			
			if (usedVehicle)
				break;
		}
		
		if (usedVehicle)
		{
			// Add used vehicle to group's usable vehicles
			if (!group.IsUsableVehicle(usedVehicle))
				group.AddUsableVehicle(usedVehicle);
				
			SetVariableOut(PORT_VEHICLE, usedVehicle);
			return ENodeResult.SUCCESS;
		}
		
		ClearVariable(PORT_VEHICLE);
		return ENodeResult.FAIL;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {};
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = { PORT_VEHICLE };
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() { return true; }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override string GetOnHoverDescription()
	{
		return "GetUsedVehicle: returns vehicle currently used by a group.";
	}
};