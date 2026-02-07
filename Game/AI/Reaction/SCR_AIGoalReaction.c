//------------------------------------------------------------------------------------------------
// GOAL REACTION BASE
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIGoalReaction : SCR_AIReactionBase
{
	[Attribute("0", UIWidgets.ComboBox, "Type of event activating the reaction", "", ParamEnumArray.FromEnum(EMessageType_Goal) )]
	EMessageType_Goal m_eType;
	
	void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message) {}
	void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message) {}
	
	float Prioritize(float basePriority, bool modify)
	{
		if (modify)
			basePriority += SCR_AIActionBase.MAX_PRIORITY;
		return basePriority; 
	}
};


//------------------------------------------------------------------------------------------------
// GOAL REACTIONS - Reactions on different orders
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIGoalReaction_Attack : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_Attack msg = SCR_AIMessage_Attack.Cast(message); // new approach: m_Target can be null!
        if (!msg)
			return;
		
		if (!SCR_AIIsAlive.IsAlive(msg.m_Target))
		{
			SCR_AISendLostMsg(utility, msg.m_Target);
			return;
		}
		
		auto behavior = new SCR_AIAttackBehavior(utility, false, msg.m_Target, msg.m_LastSeenPosition);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
			
		if (msg && msg.m_RelatedGroupActivity)
			behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);
		utility.WrapBehaviorOutsideOfVehicle(behavior);
		utility.AddAction(behavior);	
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_Attack msg = SCR_AIMessage_Attack.Cast(message);
		if (!msg)
			return;
		
		if (!SCR_AIIsAlive.IsAlive(msg.m_Target))
		{
			utility.RemoveKnownEnemy(msg.m_Target);
			return;
		}
		
		auto activity = new SCR_AIAttackActivity(utility, false, msg.m_bIsWaypointRelated, msg.m_Target, msg.m_LastSeenPosition, msg.GetSender());
		utility.AddKnownEnemy(msg.m_Target);	
		utility.AddAction(activity);	
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_AttackDone : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		utility.SetStateAllActionsOfType(EAIActionType.ATTACK,EAIActionState.COMPLETED);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_GroupAttack : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_GroupAttack msg = SCR_AIMessage_GroupAttack.Cast(message);
		if (!msg)
			return;
		
		if (!utility.m_AIInfo.HasUnitState(EUnitState.STATIC))
		{
			IEntity eTarget = msg.m_Target;
			SCR_AICombatComponent combatComp = utility.m_CombatComponent;
			
			if (!eTarget || !combatComp)
				return;
			
			// Check if this enemy is known to combat component
			if (combatComp.IsEnemyKnown(eTarget))
			{
				// Enemy is known to this soldier
				// Add attack behavior
				auto behavior = new SCR_AIAttackBehavior(utility, false, msg.m_Target, msg.m_LastSeenPosition);
				if (m_OverrideBehaviorTree != string.Empty)
					behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
			
				if (msg.m_RelatedGroupActivity)
					behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);
				
				utility.WrapBehaviorOutsideOfVehicle(behavior);
				utility.AddAction(behavior);
			}
			else
			{
				// Ignore if we must defend a waypoint
				if (!utility.IsInvestigationAllowed(msg.m_LastSeenPosition))
					return;
				
				float searchRadius = 10.0;
				if (!utility.IsInvestigationRelevant(msg.m_LastSeenPosition, searchRadius * searchRadius))
					return;
				
				// Enemy was never seen by this soldier
				// Add investigate behavior
				SCR_AIMoveAndInvestigateBehavior behavior = new SCR_AIMoveAndInvestigateBehavior(utility, false, msg.m_LastSeenPosition, isDangerous: true, radius: searchRadius);
				if (msg.m_RelatedGroupActivity)
					behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);
				
				utility.WrapBehaviorOutsideOfVehicle(behavior);
				utility.AddAction(behavior);
			}
		}
		else // must update manually location of the target, AttackStatic is already in progress
			utility.UpdateAttackParameters(msg.m_Target, msg.m_LastSeenPosition);		
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_GroupAttackDone : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		utility.SetStateAllActionsOfType(EAIActionType.MOVE_COMBAT_GROUP,EAIActionState.COMPLETED);
		utility.m_CombatComponent.ResetCombatType();
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_AttackStatic : SCR_AIGoalReaction_Attack
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_AttackStatic msg = SCR_AIMessage_AttackStatic.Cast(message); // new approach: m_Target can be null!
        if (!msg)
			return;
		
		auto behavior = new SCR_AIAttackStaticBehavior(utility, false, msg.m_Target, msg.m_LastSeenPosition);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
		
		if (msg && msg.m_RelatedGroupActivity)
			behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);

		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_AttackStaticDone : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		// only relevant for static attacks, others may receive it for the sake of simplicity of trees
		if (utility.m_AIInfo.HasUnitState(EUnitState.STATIC))
		{
			utility.SetStateAllActionsOfType(EAIActionType.ATTACK_STATIC,EAIActionState.COMPLETED);
		}
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_CoverAdvance : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_CoverAdvance msg = SCR_AIMessage_CoverAdvance.Cast(message);
		if (!msg)
			return;
		SCR_AICombatMoveGroupBehavior behavior = new SCR_AICombatMoveGroupBehavior(utility, false, msg.m_LastSeenPosition, target: msg.m_Target);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
		if (msg && msg.m_RelatedGroupActivity)
			behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);
		
		utility.AddAction(behavior);
		// we asume that bounding overwatch is setting SUPPRESSIVE which disable movement and we need move here
		utility.m_CombatComponent.ResetCombatType();
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Move : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Move.Cast(message);
		if (!msg)
			return;
		
		auto behavior = new SCR_AIMoveIndividuallyBehavior(utility, msg.m_bIsPriority, msg.m_MovePosition, Prioritize(SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY,msg.m_bIsPriority), msg.m_FollowEntity);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		if (msg && msg.m_RelatedGroupActivity)
			behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);

		utility.WrapBehaviorOutsideOfVehicle(behavior);
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{		
		auto msg = SCR_AIMessage_Move.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AIMoveActivity(utility, msg.m_bIsPriority, msg.m_bIsWaypointRelated, msg.m_MovePosition,Prioritize(SCR_AIActionBase.PRIORITY_ACTIVITY_MOVE, msg.m_bIsPriority),msg.m_FollowEntity,false);
		utility.SetStateAllActionsOfType(EAIActionType.SEEK_DESTROY,EAIActionState.FAILED); // move fails seek and destroy	
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Follow : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Follow.Cast(message);
		if (!msg)
			return;
		
		auto behavior = new SCR_AIMoveInFormationBehavior(utility, msg.m_bIsPriority, vector.Zero, Prioritize(SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_IN_FORMATION, msg.m_bIsPriority));
		
		if (msg && msg.m_RelatedGroupActivity)
			behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);

		//utility.WrapBehaviorOutsideOfVehicle(behavior); Zeli: should not be here, formation can be inside the vehicle as well. Copy+paste?
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{		
		auto msg = SCR_AIMessage_Follow.Cast(message);
		if (!msg)
			return;
		
		//SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated, vector pos, float priority = PRIORITY_ACTIVITY_FOLLOW, IEntity ent = null, bool noVehicles = false, float distance = 1.0
		auto activity = new SCR_AIFollowActivity(utility, msg.m_bIsPriority, msg.m_bIsWaypointRelated, vector.Zero, Prioritize(SCR_AIActionBase.PRIORITY_ACTIVITY_FOLLOW, msg.m_bIsPriority), msg.m_FollowEntity, false, msg.m_fDistance);
		utility.SetStateAllActionsOfType(EAIActionType.SEEK_DESTROY,EAIActionState.FAILED); // move fails seek and destroy
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Investigate : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Investigate.Cast(message);
		if (!msg)
			return;
		
		// Ignore if we must defend a waypoint
		if (!utility.IsInvestigationAllowed(msg.m_vMovePosition))
			return;
		
		if (!utility.IsInvestigationRelevant(msg.m_vMovePosition, msg.m_fRadius * msg.m_fRadius))
			return;
		
		auto behavior = new SCR_AIMoveAndInvestigateBehavior(utility, msg.m_bIsPriority, msg.m_vMovePosition, Prioritize(SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE, msg.m_bIsPriority), msg.m_bIsDangerous);
		
		if (msg && msg.m_RelatedGroupActivity)
			behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);

		//utility.WrapBehaviorOutsideOfVehicle(behavior); Zeli: should not be here, formation can be inside the vehicle as well. Copy+paste?
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_MoveInFormation : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessageGoal msg = SCR_AIMessageGoal.Cast(message);
			
		auto behavior = new SCR_AIMoveInFormationBehavior(utility, msg.m_bIsPriority, vector.Zero, Prioritize(SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_IN_FORMATION, msg.m_bIsPriority));
		
		if (msg && msg.m_RelatedGroupActivity)
			behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);

		//utility.WrapBehaviorOutsideOfVehicle(behavior); Zeli: should not be here, formation can be inside the vehicle as well. Copy+paste?
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_GetInVehicle : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
        auto msg = SCR_AIMessage_GetIn.Cast(message);
		if (!msg)
			return;
			
        auto behavior = new SCR_AIGetInVehicle(utility, msg.m_bIsPriority, msg.m_Vehicle, msg.m_eRoleInVehicle, Prioritize(SCR_AIActionBase.PRIORITY_BEHAVIOR_VEHICLE, msg.m_bIsPriority));					
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		if (msg && msg.m_RelatedGroupActivity)
			behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);

		//utility.WrapBehaviorOutsideOfVehicle(behavior); Zeli: should not be here, probably copy+paste?
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
  		auto msg = SCR_AIMessage_GetIn.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AIGetInActivity(utility, false, msg.m_bIsWaypointRelated, msg.m_Vehicle, msg.m_eRoleInVehicle);
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_GetOutVehicle : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
        auto msg = SCR_AIMessage_GetOut.Cast(message);
		if (!msg)
			return;

        auto behavior = new SCR_AIGetOutVehicle(utility, msg.m_bIsPriority, msg.m_Vehicle, Prioritize(SCR_AIActionBase.PRIORITY_BEHAVIOR_GET_OUT_VEHICLE, msg.m_bIsPriority));
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
  		auto msg = SCR_AIMessage_GetOut.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AIGetOutActivity(utility, false, msg.m_bIsWaypointRelated, msg.m_Vehicle);
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_SeekAndDestroy : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
  		auto msg = SCR_AIMessage_SeekAndDestroy.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AISeekAndDestroyActivity(utility, false, msg.m_bIsWaypointRelated, msg.m_MovePosition, ent: msg.m_FollowEntity, noVehicles: msg.m_UseVehicles);				
		utility.SetStateAllActionsOfType(EAIActionType.MOVE_IN_FORMATION,EAIActionState.FAILED);
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Heal : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Heal.Cast(message);
		if (!msg)
			return;

		auto behavior = new SCR_AIMedicHealBehavior (utility, false, msg.m_FollowEntity,false);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		if (msg && msg.m_RelatedGroupActivity)
			behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);

		utility.WrapBehaviorOutsideOfVehicle(behavior);
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_HealWait : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_HealWait.Cast(message);
		if (!msg)
			return;
		
		auto behavior = new SCR_AIHealWaitBehavior(utility, false, msg.m_HealProvider);
		utility.WrapBehaviorOutsideOfVehicle(behavior); // Should we dismount to get healed?
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Defend : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Defend.Cast(message);
		if (!msg)
			return;

		auto activity = new SCR_AIDefendActivity(utility, false, msg.m_bIsWaypointRelated, msg.m_WaypointEntity);
		if (m_OverrideBehaviorTree != string.Empty)
			activity.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.AddAction(activity);
	}	
};

[BaseContainerProps()]
class SCR_AIGoalReaction_PerformAction : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_PerformAction.Cast(message);
		if (!msg)
			return;

		auto behavior = new SCR_AIPerformActionBehavior(utility, false, msg.m_SmartActionComponent);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		if (msg && msg.m_RelatedGroupActivity)
			behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);

		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_PerformAction.Cast(message);
		if (!msg)
			return;

		auto activity = new SCR_AIPerformActionActivity(utility, false, msg.m_bIsWaypointRelated, msg.m_SmartActionEntity, msg.m_SmartActionTag);
		if (m_OverrideBehaviorTree != string.Empty)
			activity.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Cancel : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_Cancel msg = SCR_AIMessage_Cancel.Cast(message);
        if (!msg || !msg.m_RelatedGroupActivity)
			return;
		utility.FailBehaviorsOfActivity(msg.m_RelatedGroupActivity);
	}	
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIActivityBase activity = SCR_AIActivityBase.Cast(utility.GetCurrentAction());
		AIGroup group = utility.m_Owner;
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			SCR_AISendCancelMsg(utility, activity, group, agent);
		}
		activity.Fail();
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Resupply : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Resupply.Cast(message);
		if (!msg)
			return;

		auto behavior = new SCR_AIResupplyBehavior(utility, false, msg.m_ResupplyEntity, msg.m_ResupplyPosition, msg.m_MagazineWell,  true);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		if (msg && msg.m_RelatedGroupActivity)
			behavior.SetGroupActivityContext(msg.m_RelatedGroupActivity);

		utility.AddAction(behavior);
	}	
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Retreat : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_Retreat msg = SCR_AIMessage_Retreat.Cast(message);
		if (!msg)
			return;
		
		EAIRetreatBehaviorType type = msg.m_eRetreatType;
		
		SCR_AIActionBase action;
		switch (type)
		{
			case EAIRetreatBehaviorType.RETREAT_FROM_ENEMY:
				action = new SCR_AIRetreatFromCurrentEnemyBehavior(utility, false);
				break;
			case EAIRetreatBehaviorType.RETREAT_WHILE_NO_AMMO:
				action = new SCR_AIRetreatNoAmmoBehavior(utility, false);
				break;
		}
		
		if (action)
			utility.AddAction(action);
		else
			Print(string.Format("Wrong value of EAIRetreatBehaviorType: %1", type), LogLevel.ERROR);
	}
};

//--------------------------------------------------------------------------------------------------------
// Helper methods used in above

static void SCR_AISendLostMsg(SCR_AIUtilityComponent utility, IEntity target)
{
	AIAgent agent = AIAgent.Cast(utility.GetOwner());
	if (!agent)
		return;
	AICommunicationComponent mailbox = agent.GetCommunicationComponent();
	if (mailbox)
	{
		auto group = agent.GetParentGroup();
	
		SCR_AIMessage_TargetLost msg = new SCR_AIMessage_TargetLost;
		msg.m_Target = target;
		msg.SetText("Target lost from reaction");
		msg.SetReceiver(group);
		mailbox.RequestBroadcast(msg, group);
	}	
};

static void SCR_AISendCancelMsg(SCR_AIGroupUtilityComponent utility, SCR_AIActivityBase relatedActivity, AIAgent sender, AIAgent receiver)
{
	if (!utility.m_Owner)
		return;
	AICommunicationComponent mailbox = utility.m_Owner.GetCommunicationComponent();
	if (mailbox)
	{
		SCR_AIMessage_Cancel msg = new SCR_AIMessage_Cancel;
		msg.m_RelatedGroupActivity = relatedActivity;
		msg.SetText("Cancelled activity");
		msg.SetReceiver(receiver);
		mailbox.RequestBroadcast(msg, receiver);
	}	
};