[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionInputOnTaskEventIncreaseCounter : SCR_ScenarioFrameworkActionInputBase
{
	[Attribute(desc: "Insert name of the task layer or leave empty for any task")];
	protected string			m_sTaskLayerName;
	
	[Attribute("1", UIWidgets.ComboBox, "Task state", "", ParamEnumArray.FromEnum(SCR_TaskState))];
	protected SCR_TaskState			m_eEventName;
	
	protected int 					m_iActionsInput;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLogicInput input)
	{	
		super.Init(input);
		SCR_GameModeSFManager gameModeComp = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeComp) 
			return;
			
		gameModeComp.GetOnTaskStateChanged().Insert(OnActivate);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \param[in] mask
	void OnActivate(SCR_BaseTask task, SCR_ETaskEventMask mask)
	{
		if (task.GetTaskState() != m_eEventName || !m_Input)
			return;
			
		SCR_ScenarioFrameworkLayerTask taskLayer;
		string sTaskLayerName = "";
		if (SCR_ScenarioFrameworkTask.Cast(task))
		{
			taskLayer = SCR_ScenarioFrameworkTask.Cast(task).GetLayerTask();
			if (taskLayer)
			{
				sTaskLayerName = taskLayer.GetOwner().GetName();
				if (taskLayer.GetLayerTaskResolvedBeforeLoad())
					return;
			}
		} 
		
		if (m_sTaskLayerName.IsEmpty() || m_sTaskLayerName == sTaskLayerName)
		{
			if (!(mask & SCR_ETaskEventMask.TASK_ASSIGNEE_CHANGED))
				m_Input.OnActivate(1, null);
				
			if (mask & SCR_ETaskEventMask.TASK_PROPERTY_CHANGED && !(mask & SCR_ETaskEventMask.TASK_CREATED) && !(mask & SCR_ETaskEventMask.TASK_FINISHED) && !(mask & SCR_ETaskEventMask.TASK_ASSIGNEE_CHANGED))
			{	
				if (taskLayer)
				{
					SCR_ScenarioFrameworkSlotTask slotTask = taskLayer.GetSlotTask();
					if (slotTask)
						slotTask.OnTaskStateChanged(m_eEventName)
				}
			}	
		}
	}	
}