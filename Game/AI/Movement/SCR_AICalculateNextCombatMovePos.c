class SCR_AICalculateNextCombatMovePos : AITaskScripted
{
	// Inputs
	protected static const string PORT_REQUEST = "Request";
  	protected static const string PORT_TARGET_POS = "TargetPos";
	
	// Outputs
	protected static const string PORT_POS = "Pos";
	
	protected static const float RANDOM_POS_RADIUS = 2;
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{		
		IEntity myEntity = owner.GetControlledEntity();
		if (!myEntity)
			return ENodeResult.FAIL;
		
		// Read inputs
		SCR_AICombatMoveRequestBase rqBase;
		GetVariableIn(PORT_REQUEST, rqBase);
		SCR_AICombatMoveRequest_Move rq = SCR_AICombatMoveRequest_Move.Cast(rqBase);
		if (!rq)
			return SCR_AIErrorMessages.NodeErrorCombatMoveRequest(this, owner, rq);
		
		vector ownerPos = myEntity.GetOrigin();
		
		vector moveDir = SCR_AICombatMoveUtils.CalculateMoveDirection(rq.m_eDirection, ownerPos, rq.m_vMovePos); // It can't return 000!
		
		float moveDistance = rq.m_fMoveDistance;
		
		vector newPositionCenter = ownerPos + moveDir * moveDistance, newPosition;
		
		newPosition = s_AIRandomGenerator.GenerateRandomPointInRadius(0, RANDOM_POS_RADIUS, newPositionCenter, true);
		newPosition[1] = newPositionCenter[1];
		
		SetVariableOut(PORT_POS, newPosition);
		
		return ENodeResult.SUCCESS;
	}
	
	//--------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_POS
	};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_REQUEST
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	override bool VisibleInPalette() { return true; }
	
}