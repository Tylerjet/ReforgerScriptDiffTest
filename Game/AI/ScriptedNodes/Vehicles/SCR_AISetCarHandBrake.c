class SCR_AISetCarHandBrake : AITaskScripted
{	
	[Attribute("0", UIWidgets.CheckBox)]
	protected bool m_bValue;
	
	protected static const string PORT_VEHICLE = "Vehicle";
	
	//--------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		IEntity ent;
		if (!GetVariableIn(PORT_VEHICLE, ent))
			return ENodeResult.FAIL;
		
		if (!ent)
			return ENodeResult.FAIL;
		
		CarControllerComponent carController = CarControllerComponent.Cast(ent.FindComponent(CarControllerComponent));
		
		if (!carController)
			return ENodeResult.FAIL;
		
		carController.SetPersistentHandBrake(m_bValue);
		
		return ENodeResult.SUCCESS;
	}	
	
	//--------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_VEHICLE
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//--------------------------------------------------------------------------------
	override bool VisibleInPalette()
    {
        return true;
    }
	
	//--------------------------------------------------------------------------------
	override string GetOnHoverDescription()
	{
		return "Sets handbrake value of a car.";
	}	
};