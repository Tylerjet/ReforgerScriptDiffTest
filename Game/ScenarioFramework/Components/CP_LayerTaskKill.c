[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_LayerTaskKillClass : CP_LayerTaskClass
{
	// prefab properties here
}

class CP_LayerTaskKill : CP_LayerTask
{	
	
	protected ref EntitySpawnParams m_pSpawnParams = new EntitySpawnParams;
	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_CP_TaskKillSupportEntity))
		{
			Print("CP: Task Kill support entity not found in the world, task won't be created!");
			return false;
		}
		m_pSupportEntity = SCR_CP_TaskKillSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CP_TaskKillSupportEntity));
		return m_pSupportEntity != null;	
	}
	
	//------------------------------------------------------------------------------------------------
	void CP_LayerTaskKill(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = ESFTaskType.KILL;
	}	
}
