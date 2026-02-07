[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_SlotTaskClass : CP_SlotBaseClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
class CP_SlotTask : CP_SlotBase
{
	[Attribute(defvalue: "", desc: "Name of the task in list of tasks", category: "Task")]		
	protected string 		m_sTaskTitle;
	
	[Attribute(defvalue: "", desc: "Description of the task", category: "Task", )]			//TODO: make config, memory
	protected string 		m_sTaskDescription;
		
	[Attribute(defvalue: "1", desc: "What to do once task is finished", UIWidgets.Auto, category: "OnTaskFinish")];
	protected ref array<ref CP_ActionBase>	m_aActionsOnFinished;
	
	[Attribute(defvalue: "1", desc: "What to do once task is created", UIWidgets.Auto, category: "OnTaskCreate")];
	protected ref array<ref CP_ActionBase>	m_aActionsOnCreated;
	
	[Attribute(defvalue: "1", desc: "What to do once task is created", UIWidgets.Auto, category: "OnTaskFailed")];
	protected ref array<ref CP_ActionBase>	m_aActionsOnFailed;
	
	[Attribute(defvalue: "1", desc: "What to do once task progressed", UIWidgets.Auto, category: "OnTaskProgress")];
	protected ref array<ref CP_ActionBase>	m_aActionsOnProgress;
	
	[Attribute(defvalue: "1", desc: "What to do once task is updated", UIWidgets.Auto, category: "OnTaskUpdated")];
	protected ref array<ref CP_ActionBase>	m_aActionsOnUpdated;
	
	
	
	protected CP_LayerTask	m_pTaskLayer;		//parent layer where the task is defined
	
	//------------------------------------------------------------------------------------------------
	void OnTaskStateChanged(SCR_TaskState newState)
	{
		if (newState == SCR_TaskState.OPENED)
		{
			foreach (CP_ActionBase pAction : m_aActionsOnCreated)
				pAction.OnActivate(null);
		}			
		else if (newState == SCR_TaskState.FINISHED)
		{
			foreach (CP_ActionBase pAction : m_aActionsOnFinished)
				pAction.OnActivate(null);
		}
		else if (newState == SCR_TaskState.CANCELLED)
		{
			foreach (CP_ActionBase pAction : m_aActionsOnFailed)
				pAction.OnActivate(null);
		}
		else if (newState == SCR_TaskState.PROGRESSED)
		{
			foreach (CP_ActionBase pAction : m_aActionsOnProgress)
				pAction.OnActivate(null);
		}
		else
		{
			foreach (CP_ActionBase pAction : m_aActionsOnUpdated)
				pAction.OnActivate(null);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void StoreTaskSubjectToParentTaskLayer()
	{
		m_pTaskLayer = GetParentTaskLayer();
		if (m_pTaskLayer)
		{
			if (m_pTaskLayer.GetTaskSubject())
				PrintFormat("CP: Warning, another Task Subject is already set for the task layer %1", m_pTaskLayer.GetOwner().GetName());
			m_pTaskLayer.SetTaskSubject(this);
			if (m_pEntity)
				m_pTaskLayer.SetEntity(m_pEntity);
		}
		else
			PrintFormat("CP: %1 could not be spawned and cannot bind it to task %2", m_rObjectToSpawn, m_pTaskLayer.GetOwner().GetName());
	}
		
	//------------------------------------------------------------------------------------------------
	string GetTaskTitle(int iState = 0) { return m_sTaskTitle; }
	
	//------------------------------------------------------------------------------------------------
	string GetTaskDescription(int iState = 0)
	{ 
		if (!m_pEntity)
			return m_sTaskDescription;
		
		SCR_EditableEntityComponent editableEntityComp = SCR_EditableEntityComponent.Cast(m_pEntity.FindComponent(SCR_EditableEntityComponent));
		if (!editableEntityComp)
			return m_sTaskDescription;
		
		return string.Format(WidgetManager.Translate(m_sTaskDescription, editableEntityComp.GetDisplayName()));
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Get the Layer Task which is parent of this Slot 
	CP_LayerTask GetParentTaskLayer() 
	{ 
		CP_LayerTask pLayer;
		IEntity pEnt = GetOwner().GetParent();
		while (pEnt)
		{
			pLayer = CP_LayerTask.Cast(pEnt.FindComponent(CP_LayerTask));
			if (pLayer)
				return pLayer;
			
			pEnt = pEnt.GetParent();
		}
		
		return pLayer;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(CP_Area pArea = null, CP_EActivationType EActivation = CP_EActivationType.SAME_AS_PARENT, bool bInit = true)
	{
		if (m_EActivationType != EActivation)
			return;
		super.Init(pArea, EActivation);
		StoreTaskSubjectToParentTaskLayer();
	}
			
}