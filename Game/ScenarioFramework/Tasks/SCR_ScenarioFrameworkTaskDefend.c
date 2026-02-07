//------------------------------------------------------------------------------------------------
class SCR_TaskDefendClass: SCR_ScenarioFrameworkTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TaskDefend : SCR_ScenarioFrameworkTask
{	
	//------------------------------------------------------------------------------------------------
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Asset || !m_SupportEntity)
			return;
		
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(m_Asset.FindComponent(ScriptedDamageManagerComponent));
		if (objectDmgManager)
	 		objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
				
		m_SupportEntity.FailTask(this);				
	}
		
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		m_SupportEntity = SCR_ScenarioFrameworkTaskDefendSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskDefendSupportEntity));
		
		if (!m_SupportEntity)
		{
			Print("ScenarioFramework: Task Defend support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
					
		if (!m_Asset)
			return;	
				
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(m_Asset.FindComponent(ScriptedDamageManagerComponent));
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
	}
}
