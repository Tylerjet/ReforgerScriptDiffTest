/*!
Behavior for soldier to look at something.
Looking lasts for some time (duration).
There is also a timeout value, after which the action will fail even if never started.
!!! This is a base class and should not be used alone.
!!! Derive a class from it and initialize duration, radius, timeout and priority!
*/
class SCR_AIObservePositionBehavior : SCR_AIBehaviorBase
{
	protected ref SCR_BTParam<vector> m_vPosition = new SCR_BTParam<vector>("Position");
	protected ref SCR_BTParam<float> m_fDuration = new SCR_BTParam<float>("Duration");	// Initialize in derived class
	protected ref SCR_BTParam<float> m_fRadius = new SCR_BTParam<float>("Radius");		// Initialize in derived class
	protected ref SCR_BTParam<bool> m_bUseBinoculars = new SCR_BTParam<bool>("UseBinoculars"); // Initialize in derived class
	
	protected float m_fDeleteActionTime_ms;	// Initialize in derived class by InitTimeout()
	
	//------------------------------------------------------------------------------------------------------------------------
	void InitParameters(vector position)
	{
		m_vPosition.Init(this, position);
		m_fDuration.Init(this, 0);
		m_fRadius.Init(this, 0);
		m_bUseBinoculars.Init(this, false);
	}
	
	// posWorld - position to observe
	void SCR_AIObservePositionBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector posWorld)
	{
		InitParameters(posWorld);
		if (!utility)
			return;
				
		m_sBehaviorTree = "{AD1A56AE2A7ADFE8}AI/BehaviorTrees/Chimera/Soldier/ObservePositionBehavior.bt";
		m_bAllowLook = false; // Disable standard looking
		m_bResetLook = true;
		m_bUniqueInActionQueue = true;
	}
	
	override float Evaluate()
	{
		// Fail action if timeout has been reached
		float currentTime_ms = GetGame().GetWorld().GetWorldTime();
		if (currentTime_ms > m_fDeleteActionTime_ms)
		{
			Fail();
			return 0;
		}
		return m_fPriority;
	}
	
	void InitTimeout(float timeout_s)
	{
		float currentTime_ms = GetGame().GetWorld().GetWorldTime(); // Milliseconds!
		m_fDeleteActionTime_ms = currentTime_ms + 1000 * timeout_s;
	}
};

/*!
Behavior to observe supposed location of where gunshot came from.
*/
class SCR_AIObserveUnknownFireBehavior : SCR_AIObservePositionBehavior
{
	protected const float TIMEOUT_S = 16.0;
	protected const float DURATION_MIN_S = 7.0;
	protected const float DURATION_MAX_S = 13.0;
	protected const float DIRECTION_SPAN_DEG = 32.0;
	protected const float USE_BINOCULARS_DISTANCE_THRESHOLD = 70;
	
	void SCR_AIObserveUnknownFireBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity,
		vector posWorld, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_fThreat = 1.01 * SCR_AIThreatSystem.VIGILANT_THRESHOLD;
		m_fPriority = SCR_AIActionBase.PRIORITY_BEHAVIOR_OBSERVE_UNKNOWN_FIRE;
		m_fPriorityLevel.m_Value = priorityLevel;
		
		if (!utility)
			return;
		
		InitTimeout(TIMEOUT_S);
		
		m_fDuration.m_Value = Math.RandomFloat(DURATION_MIN_S, DURATION_MAX_S);
		
		IEntity controlledEntity = utility.m_OwnerAgent.GetControlledEntity();
		if (controlledEntity)
		{
			float distance = vector.Distance(controlledEntity.GetOrigin(), posWorld);
			float radius = distance * Math.Tan(Math.DEG2RAD * DIRECTION_SPAN_DEG);
			m_fRadius.m_Value = radius;
			
			m_bUseBinoculars.m_Value = distance > USE_BINOCULARS_DISTANCE_THRESHOLD;
		}
	}
};

class SCR_AIGetObservePositionBehaviorParameters: SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIObservePositionBehavior(null, null,	vector.Zero)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};