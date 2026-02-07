class SCR_AIPerformActionActivity : SCR_AIActivityBase
{
	IEntity m_SmartActionObject;
	string m_SmartActionTag
		
	void SCR_AIPerformActionActivity(SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated, IEntity entity, string tag, float priority = PRIORITY_ACTIVITY_PERFORM_ACTION)
    {
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityPerformAction.bt";
        m_eType = EAIActionType.PERFORM_ACTION;	
		m_fPriority = priority;			
		m_SmartActionObject = entity;	
		m_SmartActionTag = tag;	
    }
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " using smart action with tag " + m_SmartActionTag + " on object " + m_SmartActionObject.ToString();
	}	
	
};
