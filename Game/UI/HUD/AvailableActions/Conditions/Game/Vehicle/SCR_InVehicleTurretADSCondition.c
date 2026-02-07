//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_InVehicleTurretADSCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Return true if character is in ADS of current controlled vehicle turret
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		// Current weapon 
		BaseControllerComponent controller = data.GetCurrentVehicleController();
		if (!controller)
			return false;
		
		// Turret
		IEntity turretEntity = controller.GetOwner();
		if (!turretEntity)
			return false;
		
		TurretComponent turret = TurretComponent.Cast(turretEntity.FindComponent(TurretComponent));
		if (!turret)
			return false;
		
		// Sights 
		ScriptedSightsComponent sights = ScriptedSightsComponent.Cast(turret.FindComponent(ScriptedSightsComponent));
		if (!sights)
			return false;
	
		// In sights ADS? 
		bool result = sights.IsSightADSActive();
		
		return GetReturnResult(result);
	}
};