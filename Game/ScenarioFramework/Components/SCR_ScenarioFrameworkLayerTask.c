[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkLayerTaskClass : SCR_ScenarioFrameworkLayerBaseClass
{
	// prefab properties here
};

class SCR_ScenarioFrameworkLayerTask : SCR_ScenarioFrameworkLayerBase
{		
		
	[Attribute(desc: "Name of the task in list of tasks", category: "Task")]		
	protected string 		m_sTaskTitle;
	
	[Attribute(desc: "Description of the task", category: "Task", )]			//TODO: make config, memory
	protected string 		m_sTaskDescription;

	//[Attribute("Type of task", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ESFTaskType), category: "Task")];
	protected SCR_ESFTaskType 	m_eTypeOfTask = SCR_ESFTaskType.DEFAULT;
	
	
	[Attribute(desc: "Task prefab", category: "Task")]		
	protected ResourceName	m_sTaskPrefab;
	
	[Attribute(desc: "Marker on map is placed directly on the task subject Slot or on layer Slot", category: "Task")]		
	protected bool			m_bPlaceMarkerOnSubjectSlot;
	
	[Attribute(desc: "Overrides display name of the spawned object for task purposes", category: "Task")]
	protected string 	m_sOverrideObjectDisplayName;
	
	[Attribute(category: "OnTaskFinished")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aTriggerActionsOnFinish;
	
	protected SCR_ScenarioFrameworkSlotTask	m_TaskSubject; 				//storing this one in order to get the task title and description
	protected SCR_ScenarioFrameworkTask	m_Task;
	protected SCR_ScenarioFrameworkTaskSupportEntity m_SupportEntity;
	protected SCR_TaskState m_eLayerTaskState;
	bool m_bTaskResolvedBeforeLoad;
	
	static ref ScriptInvoker s_OnTaskSetup = new ref ScriptInvoker();
	
#ifdef WORKBENCH
	[Attribute(defvalue: "0", desc: "Show the debug shapes in Workbench", category: "Debug")];
	protected bool							m_bShowDebugShapesInWorkbench;
#endif	
	
	//------------------------------------------------------------------------------------------------
	SCR_TaskState GetLayerTaskState()
	{
		return m_eLayerTaskState;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetLayerTaskResolvedBeforeLoad()
	{
		return m_bTaskResolvedBeforeLoad;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLayerTaskState(SCR_TaskState state)
	{
		m_eLayerTaskState = state;
	}
		
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkTask GetTask()
	{
		return m_Task;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetTaskPrefab()
	{
		return m_sTaskPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTaskTitle()
	{
		return m_sTaskTitle;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTaskDescription()
	{ 
		if (!m_Entity)
			return m_sTaskDescription;
		
		if (!m_sOverrideObjectDisplayName.IsEmpty())
			return string.Format(WidgetManager.Translate(m_sTaskDescription, m_sOverrideObjectDisplayName));
		
		SCR_EditableEntityComponent editableEntityComp = SCR_EditableEntityComponent.Cast(m_Entity.FindComponent(SCR_EditableEntityComponent));
		if (!editableEntityComp)
			return m_sTaskDescription;
		
		return string.Format(WidgetManager.Translate(m_sTaskDescription, editableEntityComp.GetDisplayName()));
	}	
	
	//------------------------------------------------------------------------------------------------
	SCR_ESFTaskType GetTaskType()
	{
		return m_eTypeOfTask;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT, bool bInit = true)
	{
		if (m_eActivationType != activation)
			return;

		if (!area)
		{
			SCR_GameModeSFManager gameModeComp = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
			if (gameModeComp) 
				area = gameModeComp.GetParentArea(GetOwner());
		}
		
		m_Area = area;
		
		bool previouslyRandomized;
		if (!m_aRandomlySpawnedChildren.IsEmpty())
			previouslyRandomized = true;
		
		// Handles inheritance of faction settings from parents
		if (m_sFactionKey.IsEmpty() && m_ParentLayer && !m_ParentLayer.GetFactionKey().IsEmpty())
			SetFactionKey(m_ParentLayer.GetFactionKey());

		GetChildren();
		
		SpawnChildren(bInit, previouslyRandomized);
		ActivateLogic();
		
		foreach(SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}
		
		m_bInitiated = true;		//used to let know this slot was already initiated (to prevent spawning objects twice)
		
		if (m_fRepeatedSpawnTimer >= 0)
		{
			m_fRepeatSpawnTimeStart = Replication.Time();
			m_fRepeatSpawnTimeEnd = m_fRepeatSpawnTimeStart + (m_fRepeatedSpawnTimer * 1000);
			m_bRepeatedSpawningSet = true;
		}	
		
		if (!GetTaskManager())
		{
			Print("ScenarioFramework: Task manager not found in the world, tasks won't be created!", LogLevel.ERROR);
			return;
		}
		InitTask();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskStateChanged(SCR_TaskState previousState, SCR_TaskState newState)
	{
		m_eLayerTaskState = newState;
		
		if (newState == SCR_TaskState.FINISHED && !m_bTaskResolvedBeforeLoad)
		{
			foreach(SCR_ScenarioFrameworkActionBase triggerAction : m_aTriggerActionsOnFinish)
			{
				triggerAction.OnActivate(GetOwner());
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool SetTaskPrefab()
	{
		if (!m_SupportEntity.GetTaskPrefab())
		{
			if (m_sTaskPrefab.IsEmpty())
			{
				Print("ScenarioFramework: Task prefab not set, task won't be created!", LogLevel.ERROR);
				return false;
			}
			m_SupportEntity.SetTaskPrefab(m_sTaskPrefab);
		}
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTaskSubject(SCR_ScenarioFrameworkSlotTask taskSubject)
	{
		m_TaskSubject = taskSubject;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkSlotTask GetTaskSubject()
	{
		return m_TaskSubject;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskSupportEntity))
		{
			Print("ScenarioFramework: Default Task support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}
		m_SupportEntity = SCR_ScenarioFrameworkTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskSupportEntity));
		return true;
	}
	
	void InitTask()
	{
		if (SetSupportEntity() && SetTaskPrefab() && CreateTask())
		{
			SetupTask();
			m_Task.SetTaskLayer(this);
			if (!m_TaskSubject) 	
				return;
			
			m_TaskSubject.OnTaskStateChanged(SCR_TaskState.OPENED);
			
			if (m_TaskSubject.GetIsTerminated() || m_bIsTerminated)
			{
				if (m_eLayerTaskState == SCR_TaskState.FINISHED)
				{
					m_bTaskResolvedBeforeLoad = true;
					m_TaskSubject.SetTaskResolvedBeforeLoad(true);
					m_SupportEntity.FinishTask(m_Task);
				}
				else if (m_eLayerTaskState == SCR_TaskState.CANCELLED)
				{
					m_TaskSubject.SetTaskResolvedBeforeLoad(true);
					m_SupportEntity.FailTask(m_Task);
				}
				else if (m_eLayerTaskState == SCR_TaskState.REMOVED)
				{
					m_TaskSubject.SetTaskResolvedBeforeLoad(true);
					m_SupportEntity.CancelTask(m_Task.GetTaskID());
				}
				
				delete m_TaskSubject.GetSpawnedEntity();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CreateTask()
	{
		m_Task = SCR_ScenarioFrameworkTask.Cast(m_SupportEntity.CreateTask(this));			
		if (!m_Task)
		{
			PrintFormat("ScenarioFramework: Creating of task %1 failed! Task manager refused to create it.", m_sTaskTitle, LogLevel.ERROR);
			return false;
		}
		//TODO: make selection of the faction dynamic
		array<Faction> aPlayableFactions = {};
		SCR_Faction factionSelected;
		FactionManager manager = GetGame().GetFactionManager();
		if (!manager)
		{
			PrintFormat("ScenarioFramework: Creating of task %1 failed! Faction manager doesn't exist", m_sTaskTitle, LogLevel.ERROR);
			return false;
		}
		
		manager.GetFactionsList(aPlayableFactions);
		foreach(Faction faction : aPlayableFactions)
		{
			if (!SCR_Faction.Cast(faction))
				continue;

			if (SCR_Faction.Cast(faction).IsPlayable())
			{
				factionSelected = SCR_Faction.Cast(faction);
				break;
			}
		}
		
		if (!factionSelected)
		{
			PrintFormat("ScenarioFramework: Creating of task %1 failed! No playable faction available", m_sTaskTitle, LogLevel.ERROR);
			return false;
		}
				
		m_SupportEntity.SetTargetFaction(m_Task, factionSelected);
		vector vOrigin;
		if (m_bPlaceMarkerOnSubjectSlot && m_TaskSubject)
			vOrigin = m_TaskSubject.GetOwner().GetOrigin();
		else
			vOrigin = GetOwner().GetOrigin();
		
		m_SupportEntity.MoveTask(vOrigin, m_Task.GetTaskID());
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupTask()
	{
		//if title and description is provided, fill it in
		if (!m_sTaskTitle.IsEmpty()) 
			m_SupportEntity.SetTaskTitle(GetTask(), GetTaskTitle());
		
		if (!m_sTaskDescription.IsEmpty()) 
			m_SupportEntity.SetTaskDescription(GetTask(), GetTaskDescription());
		
		//if title and description is provided on the Task Subject, fill it in (overrides Layer Task)
		if (m_TaskSubject)
		{
			if (!m_TaskSubject.GetTaskTitle().IsEmpty())
				m_SupportEntity.SetTaskTitle(GetTask(), m_TaskSubject.GetTaskTitle());
			
			if (!m_TaskSubject.GetTaskDescription().IsEmpty())
				m_SupportEntity.SetTaskDescription(GetTask(), m_TaskSubject.GetTaskDescription());
		}
		else
		{
			Print("ScenarioFramework: ->Task: Task Subject not found doesn't exist", LogLevel.ERROR);
		}
		
		s_OnTaskSetup.Invoke(m_Task);
		
		PrintFormat("ScenarioFramework: ->Task: Generating task %1. Description: %2", string.Format(WidgetManager.Translate(m_Task.GetTitle())), string.Format(WidgetManager.Translate(m_Task.GetDescription())), LogLevel.ERROR);
	}
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		DrawDebugShape(m_bShowDebugShapesInWorkbench);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "m_bShowDebugShapesInWorkbench")
			DrawDebugShape(m_bShowDebugShapesInWorkbench);
		
		return false;
	}
#endif	
};
