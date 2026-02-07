class SCR_AIAbortFollowWaypoint: AITaskScripted
{
	private ref array<AIAgent> m_groupMembers;
	private SCR_AIWorld m_aiWorld;
	private bool m_bAbortDone; // Hotfix: to prevent double abort
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_groupMembers = new ref array<AIAgent>;
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
			NodeError(this,owner,"SCR_AIAbortFollowWaypoint must be run on group AIAgent!");
		m_aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		m_bAbortDone = false;
		return ENodeResult.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------
	// we need to to group CANCEL the follow activity
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
    {
		if (!owner || m_bAbortDone)
			return;
		
		AICommunicationComponent mailbox = owner.GetCommunicationComponent();
		if (mailbox)
		{
			SCR_AIMessage_Cancel msg = SCR_AIMessage_Cancel.Cast(mailbox.CreateMessage(m_aiWorld.GetGoalMessageOfType(EMessageType_Goal.CANCEL)));
			if ( !msg )
			{
				Debug.Error("Unable to create valid message!");
				return;
			}
			m_bAbortDone = true;	
			msg.SetText("Waypoint Follow is completed");
			msg.SetReceiver(owner);
			mailbox.RequestBroadcast(msg,owner);					
		}
	}

	//------------------------------------------------------------------------------------------------
    override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
    override bool CanReturnRunning() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription() {return "Node used as hotfix for case that follow waypoint is completed from above";}
};