class SCR_AIMoveBehaviorBase : SCR_AIBehaviorBase
{
    ref SCR_BTParam<vector> m_vPosition = new SCR_BTParam<vector>(SCR_AIActionTask.POSITION_PORT);
	
    //-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveBehaviorBase(SCR_AIBaseUtilityComponent utility, bool prioritize, vector pos = vector.Zero, float priority = PRIORITY_BEHAVIOR_MOVE)
    {
		m_vPosition.Init(this, pos);
		m_fPriority = priority;
    }
};

class SCR_AIMoveInFormationBehavior : SCR_AIMoveBehaviorBase
{
    //-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveInFormationBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, vector pos = vector.Zero, float priority = PRIORITY_BEHAVIOR_MOVE_IN_FORMATION)
    {
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/KeepInFormation.bt";
        m_eType = EAIActionType.MOVE_IN_FORMATION;
		if (prioritize)
		{
			m_bAllowLook = false;
			m_bResetLook = true;
		}
    }
};

class SCR_AIMoveIndividuallyBehavior : SCR_AIMoveBehaviorBase
{
	ref SCR_BTParam<IEntity> m_Entity = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	//-----------------------------------------------------------------------------------------------------
	override float Evaluate()
    {
		//Print(vector.Distance(m_vPosition, m_Utility.m_vOwnerPos));
			return super.Evaluate();
	}

	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveIndividuallyBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, vector pos = vector.Zero, float priority = PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY, IEntity ent = null)
    {
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/MoveIndividually.bt";
        m_eType = EAIActionType.MOVE_INDIVIDUALLY;
		m_Entity.Init(this, ent);
		if (ent)
			m_vPosition.m_Value = ent.GetOrigin();
    }
	
	//-----------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_vPosition.ValueToString();
	}
};

class SCR_AIMoveFromDangerBehavior : SCR_AIMoveBehaviorBase
{
	ref SCR_BTParam<IEntity> m_entityToAvoid = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveFromDangerBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, vector pos = vector.Zero, float priority = PRIORITY_BEHAVIOR_MOVE_FROM_DANGER, IEntity entityToAvoid = null)
    {
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/MoveFromDanger.bt";
        m_eType = EAIActionType.MOVE_FROM_DANGER;
		m_entityToAvoid.Init(this, entityToAvoid);
    }
	
	//-----------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		m_Utility.SetEndangeringVehicle(null);
		super.OnActionCompleted();
	}
	
	//-----------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		m_Utility.SetEndangeringVehicle(null);
		super.OnActionFailed();
	}
};

class SCR_AIMoveAndInvestigateBehavior : SCR_AIMoveBehaviorBase
{
	ref SCR_BTParam<bool> m_bIsDangerous = new SCR_BTParam<bool>(SCR_AIActionTask.IS_DANGEROUS_PORT);
	ref SCR_BTParam<float> m_fRadius = new SCR_BTParam<float>(SCR_AIActionTask.RADIUS_PORT);
	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveAndInvestigateBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, vector pos = vector.Zero, float priority = PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE, bool isDangerous = true, float radius = 10)
    {
		m_bIsDangerous.Init(this, isDangerous);
		m_fRadius.Init(this, radius);
		
        m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/MoveAndInvestigate.bt";
        m_eType = EAIActionType.INVESTIGATE;
		m_fPriority = priority;
		if (m_Utility)
			m_Utility.SetInvestigationDestination(pos);
    }
	
	//-----------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		// when switching from this behavior, it becomes obsolete to continue investigating lost target
		Fail();
		m_Utility.ClearInvestigationDestination();
		super.OnActionDeselected();
	}
};
