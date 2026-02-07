class SCR_HelicopterControllerComponentClass : HelicopterControllerComponentClass
{
};

/*!
	Class responsible for game helicopter.
	It connects all helicopter components together and handles all comunication between them.
*/
class SCR_HelicopterControllerComponent : HelicopterControllerComponent
{
	//------------------------------------------------------------------------------------------------
	//! Is called every time the engine starts.
	override void OnEngineStart()
	{
		IEntity owner = GetOwner();
		SCR_FuelConsumptionComponent fuelConsumption = SCR_FuelConsumptionComponent.Cast(owner.FindComponent(SCR_FuelConsumptionComponent));
		if (fuelConsumption)
			fuelConsumption.SetEnabled(true);
		
		SCR_HelicopterExhaustEffectComponent exhaustEffect = SCR_HelicopterExhaustEffectComponent.Cast(owner.FindComponent(SCR_HelicopterExhaustEffectComponent));
		if (exhaustEffect)
			exhaustEffect.OnEngineStart(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Is called every time the engine stops.
	override void OnEngineStop()
	{
		IEntity owner = GetOwner();
		SCR_FuelConsumptionComponent fuelConsumption = SCR_FuelConsumptionComponent.Cast(owner.FindComponent(SCR_FuelConsumptionComponent));
		if (fuelConsumption)
			fuelConsumption.SetEnabled(false);
		
		SCR_HelicopterExhaustEffectComponent exhaustEffect = SCR_HelicopterExhaustEffectComponent.Cast(owner.FindComponent(SCR_HelicopterExhaustEffectComponent));
		if (exhaustEffect)
			exhaustEffect.OnEngineStop();
	}
}