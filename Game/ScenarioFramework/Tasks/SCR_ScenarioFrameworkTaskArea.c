class SCR_ScenarioFrameworkTaskAreaClass: SCR_ScenarioFrameworkTaskClass
{
};

class SCR_ScenarioFrameworkTaskArea : SCR_ScenarioFrameworkTask
{	
	protected SCR_BaseTriggerEntity 		m_Trigger;
		
	//------------------------------------------------------------------------------------------------
	//! Initializes Task and registers Trigger
	override void Init()
	{	
		super.Init();
		
		RegisterTrigger();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers trigger after rehooking task asset.
	//! \param[in] object that should be a Trigger
	override void RehookTaskAsset(IEntity object)
	{
		if (!object)
			return;
		
		super.RehookTaskAsset(object);
		
		RegisterTrigger();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers trigger entity for task area, handles missing trigger or support entity errors.
	void RegisterTrigger()
	{
		m_Trigger = SCR_BaseTriggerEntity.Cast(m_Asset);
		if (!m_Trigger)
		{
			if (!m_SupportEntity)
			{
				Print("ScenarioFramework: Task Area support entity not found in the world, task won't work properly!", LogLevel.ERROR);
				return;
			}
				
			m_SupportEntity.CancelTask(this.GetTaskID());
			Print("ScenarioFramework: Trigger not found! Make sure the IDs match in both task and Slot component", LogLevel.WARNING);
	 		return;
		}
		m_Trigger.GetOnActivate().Remove(OnTriggerActivated);
		m_Trigger.GetOnActivate().Insert(OnTriggerActivated);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finishes current task for support entity on trigger activation.
	void OnTriggerActivated()
	{
		m_SupportEntity.FinishTask(this);	
	}	
}
