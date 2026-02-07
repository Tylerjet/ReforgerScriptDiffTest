// AI healing behaviour
// TODO: You have to handle situation, in which movement can be disabled (in AIConfig component)
class SCR_AIResupplyBehavior : SCR_AIBehaviorBase
{
	SCR_AIConfigComponent m_Config;
	
	ref SCR_BTParamAssignable<IEntity> m_EntityToResupply = new SCR_BTParamAssignable<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParam<vector> m_vPositionToResupplyAt = new SCR_BTParam<vector>(SCR_AIActionTask.POSITION_PORT);
	ref SCR_BTParam<typename> m_magazineWell = new SCR_BTParam<typename>(SCR_AIActionTask.MAGAZINE_WELL_PORT);
	float m_fMovePriority = 91;
	
	ref SCR_AIMoveIndividuallyBehavior m_ResupplyMove;
		
	//-------------------------------------------------------------------
	
	void SCR_AIResupplyBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, IEntity entityToResupply, vector positionToResupplyAt, typename whatToResupply, bool allowResupplyMove, float priority = PRIORITY_BEHAVIOR_RESUPPLY)
    {		
		m_EntityToResupply.Init(this, entityToResupply);
		m_vPositionToResupplyAt.Init(this, positionToResupplyAt);
		m_magazineWell.Init(this, whatToResupply);
		
		if (!utility)
			return;
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Resupply.bt";
		m_Config = m_Utility.m_ConfigComponent;
        m_eType = EAIActionType.RESUPPLY;	
		m_fPriority = priority;
       		
		if (allowResupplyMove && m_Config && m_Config.m_EnableMovement)
		{ 
			vector targetPos;
			if (m_EntityToResupply.m_Value)
			{
				vector randomPosCenter = m_EntityToResupply.m_Value.CoordToParent(vector.Zero);
				targetPos = s_AIRandomGenerator.GenerateRandomPointInRadius(2, 5, randomPosCenter, true);
				targetPos[1] = randomPosCenter[1];
			}
			else
				targetPos = positionToResupplyAt;
			
			m_ResupplyMove = new SCR_AIMoveIndividuallyBehavior(utility, false, targetPos, m_fMovePriority);
			m_Utility.AddAction(m_ResupplyMove);
		}
	}
	
	//-------------------------------------------------------------------
	override void OnActionCompleted()
	{
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_EntityToResupply.m_Value, "Unit resuplied", EAIDebugCategory.INFO, 5);
#endif
		super.OnActionCompleted();
		if (m_ResupplyMove)
			m_ResupplyMove.Complete();
	}
	
	override void OnActionFailed()
	{
		super.OnActionFailed();
		if (m_ResupplyMove)
			m_ResupplyMove.Fail();
	}
};

