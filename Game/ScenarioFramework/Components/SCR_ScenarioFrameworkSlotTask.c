[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkSlotTaskClass : SCR_ScenarioFrameworkSlotBaseClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkSlotTask : SCR_ScenarioFrameworkSlotBase
{
	[Attribute(desc: "Name of the task in list of tasks", category: "Task")]		
	protected string 		m_sTaskTitle;
	
	[Attribute(desc: "Description of the task", category: "Task", )]			//TODO: make config, memory
	protected string 		m_sTaskDescription;
		
	[Attribute(defvalue: "1", desc: "What to do once task is finished", UIWidgets.Auto, category: "OnTaskFinish")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnFinished;
	
	[Attribute(defvalue: "1", desc: "What to do once task is created", UIWidgets.Auto, category: "OnTaskCreate")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnCreated;
	
	[Attribute(defvalue: "1", desc: "What to do once task is created", UIWidgets.Auto, category: "OnTaskFailed")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnFailed;
	
	[Attribute(defvalue: "1", desc: "What to do once task progressed", UIWidgets.Auto, category: "OnTaskProgress")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnProgress;
	
	[Attribute(defvalue: "1", desc: "What to do once task is updated", UIWidgets.Auto, category: "OnTaskUpdated")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnUpdated;
	
	
	
	protected SCR_ScenarioFrameworkLayerTask	m_TaskLayer;		//parent layer where the task is defined
	protected bool m_bTaskResolvedBeforeLoad;
	
	//------------------------------------------------------------------------------------------------
	void OnTaskStateChanged(SCR_TaskState newState)
	{
		if (newState == SCR_TaskState.OPENED)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnCreated)
			{
				action.OnActivate(null);
			}
		}			
		else if (newState == SCR_TaskState.FINISHED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFinished)
			{
				action.OnActivate(null);
			}
		}
		else if (newState == SCR_TaskState.CANCELLED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFailed)
			{
				action.OnActivate(null);
			}
		}
		else if (newState == SCR_TaskState.PROGRESSED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnProgress)
			{
				action.OnActivate(null);
			}
		}
		else
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnUpdated)
			{
				action.OnActivate(null);
			};
		}
	}
	
	//------------------------------------------------------------------------------------------------
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
			if (m_TaskLayer.GetTaskSubject())
			{
				PrintFormat("ScenarioFramework: Warning, another Task Subject is already set for the task layer %1", m_TaskLayer.GetOwner().GetName(), LogLevel.WARNING);
				return;
			}
			
			m_TaskLayer.SetTaskSubject(this);
			if (m_Entity)
				m_TaskLayer.SetEntity(m_Entity);
		}
		else
		{
			PrintFormat("ScenarioFramework: %1 could not be spawned and cannot bind it to task %2", m_sObjectToSpawn, m_TaskLayer.GetOwner().GetName(), LogLevel.ERROR);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	string GetTaskTitle(int iState = 0) 
	{ 
		return m_sTaskTitle;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTaskDescription(int iState = 0)
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
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT, bool bInit = true)
	{
		if (m_eActivationType != activation)
			return;
		
		bool tempTerminated = m_bIsTerminated;
		m_bIsTerminated = false;
		
		super.Init(area, activation);
		m_bIsTerminated = tempTerminated;
		StoreTaskSubjectToParentTaskLayer();
	}
			
}