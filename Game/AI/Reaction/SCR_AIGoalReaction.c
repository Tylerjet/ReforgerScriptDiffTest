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
		
		if (!SCR_AIIsAlive.IsAlive(msg.m_TargetInfo.m_TargetEntity))
		{
			SCR_AISendLostMsg(utility, msg.m_TargetInfo.m_TargetEntity);
			return;
		}
		
		UpdateLastSeenPosition(utility.m_CombatComponent.FindTargetByEntity(msg.m_TargetInfo.m_TargetEntity), msg.m_TargetInfo);
		utility.m_CombatComponent.SetAssignedTargets({msg.m_TargetInfo.m_TargetEntity});
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_Attack msg = SCR_AIMessage_Attack.Cast(message);
		if (!msg)
			return;
		
		if (!SCR_AIIsAlive.IsAlive(msg.m_TargetInfo.m_TargetEntity))
		{
			utility.RemoveTarget(msg.m_TargetInfo.m_TargetEntity);
			return;
		}
		
		auto activity = new SCR_AIAttackActivity(utility, false, msg.m_bIsWaypointRelated, msg.m_TargetInfo, msg.GetSender());
		utility.AddOrUpdateTarget(msg.m_TargetInfo);
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
		
		if (!utility.m_AIInfo.HasUnitState(EUnitState.IN_TURRET))
		{
			IEntity eTarget = msg.m_TargetInfo.m_TargetEntity;
			SCR_AICombatComponent combatComp = utility.m_CombatComponent;
			
			if (!eTarget || !combatComp)
				return;
			
			// Check if this enemy is known to combat component
			if (combatComp.IsEnemyKnown(eTarget))
			{
				UpdateLastSeenPosition(utility.m_CombatComponent.FindTargetByEntity(eTarget), msg.m_TargetInfo);
				combatComp.SetAssignedTargets({eTarget});
			}
			else
			{
				// Ignore if we must defend a waypoint
				if (!utility.IsInvestigationAllowed(msg.m_TargetInfo.m_vLastSeenPosition))
					return;
				
				float searchRadius = 10.0;
				if (!utility.IsInvestigationRelevant(msg.m_TargetInfo.m_vLastSeenPosition))
					return;
				// Correction to surface
				msg.m_TargetInfo.m_vLastSeenPosition[1] = GetGame().GetWorld().GetSurfaceY(msg.m_TargetInfo.m_vLastSeenPosition[0],msg.m_TargetInfo.m_vLastSeenPosition[2]);
				// Enemy was never seen by this soldier
				// Add investigate behavior
				SCR_AIMoveAndInvestigateBehavior behavior = new SCR_AIMoveAndInvestigateBehavior(utility, false, msg.m_RelatedGroupActivity, msg.m_TargetInfo.m_vLastSeenPosition, isDangerous: true, radius: searchRadius);
				
				utility.WrapBehaviorOutsideOfVehicle(behavior);
				utility.AddAction(behavior);
			}
		}
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
		
		BaseTarget baseTarget = utility.m_CombatComponent.FindTargetByEntity(msg.m_TargetInfo.m_TargetEntity);
		if (!baseTarget)
			return;
		
		UpdateLastSeenPosition(baseTarget, msg.m_TargetInfo);
		auto behavior = new SCR_AIAttackStaticBehavior(utility, false, msg.m_RelatedGroupActivity, baseTarget, msg.m_TargetInfo.m_vLastSeenPosition);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_AttackStaticDone : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		// only relevant for static attacks, others may receive it for the sake of simplicity of trees
		if (utility.m_AIInfo.HasUnitState(EUnitState.IN_TURRET))
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
		SCR_AICombatMoveGroupBehavior behavior = new SCR_AICombatMoveGroupBehavior(utility, false, msg.m_RelatedGroupActivity, msg.m_TargetInfo.m_vLastSeenPosition, target: msg.m_TargetInfo.m_TargetEntity);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
		
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
		
		auto behavior = new SCR_AIMoveIndividuallyBehavior(utility, msg.m_bIsPriority, msg.m_RelatedGroupActivity, msg.m_MovePosition, Prioritize(SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY,msg.m_bIsPriority), msg.m_FollowEntity);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

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
		
		auto behavior = new SCR_AIMoveInFormationBehavior(utility, msg.m_bIsPriority, msg.m_RelatedGroupActivity, vector.Zero, Prioritize(SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_IN_FORMATION, msg.m_bIsPriority));

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
		
		if (!utility.IsInvestigationRelevant(msg.m_vMovePosition))
			return;
		
		auto behavior = new SCR_AIMoveAndInvestigateBehavior(utility, msg.m_bIsPriority, msg.m_RelatedGroupActivity, msg.m_vMovePosition,
			Prioritize(SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE, msg.m_bIsPriority),
			msg.m_bIsDangerous, targetUnitType: msg.m_eTargetUnitType);

		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_MoveInFormation : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessageGoal msg = SCR_AIMessageGoal.Cast(message);
			
		auto behavior = new SCR_AIMoveInFormationBehavior(utility, msg.m_bIsPriority, msg.m_RelatedGroupActivity, vector.Zero, Prioritize(SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_IN_FORMATION, msg.m_bIsPriority));

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
			
        auto behavior = new SCR_AIGetInVehicle(utility, msg.m_bIsPriority, msg.m_RelatedGroupActivity, msg.m_Vehicle, msg.m_eRoleInVehicle, Prioritize(SCR_AIActionBase.PRIORITY_BEHAVIOR_VEHICLE, msg.m_bIsPriority));					
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

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

        auto behavior = new SCR_AIGetOutVehicle(utility, msg.m_bIsPriority, msg.m_RelatedGroupActivity, msg.m_Vehicle, Prioritize(SCR_AIActionBase.PRIORITY_BEHAVIOR_GET_OUT_VEHICLE, msg.m_bIsPriority));
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

		auto behavior = new SCR_AIMedicHealBehavior(utility, false, msg.m_RelatedGroupActivity, msg.m_FollowEntity,false);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

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
		
		auto behavior = new SCR_AIHealWaitBehavior(utility, false, msg.m_RelatedGroupActivity, msg.m_HealProvider);
		
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

		auto behavior = new SCR_AIPerformActionBehavior(utility, false, msg.m_RelatedGroupActivity, msg.m_SmartActionComponent);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

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
				action = new SCR_AIRetreatFromCurrentEnemyBehavior(utility, false, msg.m_RelatedGroupActivity,);
				break;
			case EAIRetreatBehaviorType.RETREAT_WHILE_NO_AMMO:
				action = new SCR_AIRetreatNoAmmoBehavior(utility, false, msg.m_RelatedGroupActivity,);
				break;
		}
		
		if (action)
			utility.AddAction(action);
		else
			Print(string.Format("Wrong value of EAIRetreatBehaviorType: %1", type), LogLevel.ERROR);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_ThrowGrenadeTo : SCR_AIGoalReaction
{
	private static float THROW_PREDICTION_DISTANCE = 5.0;

	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_ThrowGrenadeTo msg = SCR_AIMessage_ThrowGrenadeTo.Cast(message);
		if (!msg)
			return;
		
		SCR_AIThrowGrenadeToBehavior behavior = new SCR_AIThrowGrenadeToBehavior(utility, msg.m_bIsPriority, msg.m_RelatedGroupActivity, msg.m_vTargetPosition);

		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
		
		if (msg.m_TargetEntity)
		{
			bool found = behavior.TraceForGrenadeThrow(utility.GetOrigin(), msg.m_TargetEntity.GetOrigin(), THROW_PREDICTION_DISTANCE, utility.m_OwnerEntity, msg.m_TargetEntity);
			if (found)
				behavior.UpdatePositionInfo(msg.m_TargetEntity.GetOrigin());
			
			utility.AddAction(behavior);
		}
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_ProvideAmmo : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_ProvideAmmo msg = SCR_AIMessage_ProvideAmmo.Cast(message);
		if (!msg)
			return;
		
		SCR_AIProvideAmmoBehavior behavior = new SCR_AIProvideAmmoBehavior(utility, msg.m_bIsPriority, msg.m_RelatedGroupActivity,
			msg.m_AmmoConsumer, msg.m_MagazineWell);
		
		utility.AddAction(behavior);
	}
}

[BaseContainerProps()]
class SCR_AIGoalReaction_PickupInventoryItems : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_PickupInventoryItems msg = SCR_AIMessage_PickupInventoryItems.Cast(message);
		if (!msg)
			return;
		
		SCR_AIPickupInventoryItemsBehavior behavior = new SCR_AIPickupInventoryItemsBehavior(utility, msg.m_bIsPriority, msg.m_RelatedGroupActivity,
			msg.m_vPickupPosition, msg.m_MagazineWellType);
		
		utility.AddAction(behavior);
	}
}

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
		msg.m_TargetInfo = new SCR_AITargetInfo(target, vector.Zero, 0);
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

static void UpdateLastSeenPosition(BaseTarget baseTarget, SCR_AITargetInfo newTargetInfo)
{
	if (baseTarget && baseTarget.GetTargetEntity() == newTargetInfo.m_TargetEntity)
		baseTarget.UpdateLastSeenPosition(newTargetInfo.m_vLastSeenPosition, newTargetInfo.m_fLastSeenTime);
}