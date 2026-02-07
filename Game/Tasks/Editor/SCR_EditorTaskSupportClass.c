[BaseContainerProps()]
class SCR_EditorTaskSupportClass: SCR_BaseTaskSupportClass
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskData CreateTaskData()
	{
		return new SCR_EditorTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_EditorTaskSupportClass()
	{
		m_TypeName = SCR_EditorTask;
	}
};