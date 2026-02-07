class SCR_AISendMessage: AITaskScripted
{
	static const string PORT_RECEIVER = "Receiver";
	static const string PORT_STRING = "StringIn";
	static const string PORT_ENTITY = "EntityIn";
	static const string PORT_INTEGER = "IntegerIn";
	static const string PORT_VECTOR = "VectorIn";
	static const string PORT_BOOL = "BoolIn";
	static const string PORT_FLOAT = "FloatIn";	
	static const string PORT_TYPENAME = "TypenameIn";
	static const string PORT_SMARTACTION = "SmartActionComponent";
	static const string PORT_PRIORITIZE = "Prioritize";	
		
	AIAgent m_Receiver;	
	
	[Attribute("", UIWidgets.EditBox, "String value", "")]
	string m_string;
	
	[Attribute("0", UIWidgets.EditBox, "Integer value", "")]
	int m_integer;
	
	[Attribute("0", UIWidgets.EditBox, "Vector value", "")]
	vector m_vector;
		
	[Attribute("0", UIWidgets.CheckBox, "Bool value", "")]
	bool m_bool;
	
	
	protected AICommunicationComponent m_Mailbox;
	protected SCR_AIWorld m_aiWorld;
	
	// Sets up input variables, as array of strings
	protected static ref TStringArray s_aVarsIn = {
		PORT_RECEIVER,
		PORT_STRING,
		PORT_ENTITY,
		PORT_INTEGER,
		PORT_VECTOR,
		PORT_BOOL,
		PORT_FLOAT,
		PORT_TYPENAME,
		PORT_PRIORITIZE
	};
    override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
    override void OnInit(AIAgent owner)
	{
		m_Mailbox = owner.GetCommunicationComponent();
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (InitSendMessageInputs(owner))
			return ENodeResult.SUCCESS;
		
		return ENodeResult.FAIL;
	}
	
	protected bool InitSendMessageInputs(AIAgent owner)
	{
		if (! owner)
		{
			Print("No owner of the task send message! Something wrong!");
			return false;
		};
		if (!m_Mailbox)
		{
			Print("Owner of the task has no AICommunicationComponent!");
			return false;
		};
		if(!m_aiWorld)
		{
			m_aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
			if (!m_aiWorld)
			{
				Print("Cannot find AIWorld to read config of messages!");
				return false;
			}
		};
		GetVariableIn(PORT_RECEIVER,m_Receiver);
		if (! m_Receiver)
		{
			m_Receiver = owner;
		};
		return true;
	}	
	
	override bool VisibleInPalette()
	{
		return false;
	}
};




