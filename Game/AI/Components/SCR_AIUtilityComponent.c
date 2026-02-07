[ComponentEditorProps(category: "GameScripted/AI", description: "Component for utility AI system calculations")]
class SCR_AIUtilityComponentClass: SCR_AIBaseUtilityComponentClass
{	
};

//------------------------------------------------------------------------------------------------
class SCR_AIUtilityComponent : SCR_AIBaseUtilityComponent
{
	GenericEntity m_OwnerEntity;
	protected SCR_CharacterControllerComponent m_OwnerController;
	SCR_AIConfigComponent m_ConfigComponent;
	SCR_AIInfoComponent m_AIInfo;
	SCR_AICombatComponent m_CombatComponent;
	protected EventHandlerManagerComponent	m_EventHandlerManagerComponent;
	
	ref SCR_AIThreatSystem m_ThreatSystem;
	ref SCR_AILookAction m_LookAction;
	ref SCR_AIBehaviorBase m_CurrentBehavior; // Used for avoiding constant casting, outside of this class use GetCurrentBehavior()

	protected SCR_AITargetInfo m_UnknownTarget;

	protected vector m_vInvestigationDestination; // Used for storing planned destination for investigation
	protected float m_fLastUpdateTime;
	protected bool m_bInvestigationDestinationSet;
	
	protected static const float DISTANCE_HYSTERESIS_FACTOR = 0.45; 	// how bigger must be old distance to new in IsInvestigationRelevant()
	protected static const float NEARBY_DISTANCE_SQ = 2500; 			// what is the minimal distance of new vs old in IsInvestigationRelevant()	
	
	//------------------------------------------------------------------------------------------------
	SCR_AIBehaviorBase EvaluateBehavior(SCR_AITargetInfo unknownTarget, SCR_AIMessageBase commandMessage, SCR_AIMessageBase infoMessage)
	{
		if (!m_OwnerController || !m_ConfigComponent || !m_OwnerEntity)
			return null;
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateBehavior START");
		if (m_bEvaluationBreakpoint)
		{
			Print("EvaluateBehavior breakpoint triggered");
			debug;
			m_bEvaluationBreakpoint = false;
		}
		#endif
		
		// Update delta time and players's position
		float time = m_OwnerEntity.GetWorld().GetWorldTime();
		float deltaTime = time - m_fLastUpdateTime;
		m_fLastUpdateTime = time;

		// Create events from commands, danger events, new targets
		m_ThreatSystem.Update(deltaTime);

		if (commandMessage)
		{
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("PerformGoalReaction: %1", commandMessage));
			#endif
			m_ConfigComponent.PerformGoalReaction(this, commandMessage);
		}
		
		// Notify all actions about message
		if (infoMessage)
		{
			bool overrideReaction = false;
			foreach (SCR_AIActionBase action : m_aActions)
				overrideReaction = overrideReaction || action.OnInfoMessage(infoMessage);
			
			#ifdef AI_DEBUG
			if (overrideReaction)
			{
				#ifdef AI_DEBUG
				AddDebugMessage(string.Format("InfoMessage consumed by action: %1", infoMessage));
				#endif
			}
			#endif
			if (!overrideReaction)
			{
				#ifdef AI_DEBUG
				AddDebugMessage(string.Format("PerformInfoReaction: %1", infoMessage));
				#endif
				m_ConfigComponent.PerformInfoReaction(this, infoMessage);
			}
		}
			
		if (unknownTarget && m_UnknownTarget != unknownTarget && m_ConfigComponent.m_Reaction_UnknownTarget)
		{
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("PerformReaction: Unknown Target: %1", unknownTarget));
			#endif
			m_ConfigComponent.m_Reaction_UnknownTarget.PerformReaction(this, m_ThreatSystem, unknownTarget.m_TargetEntity, unknownTarget.m_vLastSeenPosition);
		}
		m_UnknownTarget = unknownTarget;
		
		
		// Evaluate current weapon and target
		bool weaponEvent;
		bool selectedTargetChanged;
		bool retreatTargetChanged;
		bool compartmentChanged;
		m_CombatComponent.EvaluateWeaponAndTarget(weaponEvent, selectedTargetChanged, retreatTargetChanged, compartmentChanged);
		m_CombatComponent.EvaluateExpectedWeapon();
		
		BaseTarget selectedTarget = m_CombatComponent.GetCurrentTarget();
		if (selectedTargetChanged || compartmentChanged)
		{
			// If we have some target valid for attack, we attack it
			// Otherwise if there is a target we can't attack, we retreat
			if (selectedTarget)
			{
				if (m_ConfigComponent.m_Reaction_EnemyTarget)
				{
					#ifdef AI_DEBUG
					AddDebugMessage(string.Format("PerformReaction: Selected Target: %1", selectedTarget));
					#endif
					m_ConfigComponent.m_Reaction_EnemyTarget.PerformReaction(this, m_ThreatSystem, selectedTarget.GetTargetEntity(), selectedTarget.GetLastSeenPosition());
				}
			}
		}
		if (!selectedTarget)
		{
			BaseTarget retreatTarget = m_CombatComponent.GetRetreatTarget();
			if (retreatTarget)
			{
				if (m_ConfigComponent.m_Reaction_RetreatFromTarget)
				{
					// We can't attack anything. Shall we retreat?
					#ifdef AI_DEBUG
					AddDebugMessage(string.Format("PerformReaction: Retreat From Target: %1", retreatTarget));
					#endif
					m_ConfigComponent.m_Reaction_RetreatFromTarget.PerformReaction(this, m_ThreatSystem, retreatTarget.GetTargetEntity(), retreatTarget.GetLastSeenPosition());
				}
			}
		}
		
		// Evaluation: Remove completed behaviors, evaluate, set new behavior
		RemoveObsoleteActions();
		SCR_AIActionBase selectedAction = EvaluateActions();
		
		if (selectedAction && selectedAction != m_CurrentBehavior && (!m_CurrentBehavior || m_CurrentBehavior.IsActionInterruptable()))
		{
			SetCurrentAction(selectedAction);
			m_CurrentBehavior = SCR_AIBehaviorBase.Cast(selectedAction);
#ifdef WORKBENCH
			SCR_AIDebugVisualization.VisualizeMessage(m_OwnerEntity, SCR_AIDebug.GetBehaviorName(m_CurrentBehavior), EAIDebugCategory.BEHAVIOR, 5);
#endif
		}

		// Update look action target
		m_LookAction.Evaluate();
		
		// Evaluate if we must retreat
		if (m_CombatComponent.m_bMustRetreat && !HasActionOfType(SCR_AIRetreatNoAmmoBehavior))
		{
			SCR_AIActionBase action = new SCR_AIRetreatNoAmmoBehavior(this, false, null);
			AddAction(action);
		}
		
		m_CurrentBehavior.OnActionExecuted();
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateBehavior END\n");
		#endif
		
		return m_CurrentBehavior;
	}
	
	// Independent attacking with movement is allowed only when group combat move is not present or unit state is EUnitState.IN_TURRET
	//------------------------------------------------------------------------------------------------
	bool CanIndependentlyMove()
	{
		if (m_AIInfo && m_AIInfo.HasUnitState(EUnitState.IN_TURRET))
			return false;
		
		foreach (SCR_AIActionBase action : m_aActions)
		{
			if (action.m_eType == EAIActionType.MOVE_COMBAT_GROUP)
				return false;
		}
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIBehaviorBase GetCurrentBehavior()
	{
		return m_CurrentBehavior;
	}
	
	//------------------------------------------------------------------------------------------------
	void WrapBehaviorOutsideOfVehicle(SCR_AIActionBase action)
	{
		
	 	CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(m_OwnerEntity.FindComponent(CompartmentAccessComponent));
		if (!compartmentAccess)
			return;
			 
		if (!compartmentAccess.IsInCompartment())
			return;	
		
		if (m_AIInfo && m_AIInfo.HasUnitState(EUnitState.IN_TURRET))
			return;
		
		SCR_AIActivityBase relatedActivity;
		SCR_AIBehaviorBase behavior = SCR_AIBehaviorBase.Cast(action);
		if (behavior)
			relatedActivity = behavior.m_RelatedGroupActivity;
		
		float score = action.Evaluate();
		IEntity vehicle = compartmentAccess.GetCompartment().GetOwner();
		ECompartmentType compartmentType = SCR_AIGetUsableVehicle.CompartmentClassToType(compartmentAccess.GetCompartment().Type());
		AddAction(new SCR_AIGetOutVehicle(this, false, relatedActivity, vehicle, score + 100));
		AddAction(new SCR_AIGetInVehicle(this, false, relatedActivity, vehicle, compartmentType));
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		AIAgent agent = AIAgent.Cast(GetOwner());
		if (!agent)
			return;	
		
		m_ConfigComponent = SCR_AIConfigComponent.Cast(agent.FindComponent(SCR_AIConfigComponent));

		m_OwnerEntity = GenericEntity.Cast(agent.GetControlledEntity());
		if (!m_OwnerEntity)
			return;

		m_AIInfo = SCR_AIInfoComponent.Cast(agent.FindComponent(SCR_AIInfoComponent));
		m_CombatComponent = SCR_AICombatComponent.Cast(m_OwnerEntity.FindComponent(SCR_AICombatComponent));
		m_OwnerController = SCR_CharacterControllerComponent.Cast(m_OwnerEntity.FindComponent(SCR_CharacterControllerComponent));
		m_ThreatSystem = new ref SCR_AIThreatSystem(this);
		m_AIInfo.InitThreatSystem(m_ThreatSystem); // let the AIInfo know about the threat system - move along with creating threat system instance!
		m_LookAction = new ref SCR_AILookAction(this, false); // LookAction is not regular behavior and is evaluated separately
		m_ConfigComponent.AddDefaultBehaviors(this);
		
		m_EventHandlerManagerComponent = EventHandlerManagerComponent.Cast(m_OwnerEntity.FindComponent(EventHandlerManagerComponent));
		if (m_EventHandlerManagerComponent)
			m_EventHandlerManagerComponent.RegisterScriptHandler("OnConsciousnessChanged", this, this.OnConsciousnessChanged, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_AIUtilityComponent()
	{
		if (m_EventHandlerManagerComponent)
			m_EventHandlerManagerComponent.RemoveScriptHandler("OnConsciousnessChanged", this, this.OnConsciousnessChanged, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void FailBehaviorsOfActivity(SCR_AIActivityBase relatedActivity)
	{
		SCR_AIBehaviorBase behavior;
		for(int i=0,length = m_aActions.Count(); i<length; i++)
		{
			behavior = SCR_AIBehaviorBase.Cast(m_aActions[i]);
			if (behavior.m_RelatedGroupActivity == relatedActivity)
				behavior.Fail();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetOrigin()
	{
		return m_OwnerEntity.GetOrigin();
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetInvestigationDestination(out vector destination)
	{
		destination = m_vInvestigationDestination;
		return m_bInvestigationDestinationSet;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetInvestigationDestination(vector destination)
	{
		m_vInvestigationDestination = destination;
		m_bInvestigationDestinationSet = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearInvestigationDestination()
	{
		m_vInvestigationDestination = vector.Zero;
		m_bInvestigationDestinationSet = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks conditions specific to investigation behavior
	bool IsInvestigationAllowed(vector investigatePos)
	{
		if (m_AIInfo.HasUnitState(EUnitState.IN_TURRET))
			return false;
		
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(GetOwner());
		
		if (!agent)
			return true;
		
		AIWaypoint wp = agent.m_GroupWaypoint;
		
		// If there is no waypoint, we can investigate everywhere
		if (!wp)
			return true;
		
		// If group has a defend wp, we can investigate only inside of it
		SCR_DefendWaypoint defendWp = SCR_DefendWaypoint.Cast(wp);
		if (defendWp)
			return vector.Distance(defendWp.GetOrigin(), investigatePos) < defendWp.GetCompletionRadius();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if new investigation is relevant (ie. should be planned instead of previous investigations
	//  returns false if one should stick to the old investigation
	bool IsInvestigationRelevant(vector investigationPosition)
	{
		vector oldInvestigation,myPosition;
		
		if (!GetInvestigationDestination(oldInvestigation))
			return true;
		
		if ( vector.DistanceSq(oldInvestigation,investigationPosition) < NEARBY_DISTANCE_SQ ) //ignore nearby locations
			return false;
		
		myPosition = GetOrigin();
		if (vector.DistanceSq(myPosition,oldInvestigation) * DISTANCE_HYSTERESIS_FACTOR > vector.DistanceSq(myPosition,investigationPosition))
			return true;
		
		return false; // ignore locations further
	}
	
	//----------------------------------------------------------------------------------------------------
	bool ShouldAttackEnd(out BaseTarget enemyTarget, out bool shouldInvestigateFurther = false, out string context = string.Empty)
	{
		SCR_AIAttackBehavior attackBehavior;
		// do I have currently run attack behavior?
		attackBehavior = SCR_AIAttackBehavior.Cast(m_CurrentBehavior);
		if (attackBehavior)
		{
			enemyTarget = attackBehavior.m_Target.m_Value;			
			if (m_CombatComponent.ShouldAttackEndForTarget(enemyTarget,shouldInvestigateFurther,context))
				return true;
			else 
			{
#ifdef AI_DEBUG			
				context = "Attack in progress.";
#endif				
				return false;
			}
		};
		// do I have some attack behavior?
		foreach (SCR_AIActionBase behavior : m_aActions)
		{
			attackBehavior = SCR_AIAttackBehavior.Cast(behavior);
			if (attackBehavior)
			{
				enemyTarget = attackBehavior.m_Target.m_Value;
				if (m_CombatComponent.ShouldAttackEndForTarget(enemyTarget,shouldInvestigateFurther,context))
					return true;
			}
		}
#ifdef AI_DEBUG			
		context = "No attack.";
#endif		
		return false;
	}
	
	
	//------------------------------------------------------------------------------------------------
	bool FinishAttackForTarget(BaseTarget target, ENodeResult behaviorResult)
	{
		if (!target)
			return false;
		
		bool returnValue = false;
		
		foreach (SCR_AIActionBase action: m_aActions)
		{
			if (action.m_eState == EAIActionState.COMPLETED || action.m_eState == EAIActionState.FAILED)
				continue;
			auto attack = SCR_AIAttackBehavior.Cast(action);
			if (!attack)
			{
				auto combatMove = SCR_AICombatMoveGroupBehavior.Cast(action);
				if (combatMove && combatMove.m_Target.m_Value == target)
				{
					if (behaviorResult == ENodeResult.SUCCESS)
						action.Complete();
					else 
						action.Fail();
					returnValue = true;
				}
			}
			else
			{
				if (attack.m_Target.m_Value == target)
				{
					if (behaviorResult == ENodeResult.SUCCESS)
						action.Complete();
					else 
						action.Fail();
					returnValue = true;
				}
			}
		};
		
		return returnValue;
	}
	
	
	//------------------------------------------------------------------------------------------------
	void OnConsciousnessChanged(bool conscious)
	{
		if (!conscious)
		{
			// Send a message to group that we are wounded
			
			AIAgent agent = AIAgent.Cast(GetOwner());
			if (!agent)
				return;
			AIGroup myGroup = agent.GetParentGroup();
			if (!myGroup)
				return;
			
			AICommunicationComponent comms = agent.GetCommunicationComponent();
			if (!comms)
				return;
			
			SCR_AIMessage_Wounded msg = SCR_AIMessage_Wounded.Create(agent.GetControlledEntity());
			msg.SetReceiver(myGroup);
			comms.RequestBroadcast(msg, myGroup);
		}
	}
};

