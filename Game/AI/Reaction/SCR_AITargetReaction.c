//------------------------------------------------------------------------------------------------
// TARGET REACTION BASE
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_AITargetReactionBase : SCR_AIReactionBase
{
	void PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, IEntity target, vector lastSeenPosition) {}

};

//------------------------------------------------------------------------------------------------
// TARGET REACTIONS: These are not danger events, although they should be in some time
//------------------------------------------------------------------------------------------------


[BaseContainerProps()]
class SCR_AITargetReaction_Unknown : SCR_AITargetReactionBase
{
	// @TODO: Is it possible to send enemy target and unknown target as Target class, somehow? Can I overload danger event to gimme target?
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, IEntity target, vector lastSeenPosition)
	{
		utility.m_LookAction.LookAt(target, SCR_AILookAction.PRIO_UNKNOWN_TARGET);
	}
};

[BaseContainerProps()]
class SCR_AITargetReaction_Enemy : SCR_AITargetReactionBase
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, IEntity target, vector lastSeenPosition)
	{		
		if (!target)
			return;
		if (!SCR_AIIsAlive.IsAlive(target))
		{
			SCR_AISendLostMsg(utility, target);
			return;
		}
		
		//this shouldn't be dependent on Info component since Info component should provide state of agent to outside
		if (!utility.m_AIInfo.HasUnitState(EUnitState.STATIC))
		{		
			auto behavior = new SCR_AIAttackBehavior(utility, false, target, lastSeenPosition);
			// AddAction must be used here, not AddActionIfMissing!
			// When target switches, we must delete the old behavior.
			utility.AddAction(behavior);
			utility.WrapBehaviorOutsideOfVehicle(behavior);
			utility.m_LookAction.LookAt(target, SCR_AILookAction.PRIO_ENEMY_TARGET);
		}
		else
		{
			auto behavior = new SCR_AIAttackStaticBehavior(utility, false, target, lastSeenPosition);
			utility.AddActionIfMissing(behavior);
			utility.m_LookAction.LookAt(target, SCR_AILookAction.PRIO_ENEMY_TARGET);
		}
	}
};