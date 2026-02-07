class SCR_ScenarioFrameworkTaskClass : SCR_BaseTaskClass
{
};

class SCR_ScenarioFrameworkTask : SCR_BaseTask
{
	protected IEntity 												m_Asset;
	protected SCR_ScenarioFrameworkTaskSupportEntity		 		m_SupportEntity;
	protected SCR_ScenarioFrameworkLayerTask						m_LayerTask;
	protected SCR_ScenarioFrameworkSlotTask							m_SlotTask;
	protected string 												m_sTaskExecutionBriefing;
	string 															m_sTaskIntroVoiceline;
	protected string 												m_sSpawnedEntityName;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state sets state of the task.
	void SetTaskState(SCR_TaskState state)
	{
		SetState(state);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] layer Sets the layer task for this task.
	void SetLayerTask(SCR_ScenarioFrameworkLayerTask layer)
	{
		m_LayerTask = layer;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Layer task linked to this task.
	SCR_ScenarioFrameworkLayerTask GetLayerTask()
	{
		return m_LayerTask;
	}

	//------------------------------------------------------------------------------------------------
	//! An event called when the state of this task has been changed.
	override void OnStateChanged(SCR_TaskState previousState, SCR_TaskState newState)
	{
		if (!m_LayerTask)
			return;
		
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem || !scenarioFrameworkSystem.IsMaster())
			return;

		if (m_SlotTask)
		{
			m_SlotTask.OnTaskStateChanged(newState);
		}
		else
		{
			SCR_ScenarioFrameworkSlotTask slotTask = m_LayerTask.GetSlotTask();
			if (slotTask)
				slotTask.OnTaskStateChanged(newState);
			else
				Print("ScenarioFramework: Task Subject not found for task", LogLevel.ERROR);
		}
		
		m_LayerTask.OnTaskStateChanged(previousState, newState);
				
		if (newState == SCR_TaskState.FINISHED)
			scenarioFrameworkSystem.PopUpMessage(GetTitle(), "#AR-Tasks_StatusFinished-UC", m_TargetFaction.GetFactionKey());

		if (newState == SCR_TaskState.CANCELLED)
			scenarioFrameworkSystem.PopUpMessage(GetTitle(), "#AR-Tasks_StatusFailed-UC", m_TargetFaction.GetFactionKey());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finishes mission based on player's faction, shows message if player's faction matches target faction.
	//! \param[in] showMsg Determines if local player's faction matches target faction for message display.
	override void Finish(bool showMsg = true)
	{
		showMsg = SCR_FactionManager.SGetLocalPlayerFaction() == m_TargetFaction;
		super.Finish(showMsg);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] object Sets the asset for the task, represented by an entity object.
	void SetTaskAsset(IEntity object)
	{
		m_Asset = object;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates linked entity for this task.
	//! \param[in] object Updates blacklist for garbage system, sets task entity for support entity.
	void RehookTaskAsset(IEntity object)
	{
		if (!object)
			return;
		
		m_Asset = object;
		
		SCR_GarbageSystem garbageSystem = SCR_GarbageSystem.GetByEntityWorld(m_Asset);
		if (garbageSystem)
			garbageSystem.UpdateBlacklist(m_Asset, true);
		
		if (m_SupportEntity)
			m_SupportEntity.SetTaskEntity(m_Asset);
	}

	//------------------------------------------------------------------------------------------------
	//! \return Asset entity.
	IEntity GetAsset()
	{
		return m_Asset;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] slotTask that will be linked to this task.
	void SetSlotTask(SCR_ScenarioFrameworkSlotTask slotTask)
	{
		m_SlotTask = slotTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return linked slot task.
	SCR_ScenarioFrameworkSlotTask GetSlotTask()
	{
		return m_SlotTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Support entity.
	SCR_ScenarioFrameworkTaskSupportEntity GetSupportEntity()
	{
		return m_SupportEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] text Sets the briefing for task execution.
	void SetTaskExecutionBriefing(string text)
	{
		m_sTaskExecutionBriefing = text;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Task execution briefing string.
	string GetTaskExecutionBriefing()
	{ 
		return m_sTaskExecutionBriefing;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] name Represents the name for the spawned entity in the game world.
	void SetSpawnedEntityName(string name) 
	{ 
		m_sSpawnedEntityName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The name of the entity spawned by the method.
	string GetSpawnedEntityName() 
	{ 
		return m_sSpawnedEntityName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return formatted string with translated description and spawned entity name.
	override string GetTaskListTaskText()
	{
		return string.Format(WidgetManager.Translate(m_sDescription, m_sSpawnedEntityName));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets support entity for task.
	//! \return true if a support entity is found, false otherwise.
	protected bool SetSupportEntity()
	{
		m_SupportEntity = SCR_ScenarioFrameworkTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskSupportEntity));

		if (!m_SupportEntity)
		{
			Print("ScenarioFramework: Default Task support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}

		return m_SupportEntity != null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Serializes task execution briefing and spawned entity name.
	//! \param[in] writer Writer is used for serializing object data into a stream.
	override void Serialize(ScriptBitWriter writer)
	{
		super.Serialize(writer);
		
		writer.WriteString(m_sTaskExecutionBriefing);
		writer.WriteString(m_sSpawnedEntityName);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Deserializes task execution briefing and spawned entity name from ScriptBitReader.
	//! \param[in] reader Reads serialized data from the provided ScriptBitReader object, deserializing it into the object's fields.
	override void Deserialize(ScriptBitReader reader)
	{
		super.Deserialize(reader);
		
		string taskExecutionBriefing;
		//Reading task execution briefing
		reader.ReadString(taskExecutionBriefing);
		SetTaskExecutionBriefing(taskExecutionBriefing);
		
		string spawnedEntityName;
		//Reading spawned entity name
		reader.ReadString(spawnedEntityName);
		SetSpawnedEntityName(spawnedEntityName);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes support entity, sets task entity, updates blacklist if necessary.
	void Init()
	{
		if (SCR_Global.IsEditMode(this))
			return;

		SetSupportEntity();
		if (!m_SupportEntity)
			return;

		m_Asset = m_SupportEntity.GetTaskEntity();
		if (!m_Asset)
		{
			if (m_SlotTask)
				m_Asset = m_SlotTask.GetSpawnedEntity();
			
			if (m_Asset)
				m_SupportEntity.SetTaskEntity(m_Asset);
		}
		else
		{
			SCR_GarbageSystem garbageSystem = SCR_GarbageSystem.GetByEntityWorld(m_Asset);
			if (garbageSystem)
				garbageSystem.UpdateBlacklist(m_Asset, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes task manager if it exists, otherwise returns without action.
	//! \param[in] owner The owner represents the entity that initializes this method, typically an object in the game world.
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetTaskManager() || GetTaskManager().IsProxy())
			return;

		Init();
	}
}
