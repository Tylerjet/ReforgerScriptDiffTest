class SCR_AIAbortDefendWaypoint: AITaskScripted
{
	private ref array<AIAgent> m_groupMembers;
	private SCR_AIWorld m_aiWorld;
	private bool m_bAbortDone; // Hotfix: to prevent double abort
		
	override void OnInit(AIAgent owner)
	{
		m_groupMembers = new ref array<AIAgent>;				
		m_aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
	}

	override void OnEnter(AIAgent owner)
	{
		m_bAbortDone = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		return ENodeResult.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------
	// we need to send everybody ATTACK_STATIC_DONE and get out of vehicle if in vehicle
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
    {
		if (!owner || m_bAbortDone)
			return;
		AIGroup myGroup = AIGroup.Cast(owner);
		if (!myGroup)
		{
			NodeError(this,owner,"SCR_AIAbortDefendWaypoint must be run on group AIAgent!");
			return;
		}	
		
		myGroup.GetAgents(m_groupMembers);
				
		AICommunicationComponent mailbox = owner.GetCommunicationComponent();
		if (!mailbox)
			return;
		
		foreach (AIAgent receiver: m_groupMembers)
		{
			if (receiver)
			{
				SCR_AIInfoComponent aiInfo = SCR_AIInfoComponent.Cast(receiver.FindComponent(SCR_AIInfoComponent));
				if (!aiInfo || !aiInfo.HasUnitState(EUnitState.IN_TURRET))
					continue;
				
				SCR_AIMessage_AttackStaticDone msg1 = SCR_AIMessage_AttackStaticDone.Cast(mailbox.CreateMessage(m_aiWorld.GetGoalMessageOfType(EMessageType_Goal.ATTACK_STATIC_DONE)));
				if ( !msg1 )
				{
					Debug.Error("Unable to create valid message!");
					continue;
				}
				msg1.SetText("Waypoint Defend is completed");
				msg1.SetReceiver(receiver);
				mailbox.RequestBroadcast(msg1,receiver);
			
				ChimeraCharacter character = ChimeraCharacter.Cast(receiver.GetControlledEntity());
				if (character && character.IsInVehicle())
				{
					SCR_AIMessage_GetOut msg2 = SCR_AIMessage_GetOut.Cast(mailbox.CreateMessage(m_aiWorld.GetGoalMessageOfType(EMessageType_Goal.GET_OUT_VEHICLE)));
					if ( msg2 )
					{
						msg2.SetText("Waypoint Defend is completed, leave vehicle");
						msg2.SetReceiver(receiver);
						mailbox.RequestBroadcast(msg2,receiver);
					}
					else
						Debug.Error("Unable to create valid message!");
				}					
			}
		}
		
		SCR_AIMessage_Cancel msg3 = SCR_AIMessage_Cancel.Cast(mailbox.CreateMessage(m_aiWorld.GetGoalMessageOfType(EMessageType_Goal.CANCEL)));
		if ( msg3 )
		{
			msg3.SetText("Waypoint Defend is completed");
			msg3.SetReceiver(myGroup);
			mailbox.RequestBroadcast(msg3,myGroup);
		}
		else
			Debug.Error("Unable to create valid message!");			
		m_bAbortDone = true;
	}

	//------------------------------------------------------------------------------------------------
    override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
    override bool CanReturnRunning() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription() {return "Node used as hotfix for case that defend waypoint is completed";}
};