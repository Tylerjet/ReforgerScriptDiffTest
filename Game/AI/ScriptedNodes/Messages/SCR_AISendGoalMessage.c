class SCR_AISendGoalMessage: SCR_AISendMessageGeneric
{
	[Attribute("0", UIWidgets.ComboBox, "Message type", "", ParamEnumArray.FromEnum(EMessageType_Goal) )]
	private EMessageType_Goal m_messageType;
	
	[Attribute("0", UIWidgets.CheckBox, "Priority level")]
	int m_fPriorityLevel;
	
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
			Print("Unable to create valid message!", LogLevel.ERROR);
			return ENodeResult.FAIL;
		}	
		
		msg.SetMessageParameters(this,GetRelatedActivity(owner));
		
		return SendMessage(owner, msg);
	}
	
	override protected string GetNodeMiddleText()
	{
		string result;
		result = "Message type: " + typename.EnumToString(EMessageType_Goal,m_messageType);
		if (m_fPriorityLevel > 1)
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
