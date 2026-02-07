enum EAIActionState
{
	EVALUATED,
	RUNNING,
	SUSPENDED,
	COMPLETED,
	FAILED
};

enum EAIActionType
{
	NONE,					// Undefined - works as Null ptr.
	WAIT,					// Default behavior - used if nothing else
	IDLE,					// Looking around, playing idle animations, etc.
	RETURN_TO_DEFAULT,		// Getting to default state: Stance, speed, position, etc. It's default behavior of current waypoint
	MOVE_IN_FORMATION,		// Basic movement following formation
	MOVE_INDIVIDUALLY,		// Move independently on formation
	MOVE_COMBAT,			// Move to designated areas between covers
	GET_IN_VEHICLE,			// Get to specified vehicle
	GET_OUT_VEHICLE,		// Get out from specified vehicle
	SEEK_DESTROY,			// Different group behavior regime than ordinary move
	MOVE_COMBAT_GROUP,		// Move to designated areas between covers
	MOVE_FROM_DANGER,		// Move away from given possition
	MOVE_FROM_UNKNOWN_FIRE,	// Moves away from shooter
	MOVE_FROM_VEHICLE_HORN,	// Moves from a friendly vehicle horn
	FOLLOW,					// Follow entity
	INVESTIGATE,			// Investigate location with estimated threat level
	FIND_FIRE_POSITION,		// Find a position from where we can engage a target
	ATTACK,					// Encapsulates most attack behaviors or activities
	ATTACK_STATIC,			// Attacking from static turret or vehicle weapon
	TAKE_COVER,				// Select best cover and go to it
	REGROUP,				// Get soldiers back to formation 
	UNGROUP,				// Dissolve formation
	RETREAT,				// Retreat from battle
	HEAL,					// Healing behavior for single soldier
	HEAL_WAIT,				// Waiting to be healed by someone
	MEDIC_HEAL,				// Healing and repairing of other entities
	PERFORM_ACTION,			// Performing a user action on a object(opening doors etc.)
	RECONFIGURE,			// Reconfiguring of relays by AI
	DEFEND,					// Use covers and active objects in designated area	
	THROW_GRENADE, 			// Throw a grenade object to a given position or target
	PROVIDE_AMMO,			// Provide ammo to someone
	PICKUP_INVENTORY_ITEMS,	// Pick up inventory items on the ground
	ACTIVITY_RESUPPLY
};

class SCR_AIActionBase
{
	SCR_AIBaseUtilityComponent m_UtilityBase;
    EAIActionState m_eState;
	EAIActionType m_eType;

	// TODO: Get rid of flags we don't neccesarily need	
	bool m_bUniqueInActionQueue = true;
	bool m_bIsInterruptable = true;
	bool m_bSuspended = false;
	float m_fPriority;
	ResourceName m_sBehaviorTree;
	
	ResourceName m_sAbortBehaviorTree;
	
	ref ScriptInvoker m_OnActionCompleted = new ref ScriptInvoker();
	ref ScriptInvoker m_OnActionFailed = new ref ScriptInvoker();
	
	// Array with parameters which must be exposed to scripts.
	// For example see how m_bPrioritize is used here.
	ref array<SCR_BTParamBase> m_aParams = {};
	
	ref SCR_BTParam<bool> m_bPrioritize = new SCR_BTParam<bool>(SCR_AIActionTask.PRIORITIZE_PORT);
	
    float Evaluate()
    {
        return m_fPriority;
    }
	
	void SetActionState(EAIActionState state)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetActionState: %1, %2", typename.EnumToString(EAIActionState, state), GetActionDebugInfo()));
		#endif
		m_eState = state;
	}
	
	EAIActionState GetActionState()
	{
		return m_eState;
	}
	
	void SetSuspended(bool suspended)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetSuspended: %1, %2", suspended, GetActionDebugInfo()));
		#endif
		
		m_bSuspended = suspended;
	}
	
	bool GetSuspended()
	{
		return m_bSuspended;
	}
	
	void Complete()
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("Complete: %1", GetActionDebugInfo()));
		#endif
		OnActionCompleted();
		m_OnActionCompleted.Invoke(this);
	}
	
	void Fail()
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("Fail: %1", GetActionDebugInfo()));
		#endif
		OnActionFailed();		
		m_OnActionFailed.Invoke(this);
	}
	
	// Called for basic info to print out for specific behaviors - to override
	string GetActionDebugInfo()
	{
		return this.ToString();
	}
	
	// Called after behavior was selected after different behavior
	void OnActionSelected() {m_eState = EAIActionState.RUNNING;}

	// Called after behavior different behavior was selected instead of this one
	void OnActionDeselected() {m_eState = EAIActionState.EVALUATED;}
	
	// Called each frame behavior was selected, before it's executed in the behavior tree
	void OnActionExecuted() { }
	
	// Called when utility component recognizes completion of the behavior
	void OnActionCompleted() {m_eState = EAIActionState.COMPLETED;}
	
	// Called when utility component recognizes failure of the behavior
	void OnActionFailed() {m_eState = EAIActionState.FAILED;}
	
	// Called when any new info message arrives, regardless of state of this action.
	// When any of the messages returns true, the reaction is not invoked.
	bool OnInfoMessage(SCR_AIMessageBase msg) { return false; };
	
	bool IsActionInterruptable()
	{
		return m_bIsInterruptable;
	}
	
	void SetActionInterruptable(bool IsInterruptable)
	{
		m_bIsInterruptable = IsInterruptable;
	}
	
	void SCR_AIActionBase(SCR_AIBaseUtilityComponent utility, bool prioritize)
	{	
		m_bPrioritize.Init(m_aParams, prioritize);
		
		m_UtilityBase = utility;
		m_eState = EAIActionState.EVALUATED;
	}
	
	
	#ifdef AI_DEBUG
	protected void AddDebugMessage(string str)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(m_UtilityBase.GetOwner().FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(string.Format("%1: %2", this, str), msgType: EAIDebugMsgType.ACTION);
	}
	#endif
	
	
	//---------------------------------------------------------------------------------------------------
	// These methods are used by SCR_AIGetActionParameters and SCR_AISetActionParameters.
	// They help move data between action and behavior tree variables.
	
	void SetParametersToBTVariables(SCR_AIActionTask node)
	{
		foreach (SCR_BTParamBase param : m_aParams)
			param.SetVariableOut(node);
	}
	
	void GetParametersFromBTVariables(SCR_AIActionTask node)
	{
		foreach (SCR_BTParamBase param : m_aParams)
			param.GetVariableIn(node);
	}
	
	// Returns array with port names of all parameters of this action.
	TStringArray GetPortNames()
	{
		TStringArray namesOut = {};
		foreach (SCR_BTParamBase p : m_aParams)
			namesOut.Insert(p.m_sPortName);
		return namesOut;
	}
	
	//---------------------------------------------------------------------------------------------------
	const static float MAX_PRIORITY = 100;
	
	// Unit behaviors
	const static float PRIORITY_BEHAVIOR_RETREAT_MELEE			= 190;
	const static float PRIORITY_BEHAVIOR_RETREAT				= 170;
	const static float PRIORITY_BEHAVIOR_MOVE_FROM_DANGER		= 160;
	const static float PRIORITY_BEHAVIOR_ATTACK_HIGH_PRIORITY	= 120;	// Attack high priority
	const static float PRIORITY_BEHAVIOR_PICKUP_INVENTORY_ITEMS = 118;
	const static float PRIORITY_BEHAVIOR_RETREAT_FROM_TARGET	= 115;	//		(when attack exists, retreat does not, and the other way)
	const static float PRIORITY_BEHAVIOR_COMBAT_MOVE_GROUP		= 106;
	const static float PRIORITY_BEHAVIOR_MEDIC_HEAL				= 101;
	const static float PRIORITY_BEHAVIOR_PROVIDE_AMMO			= 100;
	const static float PRIORITY_BEHAVIOR_ATTACK_SELECTED		= 90;	// Attack selected
	const static float PRIORITY_BEHAVIOR_HEAL					= 85;
	const static float PRIORITY_BEHAVIOR_HEAL_WAIT				= 83;
	const static float PRIORITY_BEHAVIOR_THROW_GRENADE			= 73;
	const static float PRIORITY_BEHAVIOR_MOVE_FROM_VEHICLE_HORN	= 72;	
	const static float PRIORITY_BEHAVIOR_ATTACK_NOT_SELECTED	= 70;	// Attack not selected
	const static float PRIORITY_BEHAVIOR_MOVE_FROM_UNKNOWN_FIRE	= 65;
	const static float PRIORITY_BEHAVIOR_FIND_FIRE_POSITION		= 61;
	const static float PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY		= 60;
	const static float PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE	= 60;
	const static float PRIORITY_BEHAVIOR_VEHICLE				= 51;
	const static float PRIORITY_BEHAVIOR_GET_OUT_VEHICLE		= 51;
	const static float PRIORITY_BEHAVIOR_PERFORM_ACTION			= 50;
	const static float PRIORITY_BEHAVIOR_MOVE					= 30;
	const static float PRIORITY_BEHAVIOR_MOVE_IN_FORMATION		= 30;
	//const static float PRIORITY_BEHAVIOR_
	
	// Group activities
	const static float PRIORITY_ACTIVITY_SEEK_AND_DESTROY = 100;
	const static float PRIORITY_ACTIVITY_RESUPPLY		= 100;
	const static float PRIORITY_ACTIVITY_HEAL			= 100;
	const static float PRIORITY_ACTIVITY_ATTACK			= 70;
	const static float PRIORITY_ACTIVITY_MOVE			= 50;
	const static float PRIORITY_ACTIVITY_PERFORM_ACTION	= 50;
	const static float PRIORITY_ACTIVITY_DEFEND			= 50;
	const static float PRIORITY_ACTIVITY_GET_IN			= 50;
	const static float PRIORITY_ACTIVITY_GET_OUT		= 50;
	const static float PRIORITY_ACTIVITY_FOLLOW			= 50;
};