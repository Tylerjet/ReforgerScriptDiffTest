enum EAIActionState
{
	EVALUATED,
	RUNNING,
	SUSPENDED,
	COMPLETED,
	FAILED
};

class SCR_AIActionBase
{
	EAIActionState m_eState;

	// TODO: Get rid of flags we don't neccesarily need	
	bool m_bUniqueInActionQueue = true;
	bool m_bIsInterruptable = true;
	bool m_bSuspended = false;
	protected float m_fPriority;
	protected ref SCR_BTParam<float> m_fPriorityLevel = new SCR_BTParam<float>(SCR_AIActionTask.PRIORITY_LEVEL_PORT); // used when utility component calls Evaluate() on action, adds level of priority
	
	ResourceName m_sBehaviorTree;
	
	ResourceName m_sAbortBehaviorTree;
	
	ref ScriptInvoker m_OnActionCompleted = new ref ScriptInvoker();
	ref ScriptInvoker m_OnActionFailed = new ref ScriptInvoker();
	
	// Array with parameters which must be exposed to scripts.
	// For example see how m_bPrioritize is used here.
	ref array<SCR_BTParamBase> m_aParams = {};	
	
	//-------------------------------------------------------------------------------------
	float Evaluate()
	{
		return m_fPriority;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void SetPriority(float priority)
	{
		m_fPriority = priority;
	}
	
	//-------------------------------------------------------------------------------------
	float EvaluatePriorityLevel()
	{
		return m_fPriorityLevel.m_Value;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void SetPriorityLevel(int priority)
	{
		m_fPriorityLevel.m_Value = priority;
	}
	
	//-------------------------------------------------------------------------------------
	void SetActionState(EAIActionState state)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetActionState: %1, %2", typename.EnumToString(EAIActionState, state), GetActionDebugInfo()));
		#endif
		m_eState = state;
	}
	
	//-------------------------------------------------------------------------------------
	EAIActionState GetActionState()
	{
		return m_eState;
	}
	
	//-------------------------------------------------------------------------------------
	void SetSuspended(bool suspended)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetSuspended: %1, %2", suspended, GetActionDebugInfo()));
		#endif
		
		m_bSuspended = suspended;
	}
	
	//-------------------------------------------------------------------------------------
	bool GetSuspended()
	{
		return m_bSuspended;
	}
	
	//-------------------------------------------------------------------------------------
	void Complete()
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("Complete: %1", GetActionDebugInfo()));
		#endif
		OnActionCompleted();
		m_OnActionCompleted.Invoke(this);
	}
	
	//-------------------------------------------------------------------------------------
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
	
	//-------------------------------------------------------------------------------------
	bool IsActionInterruptable()
	{
		return m_bIsInterruptable || m_eState != EAIActionState.RUNNING;
	}
	
	//-------------------------------------------------------------------------------------
	void SetActionInterruptable(bool IsInterruptable)
	{
		m_bIsInterruptable = IsInterruptable;
	}
	
	//-------------------------------------------------------------------------------------
	void SCR_AIActionBase()
	{	
		m_eState = EAIActionState.EVALUATED;
	}
	
	//-------------------------------------------------------------------------------------
	#ifdef AI_DEBUG
	protected void AddDebugMessage(string str);
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
	
	// Priority levels
	const static float PRIORITY_LEVEL_NORMAL					= 0;
	const static float PRIORITY_LEVEL_PLAYER					= 1000;
	const static float PRIORITY_LEVEL_GAMEMASTER				= 2000;
	
	// Unit behaviors
	const static float PRIORITY_BEHAVIOR_RETREAT_MELEE			= 190 + PRIORITY_LEVEL_PLAYER;
	const static float PRIORITY_BEHAVIOR_MOVE_FROM_DANGER		= 160 + PRIORITY_LEVEL_PLAYER;
	const static float PRIORITY_BEHAVIOR_ATTACK_HIGH_PRIORITY	= 120;	// Attack high priority
	const static float PRIORITY_BEHAVIOR_PICKUP_INVENTORY_ITEMS = 118;
	const static float PRIORITY_BEHAVIOR_RETREAT_FROM_TARGET	= 115;	//		(when attack exists, retreat does not, and the other way)
	const static float PRIORITY_BEHAVIOR_COMBAT_MOVE_GROUP		= 106;
	const static float PRIORITY_BEHAVIOR_HEAL					= 103;
	const static float PRIORITY_BEHAVIOR_MEDIC_HEAL				= 101;
	const static float PRIORITY_BEHAVIOR_PROVIDE_AMMO			= 100;
	const static float PRIORITY_BEHAVIOR_ATTACK_SELECTED		= 90;	// Attack selected
	const static float PRIORITY_BEHAVIOR_HEAL_WAIT				= 83;
	const static float PRIORITY_BEHAVIOR_THROW_GRENADE			= 73;
	const static float PRIORITY_BEHAVIOR_MOVE_FROM_VEHICLE_HORN	= 72;	
	const static float PRIORITY_BEHAVIOR_ATTACK_NOT_SELECTED	= 70;	// Attack not selected
	const static float PRIORITY_BEHAVIOR_MOVE_FROM_UNKNOWN_FIRE	= 65;
	const static float PRIORITY_BEHAVIOR_OBSERVE_UNKNOWN_FIRE 	= 63;	// Stare at gunfire origin. !!! Priority of this must be higher than move and investigate!
	const static float PRIORITY_BEHAVIOR_DEFEND					= 62;	// Defend selected waypoint	
	const static float PRIORITY_BEHAVIOR_FIND_FIRE_POSITION		= 61;
	const static float PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY		= 60;
	const static float PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE	= 60;
	const static float PRIORITY_BEHAVIOR_VEHICLE				= 51;
	const static float PRIORITY_BEHAVIOR_GET_OUT_VEHICLE		= 51;
	const static float PRIORITY_BEHAVIOR_PERFORM_ACTION			= 50;
	const static float PRIORITY_BEHAVIOR_MOVE					= 30;
	const static float PRIORITY_BEHAVIOR_MOVE_IN_FORMATION		= 30;
	//const static float PRIORITY_BEHAVIOR_
	
	// Sequence of actions specific for dismounting turret and getting back
	const static float PRIORITY_BEHAVIOR_DISMOUNT_TURRET		= 300;
	const static float PRIORITY_BEHAVIOR_DISMOUNT_TURRET_INVESTIGATE = 21;
	const static float PRIORITY_BEHAVIOR_DISMOUNT_TURRET_GET_IN = 20;
	
	// Group activities
	const static float PRIORITY_ACTIVITY_RESUPPLY				= 100;
	const static float PRIORITY_ACTIVITY_HEAL					= 100;
	const static float PRIORITY_ACTIVITY_ATTACK					= 70;
	const static float PRIORITY_ACTIVITY_SEEK_AND_DESTROY 		= 60;
	const static float PRIORITY_ACTIVITY_MOVE					= 50;
	const static float PRIORITY_ACTIVITY_PERFORM_ACTION			= 50;
	const static float PRIORITY_ACTIVITY_DEFEND					= 50;
	const static float PRIORITY_ACTIVITY_GET_IN					= 50;
	const static float PRIORITY_ACTIVITY_GET_OUT				= 50;
	const static float PRIORITY_ACTIVITY_FOLLOW					= 50;
};