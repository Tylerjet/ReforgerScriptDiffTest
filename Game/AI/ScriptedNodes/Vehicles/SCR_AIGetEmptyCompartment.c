class SCR_AIGetEmptyCompartment : AITaskScripted
{
	static const string PORT_POSITION =	"Position";
	static const string PORT_VEHICLE =	"Vehicle";
	
	private IEntity m_VehicleEntity;
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		GetVariableIn(PORT_VEHICLE, m_VehicleEntity);	
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (m_VehicleEntity)
		{
			BaseCompartmentManagerComponent compartmentMan = BaseCompartmentManagerComponent.Cast(m_VehicleEntity.FindComponent(BaseCompartmentManagerComponent));
			if (!compartmentMan)
			{
				return NodeError(this, owner, "Missing compartment manager on IEntity" + m_VehicleEntity.ToString());
			}

			ref array<BaseCompartmentSlot>	compartments = new array<BaseCompartmentSlot>;	
			int numOfComp = compartmentMan.GetCompartments(compartments);				

			foreach (BaseCompartmentSlot comp : compartments)
			{
				if (!comp.AttachedOccupant() && comp.IsCompartmentAccessible())
				{
					SCR_AIGroupUtilityComponent groupUtil = SCR_AIGroupUtilityComponent.Cast(owner.FindComponent(SCR_AIGroupUtilityComponent));
					if (!groupUtil)
					{
						return NodeError(this, owner, "Dosen't have group component.");
					}
					SCR_AIGetInActivity getInActivity = SCR_AIGetInActivity.Cast(groupUtil.GetCurrentAction());
					if (!getInActivity)
					{
						return NodeError(this, owner, "GetEmptyCompartment outside of GetIn Action.");
					}
					
					SetVariableOut(PORT_POSITION, CompartmentClassToType(comp.Type()));
					getInActivity.AllocateCompartment(comp);
					return ENodeResult.SUCCESS;							
				}								
			}	
		}
		ClearVariable(PORT_POSITION);
		return ENodeResult.FAIL;
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
	protected static ref TStringArray s_aVarsOut = {
		PORT_POSITION
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette()
    {
        return true;
    }
	
	//------------------------------------------------------------------------------------------------
	override string GetOnHoverDescription()
	{
		return "Returns type of next usable compartment. Compartments are allocated in GetIn activity";
	}
	
	//------------------------------------------------------------------------------------------------
	static ECompartmentType CompartmentClassToType(typename type)
	{
		switch (type)
		{
			case PilotCompartmentSlot:	return ECompartmentType.Pilot;
			case CargoCompartmentSlot: 	return ECompartmentType.Cargo;
			case TurretCompartmentSlot:	return ECompartmentType.Turret;
		}
		return 0;			
	}
};