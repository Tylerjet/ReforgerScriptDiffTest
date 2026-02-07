//------------------------------------------------------------------------------------------------
class SCR_TaskDefendClass: CP_TaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TaskDefend : CP_Task
{	
	//------------------------------------------------------------------------------------------------
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;
		
		ScriptedDamageManagerComponent pObjectDmgManager = ScriptedDamageManagerComponent.Cast(m_pAsset.FindComponent(ScriptedDamageManagerComponent));
		if (pObjectDmgManager)
	 		pObjectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
				
		m_pSupportEntity.FailTask(this);				
	}
		
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_CP_TaskDefendSupportEntity))
		{
			Print("CP: Task Defend support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}
		m_pSupportEntity = SCR_CP_TaskDefendSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CP_TaskDefendSupportEntity));
		return m_pSupportEntity != null;	
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
					
		if (!m_pAsset)
			return;		
		ScriptedDamageManagerComponent pObjectDmgManager = ScriptedDamageManagerComponent.Cast(m_pAsset.FindComponent(ScriptedDamageManagerComponent));
		if (pObjectDmgManager)
			pObjectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
	}
}
