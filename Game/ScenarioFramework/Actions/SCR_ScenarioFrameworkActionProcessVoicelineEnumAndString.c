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

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
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
				manager.ProcessVoicelineEnumAndString(targetEnum, frameworkTask.m_sTaskIntroVoiceline)
		}
	}
}