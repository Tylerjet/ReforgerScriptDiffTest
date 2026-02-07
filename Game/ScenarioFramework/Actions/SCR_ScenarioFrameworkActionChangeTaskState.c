[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTaskState : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Which task to work with - Use GetTask")]
	ref SCR_ScenarioFrameworkGetTask		m_Getter;

	[Attribute("1", UIWidgets.ComboBox, "Task state", "", ParamEnumArray.FromEnum(SCR_TaskState))]
	SCR_TaskState m_eTaskState;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkTask task = SCR_ScenarioFrameworkTask.Cast(entityWrapper.GetValue());
		if (!task)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Task not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Task not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		SCR_ScenarioFrameworkTaskSupportEntity supportEntity = task.GetSupportEntity();
		if (!supportEntity)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Task support entity not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Task not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
		//Task system is a mess, so this is why we have to tackle it this abysmal way:
		if (m_eTaskState == SCR_TaskState.FINISHED)
			supportEntity.FinishTask(task);
		else if (m_eTaskState == SCR_TaskState.REMOVED)
			supportEntity.FailTask(task);
		else if (m_eTaskState == SCR_TaskState.CANCELLED)
			supportEntity.CancelTask(task.GetTaskID());
		else
			task.SetState(m_eTaskState);
		//---- REFACTOR NOTE END ----
		
		SCR_ScenarioFrameworkLayerTask layerTask = task.GetLayerTask();
		if (!layerTask)
			return;
		
		layerTask.SetLayerTaskState(m_eTaskState);
	}
}