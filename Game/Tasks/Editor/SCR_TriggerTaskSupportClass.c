[BaseContainerProps()]
class SCR_TriggerTaskSupportClass : SCR_EditorTaskSupportClass
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskData CreateTaskData()
	{
		return new SCR_TriggerTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_TriggerTaskSupportClass()
	{
		m_TypeName = SCR_TriggerTask;
	}
};