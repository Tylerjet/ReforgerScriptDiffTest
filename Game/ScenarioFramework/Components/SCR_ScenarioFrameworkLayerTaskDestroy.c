[EntityEditorProps(category: "GameScripted/ScenarioFramework/Layer", description: "")]
class SCR_ScenarioFrameworkLayerTaskDestroyClass : SCR_ScenarioFrameworkLayerTaskClass
{
}

class SCR_ScenarioFrameworkLayerTaskDestroy : SCR_ScenarioFrameworkLayerTask
{
	protected ref EntitySpawnParams m_SpawnParams = new EntitySpawnParams();
	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskDestroySupportEntity))
		{
			Print("ScenarioFramework: Task Destroy support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}
		m_SupportEntity = SCR_ScenarioFrameworkTaskDestroySupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskDestroySupportEntity));
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ScenarioFrameworkLayerTaskDestroy(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = SCR_ESFTaskType.DESTROY;
	}	
}
