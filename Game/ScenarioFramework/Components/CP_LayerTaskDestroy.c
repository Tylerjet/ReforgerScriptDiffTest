[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_LayerTaskDestroyClass : CP_LayerTaskClass
{
	// prefab properties here
}

class CP_LayerTaskDestroy : CP_LayerTask
{	
	
	protected ref EntitySpawnParams m_pSpawnParams = new EntitySpawnParams;
	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_CP_TaskDestroySupportEntity))
		{
			Print("CP: Task Destroy support entity not found in the world, task won't be created!");
			return false;
		}
		m_pSupportEntity = SCR_CP_TaskDestroySupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CP_TaskDestroySupportEntity));
		return m_pSupportEntity != null;	
	}
	
	//------------------------------------------------------------------------------------------------
	void CP_LayerTaskDestroy(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = ESFTaskType.DESTROY;
	}	
}
