[EntityEditorProps(category: "GameScripted/ScenarioFramework/Layer", description: "")]
class SCR_ScenarioFrameworkLayerTaskClearAreaClass : SCR_ScenarioFrameworkLayerTaskClass
{
}

class SCR_ScenarioFrameworkLayerTaskClearArea : SCR_ScenarioFrameworkLayerTask
{	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskClearAreaSupportEntity))
		{
			Print("ScenarioFramework: Task Destroy support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}

		m_SupportEntity = SCR_ScenarioFrameworkTaskClearAreaSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskClearAreaSupportEntity));
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ScenarioFrameworkLayerTaskClearArea(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = SCR_ESFTaskType.CLEAR_AREA;
	}
}
