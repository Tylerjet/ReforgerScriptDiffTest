[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionProcessVoicelineEnumAndString : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the enum to work with")]
	string m_sTargetEnum;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;

		array<SCR_BaseTask> tasks = {};
		taskManager.GetTasks(tasks);

		typename targetEnum = m_sTargetEnum.ToType();
		foreach (SCR_BaseTask task : tasks)
		{
			SCR_ScenarioFrameworkTask frameworkTask = SCR_ScenarioFrameworkTask.Cast(task);
			if (frameworkTask)
				scenarioFrameworkSystem.ProcessVoicelineEnumAndString(targetEnum, frameworkTask.m_sTaskIntroVoiceline)
		}
	}
}