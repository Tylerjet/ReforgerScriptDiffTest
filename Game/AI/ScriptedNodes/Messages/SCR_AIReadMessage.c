class SCR_AIReadMessage: AITaskScripted
{
	protected SCR_MailboxComponent m_Mailbox
	
    // Sets up input variables, as array of strings
   
	protected static ref TStringArray s_aVarsOut = {
		"SenderOut",
		"MessageTypeOut",
		"MessageOut"
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	protected void ClearVariables()
	{
		ClearVariable("SenderOut");
		ClearVariable("MessageTypeOut");
		ClearVariable("MessageOut");		
	}  
	
	override void OnInit (AIAgent owner)
	{
		m_Mailbox = SCR_MailboxComponent.Cast(owner.GetCommunicationComponent());
	} 
	
	override bool VisibleInPalette()
	{
		return false;
	}	
};





