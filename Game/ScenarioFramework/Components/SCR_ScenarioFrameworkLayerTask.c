#include "scripts/Game/config.c"
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
	
	protected SCR_ScenarioFrameworkSlotTask	m_SlotTask; //storing this one in order to get the task title and description
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
	string GetOverridenObjectDisplayName() 
	{ 
		return m_sOverrideObjectDisplayName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOverridenObjectDisplayName(string name) 
	{ 
		m_sOverrideObjectDisplayName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTaskTitle()
	{
		return m_sTaskTitle;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetOriginalTaskDescription()
	{ 
		return m_sTaskDescription;
	}	
	
	//------------------------------------------------------------------------------------------------
	string GetTaskDescription()
	{ 
		if (!m_sOverrideObjectDisplayName.IsEmpty() && m_SlotTask && m_SlotTask.GetOverridenObjectDisplayName().IsEmpty())
			m_SlotTask.SetOverridenObjectDisplayName(m_sOverrideObjectDisplayName);

		return m_sTaskDescription;
	}	
	
	//------------------------------------------------------------------------------------------------
	SCR_ESFTaskType GetTaskType()
	{
		return m_eTypeOfTask;
	}
	
	//------------------------------------------------------------------------------------------------
	override void DynamicReinit()
	{
		Init(null, SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DynamicDespawn()
	{
		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;
		
		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : m_aActivationConditions)
		{
			//If just one condition is false, we don't continue and interrupt the init
			if (!activationCondition.Init(GetOwner()))
			{
				InvokeAllChildrenSpawned();
				return;
			}
		}
		
		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
		{
			child.DynamicDespawn();
		}
		
		foreach (IEntity entity : m_aSpawnedEntities)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}
		
		m_SlotTask = null;
		m_aSpawnedEntities.Clear();
		
		GetOnAllChildrenSpawned().Remove(InitTask);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (m_bInitiated)
			return;
		
		if (!m_bDynamicallyDespawned && activation != m_eActivationType)
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
		
		if (!GetTaskManager())
		{
			Print("ScenarioFramework: Task manager not found in the world, tasks won't be created!", LogLevel.ERROR);
			return;
		}
		
		GetOnAllChildrenSpawned().Insert(InitTask);
		GetOnAllChildrenSpawned().Insert(AfterAllChildrenSpawned);

		GetChildren();
		
		SpawnChildren(previouslyRandomized);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskStateChanged(SCR_TaskState previousState, SCR_TaskState newState)
	{
		m_eLayerTaskState = newState;
		
		if (newState == SCR_TaskState.FINISHED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase triggerAction : m_aTriggerActionsOnFinish)
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
	void SetSlotTask(SCR_ScenarioFrameworkSlotTask slotTask)
	{
		m_SlotTask = slotTask;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkSlotTask GetSlotTask()
	{
		return m_SlotTask;
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
		//If m_Task already exists, we are reiniting it, not creating new one
		if (m_Task)
		{
			// We need to make sure that we have m_SlotTask to work with 
			if (!m_SlotTask)
				m_SlotTask = m_Task.GetSlotTask();
			
			if (!m_SlotTask)
			{
				if (!m_aRandomlySpawnedChildren.IsEmpty())
					m_SlotTask = GetSlotTask(m_aRandomlySpawnedChildren);
				else
					m_SlotTask = GetSlotTask(m_aChildren);
			}
			
			if (!m_SlotTask)
			{
				Print(string.Format("ScenarioFramework: %1 could not reinit task due to missing m_SlotTask!", GetOwner().GetName()), LogLevel.ERROR);
				return;
			}
			
			//Update task position
			vector vOrigin;
			if (m_bPlaceMarkerOnSubjectSlot && m_SlotTask)
				vOrigin = m_SlotTask.GetOwner().GetOrigin();
			else
				vOrigin = GetOwner().GetOrigin();
			
			m_SupportEntity.MoveTask(vOrigin, m_Task.GetTaskID());
			m_Task.RehookTaskAsset(m_Entity);
			
			if (m_SlotTask.GetIsTerminated() || m_bIsTerminated)
			{
				if (m_eLayerTaskState == SCR_TaskState.FINISHED)
				{
					m_bTaskResolvedBeforeLoad = true;
					m_SlotTask.SetTaskResolvedBeforeLoad(true);
					m_SupportEntity.FinishTask(m_Task);
				}
				else if (m_eLayerTaskState == SCR_TaskState.CANCELLED)
				{
					m_SlotTask.SetTaskResolvedBeforeLoad(true);
					m_SupportEntity.FailTask(m_Task);
				}
				else if (m_eLayerTaskState == SCR_TaskState.REMOVED)
				{
					m_SlotTask.SetTaskResolvedBeforeLoad(true);
					m_SupportEntity.CancelTask(m_Task.GetTaskID());
				}
				
				SCR_EntityHelper.DeleteEntityAndChildren(m_SlotTask.GetSpawnedEntity());
			}
			
			if (m_eLayerTaskState != SCR_TaskState.FINISHED)
				m_Task.Init();
			
			return;
		}
		
		if (!m_SlotTask)
		{
			Print(string.Format("ScenarioFramework: %1 could not init task due to missing m_SlotTask!", GetOwner().GetName()), LogLevel.ERROR);
			return;
		}
		
		if (SetSupportEntity() && SetTaskPrefab() && CreateTask())
		{
			SetupTask();
			m_Task.SetLayerTask(this);
			
			m_SlotTask.OnTaskStateChanged(SCR_TaskState.OPENED);
			if (m_SlotTask.GetIsTerminated() || m_bIsTerminated)
			{
				if (m_eLayerTaskState == SCR_TaskState.FINISHED)
				{
					m_bTaskResolvedBeforeLoad = true;
					m_SlotTask.SetTaskResolvedBeforeLoad(true);
					m_SupportEntity.FinishTask(m_Task);
				}
				else if (m_eLayerTaskState == SCR_TaskState.CANCELLED)
				{
					m_SlotTask.SetTaskResolvedBeforeLoad(true);
					m_SupportEntity.FailTask(m_Task);
				}
				else if (m_eLayerTaskState == SCR_TaskState.REMOVED)
				{
					m_SlotTask.SetTaskResolvedBeforeLoad(true);
					m_SupportEntity.CancelTask(m_Task.GetTaskID());
				}
				
				SCR_EntityHelper.DeleteEntityAndChildren(m_SlotTask.GetSpawnedEntity());
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
		if (m_sFactionKey.IsEmpty())
			m_sFactionKey = "US";	//set a default US one if none is set by user
		SCR_Faction testFaction = SCR_Faction.Cast(manager.GetFactionByKey(m_sFactionKey));
		
		foreach (Faction faction : aPlayableFactions)
		{
			if (!SCR_Faction.Cast(faction))
				continue;

			if (SCR_Faction.Cast(faction).IsPlayable())
			{
				if (m_sFactionKey == "")
				{
					factionSelected = SCR_Faction.Cast(faction);
					break;
				}
				
				if (faction == testFaction)
				{
					factionSelected = SCR_Faction.Cast(faction);
					break;
				}
			}
		}
		
		if (!factionSelected)
		{
			Print(string.Format("ScenarioFramework: Creating of task %1 failed for %2! No playable faction available", m_sTaskTitle, GetOwner().GetName()), LogLevel.ERROR);
			return false;
		}
				
		m_Task.SetSlotTask(m_SlotTask);
		m_SupportEntity.SetTargetFaction(m_Task, factionSelected);
		vector vOrigin;
		if (m_bPlaceMarkerOnSubjectSlot && m_SlotTask)
			vOrigin = m_SlotTask.GetOwner().GetOrigin();
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
			m_SupportEntity.SetTaskTitle(m_Task, m_sTaskTitle);
		
		if (!m_sTaskDescription.IsEmpty()) 
			m_SupportEntity.SetTaskDescription(m_Task, GetTaskDescription());
		
		//if title and description is provided on the Task Subject, fill it in (overrides Layer Task)
		if (m_SlotTask)
		{
			m_SupportEntity.SetSpawnedEntityName(m_Task, m_SlotTask.GetSpawnedEntityDisplayName());
			
			if (!m_SlotTask.GetTaskTitle().IsEmpty())
				m_SupportEntity.SetTaskTitle(m_Task, m_SlotTask.GetTaskTitle());
			
			if (!m_SlotTask.GetTaskDescription().IsEmpty())
				m_SupportEntity.SetTaskDescription(m_Task, m_SlotTask.GetTaskDescription());

	m_SupportEntity.SetTaskExecutionBriefing(m_Task, m_SlotTask.GetTaskExecutionBriefing());
		}
		else
		{
			Print(string.Format("ScenarioFramework - LayerTask: Task Subject not found doesn't exist for %1. Task won't be spawned!", GetOwner().GetName()), LogLevel.ERROR);
		}
		
		s_OnTaskSetup.Invoke(m_Task);
		PrintFormat("ScenarioFramework: -> LayerTask: SlotTask %1 - generating task %2. Description: %3", m_SlotTask.GetOwner().GetName(), WidgetManager.Translate(m_Task.GetTitle()), string.Format(WidgetManager.Translate(m_Task.GetDescription(), WidgetManager.Translate(m_Task.GetSpawnedEntityName()))));
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
