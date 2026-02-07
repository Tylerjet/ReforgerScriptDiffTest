class SCR_AIAbortWaypoint: AITaskScripted
{
	private ref array<AIAgent> m_groupMembers = {};
	private bool m_bAbortDone = true; // Hotfix: to prevent double abort, also true to prevent abort of never executed node
		
	[Attribute("1", UIWidgets.CheckBox, "Leave turret when aborting?")];
	private bool m_bLeaveTurret;
	
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
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!owner || m_bAbortDone || !aiWorld)
			return;
		SCR_AIGroup myGroup = SCR_AIGroup.Cast(owner);
		if (!myGroup)
		{
			NodeError(this,owner,"SCR_AIAbortWaypoint must be run on SCR_AIGroup agent!");
			return;
		}		
		
		myGroup.GetAgents(m_groupMembers);
				
		AICommunicationComponent mailbox = owner.GetCommunicationComponent();
		if (!mailbox)
			return;
		
		if (m_bLeaveTurret)
		{
			foreach (AIAgent receiver: m_groupMembers)
			{
				if (receiver)
				{
					SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(owner);
					if (!chimeraAgent)
						continue;
					SCR_AIInfoComponent aiInfo = chimeraAgent.m_InfoComponent;
					if (!aiInfo || !aiInfo.HasUnitState(EUnitState.IN_TURRET))
						continue;
					
					SCR_AIMessage_AttackStaticDone msg1 = SCR_AIMessage_AttackStaticDone.Cast(mailbox.CreateMessage(aiWorld.GetGoalMessageOfType(EMessageType_Goal.ATTACK_STATIC_DONE)));
					if ( !msg1 )
					{
						Debug.Error("Unable to create valid message!");
						continue;
					}
					msg1.SetText("Waypoint was aborted");
					msg1.SetReceiver(receiver);
					mailbox.RequestBroadcast(msg1,receiver);
				
					ChimeraCharacter character = ChimeraCharacter.Cast(receiver.GetControlledEntity());
					if (character && character.IsInVehicle())
					{
						SCR_AIMessage_GetOut msg2 = SCR_AIMessage_GetOut.Cast(mailbox.CreateMessage(aiWorld.GetGoalMessageOfType(EMessageType_Goal.GET_OUT_VEHICLE)));
						if ( !msg2 )
							Debug.Error("Unable to create valid message!");
						msg2.SetText("Waypoint was aborted, leave vehicle");
						msg2.SetReceiver(receiver);
						mailbox.RequestBroadcast(msg2,receiver);
					}
				}
			}
			myGroup.ReleaseCompartments();
		}	

		m_bAbortDone = true;
	}

	//------------------------------------------------------------------------------------------------
    override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
    override bool CanReturnRunning() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription() {return "Node that reacts on abort of node and sends info about waypoint's related actions to be aborted, valid for group agent only";}
};