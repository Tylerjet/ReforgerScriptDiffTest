[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_GameModeCombatOpsManagerClass : SCR_GameModeSFManagerClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------

class SCR_GameModeCombatOpsManager : SCR_GameModeSFManager
{	
	//------------------------------------------------------------------------------------------------
	override void OnTaskFinished(SCR_BaseTask task)
	{
		super.OnTaskFinished(task);
		if (task == m_ExtractionAreaTask)
			Finish();
	}
	
	override void LoadHeaderSettings()
	{
		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (!header)
			return;
		
		SCR_MissionHeaderCombatOps headerCombatOps = SCR_MissionHeaderCombatOps.Cast(header);
		if (!headerCombatOps)
			return;
		
		if (headerCombatOps.m_iMaxNumberOfTasks != -1)
			m_iMaxNumberOfTasks = headerCombatOps.m_iMaxNumberOfTasks;
		
		if (!headerCombatOps.m_aTaskTypesAvailable.IsEmpty())
		{
			m_aTaskTypesAvailable.Clear();
			foreach (SCR_ScenarioFrameworkTaskType taskType : headerCombatOps.m_aTaskTypesAvailable)
			{
				m_aTaskTypesAvailable.Insert(taskType);
			}
		}
	};
	
	//------------------------------------------------------------------------------------------------
	override void PostInit()
	{
		//go throught the Areas	again, find appropriate ones and spawn only task layers
		if (m_sForcedTaskLayer.IsEmpty())		//for debug purposes
			GenerateTasks();
		/*else
			GenerateSingleTask();*/
	}
	
	//------------------------------------------------------------------------------------------------
	//
	protected void GenerateExtractionArea()
	{
		SCR_ESFTaskType eTaskType;
		SCR_ScenarioFrameworkTaskType frameworkTaskType;
		
		eTaskType = SCR_ESFTaskType.EXTRACTION;	
		//SCR_ScenarioFrameworkArea area = SelectRandomAreaByTaskType(eTaskType);		
		SCR_ScenarioFrameworkArea area = SelectNearestAreaByTaskType(eTaskType);		
		if (area)
		{
			/*
			m_ExtractionAreaTask = CreateTask(eTaskType, area, area.Create(eTaskType));
			if (m_ExtractionAreaTask)
			{
				area.StoreTaskToArea(m_ExtractionAreaTask);
				//area.MoveTaskIconToArea(m_ExtractionAreaTask);	
			}
			*/
		}
	}	
			
}
