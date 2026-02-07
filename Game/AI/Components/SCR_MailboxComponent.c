[ComponentEditorProps(category: "GameScripted/AIComponent", description: "Scripted m_Mailbox component", icon: HYBRID_COMPONENT_ICON)]
class SCR_MailboxComponentClass: AICommunicationComponentClass
{
};

class SCR_MailboxComponent : AICommunicationComponent
{
	ref array<ref SCR_AIMessageBase> m_aMessages = {};
	ref array<ref AIDangerEvent> m_aDangerEvents = {};
	
	override void OnReceived(AIMessage pMessage)
	{
		if (pMessage)
		{
			#ifdef AI_DEBUG
			DebugLogOnReceived(pMessage);
			#endif
			
			ref AIDangerEvent dangerEvent = AIDangerEvent.Cast(pMessage);
			if (dangerEvent)
				m_aDangerEvents.Insert(dangerEvent);
			else
			{
				//PrintFormat("Message %3 reading from %1 to %2", pMessage.GetSender(), pMessage.GetReceiver(), pMessage);
				SCR_AIMessageBase msgBase = SCR_AIMessageBase.Cast(pMessage);
				if (msgBase)
					m_aMessages.Insert(msgBase);
			}
		}
	}
	
	#ifdef AI_MAILBOX_OVERFLOW_DETECTION
	protected int m_iMailboxOverflowCount = 0;
	#endif
	
	// Returns first message in the queue
	SCR_AIMessageBase ReadMessage()
	{
		#ifdef AI_MAILBOX_OVERFLOW_DETECTION
		if (m_aMessages.Count() > 100 && m_iMailboxOverflowCount < 5)
		{
			LogMailboxStatistics(LogLevel.WARNING);
			m_iMailboxOverflowCount++;
		}
		#endif
		
		if (m_aMessages.IsEmpty())
			return null;
		else
		{
			SCR_AIMessageBase message = m_aMessages[0];
			m_aMessages.RemoveOrdered(0);
			return message;
		}
	}
	
	void LogMailboxStatistics(LogLevel logLevel = LogLevel.NORMAL)
	{
		map<typename, int> msgTypeCount = new map<typename, int>();
		foreach (SCR_AIMessageBase msg : m_aMessages)
		{
			typename msgFinalType = msg.Type();
			int nMsgsOfType = msgTypeCount.Get(msgFinalType);
			nMsgsOfType++;
			msgTypeCount.Set(msgFinalType, nMsgsOfType);
		}
		
		Print(string.Format("SCR_MailboxComponent statistics: count: %1, %2, %3", m_aMessages.Count(), this, GetAIAgent()), logLevel);
		foreach (typename t, int count : msgTypeCount)
		{
			Print(string.Format("  %1: %2", t, count), logLevel);
		}
		Print(string.Format("  Total: %1", m_aMessages.Count()), logLevel);
	}
	
	AIDangerEvent ReadDangerEvent()
	{
		if (m_aDangerEvents.IsEmpty())
			return null;
		else
		{
			AIDangerEvent dangerEvent = m_aDangerEvents[0];
			m_aDangerEvents.RemoveOrdered(0);
			return dangerEvent;
		}
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
	
	void DebugGetInboxSize(out int nMessages, out int nDangerEvents)
	{
		nMessages = m_aMessages.Count();
		nDangerEvents = m_aDangerEvents.Count();
	}
	
	#ifdef AI_DEBUG
	void DebugLogBroadcastMessage(AIMessage message)
	{
		AIAgent agent = GetAIAgent();
		SCR_AIMessageBase msgBase = SCR_AIMessageBase.Cast(message);
		
		string debugTextBasic = string.Format("----> SendMessage: %1 --> %2:\n", agent, message.GetReceiver());
		
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