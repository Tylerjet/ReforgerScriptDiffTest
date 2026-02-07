[BaseContainerProps()]
class SCR_AIDangerReaction_Vehicle : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		
		IEntity vehicleEntity = dangerEvent.GetVictim();
		if (!vehicleEntity)
			return false;
		
		// Ignore if we already have an action to move from that vehicle
		if (SCR_AIMoveFromDangerBehavior.ExistsBehaviorForEntity(utility, vehicleEntity))
			return false;
		
		SCR_BaseCompartmentManagerComponent compManager = SCR_BaseCompartmentManagerComponent.Cast(vehicleEntity.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compManager)
			return false;
		
		array<IEntity> occupants = {};
		compManager.GetOccupantsOfType(occupants, ECompartmentType.Pilot);
		
		// Ignore if no driver or driver is friendly
		if (occupants.Count() == 0)
			return false;
		if (!agent.IsEnemy(occupants[0]))
			return false;
		
		// Ignore if the behavior would fail immediately after start
		if (!SCR_AIMoveFromIncomingVehicleBehavior.ExecuteBehaviorCondition(utility, vehicleEntity))
			return false;

		SCR_AIMoveFromDangerBehavior behavior = new SCR_AIMoveFromIncomingVehicleBehavior(utility, null, vector.Zero, dangerEntity: vehicleEntity);
		utility.AddAction(behavior);
		
		return true;
	}	
};
