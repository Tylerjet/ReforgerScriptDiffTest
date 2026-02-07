class SCR_AIGetOutActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<IEntity> m_Vehicle = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	void SCR_AIGetOutActivity(SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated, IEntity vehicle, float priority = PRIORITY_ACTIVITY_GET_OUT)
	{
		m_Vehicle.Init(m_aParams, vehicle);
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityGetOut.bt";			
		m_eType = EAIActionType.GET_OUT_VEHICLE;	
		m_fPriority = priority;
	}
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " leaving " + m_Vehicle.ValueToString();
	}
};

