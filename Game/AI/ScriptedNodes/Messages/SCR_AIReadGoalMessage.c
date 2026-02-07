class SCR_AIReadGoalMessage: SCR_AIReadMessage
{
	protected ref SCR_AIMessageGoal m_Message;
	// Make scripted node visible or hidden in nodes palette
    override bool VisibleInPalette()
    {
        return true;
    }

	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if ( m_Mailbox )
		{
			ClearVariables();
			m_Message = SCR_AIMessageGoal.Cast(m_Mailbox.ReadMessage(SCR_AIMessageGoal));
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
	
	void ~SCR_AIReadGoalMessage()
	{
		m_Message = null;	
	}
};




