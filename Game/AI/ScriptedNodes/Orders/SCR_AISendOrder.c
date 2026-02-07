class SCR_AISendOrder: SCR_AISendMessage
{
	static const string ORDER_TYPE = "OrderTypeIn";
	static const string ORDER_VALUE = "OrderValueIn";
	static const string ORDER_DEBUG_TEXT = "DebugText";	
	
	[Attribute("0", UIWidgets.ComboBox, "Order type", "", ParamEnumArray.FromEnum(EOrderType_Character))]
	private EOrderType_Character m_orderType;
	
	private int m_iValue // value of scripted order
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (!InitSendMessageInputs(owner) || !m_Mailbox)
			return ENodeResult.FAIL;
		
		EOrderType_Character orderType;
	
		if (!GetVariableIn(ORDER_TYPE, orderType))
			orderType = m_orderType;
		
		SCR_AIOrderBase msg = SCR_AIOrderBase.Cast(m_Mailbox.CreateMessage(m_aiWorld.GetOrderMessageOfType(orderType)));
						
		if ( !msg )
		{
			Print("Unable to create valid message!");
			return ENodeResult.FAIL;
		}	
		
		msg.SetOrderType(EAIOrderType.AIOrder_Custom);
		msg.SetOrderParameters(this);
				
		if (m_Mailbox.RequestBroadcast(msg, m_Receiver))
			return ENodeResult.SUCCESS;
		else
		{
			PrintFormat("Unable to send message from %1 to %2",owner,m_Receiver);
			return ENodeResult.FAIL;
    	};		
	}
	
	protected static ref TStringArray s_aVarsIn2 = {
		ORDER_TYPE,
		PORT_RECEIVER,
		ORDER_VALUE,
		ORDER_DEBUG_TEXT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn2;
    }
		
	
	override protected string GetNodeMiddleText()
	{
		return "Order type: " + typename.EnumToString(EOrderType_Character, m_orderType) + "\n" + "Order value: " + m_string;	
	}
	
	override string GetOnHoverDescription() 
	{ 
		return "Send Order : Send scripted orders to agents";	
	};
	
	override bool VisibleInPalette()
	{
		return true;
	}  
	
};
