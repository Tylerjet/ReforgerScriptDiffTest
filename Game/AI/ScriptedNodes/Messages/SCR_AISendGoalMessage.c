class SCR_AISendGoalMessage: SCR_AISendMessage
{
	[Attribute("0", UIWidgets.ComboBox, "Message type", "", ParamEnumArray.FromEnum(EMessageType_Goal) )]
	private EMessageType_Goal m_messageType;
	
	[Attribute("0", UIWidgets.CheckBox, "Prioritize reaction")]
	bool m_bPrioritize;
	
	[Attribute("0", UIWidgets.CheckBox, "Goal is ordered by waypoint or is autonomous?")]
	bool m_bIsWaypointRelated;
	
	// Make scripted node visible or hidden in nodes palette
    override bool VisibleInPalette()
    {
        return true;
    }

	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (!InitSendMessageInputs(owner))
			return ENodeResult.FAIL;
		
		SCR_AIMessageGoal msg = SCR_AIMessageGoal.Cast(m_Mailbox.CreateMessage(m_aiWorld.GetGoalMessageOfType(m_messageType)));
					
		if ( !msg )
		{
			Print("Unable to create valid message!");
			return ENodeResult.FAIL;
		}	
		
		msg.SetMessageParameters(this,GetRelatedActivity(owner));
		
		#ifdef AI_DEBUG
		SCR_MailboxComponent mailboxComp = SCR_MailboxComponent.Cast(m_Mailbox);
		if (mailboxComp)
			mailboxComp.DebugLogBroadcastMessage(msg, m_Receiver);
		#endif
		
		if (m_Mailbox.RequestBroadcast(msg, m_Receiver))
			return ENodeResult.SUCCESS;
		else
		{
			PrintFormat("Unable to send message from %1 to %2",owner,m_Receiver);
			return ENodeResult.FAIL;
    	};		
	}
	
	override protected string GetNodeMiddleText()
	{
		string result;
		result = "Message type: " + typename.EnumToString(EMessageType_Goal,m_messageType);
		if (m_bPrioritize)
			result += "\nPRIORITY";
		return result;	
	}	
	
	override string GetOnHoverDescription() 
	{ 
		return "Send Goal Message: Send messages for setting goals either of group or singular agent";	
	};
	
	protected SCR_AIActivityBase GetRelatedActivity(AIAgent owner)
	{
		if (SCR_AIGroup.Cast(owner))
		{
			SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(owner.FindComponent(SCR_AIGroupUtilityComponent));
			if (utility)
				return SCR_AIActivityBase.Cast(utility.GetCurrentAction());
		}	
		else
		{
			SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_AIUtilityComponent));	
			if (utility)
				return SCR_AIBehaviorBase.Cast(utility.GetCurrentAction()).m_RelatedGroupActivity;					
		}
		return null;
	}	
};




