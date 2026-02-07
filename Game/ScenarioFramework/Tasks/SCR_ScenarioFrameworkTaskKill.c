class SCR_TaskKillClass: SCR_ScenarioFrameworkTaskClass
{
};

class SCR_TaskKill : SCR_ScenarioFrameworkTask
{	
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state Checks if object is not destroyed, has asset and support entity, then removes self from damage state change event and finishes the task
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Asset || !m_SupportEntity)
			return;
		
		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Asset);
		if (objectDmgManager)
	 		objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
				
		m_SupportEntity.FinishTask(this);				
	}
	
	//------------------------------------------------------------------------------------------------
	//! Rehooks task asset, gets damage manager, and subscribes to damage state changes.
	//! \param[in] object Rehooks task asset, gets damage manager, adds damage state change listener.
	override void RehookTaskAsset(IEntity object)
	{
		if (!object)
			return;
		
		super.RehookTaskAsset(object);
		
		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Asset);
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets support entity.
	//! \return true if support entity is found, false otherwise.
	override bool SetSupportEntity()
	{
		m_SupportEntity = SCR_ScenarioFrameworkTaskKillSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskKillSupportEntity));
		
		if (!m_SupportEntity)
		{
			Print("ScenarioFramework: Task Kill support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes damage manager component, subscribes to damage state changes.
	override void Init()
	{
		super.Init();
					
		if (!m_Asset)
			return;		
			
		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Asset);
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
	}
}
