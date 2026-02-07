class SCR_AIMoveActivity : SCR_AIActivityBase
{
	ref SCR_BTParamAssignable<vector> m_vPosition = new SCR_BTParamAssignable<vector>(SCR_AIActionTask.POSITION_PORT);
	ref SCR_BTParam<IEntity> m_Entity = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParam<bool> m_bNoVehicles = new SCR_BTParam<bool>(SCR_AIActionTask.NOVEHICLES_PORT);
		
	void SCR_AIMoveActivity(SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated, vector pos, float priority = PRIORITY_ACTIVITY_MOVE, IEntity ent = null, bool noVehicles = false)
    {
		m_vPosition.Init(this, pos);
		m_vPosition.m_AssignedOut = (m_vPosition.m_Value != vector.Zero);
		m_Entity.Init(this, ent);
		m_bNoVehicles.Init(this, noVehicles);
		
        m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityMove.bt";
		m_eType = EAIActionType.MOVE_IN_FORMATION;
		m_fPriority = priority;
    }
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_vPosition.ValueToString();
	}
};

class SCR_AISeekAndDestroyActivity : SCR_AIMoveActivity
{
	void SCR_AISeekAndDestroyActivity(SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated, vector pos, float priority = PRIORITY_ACTIVITY_SEEK_AND_DESTROY, IEntity ent = null, bool noVehicles = false)
    {
		//m_bRemoveOnCompletion = false; removed: after S&D move was not realized, completed S&D was not removed
       	m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivitySeekDestroy.bt";
		m_eType = EAIActionType.SEEK_DESTROY;
		m_vPosition.Init(this, vector.Zero);		
    }
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " seek and destroy around" + m_Entity.ValueToString();
	}
};

class SCR_AIFollowActivity : SCR_AIMoveActivity
{
   	ref SCR_BTParam<float> m_fCompletionDistance = new SCR_BTParam<float>(SCR_AIActionTask.DESIREDDISTANCE_PORT);
	
	// SCR_AIBaseUtilityComponent utility, bool isWaypointRelated, vector pos, float priority = PRIORITY_ACTIVITY_MOVE, IEntity ent = null, bool noVehicles = false
	void SCR_AIFollowActivity(SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated, vector pos, float priority = PRIORITY_ACTIVITY_FOLLOW, IEntity ent = null, bool noVehicles = false, float distance = 1.0)
    {
		m_fCompletionDistance.Init(this, distance);
		
        m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityFollow.bt";
		m_eType = EAIActionType.FOLLOW;
    }
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " following entity " + m_Entity.ValueToString();
	}
};
