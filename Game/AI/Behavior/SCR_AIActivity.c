class SCR_AIActivityBase : SCR_AIActionBase
{
	SCR_AIGroupUtilityComponent m_Utility;
	ref SCR_BTParam<bool> m_bIsWaypointRelated = new SCR_BTParam<bool>(SCR_AIActionTask.WAYPOINT_RELATED_PORT);	
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void InitParameters(bool isWaypointRelated)
	{
		m_bIsWaypointRelated.Init(this, isWaypointRelated);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------	
	void SCR_AIActivityBase(SCR_AIGroupUtilityComponent utility, bool isWaypointRelated)
	{
		m_Utility = utility;
		InitParameters(isWaypointRelated);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	#ifdef AI_DEBUG
	override void AddDebugMessage(string str)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(m_Utility.GetOwner().FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(string.Format("%1: %2", this, str), msgType: EAIDebugMsgType.ACTION);
	}
	#endif
};

class SCR_AIIdleActivity : SCR_AIActivityBase
{
	//---------------------------------------------------------------------------------------------------------------------------------
	override float Evaluate()
	{
		return 1;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIIdleActivity(SCR_AIGroupUtilityComponent utility, bool isWaypointRelated)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityIdle.bt";
	}
};
