class SCR_AIRemoveVehicleFromGroup : AITaskScripted
{	
	static const string PORT_VEHICLE = "VehicleIn";
	
	[Attribute("1", UIWidgets.CheckBox, "Remove all vehicles used by any group member?")]
    bool m_bRemoveAllUsedVehicles;
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
		{
			return NodeError(this,owner, "Node must be run on SCR_AIGroup agent!");			
		}
		
		IEntity agentEntity, vehicleEntity;
		
		if (GetVariableIn(PORT_VEHICLE, vehicleEntity))
		{
			group.RemoveUsableVehicle(vehicleEntity);
			return ENodeResult.SUCCESS;
		};
		
		if (!m_bRemoveAllUsedVehicles)
		{
			return ENodeResult.FAIL;
		};
		
		array<IEntity> groupVehicles = {};
		group.GetUsableVehicles(groupVehicles);
		
		foreach (IEntity vehicle : groupVehicles)
		{
			group.RemoveUsableVehicle(vehicle);
		}
		
		return ENodeResult.SUCCESS;		
	}
		
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_VEHICLE
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Removes vehicle from the list of usable vehicles of the group. Provide either a vehicle entity or removes all vehicles used by any group member";
	}		

};