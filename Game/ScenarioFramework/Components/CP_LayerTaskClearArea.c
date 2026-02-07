[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_LayerTaskClearAreaClass : CP_LayerTaskClass
{
	// prefab properties here
}

class CP_LayerTaskClearArea : CP_LayerTask
{	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_CP_TaskClearAreaSupportEntity))
		{
			Print("CP: Task Destroy support entity not found in the world, task won't be created!");
			return false;
		}
		m_pSupportEntity = SCR_CP_TaskClearAreaSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CP_TaskClearAreaSupportEntity));
		return m_pSupportEntity != null;	
	}
	
	//------------------------------------------------------------------------------------------------
	void CP_LayerTaskClearArea(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = ESFTaskType.CLEAR_AREA;
	}
}
