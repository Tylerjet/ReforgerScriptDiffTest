[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotPickClass : SCR_ScenarioFrameworkSlotTaskClass
{
}

class SCR_ScenarioFrameworkSlotPick : SCR_ScenarioFrameworkSlotTask
{
	//TODO: make title and description as Tuple2
	[Attribute(desc: "Name of the task in list of tasks (item picked up )", category: "Task")]		
	protected string 		m_sTaskTitleUpdated1;
	
	[Attribute(desc: "Description of the task (item picked up)", category: "Task",)]			//TODO: make config, memory
	protected string 		m_sTaskDescriptionUpdated1;
	
	[Attribute(desc: "Name of the task in list of tasks (item dropped)", category: "Task")]		
	protected string 		m_sTaskTitleUpdated2;
	
	[Attribute(desc: "Description of the task (item dropped)", category: "Task",)]
	protected string 		m_sTaskDescriptionUpdated2;
	
	//------------------------------------------------------------------------------------------------
	override string GetTaskTitle(int iState = 0) 
	{ 
		if (iState == 0)
			return super.GetTaskTitle();
		else if (iState == 1)
			return m_sTaskTitleUpdated1;	
		else if (iState == 2)
			return m_sTaskTitleUpdated2;
		else if (iState == 4)
			return super.GetTaskTitle();
		else if (iState == 5)
			return m_sTaskTitleUpdated1;
		else if (iState == 6)
			return m_sTaskTitleUpdated2; 

		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;
		
		m_bDynamicallyDespawned = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetTaskDescription(int iState = 0)	
	{ 
		if (iState == 0)
			return super.GetTaskDescription();
		else if (iState == 1)
			return m_sTaskDescriptionUpdated1;	
		else if (iState == 2)
			return m_sTaskDescriptionUpdated2;
		else if (iState == 4)
			return super.GetTaskDescription();
		else if (iState == 5)
			return m_sTaskDescriptionUpdated1;
		else if (iState == 6)
			return m_sTaskDescriptionUpdated2;

		return string.Empty;
	}	
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (m_bInitiated)
		{
			StoreTaskSubjectToParentTaskLayer();
			
			if (m_ParentLayer)
				m_ParentLayer.CheckAllChildrenSpawned(this);
			
			GetOnAllChildrenSpawned().Insert(AfterAllChildrenSpawned);
			InvokeAllChildrenSpawned();
			return;
		}
		
		if (!m_bDynamicallyDespawned && activation != m_eActivationType)
			return;
		
		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : m_aActivationConditions)
		{
			//If just one condition is false, we don't continue and interrupt the init
			if (!activationCondition.Init(GetOwner()))
			{
				InvokeAllChildrenSpawned();
				return;
			}
		}
		
		super.Init(area, activation);
	}
	
	//------------------------------------------------------------------------------------------------
	override void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		super.AfterAllChildrenSpawned(this);
		
		if (!m_Entity || m_bCanBeGarbageCollected)
			return;

		auto garbageSystem = SCR_GarbageSystem.GetByEntityWorld(m_Entity);
		if (garbageSystem)
			garbageSystem.UpdateBlacklist(m_Entity, true);
	}
}
