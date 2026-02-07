[EntityEditorProps(category: "GameScripted/ScenarioFramework/Layer", description: "")]
class SCR_ScenarioFrameworkLayerTaskKillClass : SCR_ScenarioFrameworkLayerTaskClass
{
}

class SCR_ScenarioFrameworkLayerTaskKill : SCR_ScenarioFrameworkLayerTask
{
	protected ref EntitySpawnParams m_SpawnParams = new EntitySpawnParams();
	
	//------------------------------------------------------------------------------------------------
	//! Sets support entity.
	//! \return true if support entity is found and set, false otherwise.
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskKillSupportEntity))
		{
			Print("ScenarioFramework: Task Kill support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}
		m_SupportEntity = SCR_ScenarioFrameworkTaskKillSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskKillSupportEntity));
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ScenarioFrameworkLayerTaskKill(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = SCR_ESFTaskType.KILL;
	}
}
