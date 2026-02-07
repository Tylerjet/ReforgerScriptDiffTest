[BaseContainerProps()]
class SCR_AIDangerReaction_Explosion : SCR_AIDangerReaction
{
	private static const float EXPLOSION_OBSERVE_DISTANCE = 220; // Maximal distance from explosion to trigger observe behavior
	
	void CreateObserveUnknownBehavior(SCR_AIUtilityComponent utility, vector observeReactionPosition)
	{
		if (observeReactionPosition == vector.Zero || utility.m_CombatComponent.GetCurrentTarget() != null)
			return;
				
		vector myOrigin = utility.m_OwnerEntity.GetOrigin();
								
		SCR_AIMoveAndInvestigateBehavior investigateBehavior = SCR_AIMoveAndInvestigateBehavior.Cast(utility.FindActionOfType(SCR_AIMoveAndInvestigateBehavior));
		SCR_AIObserveUnknownFireBehavior oldObserveBehavior = SCR_AIObserveUnknownFireBehavior.Cast(utility.FindActionOfType(SCR_AIObserveUnknownFireBehavior));
		
		// Exit if investigating
		if (investigateBehavior && investigateBehavior.GetActionState() == EAIActionState.RUNNING)
			return;
		
		// Exit if already observing something else
		if (oldObserveBehavior)
			return;
		
		SCR_AIObserveUnknownFireBehavior observeBehavior = new SCR_AIObserveUnknownFireBehavior(utility, null, posWorld: observeReactionPosition, useMovement: false);
		utility.AddAction(observeBehavior);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		IEntity ownerEntity = utility.m_OwnerEntity;
		
		if (!ownerEntity)
			return false;
		
		vector position = dangerEvent.GetPosition();
		float distance = vector.Distance(ownerEntity.GetOrigin(), position);
		
		if (distance > SCR_AIThreatSystem.EXPLOSION_MAX_DISTANCE)
			return false;
		
		// Increase threat level
		threatSystem.ThreatExplosion(distance);
		
		// Look at explosion
		utility.m_LookAction.LookAt(position, SCR_AILookAction.PRIO_UNKNOWN_TARGET, 1.5);
		
		// Observe if not investigating or already observing something else
		if (distance <= EXPLOSION_OBSERVE_DISTANCE)
			CreateObserveUnknownBehavior(utility, position);
		
		return true;
	}
};