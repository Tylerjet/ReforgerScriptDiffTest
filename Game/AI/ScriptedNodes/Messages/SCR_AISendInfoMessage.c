class SCR_AISendInfoMessage: SCR_AISendMessageGeneric
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
			Print("Unable to create valid message!", LogLevel.ERROR);
			return ENodeResult.FAIL;
		}

		msg.SetMessageParameters(this);

		return SendMessage(owner, msg);
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

class SCR_AISendMessage_ActionFailed : SCR_AISendMessageBase
{
	[Attribute("0", UIWidgets.ComboBox, "Related action type", "", ParamEnumArray.FromEnum(EAIActionType) )]
	EAIActionType m_eActionType;

	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!InitSendMessageInputs(owner))
			return false;

		SCR_AIMessage_ActionFailed msg = new SCR_AIMessage_ActionFailed();
		msg.SetMessageParameters(this);
		msg.m_eActionType = m_eActionType;

		return SendMessage(owner, msg);
	}

	override bool VisibleInPalette()
	{
		return true;
	}
};

class SCR_AISendMessage_HealFailed : SCR_AISendMessageBase
{
	const static string PORT_TARGET_ENTITY = "TargetEntity";

	protected static const ref TStringArray s_aVarsIn2 = {PORT_RECEIVER, PORT_TARGET_ENTITY};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn2;
	}

	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!InitSendMessageInputs(owner))
			return false;

		SCR_AIMessage_HealFailed msg = new SCR_AIMessage_HealFailed();
		msg.SetMessageParameters(this);

		IEntity targetEntity;
		GetVariableIn(PORT_TARGET_ENTITY, targetEntity);

		msg.m_TargetEntity = targetEntity;

		return SendMessage(owner, msg);
	}

	override bool VisibleInPalette()
	{
		return true;
	}
};
