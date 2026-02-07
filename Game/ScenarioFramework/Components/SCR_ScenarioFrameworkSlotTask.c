[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotTaskClass : SCR_ScenarioFrameworkSlotBaseClass
{
}

class SCR_ScenarioFrameworkSlotTask : SCR_ScenarioFrameworkSlotBase
{
	[Attribute(desc: "Name of the task in list of tasks", category: "Task")]
	LocalizedString m_sTaskTitle;

	[Attribute(desc: "Description of the task", category: "Task")]
	LocalizedString m_sTaskDescription;

	[Attribute(desc: "Text for the Execution category in Briefing", category: "Task")]
	LocalizedString m_sTaskExecutionBriefing;

	[Attribute(desc: "StringID for the Intro Voiceline action to be processed. Processing must be setup after tasks are initialized.", category: "Task")]
	string m_sTaskIntroVoiceline;

	[Attribute(defvalue: "1", desc: "What to do once task is finished", UIWidgets.Auto, category: "OnTaskFinish")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnFinished;

	[Attribute(defvalue: "1", desc: "What to do once task is created", UIWidgets.Auto, category: "OnTaskCreate")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnCreated;

	[Attribute(defvalue: "1", desc: "What to do once task is created", UIWidgets.Auto, category: "OnTaskFailed")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnFailed;

	[Attribute(defvalue: "1", desc: "What to do once task progressed", UIWidgets.Auto, category: "OnTaskProgress")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnProgress;

	[Attribute(defvalue: "1", desc: "What to do once task is updated", UIWidgets.Auto, category: "OnTaskUpdated")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnUpdated;

	SCR_ScenarioFrameworkLayerTask	m_TaskLayer;		//parent layer where the task is defined
	bool m_bTaskResolvedBeforeLoad;
	bool m_bTempIsTerminated;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] newState
	void OnTaskStateChanged(SCR_TaskState newState)
	{
		if (newState == SCR_TaskState.OPENED)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnCreated)
			{
				action.OnActivate(GetOwner());
			}
		}
		else if (newState == SCR_TaskState.FINISHED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFinished)
			{
				action.OnActivate(GetOwner());
			}
		}
		else if (newState == SCR_TaskState.CANCELLED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFailed)
			{
				action.OnActivate(GetOwner());
			}
		}
		else if (newState == SCR_TaskState.PROGRESSED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnProgress)
			{
				action.OnActivate(GetOwner());
			}
		}
		else
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnUpdated)
			{
				action.OnActivate(GetOwner());
			};
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void SetTaskResolvedBeforeLoad(bool state)
	{
		m_bTaskResolvedBeforeLoad = state;
	}

	//------------------------------------------------------------------------------------------------
	protected void StoreTaskSubjectToParentTaskLayer()
	{
		m_TaskLayer = GetParentTaskLayer();
		if (m_TaskLayer)
		{
			m_TaskLayer.SetSlotTask(this);
			if (m_Entity)
				m_TaskLayer.SetEntity(m_Entity);
		}
		else
		{
			Print(string.Format("ScenarioFramework: %1 could not be spawned and cannot bind it to task %2", m_sObjectToSpawn, m_TaskLayer.GetOwner().GetName()), LogLevel.ERROR);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] iState
	//! \return
	LocalizedString GetTaskTitle(int iState = 0)
	{
		return m_sTaskTitle;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	LocalizedString GetTaskExecutionBriefing()
	{
		return m_sTaskExecutionBriefing;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] iState
	//! \return
	string GetTaskDescription(int iState = 0)
	{
			return m_sTaskDescription;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the Layer Task which is parent of this Slot
	SCR_ScenarioFrameworkLayerTask GetParentTaskLayer()
	{
		SCR_ScenarioFrameworkLayerTask layer;
		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			layer = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
			if (layer)
				return layer;

			entity = entity.GetParent();
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false)
	{
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActionsOnFinished)
		{
			activationAction.m_iNumberOfActivations = 0;
		}
		
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActionsOnCreated)
		{
			activationAction.m_iNumberOfActivations = 0;
		}
		
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActionsOnFailed)
		{
			activationAction.m_iNumberOfActivations = 0;
		}
		
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActionsOnProgress)
		{
			activationAction.m_iNumberOfActivations = 0;
		}
		
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActionsOnUpdated)
		{
			activationAction.m_iNumberOfActivations = 0;
		}
		
		m_TaskLayer = null;
		m_bTaskResolvedBeforeLoad = false;
		
		super.RestoreToDefault(includeChildren, reinitAfterRestoration);
	}

	//------------------------------------------------------------------------------------------------
	override void DynamicReinit()
	{
		Init(null, SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
	}

	//------------------------------------------------------------------------------------------------
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(DynamicDespawn);
		if (!m_Entity && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sObjectToSpawn))
		{
			GetOnAllChildrenSpawned().Insert(DynamicDespawn);
			return;
		}
		
		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;
		
		if (m_Entity)
		{
			m_vPosition = m_Entity.GetOrigin();
			InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Entity.FindComponent(InventoryItemComponent));
			if (invComp)
				invComp.m_OnParentSlotChangedInvoker.Remove(OnInventoryParentChanged);
		}
		
		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		SCR_EntityHelper.DeleteEntityAndChildren(m_Entity);
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool InitEntitySpawnCheck()
	{
		if (!m_Entity)
		{
			StoreTaskSubjectToParentTaskLayer();
			m_bIsTerminated = m_bTempIsTerminated;
		}
		
		super.InitEntitySpawnCheck();
		
		if (m_Entity)
			return true;
		else
			return false;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void FinishInit()
	{
		StoreTaskSubjectToParentTaskLayer();
		m_bIsTerminated = m_bTempIsTerminated;
		
		super.FinishInit();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		m_bTempIsTerminated = m_bIsTerminated;
		m_bIsTerminated = false;
		
		super.Init(area, activation);
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_ScenarioFrameworkSlotTask()
	{
		if (SCR_Global.IsEditMode())
			return;
		
		DynamicDespawn(this);
		
		if (!m_TaskLayer)
		{
			m_TaskLayer = GetParentTaskLayer();
			if (!m_TaskLayer || !m_TaskLayer.m_SupportEntity)
				return;
			
			SCR_ScenarioFrameworkTask task = m_TaskLayer.GetTask();
			if (task)
				m_TaskLayer.m_SupportEntity.CancelTask(task.GetTaskID());
		}
	}
}
