class SCR_AIAttackBehavior : SCR_AIBehaviorBase
{
	
#ifdef WORKBENCH
	ref Shape m_Shape;
#endif

	// Be careful, here we want to store a ref to BaseTarget
	ref SCR_BTParamRef<BaseTarget> m_Target = new SCR_BTParamRef<BaseTarget>(SCR_AIActionTask.TARGET_PORT);
	
	// Wait time before we start shooting
	ref SCR_BTParam<float> m_fWaitTime = new SCR_BTParam<float>("WaitTime");
	
    SCR_AIWorld m_AIWorld;
	
	bool m_bHasGrenades = true;
	bool m_bSelected = false;

	// This delay is executed before shooting starts
	protected static const float WAIT_TIME_UNEXPECTED = 0.5;
	protected static const float WAIT_TIME_OVERTHREATENED = 0.8;
	
	//----------------------------------------------------------------------------------
	void InitParameters(BaseTarget target, float waitTime)
	{
		m_Target.Init(this, target);
		m_fWaitTime.Init(this, waitTime);
	}
	
	//----------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		m_bSelected = true;
	}

	//----------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		if (m_eState == EAIActionState.COMPLETED || m_eState == EAIActionState.FAILED)
			return;
		super.OnActionCompleted();
	}
	
	//----------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		if (m_eState == EAIActionState.COMPLETED || m_eState == EAIActionState.FAILED)
			return;
		super.OnActionFailed();
	}
	
	//----------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " attacking " + m_Target.ValueToString();
	}
	
	//----------------------------------------------------------------------------------	
	void SCR_AIAttackBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, BaseTarget target, vector pos, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(target, 4);
		if (!utility)
			return;
		
		m_fPriorityLevel.m_Value = priorityLevel;
		m_fPriority = PRIORITY_BEHAVIOR_ATTACK_NOT_SELECTED;
		m_fThreat = 1.01 * SCR_AIThreatSystem.VIGILANT_THRESHOLD;
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Attack.bt";
		m_bAllowLook = false;
		m_bResetLook = true;
		m_bUniqueInActionQueue = false;
		m_AIWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
	}
	
	//----------------------------------------------------------------------------------
	override float Evaluate()
	{
		BaseTarget baseTarget = m_Target.m_Value;
		float targetScore = m_Utility.m_CombatComponent.m_WeaponTargetSelector.CalculateTargetScore(baseTarget);
		if (targetScore >= SCR_AICombatComponent.TARGET_SCORE_HIGH_PRIORITY_ATTACK ||
			baseTarget.IsEndangering())
			return PRIORITY_BEHAVIOR_ATTACK_HIGH_PRIORITY;
		else if (m_bSelected)
			return PRIORITY_BEHAVIOR_ATTACK_SELECTED;
		else
			return PRIORITY_BEHAVIOR_ATTACK_NOT_SELECTED;
	}
	
	//----------------------------------------------------------------------------------
	void SendMessageThrowGrenadeTo(IEntity target, vector position, string reason)
	{
		if (!m_bHasGrenades)
			return;
		
		AIAgent agent = AIAgent.Cast(m_Utility.GetOwner());
		if (!agent | !m_AIWorld)
			return;
		
		//Ensure there's at least 1 grenade in the inventory
		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryManager)
		{
			m_bHasGrenades = false;
			return;
		}	
	
		array<typename> components = {};
		components.Insert(WeaponComponent);
		components.Insert(GrenadeMoveComponent);
		IEntity grenade = inventoryManager.FindItemWithComponents(components, EStoragePurpose.PURPOSE_DEPOSIT);
		if (!grenade)
		{
			m_bHasGrenades = false;
			return;
		}	
		AICommunicationComponent mailbox = agent.GetCommunicationComponent();
		if (!mailbox)
			return;
		
		SCR_AIMessage_ThrowGrenadeTo msg = SCR_AIMessage_ThrowGrenadeTo.Cast(mailbox.CreateMessage(m_AIWorld.GetGoalMessageOfType(EMessageType_Goal.THROW_GRENADE_TO)));
		if (!msg)
			Debug.Error("Unable to create valid message!");
		msg.SetText(reason);
		msg.m_MessageType = EMessageType_Goal.THROW_GRENADE_TO;
		msg.m_TargetEntity = target;
		msg.m_vTargetPosition = position;
		msg.SetReceiver(agent);
		mailbox.RequestBroadcast(msg, agent);
	}
	
	// Sets the delay until shooting starts.
	void InitWaitTime(SCR_AIUtilityComponent utility)
	{
		float threatMeasure = utility.m_ThreatSystem.GetThreatMeasure();
		
		// Delay depending on threat
		float threatDelay;
		if (threatMeasure < SCR_AIThreatSystem.ATTACK_DELAYED_THRESHOLD)
			threatDelay = WAIT_TIME_UNEXPECTED;
		else if (threatMeasure < SCR_AIThreatSystem.THREATENED_THRESHOLD)
			threatDelay = 0;
		else
			threatDelay = WAIT_TIME_OVERTHREATENED;
		
		// Delay depending on distance
		// 0m - 0ms
		// 100m - 340ms
		// 300m - 700ms
		// 500m - 870ms
		// 800m - 1018ms
		float distance = vector.Distance(m_Utility.m_OwnerEntity.GetOrigin(), m_Target.m_Value.GetLastSeenPosition());
		float distanceDelay = (1.4 * distance) / (300 + distance);
		
		m_fWaitTime.m_Value = threatDelay + distanceDelay;
	}
};