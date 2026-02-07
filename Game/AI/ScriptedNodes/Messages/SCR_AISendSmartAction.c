class SCR_AISendSmartAction: SCR_AISendGoalMessage
{
	// Make scripted node visible or hidden in nodes palette
    override bool VisibleInPalette()
    {
        return true;
    }

	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (!InitSendMessageInputs(owner))
			return ENodeResult.FAIL;	
		
		SCR_AIMessageGoal msg = SCR_AIMessageGoal.Cast(m_Mailbox.CreateMessage(m_aiWorld.GetGoalMessageOfType(EMessageType_Goal.PERFORM_ACTION)));
					
		if ( !msg )
		{
			Print("Unable to create valid message!");
			return ENodeResult.FAIL;		
		}	
		
		msg.SetMessageParameters(this,GetRelatedActivity(owner));		
		
		if (m_Mailbox.RequestBroadcast(msg, m_Receiver))
			return ENodeResult.SUCCESS;
		else
		{
			PrintFormat("Unable to send message from %1 to %2",owner,m_Receiver);
			return ENodeResult.FAIL;
    	};		
	}
	
	protected static ref TStringArray s_aVarsIn2 = {
		PORT_RECEIVER,
		PORT_SMARTACTION
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn2;
    }
};




