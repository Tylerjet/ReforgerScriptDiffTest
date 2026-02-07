[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_GameModeSFManagerClass : SCR_BaseGameModeComponentClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
enum SCR_ScenarioFrameworkEActivationType
{
	SAME_AS_PARENT = 0,
	ON_TRIGGER_ACTIVATION,
	ON_AREA_TRIGGER_ACTIVATION,
	ON_INIT,						//when the game mode is initiated
	ON_TASKS_INIT,					//when the  game mode starts creating tasks
	CUSTOM1,						//won't spawn until something will try to spawn the object with CUSTOM as parameter
	CUSTOM2,
	CUSTOM3,
	CUSTOM4,
};

enum SCR_ESFTaskType
{
	NONE,
	DELIVER,
	DESTROY,
	DEFEND,
	KILL,
	CLEAR_AREA,
	LAST,
	EXTRACTION,
	DEFAULT
};

class SCR_GameModeSFManager : SCR_BaseGameModeComponent
{	
	[Attribute("Available Tasks for the Scenario", category: "Tasks")];
	protected ref array<ref SCR_ScenarioFrameworkTaskType> m_aTaskTypesAvailable;
	
	[Attribute( defvalue: "3", desc: "Maximal number of tasks that can be generated", category: "Tasks" )];
	protected int 				m_iMaxNumberOfTasks;
	
	[Attribute(desc: "Name of the Area which will be only one to spawn", category: "Debug")];
	protected string			m_sForcedArea;
	[Attribute(desc: "Name of the task layer to be only one to create", category: "Debug")];
	protected string			m_sForcedTaskLayer;
	
	protected ref array<SCR_ScenarioFrameworkArea> m_aAreas = {};		//all areas will be registered into this array
	protected ref ScriptInvoker m_OnAllAreasInitiated;
	protected ref ScriptInvoker m_OnTaskStateChanged;
	protected SCR_BaseTask		m_ExtractionAreaTask;
	protected SCR_BaseTask		m_LastFinishedTask;
	protected SCR_ScenarioFrameworkLayerBase		m_LastFinishedTaskLayer;
	protected bool				m_bInitialized = false;
	protected bool 				m_bMatchOver;
	protected int 				m_iNumberOfTasksSpawned;
	protected int 				m_iNumberOfSelectedAreas;
	protected ref array<string> 	m_aAreasTasksToSpawn = {};
	protected ref array<string> 	m_aLayersTaskToSpawn = {};
	protected EGameOverTypes m_eGameOverType = EGameOverTypes.COMBATPATROL_DRAW;
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlaySoundOnEntity(EntityID pEntID, string sSndName)
	{
		if (!pEntID)
			return;	

		IEntity entity = GetGame().GetWorld().FindEntityByID(pEntID);
		if (!entity)
			return;

		SCR_CommunicationSoundComponent pSndComp = SCR_CommunicationSoundComponent.Cast(entity.FindComponent(SCR_CommunicationSoundComponent));
		if (!pSndComp)
			return;

		pSndComp.PlayStr(sSndName);
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaySoundOnEntity(IEntity entity, string sSndName)
	{
		if (!entity)
			entity = GetOwner();		//play it on game mode if any entity is passed

		if (!entity)
			return;

		if (IsMaster())
			Rpc(RpcDo_PlaySoundOnEntity, entity.GetID(), sSndName);
		
		RpcDo_PlaySoundOnEntity(entity.GetID(), sSndName);		
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaySoundOnPlayer(string sSndName)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		if (!pc)
			return;
		IEntity player = pc.GetMainEntity();
		
		if (!player)
			return;
		
		PlaySoundOnEntity(player, sSndName);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTask GetLastFinishedTask()
	{
		return m_LastFinishedTask;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLayerBase GetLastFinishedTaskLayer()
	{
		return m_LastFinishedTaskLayer;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskFinished(SCR_BaseTask task)
	{
		//SCR_ScenarioFrameworkTask.Cast(task).ShowPopUpMessage("#AR-Tasks_StatusFinished-UC");
		//GetOnTaskStateChanged().Invoke(task); 
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskCreated(SCR_BaseTask task)
	{
		PopUpMessage(task.GetTitle(), "#AR-CampaignTasks_NewObjectivesAvailable-UC");
		//GetOnTaskStateChanged().Invoke(task);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskCancelled(SCR_BaseTask task)
	{
		//GetOnTaskStateChanged().Invoke(task);
	}
	
	
	//------------------------------------------------------------------------------------------------
	void OnTaskFailed(SCR_BaseTask task)
	{
		//GetOnTaskStateChanged().Invoke(task);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskUpdate(SCR_BaseTask task, SCR_ETaskEventMask mask)
	{
		if (!task) 
			return;

		if (task.GetTaskState() == SCR_TaskState.FINISHED)
		{
			m_LastFinishedTaskLayer = SCR_ScenarioFrameworkTask.Cast(task).GetTaskLayer(); 
			m_LastFinishedTask = task;
		}
		
		if (mask & SCR_ETaskEventMask.TASK_PROPERTY_CHANGED && !(mask & SCR_ETaskEventMask.TASK_CREATED) && !(mask & SCR_ETaskEventMask.TASK_FINISHED) && !(mask & SCR_ETaskEventMask.TASK_ASSIGNEE_CHANGED))
		{
			PopUpMessage(task.GetTitle(), "#AR-Workshop_ButtonUpdate");
			
			SCR_ScenarioFrameworkLayerTask taskLayer = SCR_ScenarioFrameworkTask.Cast(task).GetTaskLayer();
			SCR_ScenarioFrameworkSlotTask subject = taskLayer.GetTaskSubject();
			if (subject)
				subject.OnTaskStateChanged(SCR_TaskState.UPDATED)
		}
		
		//SCR_ScenarioFrameworkTask.Cast(task).ShowPopUpMessage("#AR-Tasks_Objective" + " " + "#AR-Workshop_ButtonUpdate");		//TODO: localize properly
		GetOnTaskStateChanged().Invoke(task, mask);
	}
	
	void SetMissionEndScreen(EGameOverTypes GameOverType)
	{
		m_eGameOverType = GameOverType;
	}
	
	//------------------------------------------------------------------------------------------------
	void Finish()
	{
		SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(m_eGameOverType, 0,0);
		
		m_bMatchOver = true;
		
		SCR_BaseGameMode.Cast(GetOwner()).EndGameMode(endData);
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsMatchOver()
	{
		return m_bMatchOver;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAllAreasInitiated()
	{
		if (!m_OnAllAreasInitiated)
			m_OnAllAreasInitiated = new ScriptInvoker();
		
		return m_OnAllAreasInitiated;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnTaskStateChanged()
	{
		if (!m_OnTaskStateChanged)
			m_OnTaskStateChanged = new ScriptInvoker();
		
		return m_OnTaskStateChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkArea SelectNearestAreaByTaskType(SCR_ESFTaskType eTaskType)
	{
		if (m_aAreas.IsEmpty())
			return null;

		SCR_ScenarioFrameworkArea selectedArea = null;
		if (!m_LastFinishedTask)
			return null;

		vector vTaskPos = m_LastFinishedTask.GetOrigin();
		float fMinDistance = float.MAX;
		float fDistance = 0;
		for (int i = 0, count = m_aAreas.Count(); i < count; i++)
		{
			if (!m_aAreas[i].GetIsTaskSuitableForArea(eTaskType))
				continue;
			
			fDistance = vector.Distance(vTaskPos, m_aAreas[i].GetOwner().GetOrigin());
			if (fDistance < fMinDistance)
			{
				fMinDistance = fDistance;
				selectedArea = m_aAreas[i];
			}
		}
		return selectedArea;
	}

	
	//------------------------------------------------------------------------------------------------	
	bool IsMaster()// IsServer
	{
		RplComponent comp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!comp)
			return false;			//by purpose - debug

		return !comp.IsProxy();
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterArea(SCR_ScenarioFrameworkArea area)
	{
		if (m_aAreas.Find(area) == -1)
			m_aAreas.Insert(area);
	}
	
	//------------------------------------------------------------------------------------------------
	void StoreAreaStates(out notnull array<ref SCR_ScenarioFrameworkAreaStruct> outEntries)
	{
		if (!m_aAreas)
			return;
		
		for (int i = m_aAreas.Count() - 1; i >= 0; i--)
		{
			SCR_ScenarioFrameworkAreaStruct struct = new SCR_ScenarioFrameworkAreaStruct();
			m_aAreas[i].StoreState(struct);
			outEntries.Insert(struct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadAreaStates(notnull array<ref SCR_ScenarioFrameworkAreaStruct> loadedAreaStruct)
	{
		if (!m_aAreas)
			return;
		
		IEntity entity;
		SCR_ScenarioFrameworkArea area;
		foreach (SCR_ScenarioFrameworkAreaStruct areaStruct : loadedAreaStruct)
		{
			entity = GetGame().GetWorld().FindEntityByName(areaStruct.GetName());
			if (!entity)
				continue;

			area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (!area)
				continue;
			
			LoadAreaStructs(area, areaStruct);
			LoadNestedAreaStructs(areaStruct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadAreaStructs(SCR_ScenarioFrameworkArea area, SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetAreaSelected())
		{
			m_aAreasTasksToSpawn.Insert(areaStruct.GetName());
			m_aLayersTaskToSpawn.Insert(areaStruct.GetLayerTaskname());
		}
		
		if (areaStruct.GetDeliveryPointNameForItem())
			area.StoreDeliveryPoint(areaStruct.GetDeliveryPointNameForItem());
			
		if (areaStruct.GetRandomlySpawnedChildren())
			area.SetRandomlySpawnedChildren(areaStruct.GetRandomlySpawnedChildren());
			
		LoadRepeatedSpawnAreaStructs(area, areaStruct);
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadRepeatedSpawnAreaStructs(SCR_ScenarioFrameworkArea area, SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetEnableRepeatedSpawn())
		{
			area.SetEnableRepeatedSpawn(areaStruct.GetEnableRepeatedSpawn());
			if (areaStruct.GetRepeatedSpawnNumber())
				area.SetRepeatedSpawnNumber(areaStruct.GetRepeatedSpawnNumber());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadNestedAreaStructs(SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetLayerStructs())
			LoadLayer(areaStruct.GetLayerStructs());
			
		if (areaStruct.GetLogicStructs())
			LoadLogic(areaStruct.GetLogicStructs());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadLayer(notnull array<ref SCR_ScenarioFrameworkLayerStruct> loadedLayerStruct)
	{
		IEntity entity;
		SCR_ScenarioFrameworkLayerBase layer;
		foreach (SCR_ScenarioFrameworkLayerStruct layerStruct : loadedLayerStruct)
		{
			entity = GetGame().GetWorld().FindEntityByName(layerStruct.GetName());
			if (!entity)
				continue;
			
			layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
				continue;
			
			LoadLayerStructs(layer, layerStruct);
			LoadNestedLayerStructs(layerStruct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadLayerStructs(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layerStruct.GetIsTerminated())
		{
			layer.SetIsTerminated(layerStruct.GetIsTerminated());
			return;
		}
			
		LoadRepeatedSpawnLayerStructs(layer, layerStruct);
		LoadLayerStructSlots(layer, layerStruct);
			
		//Layer task handling
		SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layer);
		if (layerTask)
			layerTask.SetLayerTaskState(layerStruct.GetLayerTaskState());
			
		if (layerStruct.GetRandomlySpawnedChildren())
			layer.SetRandomlySpawnedChildren(layerStruct.GetRandomlySpawnedChildren());
			
		LoadNestedLayerStructs(layerStruct);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadRepeatedSpawnLayerStructs(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layerStruct.GetEnableRepeatedSpawn())
		{
			layer.SetEnableRepeatedSpawn(layerStruct.GetEnableRepeatedSpawn());
			if (layerStruct.GetRepeatedSpawnNumber())
				layer.SetRepeatedSpawnNumber(layerStruct.GetRepeatedSpawnNumber());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadLayerStructSlots(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		SCR_ScenarioFrameworkSlotBase slot;
		SCR_ScenarioFrameworkSlotAI slotAI;
		slot = SCR_ScenarioFrameworkSlotBase.Cast(layer.FindComponent(SCR_ScenarioFrameworkSlotBase));
		if (slot)
		{
			slotAI = SCR_ScenarioFrameworkSlotAI.Cast(slot.FindComponent(SCR_ScenarioFrameworkSlotAI));
			if (slotAI)
			{
				if (layerStruct.GetAIPrefabsForRemoval())
					slotAI.SetAIPrefabsForRemoval(layerStruct.GetAIPrefabsForRemoval());
			}
				
			if (layerStruct.GetRandomlySpawnedObject())
				slot.SetRandomlySpawnedObject(layerStruct.GetRandomlySpawnedObject());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadNestedLayerStructs(SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layerStruct.GetLayerStructs())
			LoadLayer(layerStruct.GetLayerStructs());
			
		if (layerStruct.GetLogicStructs())
			LoadLogic(layerStruct.GetLogicStructs());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadLogic(notnull array<ref SCR_ScenarioFrameworkLogicStruct> entries)
	{
		IEntity entity;
		SCR_ScenarioFrameworkLogic logic;
		SCR_ScenarioFrameworkLogicCounter logicCounter;
		foreach (SCR_ScenarioFrameworkLogicStruct logicInfo : entries)
		{
			entity = GetGame().GetWorld().FindEntityByName(logicInfo.GetName());
			if (!entity)
				continue;
			
			logic = SCR_ScenarioFrameworkLogic.Cast(entity.FindComponent(SCR_ScenarioFrameworkLogic));
			if (!logic)
				continue;
			
			if (logicInfo.GetIsTerminated())
			{
				logic.SetIsTerminated(logicInfo.GetIsTerminated());
				continue;
			}
			
			logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(logic.FindComponent(SCR_ScenarioFrameworkLogicCounter));
			if (!logicCounter)
				continue;
			
			if (logicInfo.GetCounterValue())
			{
				logicCounter.SetCounterValue(logicInfo.GetCounterValue());
				continue;
			}		
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadHeaderSettings();
	
	//------------------------------------------------------------------------------------------------
	protected bool Init()
	{
		if (!IsMaster())
			return false;

		LoadHeaderSettings();
		
		// Spawn everything inside the Area except the task layers
		foreach(SCR_ScenarioFrameworkArea area : m_aAreas)
		{
			if (m_sForcedArea.IsEmpty())		//for debug purposes
			{
				area.Init();	
			}
			else
			{
				if (area.GetOwner().GetName() == m_sForcedArea)
					area.Init(area);
			}
		}
				
		SCR_BaseTaskManager.s_OnTaskFinished.Insert(OnTaskFinished);		
		//SCR_BaseTaskManager.s_OnTaskCreated.Insert(OnTaskCreated);
		SCR_ScenarioFrameworkLayerTask.s_OnTaskSetup.Insert(OnTaskCreated);	
		SCR_BaseTaskManager.s_OnTaskCancelled.Insert(OnTaskCancelled);
		SCR_BaseTaskManager.s_OnTaskFailed.Insert(OnTaskFailed);
		SCR_BaseTaskManager.s_OnTaskUpdate.Insert(OnTaskUpdate);
		
		//if someone registered for the event, then call it
		if (m_OnAllAreasInitiated)
			m_OnAllAreasInitiated.Invoke();
		
		PostInit();
		
		m_bInitialized = true;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PostInit();
	
	//------------------------------------------------------------------------------------------------
	//! From all the available Tasks and number of which is supposed to spawn, it shuffles the order for random generation purposes.
	protected void ShuffleTasks()
	{
		SCR_ESFTaskType eTaskType;
		SCR_ScenarioFrameworkTaskType frameworkTaskType;
		
		for (int i = 0, count = m_aTaskTypesAvailable.Count(); i < count; i++)
		{
			m_aTaskTypesAvailable.SwapItems(m_aTaskTypesAvailable.GetRandomIndex(), m_aTaskTypesAvailable.GetRandomIndex());
		}
		
		for (int i = 0; i < m_iMaxNumberOfTasks; i++)
		{
			m_iNumberOfSelectedAreas = 0;
			foreach (SCR_ScenarioFrameworkArea area : m_aAreas)
			{
				if (area.GetIsAreaSelected())
					m_iNumberOfSelectedAreas++;
			}
			
			frameworkTaskType = m_aTaskTypesAvailable.Get(i);
			eTaskType = frameworkTaskType.GetTaskType();	
			if (eTaskType == SCR_ESFTaskType.NONE)
				continue;
			
			SCR_ScenarioFrameworkArea area;
			if (m_sForcedArea.IsEmpty())
				area = SelectRandomAreaByTaskType(eTaskType);		
			else
				SpawnForcedArea(area);
			
			if (area)
			{
				SpawnAreaWithTask(area, eTaskType, frameworkTaskType);
				i--;
			}
			
			else
			{
				foreach (SCR_ScenarioFrameworkTaskType taskTypeAvailable : m_aTaskTypesAvailable)
				{
					eTaskType = taskTypeAvailable.GetTaskType();
					if (eTaskType == SCR_ESFTaskType.NONE)
						continue;
					
					if (m_sForcedArea.IsEmpty())
					{
						area = SelectRandomAreaByTaskType(eTaskType);
						if (area)
							break;	
					}
					else
					{
						SpawnForcedArea(area);
					}
				}
				
				if (area)
				{
					SpawnAreaWithTask(area, eTaskType, frameworkTaskType);
					i--;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnAreaWithTask(SCR_ScenarioFrameworkArea area, SCR_ESFTaskType eTaskType, SCR_ScenarioFrameworkTaskType frameworkTaskType)
	{
		SCR_ScenarioFrameworkLayerTask layer = area.Create(eTaskType);
		PrintFormat("ScenarioFramework: Creating area %1", area.GetOwner().GetName());
		Print("ScenarioFramework: ---------------------------------------------------------------");
		m_aTaskTypesAvailable.RemoveItem(frameworkTaskType);
		m_iMaxNumberOfTasks--;
		m_iNumberOfTasksSpawned++;
	}
	
	//------------------------------------------------------------------------------------------------
	//! It allows you to spawn just one selected area out of all the areas you have in your scenario. 
	//! This is a helpful way for Scenario Creators to quickly test some desired area without having to disable everything else.
	void SpawnForcedArea(SCR_ScenarioFrameworkArea area)
	{
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sForcedArea);
		if (entity)
			area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
	}
	
	//------------------------------------------------------------------------------------------------
	// Main function responsible for selecting available tasks and spawning the areas related to the tasks
	void GenerateTasks()
	{
		if (m_aTaskTypesAvailable.IsEmpty())
		{
			Print("ScenarioFramework: Available tasks are empty, no new tasks will be generated.");
			return;
		}
		
		int m_iMinNumberOfTasks = m_iMaxNumberOfTasks;
		if (m_aTaskTypesAvailable.Count() < m_iMinNumberOfTasks)
			Print("ScenarioFramework: Number of available tasks is lower than the minimum number of tasks!", LogLevel.WARNING);
		
		array<int> aAllTaskTypesAvailable = {};
		if (m_aAreas.IsEmpty())
		{
			Print("ScenarioFramework: There are no Areas to generate tasks from");
			return;
		}
		else
		{
			foreach (SCR_ScenarioFrameworkArea area : m_aAreas)
			{
				array<SCR_ESFTaskType> aTempTaskTypes = {};
				area.GetAvailableTaskTypes(aTempTaskTypes);
				aAllTaskTypesAvailable.InsertAll(aTempTaskTypes);
			}
		}
		
		int maxPossibleShuffles = aAllTaskTypesAvailable.Count();
		if (maxPossibleShuffles == 0)
		{
			Print("ScenarioFramework: Available areas do not have any tasks to generate");
			return;
		}
		
		if (m_iMaxNumberOfTasks > m_aTaskTypesAvailable.Count())
			m_iMaxNumberOfTasks = m_aTaskTypesAvailable.Count();
		
		Print("ScenarioFramework: ---------------------- Generating tasks -------------------");
		
		if (m_aLayersTaskToSpawn.IsEmpty())
		{
			while (m_iNumberOfTasksSpawned < m_iMinNumberOfTasks && m_aAreas.Count() > 0 && maxPossibleShuffles > 0)
			{
				if (m_iNumberOfTasksSpawned >= m_iNumberOfSelectedAreas && m_iNumberOfSelectedAreas != 0)
					break;
				
				ShuffleTasks();
				maxPossibleShuffles--;
			
			}
		}
		else
		{
			int taskLayersCount = m_aLayersTaskToSpawn.Count();
			for (int i = 0; i < taskLayersCount; i++)
			{
				GenerateSingleTask(i);
			}
		}

		Print("ScenarioFramework: ---------------------- Generation of tasks completed -------------------");
	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateSingleTask(int index)
	{
		string targetLayer = m_aLayersTaskToSpawn[index];
		string targetArea = m_aAreasTasksToSpawn[index];
		
		IEntity layerEntity = GetGame().GetWorld().FindEntityByName(targetLayer);
		if (!layerEntity)
			return;
		
		SCR_ScenarioFrameworkLayerTask taskComponent = SCR_ScenarioFrameworkLayerTask.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!taskComponent)
			return;
		
		IEntity areaEntity = GetGame().GetWorld().FindEntityByName(targetArea);
		if (!areaEntity)
			areaEntity = layerEntity.GetParent();

		if (!areaEntity)
			return;
				
		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(areaEntity.FindComponent(SCR_ScenarioFrameworkArea));
		if (!area)
			return;

		area.SetAreaSelected(true);
		taskComponent.SetActivationType(SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT);
		
		area.Create(taskComponent);
		PrintFormat("ScenarioFramework: Creating area %1", area.GetOwner().GetName());
		Print("ScenarioFramework: ---------------------------------------------------------------");
		m_iMaxNumberOfTasks--;
		m_iNumberOfTasksSpawned++;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkArea SelectRandomAreaByTaskType(SCR_ESFTaskType eTaskType)
	{
		if (m_aAreas.IsEmpty())
			return null;
		
		SCR_ScenarioFrameworkArea selectedArea;
		array<SCR_ScenarioFrameworkArea> aAreasCopy = {};
		aAreasCopy.Copy(m_aAreas);
		for (int i = 0, count = m_aAreas.Count(); i < count; i++)
		{
			selectedArea = aAreasCopy.GetRandomElement();
			if (!selectedArea.GetIsAreaSelected() && selectedArea.GetIsTaskSuitableForArea(eTaskType))
			{
				selectedArea.SetAreaSelected(true);
				return selectedArea;
			}
			else
			{
				aAreasCopy.RemoveItem(selectedArea);
			}
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get parent area the object is nested into
	SCR_ScenarioFrameworkArea GetParentArea(IEntity child) 
	{ 
		if (!child)
			return null;
			
		SCR_ScenarioFrameworkArea layer;
		IEntity entity = child.GetParent();
		while (entity)
		{
			layer = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (layer)
				return layer;
			
			entity = entity.GetParent();
		}
		
		return layer;
	}
		
	//------------------------------------------------------------------------------------------------
	override void EOnFixedFrame(IEntity owner, float timeSlice);
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		GetGame().GetCallqueue().CallLater(Init,1000,false); //TODO: make the init order properly (the init should start after all Areas are registered)
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void PopUpMessage(string sTitle, string sSubtitle)
	{
		if (IsMaster())
			SCR_PopUpNotification.GetInstance().PopupMsg(sTitle, text2: sSubtitle);
		
		Rpc(RpcDo_PopUpMessage, sTitle, sSubtitle);
		
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PopUpMessage(string sTitle, string sSubtitle)
	{
		SCR_PopUpNotification.GetInstance().PopupMsg(sTitle, text2: sSubtitle);
	}
}
