class SCR_AIAttackBehavior : SCR_AIBehaviorBase
{
	
#ifdef WORKBENCH
	ref Shape m_Shape;
#endif

	// Be careful, here we want to store a ref to BaseTarget
	ref SCR_BTParamRef<BaseTarget> m_Target = new SCR_BTParamRef<BaseTarget>(SCR_AIActionTask.TARGET_PORT);
	
    SCR_AIWorld m_AIWorld;
	
	bool m_bHasGrenades = true;

	bool m_bSelected = false;
	
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
	void SCR_AIAttackBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity, BaseTarget target, vector pos)
    {
		m_Target.Init(this, target);
		
		if (!utility)
			return;
		
		m_fPriority = PRIORITY_BEHAVIOR_ATTACK_NOT_SELECTED;
			
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Attack.bt";
		m_bAllowLook = false;
		m_bResetLook = true;
		m_eType = EAIActionType.ATTACK;
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
};