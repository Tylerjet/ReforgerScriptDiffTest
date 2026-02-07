[EntityEditorProps(category: "GameScripted/ScenarioFramework/Layer", description: "")]
class SCR_ScenarioFrameworkLayerTaskClearAreaClass : SCR_ScenarioFrameworkLayerTaskClass
{
}

class SCR_ScenarioFrameworkLayerTaskClearArea : SCR_ScenarioFrameworkLayerTask
{	
	//------------------------------------------------------------------------------------------------
	//! Sets support entity for scenario framework task clear area if found in world
	//! \return true if support entity is found and set, false otherwise.
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
