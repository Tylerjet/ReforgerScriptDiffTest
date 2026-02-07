/*!
Class which represents debug panel which can be shown for each AI unit or group.
*/
class SCR_AIAgentDebugPanel : Managed
{
	AIAgent m_Agent;
	SCR_AIGroup m_Group;
	IEntity m_Entity;
	protected bool m_bRequestClose = false;
	
	protected string m_sWindowTitle;
	
	protected bool m_bShowPerception;
	
	// The debug panel can be created for non-AI as well, for instance for vehicles.
	void SCR_AIAgentDebugPanel(AIAgent agent, IEntity entity)
	{
		m_Agent = agent;
		m_Group = SCR_AIGroup.Cast(agent);
		m_Entity = entity;
	}
	
	bool Update(float timeSlice)
	{
		Class objForTitle;
		if (m_Agent)
			objForTitle = m_Agent;
		else if (m_Entity)
			objForTitle = m_Entity;
		if (m_sWindowTitle.IsEmpty())
			m_sWindowTitle = string.Format("DbgPnl %1", objForTitle);
		
		DbgUI.Begin(m_sWindowTitle);
		
		SCR_AIInfoBaseComponent baseInfoComp;
		SCR_AIInfoComponent infoComp;			// For units
		SCR_AIGroupInfoComponent groupInfoComp;	// For groups
		SCR_MailboxComponent mailboxComp;
		PerceptionComponent perception;
		PerceivableComponent perceivable;
		SCR_AIBaseUtilityComponent utilityComp;
		SCR_AIUtilityComponent unitUtilityComp;
		SCR_AICombatComponent combatComp;
		
		if (!m_Agent && !m_Group && !m_Entity)
		{
			DbgUI.Text("The target doesn't exist!");
		}
		else
		{
			// Agent name
			string agentName = GetAgentDebugName();
			DbgUI.Text(agentName);
			
			
			if (m_Agent)
			{
				baseInfoComp = SCR_AIInfoBaseComponent.Cast(m_Agent.FindComponent(SCR_AIInfoBaseComponent));
				infoComp = SCR_AIInfoComponent.Cast(baseInfoComp);
				groupInfoComp = SCR_AIGroupInfoComponent.Cast(baseInfoComp);
				mailboxComp = SCR_MailboxComponent.Cast(m_Agent.FindComponent(SCR_MailboxComponent));
				utilityComp = SCR_AIBaseUtilityComponent.Cast(m_Agent.FindComponent(SCR_AIBaseUtilityComponent));
				if (utilityComp)
				{
					unitUtilityComp = SCR_AIUtilityComponent.Cast(utilityComp);
				}
			}
				
			
			if (m_Entity)
			{
				combatComp = SCR_AICombatComponent.Cast(m_Entity.FindComponent(SCR_AICombatComponent));
				perception = PerceptionComponent.Cast(m_Entity.FindComponent(PerceptionComponent));
				perceivable = PerceivableComponent.Cast(m_Entity.FindComponent(PerceivableComponent));
			}
			
			if (m_Agent)
				DbgUI.Text(string.Format("LOD: %1", m_Agent.GetLOD()));
			
			if (infoComp)
			{
				string strUnitState = string.Format("Unit State: %1", EnumFlagsToString(EUnitState, infoComp.GetUnitStates()));
				string strUnitRoles = string.Format("Unit Roles: %1", EnumFlagsToString(EUnitRole, infoComp.GetRoles()));
				string strUnitBusy = string.Format("Unit Busy:  %1", typename.EnumToString(EUnitAIState, infoComp.GetAIState()));
				DbgUI.Text(strUnitState);
				DbgUI.Text(strUnitRoles);
				DbgUI.Text(strUnitBusy);
			}
			else if (groupInfoComp)
			{
				DbgUI.Text(string.Format("Control Mode: %1", typename.EnumToString(EGroupControlMode, groupInfoComp.GetGroupControlMode())));
			}
			
			if (combatComp)
			{
				EAICombatType combatType = combatComp.GetCombatType();
				BaseTarget currentEnemy = combatComp.GetCurrentTarget();
				DbgUI.Text(string.Format("Combat Type: %1", typename.EnumToString(EAICombatType, combatType)));
				DbgUI.Text(string.Format("Enemy: %1", currentEnemy.ToString()));
			}
			
			if (mailboxComp)
			{
				int nRxMessages;
				int nRxDangers;
				mailboxComp.DebugGetInboxSize(nRxMessages, nRxDangers);
				DbgUI.Text(string.Format("Mailbox Rx Queue: %1, %2", nRxMessages, nRxDangers));
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
					float actionPriority = currentAction.Evaluate() + currentAction.EvaluatePriorityLevel();
					string currentActionStr = string.Format("  > %1 %2", actionPriority.ToString(5, 1), currentAction.Type().ToString());
					DbgUI.Text(currentActionStr);
				}
				
				DbgUI.Text("Actions:");
				foreach (int i, SCR_AIActionBase action : utilityComp.m_aActions)
				{
					float actionPriority = action.Evaluate() + action.EvaluatePriorityLevel();
					string strSuspended = string.Empty;
					if (action.m_bSuspended)
						strSuspended = "(S) ";
					string actionStr = string.Format("    %1 %2%3 %4", i, strSuspended, actionPriority.ToString(5, 1), action.Type().ToString());
					DbgUI.Text(actionStr);
				}
			}
			
			// Dump debug messages button
			if (baseInfoComp)
			{
				int dumpDbgMsgsDuration;
				EAIDebugMsgType dbgMsgType;
				DbgUI.Text("Dump Debug Messages:");
				
				DbgUI.Combo("Age", dumpDbgMsgsDuration, {"All", "120 sec", "30 sec", "5 sec"});
				
				int dbgMsgTypeSelection;
				array<string> dbgMsgTypeNames = {};
				dbgMsgTypeNames.Copy(SCR_AIDebugMessage.s_aAiDebugMsgTypeLabels);
				dbgMsgTypeNames.InsertAt("All", 0);
				DbgUI.SameLine();
				DbgUI.Combo("Type", dbgMsgTypeSelection, dbgMsgTypeNames);
				dbgMsgType = dbgMsgTypeSelection - 1;
				bool useDbgMsgTypeFilter = dbgMsgTypeSelection != 0;
				
				DbgUI.SameLine();
				bool dumpMsgs = DbgUI.Button("Dump");
				
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
			}
			
			// Request breakpoint button
			if (utilityComp)
			{
				DbgUI.Text("Breakpoint At:");
				DbgUI.SameLine();
				bool rqBreak = DbgUI.Button("Utility Comp.");
				if (rqBreak && utilityComp)
				{
					utilityComp.SetEvaluationBreakpoint();
				}
			}
			
			// Show perceivable component
			if (perceivable)
			{
				bool showPerceivable;
				DbgUI.Check("Show Perceivable", showPerceivable);
				if (showPerceivable)
				{
					ShowPerceivableComponent(perceivable);
				}
			}
			
			// Show perception
			if (perception && combatComp)
			{
				DbgUI.Check("Show Targets", m_bShowPerception);
				if (m_bShowPerception)
				{
					ShowPerceptionEnemies(m_Entity, perception, combatComp);
				}
			}
		}
		
		// Close button
		m_bRequestClose = DbgUI.Button("Close");
		
		// Locate button
		DbgUI.SameLine();
		bool locate = DbgUI.Button("Locate");
		if (locate)
		{
			array<string> locateTexts = {"I am here!", "Look at me!", "Hey! Look here!", "Here I am!"};
			IEntity ent;
			if (m_Agent)
				ent = m_Agent.GetControlledEntity();
			else if (m_Group)
				ent = m_Group;
			else
				ent = m_Entity;
			SCR_AIDebugVisualization.VisualizeMessage(ent, locateTexts.GetRandomElement(), EAIDebugCategory.NONE, 0.75, Color.Red, fontSize: 20, ignoreCategory: true);
		}
		
		// Kill button
		if (m_Agent && !m_Group)
		{
			DbgUI.SameLine();
			bool forceDeath = DbgUI.Button("Kill");
			if (forceDeath)
			{
				CharacterControllerComponent cntrlComp = CharacterControllerComponent.Cast(m_Entity.FindComponent(CharacterControllerComponent));
				if (cntrlComp)
					cntrlComp.ForceDeath();
			}
		}
			
		DbgUI.End();
		
		return m_bRequestClose;
	}
	
	//! Lists enemies from perception component
	void ShowPerceptionEnemies(IEntity myEntity, PerceptionComponent perception, SCR_AICombatComponent combatComponent)
	{
		vector myPos = myEntity.GetOrigin();
		
		array<ETargetCategory> targetCategories = {
			ETargetCategory.UNKNOWN,
			ETargetCategory.ENEMY,
			ETargetCategory.DETECTED,
			//ETargetCategory.FRIENDLY,
			//ETargetCategory.FACTIONLESS,
			ETargetCategory.STATIC,
		};
		
		FactionAffiliationComponent myFactionComp = FactionAffiliationComponent.Cast(myEntity.FindComponent(FactionAffiliationComponent));
		Faction myFaction = myFactionComp.GetAffiliatedFaction();
		
		BaseTarget selectedTarget = combatComponent.GetCurrentTarget();
		
		DbgUI.Text("[ID Category TimeSinceSeen Dngr Type Exp Detect Ident]");
		
		array<BaseTarget> targets = {};
		int targetId = 0;
		foreach (ETargetCategory targetCategory : targetCategories)
		{
			targets.Clear();
			perception.GetTargetsList(targets, targetCategory);
			foreach (int i, BaseTarget baseTarget : targets)
			{
				IEntity targetEntity = baseTarget.GetTargetEntity();
				if (!targetEntity)
					continue;
				
				// Don't list target if it has same faction as we do
				FactionAffiliationComponent targetFactionComp = FactionAffiliationComponent.Cast(targetEntity.FindComponent(FactionAffiliationComponent));
				if (targetFactionComp)
				{
					if (targetFactionComp.GetAffiliatedFaction() == myFaction)
						continue;
				}
				
				EntityPrefabData prefabData = targetEntity.GetPrefabData();
				ResourceName prefabName = prefabData.GetPrefabName();
				
				/*
				array<IEntity> __entities = {};
				array<ResourceName> __prefabNames = {};
				
				Print(string.Format("Target: %1", targetEntity));
				IEntity __parent = targetEntity;
				while (__parent)
				{
					__entities.Insert(__parent);
					ResourceName __prefabName = __parent.GetPrefabData().GetPrefabName();
					__prefabNames.Insert(__prefabName);
					
					Print(string.Format("  %1 %2", __parent, __prefabName));
					
					__parent = __parent.GetParent();
				}
				
				Print(" ");
				*/
				
				string strTimeSinceSeen = baseTarget.GetTimeSinceSeen().ToString(5, 2);
				string strEndangering;
				if (baseTarget.IsEndangering())
					strEndangering = "DNGR";
				else
					strEndangering = "    ";
				
				array<string> substrings = {};
				substrings.Clear();
				string strPrefabName = string.Empty;
				if (!prefabName.IsEmpty())
				{
					prefabName.Split("/", substrings, true);
					if (!substrings.IsEmpty())
						strPrefabName = substrings[substrings.Count()-1];
				}
				
				string strSelected = " ";
				if (selectedTarget == baseTarget)
					strSelected = ">";
				
				string strDistance = vector.Distance(myPos, targetEntity.GetOrigin()).ToString(5, 2);
				
				string strType = typename.EnumToString(EAIUnitType, baseTarget.GetUnitType());
				
				float recognitionDetect;
				float recognitionIdentify;
				baseTarget.GetAccumulatedRecognition(recognitionDetect, recognitionIdentify);
				
				string str = string.Format("%1 %2 %3 %4s %5 %6 %7 %8 %9",
					targetId, strSelected, typename.EnumToString(ETargetCategory, targetCategory), strTimeSinceSeen, strEndangering, strType,
					baseTarget.GetExposure().ToString(3,2),
					recognitionDetect.ToString(3, 2),
					recognitionIdentify.ToString(3, 2));
				DbgUI.Text(str);
				
				DbgUI.Text(string.Format("%1   %2 %3", targetId, targetEntity, strPrefabName));
				
				bool showRecognition = false;
				DbgUI.Check(string.Format("%1 Recognition", targetId), showRecognition); 
				if (showRecognition)
				{
					
					DbgUI.Text(string.Format("%1   Exp: %2, Rec: Detect: %3 Identify: %4",
						targetId, baseTarget.GetExposure().ToString(3, 2), recognitionDetect.ToString(3, 2), recognitionIdentify.ToString(3, 2)));
					
					const int plotWidth = 200;
					const int plotHeight = 150;
					const int plotHistory = 800;
					DbgUI.PlotLive(string.Format("%1 Detection", targetId), plotWidth, plotHeight, recognitionDetect, 300);
					DbgUI.PlotLive(string.Format("%1 Identification", targetId), plotWidth, plotHeight, recognitionIdentify, 300);
				}
				
				targetId++;
			}
		}
	}
	
	void ShowPerceivableComponent(PerceivableComponent p)
	{
		DbgUI.Text("Recognition Factors:");
		DbgUI.Text(string.Format("  Visual:   %1", p.GetVisualRecognitionFactor()));
		DbgUI.Text(string.Format("    Illumination: %1", p.GetIlluminationFactor()));
		DbgUI.Text(string.Format("  Acoustic: %1", p.GetAcousticRecognitionFactor()));
		DbgUI.Text(string.Format("Est. visual size: %1", p.GetEstimatedVisualSize()));
	}
	
	//! Returns agent name based on faction and callsign
	string GetAgentDebugName()
	{
		if (m_Group)
		{
			// Group
			string company, platoon, squad, character, format, returnString;
		 	m_Group.GetCallsigns(company, platoon, squad, character, format);
			returnString.Format(format, company, platoon, squad, character);
			return returnString;
			
		}
		else if (m_Agent)
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
		else
		{
			return string.Empty;
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