//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskClass : SCR_BaseTaskClass
{
};

//------------------------------------------------------------------------------------------------
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
	void SetTaskState(SCR_TaskState state)
	{
		SetState(state);
	}

	//------------------------------------------------------------------------------------------------
	void SetLayerTask(SCR_ScenarioFrameworkLayerTask layer)
	{
		m_LayerTask = layer;
	}

	//------------------------------------------------------------------------------------------------
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
		
		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager || !gameModeManager.IsMaster())
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
			gameModeManager.PopUpMessage(GetTitle(), "#AR-Tasks_StatusFinished-UC", m_TargetFaction.GetFactionKey());

		if (newState == SCR_TaskState.CANCELLED)
			gameModeManager.PopUpMessage(GetTitle(), "#AR-Tasks_StatusFailed-UC", m_TargetFaction.GetFactionKey());
	}
	
	//------------------------------------------------------------------------------------------------
	override void Finish(bool showMsg = true)
	{
		showMsg = SCR_FactionManager.SGetLocalPlayerFaction() == m_TargetFaction;
		super.Finish(showMsg);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTaskAsset(IEntity object)
	{
		m_Asset = object;
	}
	
	//------------------------------------------------------------------------------------------------
	void RehookTaskAsset(IEntity object)
	{
		m_Asset = object;
		
		if (m_SupportEntity)
			m_SupportEntity.SetTaskEntity(m_Asset);
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetAsset()
	{
		return m_Asset;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSlotTask(SCR_ScenarioFrameworkSlotTask slotTask)
	{
		m_SlotTask = slotTask;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkSlotTask GetSlotTask()
	{
		return m_SlotTask;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkTaskSupportEntity GetSupportEntity()
	{
		return m_SupportEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTaskExecutionBriefing(string text)
	{
		m_sTaskExecutionBriefing = text;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTaskExecutionBriefing()
	{ 
		return m_sTaskExecutionBriefing;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpawnedEntityName(string name) 
	{ 
		m_sSpawnedEntityName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetSpawnedEntityName() 
	{ 
		return m_sSpawnedEntityName;
	}
	
	//------------------------------------------------------------------------------------------------
	//Description fetch for the Task List
	override string GetTaskListTaskText()
	{
		return string.Format(WidgetManager.Translate(m_sDescription, m_sSpawnedEntityName));
	}
	
	//------------------------------------------------------------------------------------------------
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
	override void Serialize(ScriptBitWriter writer)
	{
		super.Serialize(writer);
		
		writer.WriteString(m_sTaskExecutionBriefing);
		writer.WriteString(m_sSpawnedEntityName);
	}
	
	//------------------------------------------------------------------------------------------------
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
			
			if (!m_Asset)
			{
				m_SupportEntity.CancelTask(this.GetTaskID());
				Print("ScenarioFramework: Task subject not found!", LogLevel.ERROR);
				return;
			}
			
			m_SupportEntity.SetTaskEntity(m_Asset);
		}
		
		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager)
			return;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetTaskManager() || GetTaskManager().IsProxy())
			return;

		Init();
	}
}
