[ComponentEditorProps(category: "GameScripted/AIComponent", description: "Scripted m_Mailbox component", color: "0 0 255 255", icon: HYBRID_COMPONENT_ICON)]
class SCR_MailboxComponentClass: AICommunicationComponentClass
{
};

class SCR_MailboxComponent : AICommunicationComponent
{
	ref array<ref SCR_AIMessageBase> m_aMessages = new ref array<ref SCR_AIMessageBase>;
	ref array<ref AIDangerEvent> m_aDangerEvents = new ref array<ref AIDangerEvent>;
	
	override void OnReceived(AIMessage pMessage)
	{
		if (pMessage)
		{
			#ifdef AI_DEBUG
			DebugLogOnReceived(pMessage);
			#endif
			
			ref AIDangerEvent dangerEvent = AIDangerEvent.Cast(pMessage);
			if (dangerEvent)
			{
				m_aDangerEvents.Insert(dangerEvent);
			}
			else
			{
				//PrintFormat("Message %3 reading from %1 to %2", pMessage.GetSender(), pMessage.GetReceiver(), pMessage);
				ref SCR_AIMessageBase msg = SCR_AIMessageBase.Cast(pMessage);
				if (msg)
					m_aMessages.Insert(msg);
			}
		}
	}
	
	SCR_AIMessageBase ReadMessage(typename TypeName)
	{
		for (int index = 0, length = m_aMessages.Count(); index < length; index++)
		{
			if (m_aMessages[index].IsInherited(TypeName))
			{ 
				SCR_AIMessageBase message = m_aMessages[index];				
				m_aMessages.RemoveOrdered(index);
				return message;
			}	
		};
		return null;		
	}
	
	AIDangerEvent ReadDangerEvent()
	{
		for (int index = 0, length = m_aDangerEvents.Count(); index < length; index++)
		{
			AIDangerEvent dangerEvent = m_aDangerEvents[index];
			m_aDangerEvents.RemoveOrdered(index);
			return dangerEvent;
		};
		return null;
	}
	
	bool IsCommunicationEnabled()
	{
		return IsActive();
	}
	
	void ~SCR_MailboxComponent()
	{
		if (m_aMessages) m_aMessages.Clear();
		m_aMessages = null;
		
		if (m_aDangerEvents) m_aDangerEvents.Clear();
		m_aDangerEvents = null; 
	}
	
	//-------------------------------------------------------------------------------------------------
	// Debugging
	
	#ifdef AI_DEBUG
	void DebugLogBroadcastMessage(AIMessage message, AIAgent receiver)
	{
		AIAgent agent = GetAIAgent();
		SCR_AIMessageBase msgBase = SCR_AIMessageBase.Cast(message);
		
		string debugTextBasic = string.Format("----> SendMessage: %1 --> %2:\n", agent, receiver);
		
		string debugText;
		if (msgBase)
			debugText = debugTextBasic + msgBase.GetDebugText();
		else
			debugText = debugTextBasic + string.Format("\t%1", message);
		
		SCR_AIDebug.DebugLog(debugText);
		AddDebugMessage(debugText);
	}
	
	void DebugLogOnReceived(AIMessage message)
	{
		AIAgent agent = GetAIAgent();
		
		AIAgent sender = message.GetSender();
		
		string debugTextBasic = string.Format("<---- RecvMessage: %1 <-- %2\n", agent, sender);
		
		SCR_AIMessageBase msgBase = SCR_AIMessageBase.Cast(message);
		
		string debugText;
		if (msgBase)
			debugText = debugTextBasic + msgBase.GetDebugText();
		else
			debugText = debugTextBasic + string.Format("\t%1", message);
		
		SCR_AIDebug.DebugLog(debugText);
		AddDebugMessage(debugText);
	}
	
	//--------------------------------------------------------------------------------------------
	protected void AddDebugMessage(string str)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(GetAIAgent().FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(str, msgType: EAIDebugMsgType.MAILBOX);
	}
	#endif
};