class SCR_AIAttackBehavior : SCR_AIBehaviorBase
{
	// Placeholder data
 	// TODO: This data should be read from a config
	private static const float RANDOMIZE_RATIO = 3;
	private static const float LAST_SEEN_TIMEOUT = 5; // how long in until invisible target becomes logically invisible
	private static const float INVESTIGATE_TIME = 7; // how long before investigating the target's location 
	private static const float FORGET_TIME = 60; // how long before forgetting the target 
	private static const float MAX_FIRE_DIST = 950;
	private static const float FIRE_PAUSE_MIN = 0.1;
	private static const float FIRE_PAUSE_MAX = 4;
	
#ifdef WORKBENCH
	ref Shape m_Shape;
#endif

	ref SCR_BTParam<IEntity> m_Target = new SCR_BTParam<IEntity>(SCR_AIActionTask.TARGET_PORT);
    ref SCR_BTParam<vector> m_vPosition = new SCR_BTParam<vector>(SCR_AIActionTask.TARGETPOSITION_PORT); // Last seen position
	
	SCR_AIConfigComponent m_Config;
	SCR_AICombatComponent m_CombatComponent;
	BaseWorld m_World;
	SCR_AIWorld m_AIWorld;
	ref array<BaseTarget> m_Friendlies = new ref array<BaseTarget>;
	
    vector m_vAimingErrorOffset;

	float m_fLastSeenTime;
	float m_fDistance;	
	bool m_bInvestigate = false; // used to remember sending investigate lost target
	
	//----------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		m_fPriority = PRIORITY_BEHAVIOR_ATTACK_SELECTED;
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
	bool LastSeenTimeout()
	{
		return (m_fLastSeenTime > LAST_SEEN_TIMEOUT);
	}
	
	//----------------------------------------------------------------------------------
    override float Evaluate()
    {
		UpdateTargetInfo();		

		if (LastSeenTimeout())
		{			
			if (!m_Target.m_Value)
			{
				SendMessageTargetLost(m_Target.m_Value,m_vPosition.m_Value,"Target does not exist");
				Fail();
				return 0;
			}
			

			if (m_fLastSeenTime > FORGET_TIME)
			{
				// ending combat move and attack: target is lost, informing group
				TalkNegative();
				SendOrderWeaponLowered();
				SendMessageTargetLost(m_Target.m_Value,m_vPosition.m_Value,"Looking for enemy has timed out");
				Fail();
				return 0;
			}	
			
			// investigate or use suppressive fire
			if (m_fLastSeenTime > INVESTIGATE_TIME)
			{
				// if static -> forget about target
				if (m_Utility.m_AIInfo.HasUnitState(EUnitState.STATIC))
				{
					TalkNegative();
					SendOrderWeaponLowered();
					SendMessageTargetLost(m_Target.m_Value,m_vPosition.m_Value,"Looking for enemy has timed out");
					Fail();
					return 0;
				}	
				// if normal combat mode and allow investigate -> move individually
				else if (m_CombatComponent.GetCombatType() == EAICombatType.NORMAL && m_CombatComponent.IsActionAllowed(EAICombatActions.MOVEMENT_TO_LAST_SEEN))
				{
					if (m_bInvestigate)
						SendMessageInvestigatingTarget(m_vPosition.m_Value, true, "Looking for target on last seen position");
					m_bInvestigate = false;
					return 0;
				}
				else if (m_CombatComponent.GetCombatType() == EAICombatType.SUPPRESSIVE)
				{
					return m_fPriority;
				}
				
				return 0;
			}
		}
		else 
		{
			m_bInvestigate = true;
		}
		//Print("S Attack: " + score);
		return m_fPriority;
    }

	//----------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " attacking " + m_Target.ValueToString();
	}
	
	//----------------------------------------------------------------------------------	
	void SCR_AIAttackBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, IEntity target, vector pos)
    {
		m_Target.Init(this, target);
		m_vPosition.Init(this, pos);	
	
		if (!utility)
			return;
		
		m_fPriority = PRIORITY_BEHAVIOR_ATTACK_NOT_SELECTED;
			
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Attack.bt";
		m_bAllowLook = false;
		m_bResetLook = true;
		m_eType = EAIActionType.ATTACK;
        m_bUniqueInActionQueue = true;
		m_Config = m_Utility.m_ConfigComponent;
		m_CombatComponent = m_Utility.m_CombatComponent;
		m_World = GetGame().GetWorld();
		m_AIWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());	
		UpdateTargetInfo();
    }
	
	//----------------------------------------------------------------------------------
	void UpdateTargetInfo()
	{
		BaseTarget target = m_CombatComponent.GetCurrentEnemy();
		//note: at this moment since evaluate current target is callend once in a while
		//GetCurrentEnemy can return null even though I see the enemy
		if (!target)
			target = m_CombatComponent.GetLastSeenEnemy();
		
		if (!target)
			return;
		
		m_fLastSeenTime = target.GetTimeSinceSeen();
		m_vPosition.m_Value = target.GetLastSeenPosition();
		m_Target.m_Value = target.GetTargetEntity();		
	}
	
	//----------------------------------------------------------------------------------
	override IEntity GetActionParameter_Entity()
	{
		return m_Target.m_Value;
	}

	//----------------------------------------------------------------------------------	
	void SendMessageTargetLost(IEntity target, vector position, string reason)
	{
		AIAgent agent = AIAgent.Cast(m_Utility.GetOwner());
		if (!agent | !m_AIWorld)
			return;
		
		AIAgent receiver = agent.GetParentGroup();
		AICommunicationComponent mailbox = agent.GetCommunicationComponent();
		if (!mailbox)
			return;
		
		SCR_AIMessage_TargetLost msg = SCR_AIMessage_TargetLost.Cast(mailbox.CreateMessage(m_AIWorld.GetInfoMessageOfType(EMessageType_Info.TARGET_LOST)));
		if ( !msg )
			Debug.Error("Unable to create valid message!");
		msg.SetText(reason);
		msg.SetReceiver(receiver);
		msg.m_MessageType = EMessageType_Info.TARGET_LOST;	
		msg.m_Target = target;
		msg.m_LastSeenPosition = position;
		mailbox.RequestBroadcast(msg,receiver);
	}
	
	//----------------------------------------------------------------------------------	
	void SendMessageInvestigatingTarget(vector position, bool isDangerous, string reason)
	{
		AIAgent agent = AIAgent.Cast(m_Utility.GetOwner());
		if (!agent | !m_AIWorld)
			return;
		
		AIAgent receiver = agent; // TODO: eventually I should tell the group too
		AICommunicationComponent mailbox = agent.GetCommunicationComponent();
		if (!mailbox)
			return;
				
		SCR_AIMessage_Investigate msg = SCR_AIMessage_Investigate.Cast(mailbox.CreateMessage(m_AIWorld.GetGoalMessageOfType(EMessageType_Goal.INVESTIGATE)));
		if ( !msg )
			Debug.Error("Unable to create valid message!");
		msg.SetText(reason);
		msg.SetReceiver(receiver);			
		msg.m_vMovePosition = position;
		msg.m_bIsDangerous = isDangerous;
		mailbox.RequestBroadcast(msg,receiver);
	}
	
	//----------------------------------------------------------------------------------
	void SendOrderWeaponLowered()
	{
		AIAgent agent = AIAgent.Cast(m_Utility.GetOwner());
		if (!agent | !m_AIWorld)
			return;
		
		AICommunicationComponent mailbox = agent.GetCommunicationComponent();
		if (!mailbox)
			return;

		SCR_AIOrder_WeaponRaised order = SCR_AIOrder_WeaponRaised.Cast(mailbox.CreateMessage(m_AIWorld.GetOrderMessageOfType(EOrderType_Character.WEAPON_RAISED)));
		order.SetText("Enemy lost, raising weapon");
		order.SetReceiver(agent);
		order.m_bWeaponRaised = false;
		mailbox.RequestBroadcast(order,agent);
	}
	
	//----------------------------------------------------------------------------------
	void TalkNegative()
	{
		if (m_Utility && m_Utility.m_OwnerEntity)
		{
			SCR_CommunicationSoundComponent commComp = SCR_CommunicationSoundComponent.Cast(m_Utility.m_OwnerEntity.FindComponent(SCR_CommunicationSoundComponent));
			if (commComp)
			{
				commComp.SetSignalValueStr("Seed",Math.RandomFloat(0,1));
				commComp.SoundEvent("SOUND_CP_NEGATIVEFEEDBACK_NEGATIVE");
			}
		}
	}
};