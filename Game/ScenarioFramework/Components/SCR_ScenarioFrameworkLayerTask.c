[EntityEditorProps(category: "GameScripted/ScenarioFramework/Layer", description: "")]
class SCR_ScenarioFrameworkLayerTaskClass : SCR_ScenarioFrameworkLayerBaseClass
{
}

class SCR_ScenarioFrameworkLayerTask : SCR_ScenarioFrameworkLayerBase
{
	[Attribute(desc: "Name of the task in list of tasks", category: "Task")]
	string m_sTaskTitle;

	[Attribute(desc: "Description of the task", category: "Task", )]			//TODO: make config, memory
	string m_sTaskDescription;

	//[Attribute("Type of task", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ESFTaskType), category: "Task")];
	SCR_ESFTaskType m_eTypeOfTask = SCR_ESFTaskType.DEFAULT;

	[Attribute(desc: "Task prefab", category: "Task")]
	ResourceName m_sTaskPrefab;

	[Attribute(desc: "Marker on map is placed directly on the task subject Slot or on layer Slot", category: "Task")]
	bool m_bPlaceMarkerOnSubjectSlot;

	[Attribute(desc: "Overrides display name of the spawned object for task purposes", category: "Task")]
	string m_sOverrideObjectDisplayName;

	[Attribute(category: "OnTaskFinished")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aTriggerActionsOnFinish;

	SCR_ScenarioFrameworkSlotTask m_SlotTask; //storing this one in order to get the task title and description
	SCR_ScenarioFrameworkTask m_Task;
	SCR_ScenarioFrameworkTaskSupportEntity m_SupportEntity;
	SCR_TaskState m_eLayerTaskState;
	bool m_bTaskResolvedBeforeLoad;

	static const ref ScriptInvoker s_OnTaskSetup = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return Layer task state in current task layer.
	SCR_TaskState GetLayerTaskState()
	{
		return m_eLayerTaskState;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Indicates if task was resolved before loading.
	bool GetLayerTaskResolvedBeforeLoad()
	{
		return m_bTaskResolvedBeforeLoad;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state Sets layer task state
	void SetLayerTaskState(SCR_TaskState state)
	{
		m_eLayerTaskState = state;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the current task.
	SCR_ScenarioFrameworkTask GetTask()
	{
		return m_Task;
	}

	//------------------------------------------------------------------------------------------------
	//! \return The return value represents the prefab for the task in the method.
	ResourceName GetTaskPrefab()
	{
		return m_sTaskPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the display name for an overridden object.
	string GetOverridenObjectDisplayName()
	{
		return m_sOverrideObjectDisplayName;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] name Sets object display name override.
	void SetOverridenObjectDisplayName(string name)
	{
		m_sOverrideObjectDisplayName = name;
	}

	//------------------------------------------------------------------------------------------------
	//! \return The return value represents the title of the task.
	string GetTaskTitle()
	{
		return m_sTaskTitle;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Original task description string returned by the method.
	string GetOriginalTaskDescription()
	{
		return m_sTaskDescription;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the task description, or sets it if an override object display name is provided.
	string GetTaskDescription()
	{
		if (!m_sOverrideObjectDisplayName.IsEmpty() && m_SlotTask && m_SlotTask.GetOverriddenObjectDisplayName().IsEmpty())
			m_SlotTask.SetOverriddenObjectDisplayName(m_sOverrideObjectDisplayName);

		return m_sTaskDescription;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Task type enum representing the type of task in the mission.
	SCR_ESFTaskType GetTaskType()
	{
		return m_eTypeOfTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Restores default settings, clears children, removes spawned entities, optionally reinitializes after restoration.
	//! \param[in] includeChildren Restores default settings for this entity and its children if includeChildren is true.
	//! \param[in] reinitAfterRestoration Restores entity to default state, optionally reinitializes after restoration.
	//! \param[in] affectRandomization determines whether to clear all randomly spawned children entities after restoring default settings.
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false, bool affectRandomization = true)
	{
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aTriggerActionsOnFinish)
		{
			activationAction.m_iNumberOfActivations = 0;
		}
		
		m_SlotTask = null;
		
		if (m_SupportEntity)
			m_SupportEntity.CancelTask(m_Task.GetTaskID());
		
		m_Task = null;
		m_SupportEntity = null;
		SetLayerTaskState(SCR_TaskState.OPENED);
		m_bTaskResolvedBeforeLoad = false;
		
		super.RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes layer with same activation type as parent.
	override void DynamicReinit()
	{
		Init(null, SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
	}

	//------------------------------------------------------------------------------------------------
	//! Dynamically despawns this layer.
	//! \param[in] layer for which this is called.
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(DynamicDespawn);
		if (!m_bInitiated)
		{
			GetOnAllChildrenSpawned().Insert(DynamicDespawn);
			return;
		}

		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;

		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : m_aActivationConditions)
		{
			//If just one condition is false, we don't continue and interrupt the init
			if (!activationCondition.Init(GetOwner()))
			{
				if (m_ParentLayer)
					m_ParentLayer.CheckAllChildrenSpawned(this);

				return;
			}
		}

		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		m_aChildren.RemoveItem(null);
		foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
		{
			child.DynamicDespawn(this);
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
	//! Initializes tasks after all children spawned, logs error if task manager not found.
	override void FinishInit()
	{
		if (!GetTaskManager())
		{
			Print("ScenarioFramework: Task manager not found in the world, tasks won't be created!", LogLevel.ERROR);
			return;
		}

		GetOnAllChildrenSpawned().Insert(InitTask);
		
		super.FinishInit();
	}

	//------------------------------------------------------------------------------------------------
	//! Triggers actions on task finish if not resolved before load.
	//! \param[in] previousState represents the current state of the task before it transitions to new state.
	//! \param[in] newState Triggers actions on task finish state change.
	void OnTaskStateChanged(SCR_TaskState previousState, SCR_TaskState newState)
	{
		if (newState == SCR_TaskState.FINISHED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase triggerAction : m_aTriggerActionsOnFinish)
			{
				triggerAction.OnActivate(GetOwner());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets task prefab for support entity
	//! \return true if task prefab is set, false otherwise.
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
	//! \param[in] slotTask related to this layer task.
	void SetSlotTask(SCR_ScenarioFrameworkSlotTask slotTask)
	{
		m_SlotTask = slotTask;
	}

	//------------------------------------------------------------------------------------------------
	//! \return related to this layer task.
	SCR_ScenarioFrameworkSlotTask GetSlotTask()
	{
		return m_SlotTask;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets support entity for layer task
	//! \return true if support entity is found and set, false otherwise.
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

	//------------------------------------------------------------------------------------------------
	//! Initializes task, sets up support entity, and handles task state changes based on scenario framework layer.
	//! \param[in] layer for which this task is to be initialized
	void InitTask(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(InitTask);
		
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
					m_bTaskResolvedBeforeLoad = true;
					m_SlotTask.SetTaskResolvedBeforeLoad(true);
					m_SupportEntity.FailTask(m_Task);
				}
				else if (m_eLayerTaskState == SCR_TaskState.REMOVED)
				{
					m_bTaskResolvedBeforeLoad = true;
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
			if (!m_aRandomlySpawnedChildren.IsEmpty())
				m_SlotTask = GetSlotTask(m_aRandomlySpawnedChildren);
			else
				m_SlotTask = GetSlotTask(m_aChildren);
			
			if (!m_SlotTask)
			{
				Print(string.Format("ScenarioFramework: %1 could not init task due to missing m_SlotTask!", GetOwner().GetName()), LogLevel.ERROR);
				return;
			}
		}

		if (SetSupportEntity() && SetTaskPrefab() && CreateTask())
		{
			SetupTask();
			m_Task.SetLayerTask(this);

			m_SlotTask.OnTaskStateChanged(SCR_TaskState.OPENED);
			
			if (m_eLayerTaskState == SCR_TaskState.FINISHED)
			{
				m_bTaskResolvedBeforeLoad = true;
				m_SlotTask.SetTaskResolvedBeforeLoad(true);
				m_SupportEntity.FinishTask(m_Task);
			}
			else if (m_eLayerTaskState == SCR_TaskState.CANCELLED)
			{
				m_bTaskResolvedBeforeLoad = true;
				m_SlotTask.SetTaskResolvedBeforeLoad(true);
				m_SupportEntity.FailTask(m_Task);
			}
			else if (m_eLayerTaskState == SCR_TaskState.REMOVED)
			{
				m_bTaskResolvedBeforeLoad = true;
				m_SlotTask.SetTaskResolvedBeforeLoad(true);
				m_SupportEntity.CancelTask(m_Task.GetTaskID());
			}
			
			if (m_SlotTask.GetIsTerminated() || m_bIsTerminated)
				SCR_EntityHelper.DeleteEntityAndChildren(m_SlotTask.GetSpawnedEntity());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Creates a task for the support entity, sets its target faction, and moves it to the specified location.
	//! \return true if the task creation is successful, false otherwise.
	protected bool CreateTask()
	{
		m_Task = SCR_ScenarioFrameworkTask.Cast(m_SupportEntity.CreateTask(this));
		if (!m_Task)
		{
			Print(string.Format("ScenarioFramework: Creating of task %1 failed! Task manager refused to create it.", m_sTaskTitle), LogLevel.ERROR);
			return false;
		}
		//TODO: make selection of the faction dynamic
		array<Faction> aPlayableFactions = {};
		SCR_Faction factionSelected;
		FactionManager manager = GetGame().GetFactionManager();
		if (!manager)
		{
			Print(string.Format("ScenarioFramework: Creating of task %1 failed! Faction manager doesn't exist", m_sTaskTitle), LogLevel.ERROR);
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
	//! Sets up a task with optional title, description, and spawned entity name from a slot task, or prints an error message
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
			m_Task.m_sTaskIntroVoiceline = m_SlotTask.m_sTaskIntroVoiceline;
		}
		else
		{
			Print(string.Format("ScenarioFramework - LayerTask: Task Subject not found doesn't exist for %1. Task won't be spawned!", GetOwner().GetName()), LogLevel.ERROR);
		}

		s_OnTaskSetup.Invoke(m_Task);
		
		//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
		
		string cachedLanguage;
		WidgetManager.GetLanguage(cachedLanguage);
		if (cachedLanguage != "en_us")
			WidgetManager.SetLanguage("en_us");
		
		Print(string.Format("ScenarioFramework: -> LayerTask: SlotTask %1 - generating task %2. Description: %3", m_SlotTask.GetOwner().GetName(), WidgetManager.Translate(m_Task.GetTitle()), string.Format(WidgetManager.Translate(m_Task.GetDescription(), WidgetManager.Translate(m_Task.GetSpawnedEntityName())))), LogLevel.NORMAL);
		
		if (cachedLanguage != "en_us")
			WidgetManager.SetLanguage(cachedLanguage);
		
		//---- REFACTOR NOTE END ----
	}

	//------------------------------------------------------------------------------------------------
	//! Removes task from support entity and despawns if in edit mode or task is cancelled.
	void ~SCR_ScenarioFrameworkLayerTask()
	{
		if (SCR_Global.IsEditMode())
			return;
		
		DynamicDespawn(this);
		if (m_SupportEntity && m_Task)
			m_SupportEntity.CancelTask(m_Task.GetTaskID());
	}
}
