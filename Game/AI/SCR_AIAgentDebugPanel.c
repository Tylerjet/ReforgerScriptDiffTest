/*!
Class which represents debug panel which can be shown for each AI unit or group.
*/
class SCR_AIAgentDebugPanel : Managed
{
	AIAgent m_Agent;
	SCR_AIGroup m_Group;
	IEntity m_ControlledEnt;
	protected bool m_bRequestClose = false;
	
	protected string m_sWindowTitle;
	
	void SCR_AIAgentDebugPanel(AIAgent agent)
	{
		m_Agent = agent;
		m_Group = SCR_AIGroup.Cast(agent);
		m_ControlledEnt = agent.GetControlledEntity();
	}
	
	bool Update(float timeSlice)
	{
		if (m_sWindowTitle.IsEmpty())
			m_sWindowTitle = string.Format("DbgPnl %1", m_Agent);
		
		DbgUI.Begin(m_sWindowTitle);
		
		SCR_AIInfoBaseComponent baseInfoComp;
		SCR_AIInfoComponent infoComp;			// For units
		SCR_AIGroupInfoComponent groupInfoComp;	// For groups
		
		if (!m_Agent && !m_Group)
		{
			DbgUI.Text("Agent is null!");
		}
		else
		{
			// Agent name
			string agentName = GetAgentDebugName();
			DbgUI.Text(agentName);
			
			
			baseInfoComp = SCR_AIInfoBaseComponent.Cast(m_Agent.FindComponent(SCR_AIInfoBaseComponent));
			infoComp = SCR_AIInfoComponent.Cast(baseInfoComp);
			groupInfoComp = SCR_AIGroupInfoComponent.Cast(baseInfoComp);
			
			SCR_AIBaseUtilityComponent utilityComp = SCR_AIBaseUtilityComponent.Cast(m_Agent.FindComponent(SCR_AIBaseUtilityComponent));
			SCR_AIUtilityComponent unitUtilityComp = SCR_AIUtilityComponent.Cast(utilityComp);
			
			SCR_AICombatComponent combatComp;
			
			if (m_ControlledEnt)
				combatComp = SCR_AICombatComponent.Cast(m_ControlledEnt.FindComponent(SCR_AICombatComponent));
			
			if (infoComp)
			{
				string strUnitState = string.Format("Unit State: %1", EnumFlagsToString(EUnitState, infoComp.GetUnitStates()));
				string strUnitRoles = string.Format("Unit Roles: %1", EnumFlagsToString(EUnitRole, infoComp.GetRoles()));
				DbgUI.Text(strUnitState);
				DbgUI.Text(strUnitRoles);
			}
			else if (groupInfoComp)
			{
				DbgUI.Text(string.Format("Control Mode: %1", typename.EnumToString(EGroupControlMode, groupInfoComp.GetGroupControlMode())));
			}
			
			if (combatComp)
			{
				EAICombatType combatType = combatComp.GetCombatType();
				BaseTarget currentEnemy = combatComp.GetCurrentEnemy();
				DbgUI.Text(string.Format("Combat Type: %1", typename.EnumToString(EAICombatType, combatType)));
				DbgUI.Text(string.Format("Enemy: %1", currentEnemy.ToString()));
			}
			
			// Utility component actions
			if (utilityComp)
			{
				// Threat
				if (unitUtilityComp)
				{
					EAIThreatState threatState = unitUtilityComp.m_ThreatSystem.GetState();
					DbgUI.Text(string.Format("Threat: %1 %2", unitUtilityComp.m_ThreatSystem.GetThreatMeasure().ToString(6, 3), typename.EnumToString(EAIThreatState, threatState)));
				}
				
				SCR_AIActionBase currentAction = utilityComp.m_CurrentAction;
				if (!currentAction)
					DbgUI.Text("Current Action: null");
				else
				{
					DbgUI.Text("Current Action:");
					float actionPriority = currentAction.Evaluate();
					string currentActionStr = string.Format("  > %1 %2", actionPriority.ToString(3, 1), typename.EnumToString(EAIActionType, currentAction.m_eType));
					DbgUI.Text(currentActionStr);
				}
				
				DbgUI.Text("Actions:");
				foreach (SCR_AIActionBase action : utilityComp.m_aActions)
				{
					float actionPriority = action.Evaluate();
					string actionStr = string.Format("    %1 %2", actionPriority.ToString(3, 1), typename.EnumToString(EAIActionType, action.m_eType));
					DbgUI.Text(actionStr);
				}
			}
			
			// Dump debug messages button
			int dumpDbgMsgsDuration;
			EAIDebugMsgType dbgMsgType;
			DbgUI.Text("Dump Debug Msgs:");
			
			DbgUI.Combo("Msg Age", dumpDbgMsgsDuration, {"All", "120 sec", "30 sec", "5 sec"});
			
			int dbgMsgTypeSelection;
			array<string> dbgMsgTypeNames = {};
			dbgMsgTypeNames.Copy(SCR_AIDebugMessage.s_aAiDebugMsgTypeLabels);
			dbgMsgTypeNames.InsertAt("All", 0);
			DbgUI.Combo("Msg Type", dbgMsgTypeSelection, dbgMsgTypeNames);
			dbgMsgType = dbgMsgTypeSelection - 1;
			bool useDbgMsgTypeFilter = dbgMsgTypeSelection != 0;
			
			bool dumpMsgs = DbgUI.Button("Dump to log file");
			
			if (dumpMsgs)
			{
				int msgAgeThresholdMs;
				switch (dumpDbgMsgsDuration)
				{
					case 0: msgAgeThresholdMs = -1; break;
					case 1: msgAgeThresholdMs = 120*1000; break;
					case 2: msgAgeThresholdMs = 30*1000; break;
					case 3: msgAgeThresholdMs = 5*1000; break;
				}
				#ifdef AI_DEBUG
				baseInfoComp.DumpDebugMessages(useTypeFilter: useDbgMsgTypeFilter, msgTypeFilter: dbgMsgType, ageThresholdMs: msgAgeThresholdMs);
				#endif
			}
			
			// Request breakpoint button
			DbgUI.Text("Breakpoint At:");
			bool rqBreak = DbgUI.Button("Utility");
			if (rqBreak && utilityComp)
			{
				utilityComp.SetEvaluationBreakpoint();
			}
		}
		
		// Close button
		m_bRequestClose = DbgUI.Button("Close");
		
		// Locate button
		if (m_Agent && !m_Group)
		{
			DbgUI.SameLine();
			bool locate = DbgUI.Button("Locate");
			if (locate)
			{
				array<string> locateTexts = {"I am here!", "Look at me!", "Hey! Look here!", "Here I am!"};
				IEntity ent;
				if (m_Agent)
					ent = m_Agent.GetControlledEntity();
				else
					ent = m_Group;
				SCR_AIDebugVisualization.VisualizeMessage(ent, locateTexts.GetRandomElement(), EAIDebugCategory.NONE, 0.75, Color.Red, fontSize: 20, ignoreCategory: true);
			}
		}
		
		// Kill button
		if (m_Agent && !m_Group)
		{
			DbgUI.SameLine();
			bool forceDeath = DbgUI.Button("Kill");
			if (forceDeath)
			{
				CharacterControllerComponent cntrlComp = CharacterControllerComponent.Cast(m_ControlledEnt.FindComponent(CharacterControllerComponent));
				if (cntrlComp)
					cntrlComp.ForceDeath();
			}
		}
			
		DbgUI.End();
		
		return m_bRequestClose;
	}
	
	//! Returns agent name based on faction and callsign
	string GetAgentDebugName()
	{
		if (m_Group)
		{
			// Group
			return m_Group.GetCallsignSingleString();
		}
		else
		{
			// Unit
			SCR_CallsignCharacterComponent callsignComp = SCR_CallsignCharacterComponent.Cast(m_Agent.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));
		
			FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(m_Agent.GetControlledEntity().FindComponent(FactionAffiliationComponent));
			
			string str;
			
			if (factionComp)
			{
				string faction = factionComp.GetAffiliatedFaction().GetFactionKey();
				str = str + string.Format("[%1] ", faction);
			}
			
			if (callsignComp)
			{			
				string company, platoon, squad, character, format;
				bool setCallsign = callsignComp.GetCallsignNames(company, platoon, squad, character, format);
				if (setCallsign)
				{
					string callsign = WidgetManager.Translate(format, company, platoon, squad, character);
					str = str + string.Format(" %1", callsign);
				}
			}
			return str;
		}
	}
	
	//! Formats enum flags to string
	static string EnumFlagsToString(typename t, int value)
	{
		int tVarCount = t.GetVariableCount();
		string strOut;
		for (int i = 0; i < tVarCount; i++)
		{
			int flag;
			t.GetVariableValue(null, i, flag);
			if (value & flag)
				strOut = strOut + string.Format("%1 ", typename.EnumToString(t, flag));
		}
		return strOut;
	}
};