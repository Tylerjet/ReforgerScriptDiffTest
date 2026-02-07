[EntityEditorProps(category: "GameScripted/Tasks", description: "Transport task support entity.", color: "0 0 255 255")]
class SCR_ScenarioFrameworkTaskSupportEntityClass: SCR_BaseTaskSupportEntityClass
{
};

class SCR_ScenarioFrameworkTaskSupportEntity : SCR_BaseTaskSupportEntity
{
	protected IEntity 	m_Entity;
	
	//------------------------------------------------------------------------------------------------
	//! Finishes scenario task, checks conditions, and calls superclass method.
	//! \param[in] task Finishes scenario framework task, checks conditions for completion.
	override void FinishTask(notnull SCR_BaseTask task)
	{
		SCR_ScenarioFrameworkTask scenarioFrameworkTask = SCR_ScenarioFrameworkTask.Cast(task);
		if (!scenarioFrameworkTask)
			return;
		
		SCR_ScenarioFrameworkSlotTask slotTask = scenarioFrameworkTask.GetSlotTask();
		if (!slotTask)
			return;
		
		SCR_ScenarioFrameworkLayerTask layerTask = scenarioFrameworkTask.GetLayerTask();
		if (!layerTask)
			return;
		
		IEntity owner = slotTask.GetOwner();
		if (!SCR_ScenarioFrameworkActivationConditionBase.EvaluateEmptyOrConditions(slotTask.m_eFinishConditionLogic, slotTask.m_aFinishConditions, owner))
			return;
		
		super.FinishTask(task);
		layerTask.SetLayerTaskState(SCR_TaskState.FINISHED);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! \return The return value represents the entity associated with this task.
	IEntity GetTaskEntity()
	{
		return m_Entity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entity Represents an object or entity in the game world assigned to this task.
	void SetTaskEntity(IEntity entity)
	{
		m_Entity = entity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns entity for scenario layer task and creates base task.
	//! \param[in] layer Spawned entity layer for task creation.
	//! \return the spawned entity for the layer task.
	SCR_BaseTask CreateTask(SCR_ScenarioFrameworkLayerTask layer)
	{
		m_Entity = layer.GetSpawnedEntity();
		return super.CreateTask();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] sTaskPrefab The method sets the prefab for a task.
	void SetTaskPrefab(ResourceName sTaskPrefab)
	{
		m_sTaskPrefab = sTaskPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the facton of the character if it exists, otherwise null.
	//! \param[in] unit Represents an in-game unit entity, used to determine its faction in the method.
	//! \return the character's faction.
	protected SCR_Faction GetCharacterFaction(IEntity unit)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(unit);
		if (!character)
			return null;

		return SCR_Faction.Cast(character.GetFaction());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets spawned entity name for a task in scenario framework.
	//! \param[in] task Sets spawned entity name for a scenario task.
	//! \param[in] name Sets the name for spawned entity in scenario task.
	void SetSpawnedEntityName(notnull SCR_ScenarioFrameworkTask task, string name)
	{
		int taskID = task.GetTaskID();
		RPC_SpawnedEntityName(taskID, name);
		Rpc(RPC_SpawnedEntityName, taskID, name);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets spawned entity name for a scenario task.
	//! \param[in] taskID TaskID represents the unique identifier for the scenario task in the task manager.
	//! \param[in] name Represents the name of an entity spawned in-game, used for identification purposes.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SpawnedEntityName(int taskID, string name)
	{
		if (!GetTaskManager())
			return;
		
		SCR_ScenarioFrameworkTask task = SCR_ScenarioFrameworkTask.Cast(GetTaskManager().GetTask(taskID));
		if (!task)
			return;
		
		task.SetSpawnedEntityName(name);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets task execution briefing for given task with description.
	//! \param[in] task Sets briefing for a specific task with given description.
	//! \param[in] description Sets the briefing description for a specific task in the scenario.
	void SetTaskExecutionBriefing(notnull SCR_ScenarioFrameworkTask task, string description)
	{
		int taskID = task.GetTaskID();
		RPC_TaskExecutionBriefing(taskID, description);
		Rpc(RPC_TaskExecutionBriefing, taskID, description);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets task execution briefing description for specified task ID.
	//! \param[in] taskID Task ID is an identifier for a specific task in the scenario, used to reference it in the method for setting its execution brief
	//! \param[in] description Sets briefing description for a specific task.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_TaskExecutionBriefing(int taskID, string description)
	{
		if (!GetTaskManager())
			return;
		
		SCR_ScenarioFrameworkTask task = SCR_ScenarioFrameworkTask.Cast(GetTaskManager().GetTask(taskID));
		if (!task)
			return;
		
		task.SetTaskExecutionBriefing(description);
	}
}