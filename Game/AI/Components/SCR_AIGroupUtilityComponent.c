[ComponentEditorProps(category: "GameScripted/AI", description: "Component for utility AI system for groups", color: "0 0 255 255")]
class SCR_AIGroupUtilityComponentClass: SCR_AIBaseUtilityComponentClass
{
};

class SCR_AIGroupUtilityComponent : SCR_AIBaseUtilityComponent
{
	static const int MAX_FIRE_TEAMS_COUNT = 4;
	
	SCR_AIGroup m_Owner;
	SCR_AIConfigComponent m_ConfigComponent;
	SCR_AIGroupInfoComponent m_GroupInfo;
	ref array<IEntity> m_aListOfKnownEnemies = new ref array<IEntity>;
	ref array<SCR_AIInfoComponent> m_aListOfAIInfo = new ref array<SCR_AIInfoComponent>;
	ref array<EFireTeams> m_aFireteamsForKnownEnemies = new ref array<EFireTeams>; // maps list of known enemies to fireteams - MUST keep the same order!
	ref array<vector> m_aPositionsForKnownEnemies = new ref array<vector>; // maps list of known enemies to last known positions - MUST keep the same order!
	
	protected bool m_bFireteamsInitialized; 
	protected bool m_bRestartActivity;
	protected bool m_bNewGroupMemberAdded;
	protected ref SCR_AIActivityBase m_CurrentActivity;
	
	protected ref ScriptInvoker Event_OnEnemyDetected = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnNoEnemy = new ScriptInvoker;
	
	//---------------------------------------------------------------------------------------------------
	SCR_AIActivityBase EvaluateActivity(SCR_AIMessageGoal goalMessage,SCR_AIMessageInfo infoMessage, out bool restartActivity)
	{
		ref SCR_AIActivityBase activity;
		
		if (!m_ConfigComponent)
			return null;
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateActivity START");
		if (m_bEvaluationBreakpoint)
		{
			Print("EvaluateActivity breakpoint triggered");
			debug;
			m_bEvaluationBreakpoint = false;
		}
		#endif
		
		// process new goal
		if (goalMessage)
		{
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("PerformGoalReaction: %1", goalMessage));
			#endif
			m_ConfigComponent.PerformGoalReaction(this, goalMessage);
		}
			
		if (infoMessage)
		{
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("PerformInfoReaction: %1", infoMessage));
			#endif
			m_ConfigComponent.PerformInfoReaction(this, infoMessage);
		}
			
		RemoveObsoleteActions();
		
		//TODO remove this by AI reporting this independently on attack Tree
		ReportDeadEnemies();
		
		activity = SCR_AIActivityBase.Cast(EvaluateActions());
		
		if (activity && (!m_CurrentActivity || (m_CurrentActivity != activity && m_CurrentActivity.IsActionInterruptable())))
		{
			SetCurrentAction(activity);
			UpdateGroupControlMode(activity);
			m_CurrentActivity = activity;
			m_bRestartActivity = true;
			if (m_bNewGroupMemberAdded)
				m_bNewGroupMemberAdded = false;
			
#ifdef WORKBENCH
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_ACTIVITY))
				PrintFormat("Agent %1 activity %2",m_Owner,m_CurrentActivity.GetActionDebugInfo());
#endif				
		}
		else
		{
			if (m_bNewGroupMemberAdded)
			{
				m_bRestartActivity = true;
				m_bNewGroupMemberAdded = false;
			}
			else
				m_bRestartActivity = false;
		}
		restartActivity = m_bRestartActivity;
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateActivity END\n");
		#endif
		
		return m_CurrentActivity;
	}	
	
	//---------------------------------------------------------------------------------------------------
	// TODO - ticket: 20384
	// temporal function unless solidier will report lost and dead enemies
	void ReportDeadEnemies()
	{
		if (!m_Owner)
			return;
		AICommunicationComponent mailbox = m_Owner.GetCommunicationComponent();
		if (mailbox)
		{
			foreach (IEntity e : m_aListOfKnownEnemies)
			{
				if (!SCR_AIIsAlive.IsAlive(e))
				{
					SCR_AIMessage_TargetEliminated msg = new SCR_AIMessage_TargetEliminated;
					msg.m_MessageType = EMessageType_Info.TARGET_ELIMINATED;
					msg.SetText("Temporal remove from group util");
					msg.SetReceiver(m_Owner);
					msg.m_Target = e;
					mailbox.RequestBroadcast(msg, AIAgent.Cast(e));
				}
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	bool AddKnownEnemy(IEntity enemy)
	{
		if (!enemy)
			return false;
		
		if ( AIAgent.Cast(enemy) ) 
			enemy = AIAgent.Cast(enemy).GetControlledEntity();
		
		if (m_aListOfKnownEnemies.IsEmpty())
		{
			Faction enemyFaction;
			FactionAffiliationComponent fComp = FactionAffiliationComponent.Cast(enemy.FindComponent(FactionAffiliationComponent));
		
			if (fComp)
				enemyFaction = fComp.GetAffiliatedFaction();
			
			Event_OnEnemyDetected.Invoke(m_Owner, enemyFaction);
		}
		else if (m_aListOfKnownEnemies.Find(enemy) >= 0)
			return false;
		
		m_aListOfKnownEnemies.Insert(enemy);
		m_aFireteamsForKnownEnemies.Insert(EFireTeams.NONE);
		m_aPositionsForKnownEnemies.Insert(vector.Zero);
		return true;
	}
	
	//---------------------------------------------------------------------------------------------------
	void RemoveKnownEnemy(IEntity enemy)
	{
		if ( AIAgent.Cast(enemy) ) 
			enemy = AIAgent.Cast(enemy).GetControlledEntity();
		int index = m_aListOfKnownEnemies.Find(enemy);
		if (index > -1)
		{
			m_aFireteamsForKnownEnemies.RemoveOrdered(index);
			m_aPositionsForKnownEnemies.RemoveOrdered(index);
			m_aListOfKnownEnemies.RemoveItemOrdered(enemy);
			if (m_aListOfKnownEnemies.IsEmpty())
				Event_OnNoEnemy.Invoke(m_Owner);
		}
		
	}

	//---------------------------------------------------------------------------------------------------
	// updates info of group members to planner - should be called when adding or removing group member 	
	bool AddAgentInfo(AIAgent agent)
	{
		SCR_AIInfoComponent info = SCR_AIInfoComponent.Cast(agent.FindComponent(SCR_AIInfoComponent));
		
		if (!info)
			return false;
		
		m_aListOfAIInfo.Insert(info);	
		return true;
	}
	
	//---------------------------------------------------------------------------------------------------
	bool RemoveAgentInfo(AIAgent agent)
	{
		for (int i = 0,length = m_aListOfAIInfo.Count(); i < length; i++)
		{
			if (!m_aListOfAIInfo[i])
			{
				Debug.Error("Null AI info occured"); // investigate when this happens!
				m_aListOfAIInfo.RemoveOrdered(i);
			}
			else if (m_aListOfAIInfo[i].IsOwnerAgent(agent))
			{
				m_aListOfAIInfo.RemoveOrdered(i);
				return true;
			}
		}
		return false;
	}
	
	//---------------------------------------------------------------------------------------------------	
	void ~SCR_AIGroupUtilityComponent()
	{
		if ( m_aListOfKnownEnemies )
		{ 
			m_aListOfKnownEnemies.Clear();
			m_aListOfKnownEnemies = null;
		}	
		m_CurrentActivity = null;
		m_aFireteamsForKnownEnemies = null;	
		m_aPositionsForKnownEnemies = null;	
	}
	
	//---------------------------------------------------------------------------------------------------
	// returns desired number of fireteams depending on number of units in the group
	int GetNumberOfFireTeams()
	{
		//there should always be at leas one fireteam
		return Math.Min(1 + Math.Floor(m_aListOfAIInfo.Count() / MAX_FIRE_TEAMS_COUNT), MAX_FIRE_TEAMS_COUNT);	
	}
	
	//---------------------------------------------------------------------------------------------------
	bool IsFireteamsInitialized()
	{
		return m_bFireteamsInitialized;
	}
	
	//---------------------------------------------------------------------------------------------------
	// sets membership of fireteam for each group member - TODO: handle death
	void InitFireTeams()
	{
		m_bFireteamsInitialized = true;
		
		int noOfFireTeams = GetNumberOfFireTeams();
		for (int i = 0, numberOfGroupMembers = m_aListOfAIInfo.Count(); i< numberOfGroupMembers; i++)
		{
			if (m_aListOfAIInfo[i])
				m_aListOfAIInfo[i].SetFireTeam(i % noOfFireTeams + 1);	
			else
				Debug.Error("Null AI info occured"); // investigate when this happens!
		}
	}
	
	//---------------------------------------------------------------------------------------------------	
	bool IsSomeEnemyKnown()
	{
		if (m_aListOfKnownEnemies.Count() == 0)
			return false;
		
		// there can be NULLs inside known enemies so we remove them
		for (int i = m_aListOfKnownEnemies.Count() - 1; i >= 0; i--)
		{
			if (m_aListOfKnownEnemies[i] == null)
			{
				m_aFireteamsForKnownEnemies.RemoveOrdered(i);
				m_aPositionsForKnownEnemies.RemoveOrdered(i);
				m_aListOfKnownEnemies.RemoveOrdered(i);
			}
			else
			{
				return true;
			}
		}
		
		// if array was filled just with NULLs now will be empty
		if (m_aListOfKnownEnemies.Count() == 0)
			return false;
		
		return true;
	}
	
	//---------------------------------------------------------------------------------------------------	
	// this should inform newly added group member about current activity
	protected void OnAgentAdded(AIAgent child)
	{
		m_bNewGroupMemberAdded = true;
	}
	

	//---------------------------------------------------------------------------------------------------	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_Owner = SCR_AIGroup.Cast(owner);
		if (!m_Owner)
			return;	
		
		m_ConfigComponent = SCR_AIConfigComponent.Cast(m_Owner.FindComponent(SCR_AIConfigComponent));
		
		m_ConfigComponent.AddDefaultActivities(this);
		//AddAction(new SCR_AIIdleActivity(this))
		
		m_Owner.GetOnAgentAdded().Insert(OnAgentAdded);
		
		m_GroupInfo = SCR_AIGroupInfoComponent.Cast(m_Owner.FindComponent(SCR_AIGroupInfoComponent));
	}
	
	//---------------------------------------------------------------------------------------------------
	void UpdateGroupControlMode(SCR_AIActivityBase currentActivity)
	{
		#ifdef AI_DEBUG
		AddDebugMessage("UpdateGroupControlMode");
		#endif
		
		if (m_GroupInfo)
		{
			if (currentActivity.m_bIsWaypointRelated)
				m_GroupInfo.SetGroupControlMode(EGroupControlMode.FOLLOWING_WAYPOINT);
			else if (currentActivity.m_eType == EAIActionType.IDLE)
				m_GroupInfo.SetGroupControlMode(EGroupControlMode.IDLE);
			else
				m_GroupInfo.SetGroupControlMode(EGroupControlMode.AUTONOMOUS);
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnEnemyDetected()
	{
		return Event_OnEnemyDetected;
	}
	
	//---------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnNoEnemy()
	{
		return Event_OnNoEnemy;
	}	
};