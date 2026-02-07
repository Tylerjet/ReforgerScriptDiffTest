class SCR_AISendInfoMessage: SCR_AISendMessage
{
	[Attribute("0", UIWidgets.ComboBox, "Message type", "", ParamEnumArray.FromEnum(EMessageType_Info) )]
	private EMessageType_Info m_messageType;
	
	// Make scripted node visible or hidden in nodes palette
    override bool VisibleInPalette()
    {
        return true;
    }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (!InitSendMessageInputs(owner))
			return ENodeResult.FAIL;
		
		SCR_AIMessageBase msg = SCR_AIMessageBase.Cast(m_Mailbox.CreateMessage(m_aiWorld.GetInfoMessageOfType(m_messageType)));
		if ( !msg )
		{
			Print("Unable to create valid message!");
			return ENodeResult.FAIL;
		}	
		
		msg.SetMessageParameters(this);
		
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
		return "Message type: " + typename.EnumToString(EMessageType_Info,m_messageType);
	}
	
	override string GetOnHoverDescription() 
	{ 
		return "Send Info Message: Send messages for information to the group";
	};
};




