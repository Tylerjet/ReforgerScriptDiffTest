class SCR_AIPlayAnimation : AITaskScripted
{
	static const string PORT_ENTITY_IN				= "RootEntityIn";
	static const string PORT_AGENT_SCRIPT_IN		= "AgentScriptIn";
	static const string PORT_ANIMATION_INDEX_IN		= "AnimationIndexIn";
	static const string PORT_ABORT_SLOW				= "AbortSlowIn";
	
	protected SCR_AIAnimation_Base m_AIAnimation;
	protected bool m_bAbortDone;
	protected bool m_bInPlayed;
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_bAbortDone = false;
		m_bInPlayed = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		vector mat[4];
		if (!m_bInPlayed)
		{
			IEntity rootEntity;
			if (!GetVariableIn(PORT_ENTITY_IN, rootEntity))
				return ENodeResult.FAIL;
			SCR_AIAnimationScript agentScript;
			int animationIndex;
			GetVariableIn(PORT_AGENT_SCRIPT_IN, agentScript);
			GetVariableIn(PORT_ANIMATION_INDEX_IN, animationIndex);
			if (!rootEntity || !agentScript || !agentScript.IsAnimationIndexValid(animationIndex))
				return ENodeResult.FAIL;
			agentScript.GetAnimationWorldTransform(rootEntity, animationIndex, mat);
			m_AIAnimation = agentScript.GetAnimationClass(animationIndex);
			if (!m_AIAnimation)
				return ENodeResult.FAIL;
			m_AIAnimation.StartAnimation(owner.GetControlledEntity(), mat);
			m_bInPlayed = true;
		}
		
		return ENodeResult.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (!m_bAbortDone && m_AIAnimation && m_bInPlayed)
		{
			bool abortSlow;
			GetVariableIn(PORT_ABORT_SLOW, abortSlow);
			m_AIAnimation.StopAnimation(owner.GetControlledEntity(), !abortSlow);
		}	
		m_bAbortDone = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette() { return true; }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {	
		PORT_ENTITY_IN,
		PORT_AGENT_SCRIPT_IN, 
		PORT_ANIMATION_INDEX_IN,
		PORT_ABORT_SLOW,
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanReturnRunning() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override string GetOnHoverDescription()
	{
		return "PlayAnimation: plays animation stored in SCR_AIAnimationScript given by index of that animation and root entity (usually the waypoint). OnAbort plays OUT animation.\nThe node does not check when animation ends! Must be aborted from above!";
	}
};