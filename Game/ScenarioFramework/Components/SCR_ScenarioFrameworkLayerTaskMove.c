[EntityEditorProps(category: "GameScripted/ScenarioFramework/Layer", description: "")]
class SCR_ScenarioFrameworkLayerTaskMoveClass : SCR_ScenarioFrameworkLayerTaskClass
{
}

class SCR_ScenarioFrameworkLayerTaskMove : SCR_ScenarioFrameworkLayerTask
{	
	//------------------------------------------------------------------------------------------------
	//! Sets support entity for scenario framework task move if found in world
	//! \return true if support entity is found and set, false otherwise.
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskExtractSupportEntity))
		{
			Print("ScenarioFramework: Task Extract support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}

		m_SupportEntity = SCR_ScenarioFrameworkTaskExtractSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskExtractSupportEntity));
		return true;
	}
}
