[EntityEditorProps(category: "GameScripted/Tasks", description: "Transport task support entity.", color: "0 0 255 255")]
class SCR_ScenarioFrameworkTaskClearAreaSupportEntityClass: SCR_ScenarioFrameworkTaskSupportEntityClass
{
};

class SCR_ScenarioFrameworkTaskClearAreaSupportEntity : SCR_ScenarioFrameworkTaskSupportEntity
{
	//------------------------------------------------------------------------------------------------
	//! Spawns entity for scenario layer task.
	//! \param[in] layer Spawned entity layer for task creation.
	//! \return the spawned entity for the task layer.
	override SCR_BaseTask CreateTask(SCR_ScenarioFrameworkLayerTask layer)
	{
		m_Entity = layer.GetSpawnedEntity();
		return super.CreateTask();
	}
}