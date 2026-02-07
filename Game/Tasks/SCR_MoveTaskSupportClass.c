[BaseContainerProps()]
class SCR_MoveTaskSupportClass : SCR_EditorTaskSupportClass
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskData CreateTaskData()
	{
		return new SCR_MoveTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MoveTaskSupportClass()
	{
		m_TypeName = SCR_MoveTask;
	}
};