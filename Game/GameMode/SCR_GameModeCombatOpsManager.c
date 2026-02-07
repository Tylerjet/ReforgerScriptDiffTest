[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_GameModeCombatOpsManagerClass : SCR_GameModeSFManagerClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------

class SCR_GameModeCombatOpsManager : SCR_GameModeSFManager
{	
	//------------------------------------------------------------------------------------------------
	override void OnTaskFinished(SCR_BaseTask pTask)
	{
		super.OnTaskFinished(pTask);
		if (pTask == m_pExtractionAreaTask)
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
		
		m_aTaskTypesAvailable = headerCombatOps.m_aTaskTypesAvailable;
	};
	
	//------------------------------------------------------------------------------------------------
	protected void ShuffleTasks()
	{
		ESFTaskType eTaskType;
		CP_TaskType pTaskType;
		
		for (int i = 0; i < m_aTaskTypesAvailable.Count(); i++)
		{
			m_aTaskTypesAvailable.SwapItems(m_aTaskTypesAvailable.GetRandomIndex(), m_aTaskTypesAvailable.GetRandomIndex());
		}
		
		for (int i = 0; i < m_iMaxNumberOfTasks; i++)
		{
			m_iNumberOfSelectedAreas = 0;
			foreach (CP_Area area : m_aAreas)
			{
				if (area.GetIsAreaSelected())
					m_iNumberOfSelectedAreas++;
			}
			
			pTaskType = m_aTaskTypesAvailable.Get(i);
			eTaskType = pTaskType.GetTaskType();	
			if (eTaskType == ESFTaskType.NONE)
				continue;
			CP_Area pArea;
			if (m_sForcedArea.IsEmpty())
			{
				pArea = SelectRandomAreaByTaskType(eTaskType);		
			}
			else
			{
				IEntity pEnt = GetGame().GetWorld().FindEntityByName(m_sForcedArea);	//for debug
				if (pEnt)
				{
					pArea = CP_Area.Cast(pEnt.FindComponent(CP_Area));
				}
			}
			
			if (pArea)
			{
				CP_LayerTask pLayer = pArea.Create(eTaskType);
				PrintFormat("CP: Creating area %1", pArea.GetOwner().GetName());
				Print("CP: ---------------------------------------------------------------");
				m_aTaskTypesAvailable.RemoveItem(pTaskType);
				m_iMaxNumberOfTasks--;
				m_iNumberOfTasksSpawned++;
				i--;
			}
			else
			{
				foreach (CP_TaskType taskType : m_aTaskTypesAvailable)
				{
					eTaskType = taskType.GetTaskType();
					if (eTaskType == ESFTaskType.NONE)
						continue;
					if (m_sForcedArea.IsEmpty())
					{
						pArea = SelectRandomAreaByTaskType(eTaskType);
						if (pArea)
							break;	
					}
					else
					{
						IEntity pEnt = GetGame().GetWorld().FindEntityByName(m_sForcedArea);	//for debug
						if (pEnt)
						{
							pArea = CP_Area.Cast(pEnt.FindComponent(CP_Area));
						}
					}
				}
				if (pArea)
				{
					CP_LayerTask pLayer = pArea.Create(eTaskType);
					PrintFormat("CP: Creating area %1", pArea.GetOwner().GetName());
					Print("CP: ---------------------------------------------------------------");
					m_aTaskTypesAvailable.RemoveItem(pTaskType);
					m_iMaxNumberOfTasks--;
					m_iNumberOfTasksSpawned++;
					i--;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Main function responsible for selecting available tasks and spawning the areas related to the tasks
	override void GenerateTasks()
	{
		if (m_aTaskTypesAvailable.IsEmpty())
			return;
		
		int m_iMinNumberOfTasks = m_iMaxNumberOfTasks;
		
		array<int> aAllTaskTypesAvailable = {};
		if ( m_aAreas.Count() == 0)
		{
			Print("CP: There are no Areas to generate tasks from");
			return;
		}
		else
		{
			foreach (CP_Area area : m_aAreas)
			{
				array<ESFTaskType> aTempTaskTypes = {};
				area.GetAvailableTaskTypes(aTempTaskTypes);
				aAllTaskTypesAvailable.InsertAll(aTempTaskTypes);
			}
		}
		
		int maxPossibleShuffles = aAllTaskTypesAvailable.Count();
		if (maxPossibleShuffles == 0)
		{
			Print("CP: Available areas do not have any tasks to generate");
			return;
		}
		
		if (m_iMaxNumberOfTasks > m_aTaskTypesAvailable.Count())
			m_iMaxNumberOfTasks = m_aTaskTypesAvailable.Count();
		
		Print("CP: ---------------------- Generating tasks -------------------");
		while (m_iNumberOfTasksSpawned < m_iMinNumberOfTasks && m_aAreas.Count() > 0 && maxPossibleShuffles > 0)
		{
			if (m_iNumberOfTasksSpawned >= m_iNumberOfSelectedAreas && m_iNumberOfSelectedAreas != 0)
				break;
			
			ShuffleTasks();
			maxPossibleShuffles--;
		
		}
		Print("CP: ---------------------- Generation of tasks completed -------------------");
	
	}
		
	//------------------------------------------------------------------------------------------------
	// Mainly for debug purposes
	/*
	protected void GenerateSingleTask()
	{
		IEntity pEnt = GetGame().GetWorld().FindEntityByName(m_sForcedTaskLayer);
		if (!pEnt)
			return;
		CP_LayerTask pTaskComponent = CP_LayerTask.Cast(pEnt.FindComponent(CP_LayerTask));
		if (!pTaskComponent)
			return;
		IEntity pEntArea = GetGame().GetWorld().FindEntityByName(m_sForcedArea);
		if (!pEntArea)
		{
			pEntArea = pEnt.GetParent();
		}
		if (!pEntArea)
			return;		
		CP_Area pArea = CP_Area.Cast(pEntArea.FindComponent(CP_Area));
		if (!pArea)
			return;
		pArea.Create(pTaskComponent);
		PrintFormat("CP: Creating area %1", pArea.GetOwner().GetName());
		SCR_BaseTask pTask = CreateTask(pTaskComponent.GetTaskType(), pArea, pTaskComponent);
		if (!pTask)
			return;
		SetupTask(pArea, pTask)
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	CP_Area SelectRandomAreaByTaskType(ESFTaskType eTaskType)
	{
		if (m_aAreas.IsEmpty())
			return null;
		CP_Area pSelectedArea;
		array<CP_Area> aAreasCopy = {};
		aAreasCopy.Copy(m_aAreas);
		for (int i = 0; i < m_aAreas.Count(); i++)
		{
			int iSeed = Math.Randomize(-1);
			pSelectedArea = aAreasCopy.GetRandomElement();
			if (!pSelectedArea.GetIsAreaSelected() && pSelectedArea.GetIsTaskSuitableForArea(eTaskType))
			{
				pSelectedArea.SetAreaSelected(true);
				return pSelectedArea;
			}
			else
			{
				aAreasCopy.RemoveItem(pSelectedArea);
			}
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PostInit()
	{
		//go throught the Areas	again, find appropriate ones and spawn only task layers
		if (m_sForcedTaskLayer.IsEmpty())		//for debug purposes
			GenerateTasks();
		else
			GenerateSingleTask();
	}
	
	//------------------------------------------------------------------------------------------------
	//
	protected void GenerateExtractionArea()
	{
		ESFTaskType eTaskType;
		CP_TaskType pTaskType;
		
		eTaskType = ESFTaskType.EXTRACTION;	
		//CP_Area pArea = SelectRandomAreaByTaskType(eTaskType);		
		CP_Area pArea = SelectNearestAreaByTaskType(eTaskType);		
		if (pArea)
		{
			/*
			m_pExtractionAreaTask = CreateTask(eTaskType, pArea, pArea.Create(eTaskType));
			if (m_pExtractionAreaTask)
			{
				pArea.StoreTaskToArea(m_pExtractionAreaTask);
				//pArea.MoveTaskIconToArea(m_pExtractionAreaTask);	
			}
			*/
		}
	}	
			
}
