/*!
Base class of node which gets action parameters from a BT node.

Please refer to SCR_AIGetActionParameters documentation to understand how this class works.
*/

class SCR_AISetActionParameters : SCR_AIActionTask
{
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIActionBase action = GetExecutedAction();
		
		if (!action)
			return ENodeResult.FAIL;
		
		action.GetParametersFromBTVariables(this);
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() { return false; }
};



class SCR_AISetSeekDestroyActivityParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AISeekAndDestroyActivity(null, false, false, vector.Zero)).GetPortNames();
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	override bool VisibleInPalette() { return true; }
};

class SCR_AISetDefendActivityParameters : SCR_AIActionTask
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AIDefendActivity(null, false, false, null)).GetPortNames();
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	override bool VisibleInPalette() { return true; }
};

