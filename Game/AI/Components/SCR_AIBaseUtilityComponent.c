[ComponentEditorProps(category: "GameScripted/AI", description: "Component for utility AI system calculations")]
class SCR_AIBaseUtilityComponentClass: ScriptComponentClass
{
};

// TODO:
// - For handling orders, use enums

//------------------------------------------------------------------------------------------------
class SCR_AIBaseUtilityComponent : ScriptComponent
{
	ref array<ref SCR_AIActionBase> m_aActions = new ref array<ref SCR_AIActionBase>();
	ref SCR_AIActionBase m_CurrentAction;
	ref SCR_AIActionBase m_ExecutedAction;
	AIAgent m_OwnerAgent;
	
	// This can be set externally to request a breakpoint on next EvaluateBehavior
	#ifdef AI_DEBUG
	bool m_bEvaluationBreakpoint;
	#endif
	
	//--------------------------------------------------------------------------------------------
	SCR_AIActionBase EvaluateActions()
	{
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateActions");
		#endif
		
		float currentScore;
		float highScore = -float.MAX;
		int index = -1;
		for (int i = m_aActions.Count() - 1; i >= 0; i--)
		{
			SCR_AIActionBase action = m_aActions[i];
			
			// Evaluate anyway even though might be suspended
			currentScore = action.Evaluate();
			
			#ifdef AI_DEBUG
			string _strSuspended = string.Empty;
			if (action.m_bSuspended)
				_strSuspended = "(S) ";
			AddDebugMessage(string.Format("    %1%2 %3 %4", _strSuspended, currentScore.ToString(3, 1), typename.EnumToString(EAIActionType, action.m_eType), action.GetActionDebugInfo()));
			#endif
			
			// Ignore if suspended
			if (action.m_bSuspended)
				continue;
			
			if (currentScore > highScore)
			{
				highScore = currentScore;
				index = i;
			}
		}
		if (index < 0)
			return null;
		return m_aActions[index];
	}
	
	//--------------------------------------------------------------------------------------------
	void AddAction(SCR_AIActionBase action)
	{
		#ifdef AI_DEBUG
		string _debugMsg = string.Format("AddAction: %1", action.GetActionDebugInfo());
		AddDebugMessage(_debugMsg);
		AddDebugMessage(_debugMsg, EAIDebugMsgType.ACTION);
		#endif
		
		if (!action)
			return;
		
		if (action.m_bUniqueInActionQueue)
		{
			typename type = action.Type();
			for (int i = 0, len = m_aActions.Count(); i < len; i++)
			{
				if (m_aActions[i].Type() == type)
				{
					m_aActions[i].Fail();
					m_aActions[i] = action;
					return;
				}
			}
		}
		m_aActions.Insert(action);
	}
	
	//--------------------------------------------------------------------------------------------
	//! Return true when action was added
	bool AddActionIfMissing(SCR_AIActionBase action)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("AddActionIfMissing: %1", action.GetActionDebugInfo()));
		#endif
		
		if (!action)
			return false;
	
		typename type = action.Type();
		foreach (SCR_AIActionBase a : m_aActions)
		{
			if (a.Type() == type)
				return false;
		}
		m_aActions.Insert(action);
		return true;
	}
	
	//--------------------------------------------------------------------------------------------
	bool RemoveObsoleteActions()
	{		
		bool result = false;
		for (int i = m_aActions.Count() -1; i >= 0; i--)
		{
			SCR_AIActionBase actionToRemove = m_aActions[i];
			if (actionToRemove.m_eState == EAIActionState.COMPLETED || actionToRemove.m_eState == EAIActionState.FAILED)
			{
				#ifdef AI_DEBUG
				AddDebugMessage(string.Format("RemoveObsoleteActions: %1", m_aActions[i].GetActionDebugInfo()));
				#endif
				
				m_aActions.RemoveOrdered(i);
				result = true;
			}
		}
		return result;
	}
	
	//--------------------------------------------------------------------------------------------
	void SetStateAllActionsOfType(EAIActionType actionType,EAIActionState actionState)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetStateAllActionsOfType: %1 %2", typename.EnumToString(EAIActionType, actionType), typename.EnumToString(EAIActionState, actionState)));
		#endif
		
		for (int i = 0, len = m_aActions.Count(); i < len; i++)
		{
			if (m_aActions[i].m_eType == actionType)
			{
				switch (actionState)
				{
					case EAIActionState.COMPLETED :
					{
						m_aActions[i].Complete();
						break;
					}
					case EAIActionState.FAILED :
					{
						m_aActions[i].Fail();
						break;
					}
				}	
				if (m_aActions[i].m_bUniqueInActionQueue)
					break;
			}
		}		
	}	
	
	//--------------------------------------------------------------------------------------------
	void SetCurrentAction(SCR_AIActionBase actionToSet)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetCurrentAction: %1", actionToSet.GetActionDebugInfo()));
		#endif
		
		if (m_CurrentAction)
			m_CurrentAction.OnActionDeselected();
		
		m_CurrentAction = actionToSet;
		
		if (m_CurrentAction)
			m_CurrentAction.OnActionSelected();
	}
	
	//--------------------------------------------------------------------------------------------
	void SetExecutedAction(SCR_AIActionBase actionToSet)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetExecutedAction: %1", actionToSet.GetActionDebugInfo()));
		#endif
		
		m_ExecutedAction = actionToSet;
	}

	//--------------------------------------------------------------------------------------------	
	SCR_AIActionBase GetCurrentAction()
	{
		return m_CurrentAction;
	}
	
	//--------------------------------------------------------------------------------------------
	SCR_AIActionBase GetExecutedAction()
	{
		return m_ExecutedAction;
	}
	
	//--------------------------------------------------------------------------------------------
	bool HasActionOfType(typename actionType)
	{
		foreach (SCR_AIActionBase a : m_aActions)
		{
			if (a.Type() == actionType)
				return true;
		}
		
		return false;
	}
	
	//--------------------------------------------------------------------------------------------
	bool HasActionOfType(EAIActionType actionType)
	{
		foreach (SCR_AIActionBase a : m_aActions)
		{
			if (a.m_eType == actionType)
				return true;
		}
		
		return false;
	}
	
	//--------------------------------------------------------------------------------------------
	//! Finds action of exactly this type
	SCR_AIActionBase FindActionOfType(typename actionType)
	{
		foreach (SCR_AIActionBase a : m_aActions)
		{
			if (a.Type() == actionType)
				return a;
		}
		
		return null;
	}
	
	//--------------------------------------------------------------------------------------------
	//! Finds action of provided type or a type inherited from it
	SCR_AIActionBase FindActionOfInheritedType(typename actionType)
	{
		foreach (SCR_AIActionBase a : m_aActions)
		{
			if (a.Type().IsInherited(actionType))
				return a;
		}
		
		return null;
	}
	
	//--------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		m_OwnerAgent = AIAgent.Cast(owner);
	}
	
	//--------------------------------------------------------------------------------------------
	void SendMessage(AIMessage msg, AIAgent receiver = null)
	{
		AICommunicationComponent commsComp = m_OwnerAgent.GetCommunicationComponent();
		if (!commsComp)
			return;
		
		commsComp.RequestBroadcast(msg, receiver);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEvaluationBreakpoint()
	{
		#ifdef AI_DEBUG
		m_bEvaluationBreakpoint = true;
		#endif
	}
	
	#ifdef AI_DEBUG
	//--------------------------------------------------------------------------------------------
	protected void AddDebugMessage(string str, EAIDebugMsgType messageType = EAIDebugMsgType.UTILITY)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(GetOwner().FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(str, msgType: messageType);
	}
	#endif
};