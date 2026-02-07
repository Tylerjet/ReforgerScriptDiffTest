//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskClass : SCR_BaseTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTask : SCR_BaseTask
{
	protected SCR_ScenarioFrameworkArea 							m_Area;
	protected IEntity 												m_Asset;
	protected SCR_ScenarioFrameworkTaskSupportEntity		 		m_SupportEntity;
	protected SCR_ScenarioFrameworkLayerTask						m_Layer;
	protected bool													m_bInitiated = false;

	//------------------------------------------------------------------------------------------------
	//!
	/*
	void ShowPopUpMessage(string subtitle)
	{
		if (!m_bInitiated)
			return;

		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager)
			return;
		else
		{
			gameModeManager.PopUpMessage(GetTitle(), subtitle);
		}
		//if (m_bInitiated)
			//SCR_PopUpNotification.GetInstance().PopupMsg(GetTitle(), text2: subtitle);
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	void SetTaskState(SCR_TaskState state)
	{
		SetState(state);
	}

	//------------------------------------------------------------------------------------------------
	void SetTaskLayer(SCR_ScenarioFrameworkLayerTask layer)
	{
		m_Layer = layer;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLayerTask GetTaskLayer()
	{
		return m_Layer;
	}

	//------------------------------------------------------------------------------------------------
	//! An event called when the state of this task has been changed.
	override void OnStateChanged(SCR_TaskState previousState, SCR_TaskState newState)
	{
		//super.OnStateChanged(previousState, newState);

		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager)
			return;

		if (newState == SCR_TaskState.FINISHED)
			gameModeManager.PopUpMessage(GetTitle(), "#AR-Tasks_StatusFinished-UC");

		if (newState == SCR_TaskState.CANCELLED)
			gameModeManager.PopUpMessage(GetTitle(), "#AR-Tasks_StatusFailed-UC");

		if (!gameModeManager.IsMaster() || !m_Layer)
			return;

		m_Layer.GetTaskSubject().OnTaskStateChanged(newState);
		m_Layer.OnTaskStateChanged(previousState, newState);
	}

	//------------------------------------------------------------------------------------------------
	void SetTaskSubject(IEntity object)
	{
		m_Asset = object;
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetTaskSubject()
	{
		return m_Asset;
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
	void Init()
	{
		if (SCR_Global.IsEditMode(this))
			return;

		//m_SupportEntity = SCR_ScenarioFrameworkTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskSupportEntity));
		SetSupportEntity();
		if (!m_SupportEntity)
			return;

		m_Asset = m_SupportEntity.GetTaskEntity();
		if (!m_Asset)
		{
			m_SupportEntity.CancelTask(this.GetTaskID());
			Print("ScenarioFramework: Task subject not found!", LogLevel.ERROR);
			return;
		}

		if (!m_SupportEntity)
			return;

		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager)
			return;

		m_bInitiated = true;
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
