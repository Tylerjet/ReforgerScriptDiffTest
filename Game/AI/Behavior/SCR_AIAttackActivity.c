class SCR_AIAttackActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<IEntity> m_Target = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
    ref SCR_BTParamAssignable<vector> m_vPosition = new SCR_BTParamAssignable<vector>(SCR_AIActionTask.POSITION_PORT);
	ref SCR_BTParam<AIAgent> m_Reporter = new SCR_BTParam<AIAgent>(SCR_AIActionTask.REPORTER_PORT);
	AIGroup m_Group;
	SCR_AIGroupUtilityComponent m_GroupUtilityComponent;
	
	void SCR_AIAttackActivity(SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated, IEntity target = null, vector pos = vector.Zero, AIAgent reporter = null)
    {
		m_Target.Init(this, target);
		m_vPosition.Init(this, pos);
		m_Reporter.Init(this, reporter);
		
		if (!utility)
			return;
		
        m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityAttack.bt";
		m_eType = EAIActionType.ATTACK;
		m_GroupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(utility);
		m_Group = m_GroupUtilityComponent.m_Owner;
		m_fPriority = PRIORITY_ACTIVITY_ATTACK; // score of group will to attack will depend on group regime or state - open fire / stealth now set to HIGH_PRIORITY
    }
	
	// this method assure that every member of the group will end his attack move behavior
	void CompleteAttacks()
	{
		if (m_GroupUtilityComponent.IsSomeEnemyKnown())
		{
			return;
		}
		
		array<AIAgent> groupMemebers  = new array<AIAgent>;
		m_Group.GetAgents(groupMemebers);
		AICommunicationComponent mailbox = m_Group.GetCommunicationComponent();
		if (!mailbox)
			return;

		foreach(AIAgent agent : groupMemebers)
		{
			SCR_AIMessage_GroupAttackDone msg = new SCR_AIMessage_GroupAttackDone;
			msg.m_MessageType = EMessageType_Goal.GROUP_ATTACK_DONE;
			msg.SetText("ClosingGroupAttack from activity");
			msg.SetReceiver(agent);
			mailbox.RequestBroadcast(msg, agent);
		}
	}
	
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		CompleteAttacks();
	}
	
	override void OnActionFailed()
	{
		super.OnActionFailed();
		CompleteAttacks();
	}
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " attacking " + m_Target.ValueToString();
	}
};

