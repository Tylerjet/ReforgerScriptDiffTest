/*!
Base class of node which outputs action parameters to Behavior Tree variables.

If you need to expose action parameters to a BT, you must create a scripted node class which inherits from SCR_AIGetActionParameters.

The action class must use SCR_BTParam for variables which must be exposed to Behavior Trees.

For example:
class SCR_AIGetFlyHelicopterParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIFlyHelicopterBehavior(null, false)).GetPortNames(); // (1)
	override TStringArray GetVariablesOut() { return s_aVarsOut; }										//
	protected override bool VisibleInPalette() { return true; }											// (2)
};

(1) We need to create an action object once to get port names. We store the resulting array into a static variable.
Please ensure that constructor does not call any other code which is inaccessible during script compilation!

(2) Remember to override VisibleInPallete!
*/

class SCR_AIGetActionParameters : SCR_AIActionTask
{
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!IsActionValid())
			return ENodeResult.FAIL;
		
		m_Action.SetParametersToBTVariables(this);
		
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette() { return false; }
};



//------------------------------------------------------------------------------------------------
class SCR_AIGetGetInVehicleBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIGetInVehicle(null, false, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected override bool VisibleInPalette() { return true; }
};

class SCR_AIGetOutVehicleBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIGetOutVehicle(null, false, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected override bool VisibleInPalette() { return true; }
};

class SCR_AIGetGetInActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIGetInActivity(null, false, false)).GetPortNames();
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	protected override bool VisibleInPalette() { return true; }
};

class SCR_AIGetGetOutActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIGetOutActivity(null, false, false, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected override bool VisibleInPalette() { return true; }
};


class SCR_AIGetAttackActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIAttackActivity(null, false, false)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIAttackActivity attackActivity = SCR_AIAttackActivity.Cast(m_Action);
		if (attackActivity)
		{
			attackActivity.m_vPosition.m_AssignedOut = attackActivity.m_vPosition.m_Value != vector.Zero;
		}
		
		return super.EOnTaskSimulate(owner, dt);
	}
};

class SCR_AIGetMoveActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIMoveActivity(null, false, false, vector.Zero)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};


class SCR_AIGetFollowActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIFollowActivity(null, false, false, vector.Zero)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AIGetDefendActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIDefendActivity(null, false, false, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AIGetCombatMoveGroupBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AICombatMoveGroupBehavior(null, false)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AIGetAttackBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIAttackBehavior(null, false, null, vector.Zero)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AIGetPerformActionParameters : SCR_AIGetActionParameters
{	
	static ref TStringArray s_aVarsOut = (new SCR_AIPerformActionBehavior(null, false, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AIGetResupplyBehaviorParameters : SCR_AIGetActionParameters
{	
	static ref TStringArray s_aVarsOut = (new SCR_AIResupplyBehavior(null, false, null, vector.Zero, Class, false)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIResupplyBehavior resupplyBehavior = SCR_AIResupplyBehavior.Cast(m_Action);
		
		if (resupplyBehavior)
			resupplyBehavior.m_EntityToResupply.m_AssignedOut = resupplyBehavior.m_EntityToResupply.m_Value != null;
		
		return super.EOnTaskSimulate(owner, dt);
	}
};

class SCR_AIGetResupplyActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (SCR_AIResupplyActivity(null, false, false, null, Class)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AIGetInvestigateBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIMoveAndInvestigateBehavior(null, false)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AIGetHealActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIHealActivity(null, false, false, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AIGetHealWaitBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIHealWaitBehavior(null, false, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AIGetMedicHealBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIMedicHealBehavior(null, false, null, false)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AIGetMoveFromDangerBehaviorParameters  : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIMoveFromDangerBehavior(null, false)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AIGetMoveIndividuallyBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIMoveIndividuallyBehavior(null, false)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AIGetIdleBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIIdleBehavior(null, false)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIIdleBehavior behavior = SCR_AIIdleBehavior.Cast(m_Action);
		
		if (behavior)
		{
			if (behavior.m_vPosition.m_Value == vector.Zero)
				behavior.m_vPosition.m_Value = behavior.m_Utility.GetOrigin();
		}
		
		return super.EOnTaskSimulate(owner, dt);
	}
};