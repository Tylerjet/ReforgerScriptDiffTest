[ComponentEditorProps(category: "GameScripted/AI", description: "Component for utility AI system calculations", color: "0 0 255 255")]
class SCR_AIUtilityComponentClass: SCR_AIBaseUtilityComponentClass
{	
};

//------------------------------------------------------------------------------------------------
class SCR_AIUtilityComponent : SCR_AIBaseUtilityComponent
{
	GenericEntity m_OwnerEntity;
	SCR_CharacterControllerComponent m_OwnerController;
	SCR_AIConfigComponent m_ConfigComponent;
	SCR_AIInfoComponent m_AIInfo;
	SCR_AICombatComponent m_CombatComponent;
	
	ref SCR_AIThreatSystem m_ThreatSystem;
	ref SCR_AILookAction m_LookAction;
	ref SCR_AIBehaviorBase m_CurrentBehavior; // Used for avoiding constant casting, outside of this class use GetCurrentBehavior()
	ScriptedDamageManagerComponent m_DamageManager; // for checking health
	CompartmentAccessComponent m_CompartmentAccess; // for checking if character is inside vehicle or not
	protected IEntity m_EndangeringVehicle; 		// for caching endangering vehicle
	
	SCR_AITargetInfo m_UnknownTarget;

	protected vector m_vInvestigationDestination; // Used for storing planned destination for investigation
	protected float m_fLastUpdateTime;
	protected bool m_bInvestigationDestinationSet;
	
	static const float DISTANCE_HYSTERESIS_FACTOR = 0.45; 	// how bigger must be old distance to new in IsInvestigationRelevant()
	static const float NEARBY_DISTANCE_SQ = 2500; 			// what is the minimal distance of new vs old in IsInvestigationRelevant()	

	//------------------------------------------------------------------------------------------------
	SCR_AIBehaviorBase EvaluateBehavior(SCR_AITargetInfo unknownTarget, SCR_AIMessageBase commandMessage)
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
			
		if (unknownTarget && m_UnknownTarget != unknownTarget && m_ConfigComponent.m_Reaction_UnknownTarget)
		{
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("PerformReaction: Unknown Target: %1", unknownTarget));
			#endif
			m_ConfigComponent.m_Reaction_UnknownTarget.PerformReaction(this, m_ThreatSystem, unknownTarget.m_Target, unknownTarget.m_LastSeenPosition);
		}
		m_UnknownTarget = unknownTarget;
		
		m_CombatComponent.EvaluateCurrentTarget();		
		
		if (m_CombatComponent.IsTargetChanged() && m_ConfigComponent.m_Reaction_EnemyTarget)
		{
			BaseTarget enemy = m_CombatComponent.GetCurrentEnemy();
			if (enemy)
			{
				#ifdef AI_DEBUG
				AddDebugMessage(string.Format("PerformReaction: Enemy Target: %1", enemy));
				#endif
				m_ConfigComponent.m_Reaction_EnemyTarget.PerformReaction(this, m_ThreatSystem, enemy.GetTargetEntity(), enemy.GetLastSeenPosition());
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
		
		m_CurrentBehavior.OnActionExecuted();
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateBehavior END\n");
		#endif
		
		return m_CurrentBehavior;
	}

	
	// Independent attacking with movement is allowed only when group combat move is not present or unit state is EUnitState.STATIC
	//------------------------------------------------------------------------------------------------
	bool CanIndependentlyMove()
	{
		if (m_AIInfo && m_AIInfo.HasUnitState(EUnitState.STATIC))
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
		
		if (m_AIInfo && m_AIInfo.HasUnitState(EUnitState.STATIC))
			return;
		
		float score = action.Evaluate();
		IEntity vehicle = compartmentAccess.GetCompartment().GetOwner();
		ECompartmentType compartmentType = SCR_AIGetUsableVehicle.CompartmentClassToType(compartmentAccess.GetCompartment().Type());
		AddAction(new SCR_AIGetOutVehicle(this, false, vehicle, score + 100));
		AddAction(new SCR_AIGetInVehicle(this, false, vehicle, compartmentType));
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateAttackParameters(IEntity target, vector position)
	{
		foreach (SCR_AIActionBase action: m_aActions)
		{
			if (action.IsInherited(SCR_AIAttackBehavior))
			{
				SCR_AIAttackBehavior behavior = SCR_AIAttackBehavior.Cast(action);
				behavior.m_vPosition.m_Value = position;
				behavior.m_Target.m_Value = target; // may be null!				
			}
		}
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

		m_DamageManager = ScriptedDamageManagerComponent.Cast(m_OwnerEntity.FindComponent(ScriptedDamageManagerComponent));
		m_AIInfo = SCR_AIInfoComponent.Cast(agent.FindComponent(SCR_AIInfoComponent));
		m_CombatComponent = SCR_AICombatComponent.Cast(m_OwnerEntity.FindComponent(SCR_AICombatComponent));
		m_OwnerController = SCR_CharacterControllerComponent.Cast(m_OwnerEntity.FindComponent(SCR_CharacterControllerComponent));
		m_ThreatSystem = new ref SCR_AIThreatSystem(this);
		m_AIInfo.InitThreatSystem(m_ThreatSystem); // let the AIInfo know about the threat system - move along with creating threat system instance!
		m_LookAction = new ref SCR_AILookAction(this, false); // LookAction is not regular behavior and is evaluated separately
		m_ConfigComponent.AddDefaultBehaviors(this);
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
		if (m_AIInfo.HasUnitState(EUnitState.STATIC))
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
	bool IsInvestigationRelevant(vector investigationPosition, float investigationRadiusSq)
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
	
	//------------------------------------------------------------------------------------------------
	IEntity GetEndangeringVehicle()
	{
		return m_EndangeringVehicle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEndangeringVehicle(IEntity vehicle)
	{
		m_EndangeringVehicle = vehicle;
	}
};

