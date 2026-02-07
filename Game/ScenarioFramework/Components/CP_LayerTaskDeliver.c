[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_LayerTaskDeliverClass : CP_LayerTaskClass
{
	// prefab properties here
}

class CP_LayerTaskDeliver : CP_LayerTask
{	
	[Attribute(defvalue: "", desc: "Name of the task in list of tasks after item is possessed.", category: "Task")]		
	protected string 		m_sTaskTitleUpdated;
	
	[Attribute(defvalue: "", desc: "Description of the task after item is possessed", category: "Task", )]			//TODO: make config, memory
	protected string 		m_sTaskDescriptionUpdated;
	
	protected IEntity		m_pDeliverPoint;
	
	//------------------------------------------------------------------------------------------------
	override void SetEntity(IEntity pEnt)
	{
		m_pEntity = pEnt;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDeliveryPointEntity(IEntity pEnt)
	{
		m_pDeliverPoint = pEnt;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetDeliveryPointEntity() { return m_pDeliverPoint; }
	
	//------------------------------------------------------------------------------------------------
	override void SetupTask()
	{
		super.SetupTask();
		SCR_TaskDeliver.Cast(m_pTask).SetDeliveryTrigger(SCR_BaseTriggerEntity.Cast(m_pDeliverPoint));
		PrintFormat("CP: ->Task: The %1 is set as the Delivery point for task %2.", m_pDeliverPoint, GetTask().GetTitle());
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateTaskTitleAndDescription()
	{
		if (!m_sTaskTitleUpdated.IsEmpty()) 
			m_pTask.SetTitle(m_sTaskTitleUpdated);
		if (!m_sTaskDescriptionUpdated.IsEmpty()) 
			m_pTask.SetDescription(m_sTaskDescriptionUpdated);
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_CP_TaskDeliverSupportEntity))
		{
			Print("CP: Task Deliver support entity not found in the world, task won't be created!");
			return false;
		}
		m_pSupportEntity = SCR_CP_TaskDeliverSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CP_TaskDeliverSupportEntity));
		return m_pSupportEntity != null;	
	}
	
	//------------------------------------------------------------------------------------------------
	void CP_LayerTaskDeliver(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = ESFTaskType.DELIVER;
	}
}
