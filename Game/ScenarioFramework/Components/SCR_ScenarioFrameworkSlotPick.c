[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkSlotPickClass : SCR_ScenarioFrameworkSlotTaskClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
/*!
	Class generated via ScriptWizard.
*/
class SCR_ScenarioFrameworkSlotPick : SCR_ScenarioFrameworkSlotTask
{
	
	//TODO: make title and description as Tuple2
	[Attribute(desc: "Name of the task in list of tasks (item picked up )", category: "Task")]		
	protected string 		m_sTaskTitleUpdated1;
	
	[Attribute(desc: "Description of the task (item picked up)", category: "Task",)]			//TODO: make config, memory
	protected string 		m_sTaskDescriptionUpdated1;
	
	[Attribute(desc: "Name of the task in list of tasks (item dropped)", category: "Task")]		
	protected string 		m_sTaskTitleUpdated2;
	
	[Attribute(desc: "Description of the task (item dropped)", category: "Task",)]
	protected string 		m_sTaskDescriptionUpdated2;
	
	
	//------------------------------------------------------------------------------------------------
	override string GetTaskTitle(int iState = 0) 
	{ 
		if (iState == 0)
			return super.GetTaskTitle();
		else if (iState == 1)
			return m_sTaskTitleUpdated1;	
		else if (iState == 2)
			return m_sTaskTitleUpdated2;
		else if (iState == 4)
			return super.GetTaskTitle();
		else if (iState == 5)
			return m_sTaskTitleUpdated1;
		else if (iState == 6)
			return m_sTaskTitleUpdated2; 
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetTaskDescription(int iState = 0)	
	{ 
		if (iState == 0)
			return super.GetTaskDescription();
		else if (iState == 1)
			return m_sTaskDescriptionUpdated1;	
		else if (iState == 2)
			return m_sTaskDescriptionUpdated2;
		else if (iState == 4)
			return super.GetTaskDescription();
		else if (iState == 5)
			return m_sTaskDescriptionUpdated1;
		else if (iState == 6)
			return m_sTaskDescriptionUpdated2; 
		return string.Empty;
	}	
	
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT, bool bInit = true)
	{
		if (m_eActivationType != activation)
			return;
			
		super.Init(area, activation);
		SCR_ScenarioFrameworkLayerTaskDeliver layer = SCR_ScenarioFrameworkLayerTaskDeliver.Cast(GetParentTaskLayer());
		if (!layer)
			return;

		string sTaskName;
		SCR_ScenarioFrameworkTask task = layer.GetTask();
		if (!task)
			sTaskName = layer.GetOwner().GetName();
		else
			sTaskName = task.GetTitle();
	}
};
