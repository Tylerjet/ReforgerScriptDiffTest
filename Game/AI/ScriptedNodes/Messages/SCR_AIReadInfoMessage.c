class SCR_AIReadInfoMessage: SCR_AIReadMessage
{
	protected ref SCR_AIMessageInfo m_Message;
	// Make scripted node visible or hidden in nodes palette
    override bool VisibleInPalette()
    {
        return true;
    }

	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (! owner)
			Print("No owner of the task read m_Message! Something wrong!");
		
		if ( m_Mailbox )
		{
			ClearVariables();
			m_Message = SCR_AIMessageInfo.Cast(m_Mailbox.ReadMessage(SCR_AIMessageInfo));
			if (m_Message)
			{
				SetVariableOut("SenderOut",m_Message.GetSender());
				SetVariableOut("MessageTypeOut",m_Message.m_MessageType);
				SetVariableOut("MessageOut",m_Message);
				return ENodeResult.SUCCESS;
			}
			else 
				return ENodeResult.FAIL;		
		}
		else
			return ENodeResult.FAIL;		
    }
	
	void ~SCR_AIReadInfoMessage()
	{
		m_Message = null;	
	}	
};




