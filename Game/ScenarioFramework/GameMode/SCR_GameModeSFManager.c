[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_GameModeSFManagerClass : SCR_BaseGameModeComponentClass
{
}

class SCR_GameModeSFManager : SCR_BaseGameModeComponent
{	
	[Attribute("Available Tasks for the Scenario", category: "Tasks")];
	protected ref array<ref SCR_ScenarioFrameworkTaskType> m_aTaskTypesAvailable;
	
	[Attribute( defvalue: "3", desc: "Maximal number of tasks that can be generated", category: "Tasks" )];
	protected int m_iMaxNumberOfTasks;
	
	[Attribute(UIWidgets.Auto, desc: "Actions that will be activated after tasks are initialized", category: "Tasks")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase> m_aAfterTasksInitActions;
	
	[Attribute(desc: "List of Core Areas that are essential for the Scenario to spawn alongside Debug Areas", category: "Debug")];
	protected ref array<string> m_aCoreAreas;
	
	[Attribute(desc: "List of Areas that will be spawned (Optionally with desired Layer Task) as opposed to leaving it to random generation", category: "Debug")];
	protected ref array<ref SCR_ScenarioFrameworkDebugArea> m_aDebugAreas;
	
	[Attribute(desc: "Should the dynamic Spawn/Despawn based on distance from player characters be enabled for the whole GameMode?", category: "Dynamic Spawn/Despawn")];
	protected bool m_bDynamicDespawn;
	
	[Attribute(defvalue: "4", UIWidgets.Slider, params: "0 600 1", desc: "How frequently is dynamic spawn/despawn being checked in seconds", category: "Dynamic Spawn/Despawn")]
	protected int m_iUpdateRate;
	
	[Attribute(desc: "Config with voice over data for whole scenario", params: "conf class=SCR_VoiceoverData")]
	ResourceName m_sVoiceOverDataConfig;
	
	protected bool m_bMatchOver;
	protected int m_iCurrentlySpawnedLayerTasks;
	
	protected ref ScriptInvoker m_OnAllAreasInitiated;
	protected ref ScriptInvoker m_OnTaskStateChanged;
	
	protected SCR_ScenarioFrameworkLayerBase m_LastFinishedTaskLayer;
	protected SCR_BaseTask m_LastFinishedTask;
	EGameOverTypes m_eGameOverType = EGameOverTypes.COMBATPATROL_DRAW;
	
	ref array<SCR_ScenarioFrameworkArea> m_aAreas = {};
	ref array<SCR_ScenarioFrameworkArea> m_aSelectedAreas = {};
	ref array<SCR_ScenarioFrameworkLayerTask> m_aLayerTasksToBeInitialized = {};
	ref array<SCR_ScenarioFrameworkLayerTask> m_aLayerTasksForRandomization = {};
	ref array<int> m_aIntroVoicelineIndexes = {};
	ref array<string> m_aAreasTasksToSpawn = {};
	ref array<string> m_aLayersTaskToSpawn = {};
	ref array<SCR_ESFTaskType> m_aESFTaskTypesAvailable = {};
	ref array<SCR_ESFTaskType> m_aESFTaskTypeForRandomization = {};
	
	protected ref array<ref Tuple3<SCR_ScenarioFrameworkArea, vector, int>> m_aSpawnedAreas = {};
	protected ref array<ref Tuple3<SCR_ScenarioFrameworkArea, vector, int>> m_aDespawnedAreas = {};
	protected ref array<ref Tuple3<EntityID, bool, float>> m_aDebugShapesLayers = {};
	protected ref array<vector> m_aObservers = {};
	protected ref map<string, string> m_VariableMap = new map<string, string>;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pEntID
	//! \param[in] sSndName
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlaySoundOnEntity(EntityID pEntID, string sSndName)
	{
		if (!pEntID)
			return;	

		IEntity entity = GetGame().GetWorld().FindEntityByID(pEntID);
		if (!entity)
			return;

		SoundComponent pSndComp = SoundComponent.Cast(entity.FindComponent(SoundComponent));
		if (!pSndComp)
			return;

		pSndComp.SoundEvent(sSndName);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \param[in] sSndName
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
	//!
	//! \param[in] playerID
	//! \param[in] eventName
	//! \param[in] entityID
	void PlayIntroVoiceline(int playerID, string eventName, EntityID entityID)
	{
		Rpc(RpcDo_PlayIntroVoiceline, playerID, eventName, entityID);
		RpcDo_PlayIntroVoiceline(playerID, eventName, entityID);	
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] eventName
	//! \param[in] entityID
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlayIntroVoiceline(int playerID, string eventName, EntityID entityID)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;
		
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		if (playerID != playerController.GetPlayerId())
			return;
		
		IEntity entity;
		if (entityID)
		{
			entity = GetGame().GetWorld().FindEntityByID(entityID);
			if (!entity)
				return;
		}
		else
		{
			entity = playerManager.GetPlayerControlledEntity(playerID);
			if (!entity)
				return;
		}
		
		SignalsManagerComponent signalComp = SignalsManagerComponent.Cast(entity.FindComponent(SignalsManagerComponent));
		if (!signalComp)
			return;

		if (m_aIntroVoicelineIndexes.IsEmpty())
		{
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("PlanName"), 0);
		}
		else
		{
			int seed;
			foreach (int index : m_aIntroVoicelineIndexes)
			{
				seed += index;
			}
			
			Math.Randomize(seed);
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("PlanName"), Math.RandomIntInclusive(0, 2));
		}
		
		int indexCount = m_aIntroVoicelineIndexes.Count();
		
		if (indexCount > 0)
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective1"), m_aIntroVoicelineIndexes[0]);
		else
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective1"), 1);

		if (indexCount > 1)
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective2"), m_aIntroVoicelineIndexes[1]);
		else
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective2"), 2);
		
		if (indexCount > 2)
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective3"), m_aIntroVoicelineIndexes[2]);
		else
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective3"), 3);
		
		if (indexCount > 3)
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective4"), m_aIntroVoicelineIndexes[3]);
		else
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective4"), 4);
		
		if (indexCount > 4)
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective5"), m_aIntroVoicelineIndexes[4]);
		else
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective5"), 5);
		
		if (indexCount > 5)
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective6"), m_aIntroVoicelineIndexes[5]);
		else
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective6"), 6);
		
		SoundComponent soundComp = SoundComponent.Cast(entity.FindComponent(SoundComponent));
		if (!soundComp)
			return;
		
		soundComp.SoundEvent(eventName);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] objectID replication id of entity from which this sound event will be played
	//! \param[in] soundFile resource name of a sound file that contains desired event
	//! \param[in] soundEventName name of a sound event that will be used to play a sound
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlaySoundOnEntityPosition(RplId objectID, string soundFile, string soundEventName)
	{
		IEntity object = IEntity.Cast(Replication.FindItem(objectID));
		if(!object)
			return;

		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;

		SCR_AudioSourceConfiguration audioConfig = new SCR_AudioSourceConfiguration();
		audioConfig.m_sSoundProject = soundFile;
		audioConfig.m_sSoundEventName = soundEventName;
		audioConfig.m_eFlags = EAudioSourceConfigurationFlag.FinishWhenEntityDestroyed;
		soundManagerEntity.CreateAndPlayAudioSource(object, audioConfig);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays sound event on the position of provided entity
	//! \param[in] object where taht sound will be played
	//! \param[in] soundFile resource name of a sound file that contains desired event
	//! \param[in] soundEventName name of a sound event that will be used to play a sound
	void PlaySoundOnEntityPosition(IEntity object, string soundFile, string soundEventName)
	{
		RplId objectID = Replication.FindId(object);
		if (!objectID.IsValid())
			return;

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(soundFile))
			return;

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(soundEventName))
			return;

		if (IsMaster())
			Rpc(RpcDo_PlaySoundOnEntityPosition, objectID, soundFile, soundEventName);

		RpcDo_PlaySoundOnEntityPosition(objectID, soundFile, soundEventName);		
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_BaseTask GetLastFinishedTask()
	{
		return m_LastFinishedTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ScenarioFrameworkLayerBase GetLastFinishedTaskLayer()
	{
		return m_LastFinishedTaskLayer;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	array<SCR_ScenarioFrameworkArea> GetAreas()
	{
		return m_aAreas;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	void OnTaskCreated(SCR_BaseTask task)
	{
		Faction faction =  task.GetTargetFaction();
		if (faction)
			PopUpMessage(task.GetTitle(), "#AR-CampaignTasks_NewObjectivesAvailable-UC", faction.GetFactionKey());
		else
			PopUpMessage(task.GetTitle(), "#AR-CampaignTasks_NewObjectivesAvailable-UC");
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \param[in] mask
	void OnTaskUpdate(SCR_BaseTask task, SCR_ETaskEventMask mask)
	{
		if (!SCR_ScenarioFrameworkTask.Cast(task))
			return;

		Faction faction =  task.GetTargetFaction();
		
		if (task.GetTaskState() == SCR_TaskState.FINISHED)
		{
			m_LastFinishedTaskLayer = SCR_ScenarioFrameworkTask.Cast(task).GetLayerTask(); 
			m_LastFinishedTask = task;
		}
		
		if (mask & SCR_ETaskEventMask.TASK_PROPERTY_CHANGED && !(mask & SCR_ETaskEventMask.TASK_CREATED) && !(mask & SCR_ETaskEventMask.TASK_FINISHED) && !(mask & SCR_ETaskEventMask.TASK_ASSIGNEE_CHANGED))
		{
			if (faction)
				PopUpMessage(task.GetTitle(), "#AR-Workshop_ButtonUpdate", faction.GetFactionKey());
			else
				PopUpMessage(task.GetTitle(), "#AR-Workshop_ButtonUpdate");

			SCR_ScenarioFrameworkLayerTask taskLayer = SCR_ScenarioFrameworkTask.Cast(task).GetLayerTask();
			SCR_ScenarioFrameworkSlotTask subject = taskLayer.GetSlotTask();
			if (subject)
				subject.OnTaskStateChanged(SCR_TaskState.UPDATED);
		}

		GetOnTaskStateChanged().Invoke(task, mask);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] GameOverType
	void SetMissionEndScreen(EGameOverTypes GameOverType)
	{
		m_eGameOverType = GameOverType;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void Finish()
	{
		SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(m_eGameOverType, 0,0);
		
		m_bMatchOver = true;
		
		SCR_BaseGameMode.Cast(GetOwner()).EndGameMode(endData);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsMatchOver()
	{
		return m_bMatchOver;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnAllAreasInitiated()
	{
		if (!m_OnAllAreasInitiated)
			m_OnAllAreasInitiated = new ScriptInvoker();
		
		return m_OnAllAreasInitiated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnTaskStateChanged()
	{
		if (!m_OnTaskStateChanged)
			m_OnTaskStateChanged = new ScriptInvoker();
		
		return m_OnTaskStateChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] eTaskType
	//! \return
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
	//!
	//! \return
	bool IsMaster() // IsServer
	{
		RplComponent comp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!comp)
			return false;			//by purpose - debug

		return !comp.IsProxy();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] area
	void RegisterArea(SCR_ScenarioFrameworkArea area)
	{
		if (m_aAreas.Find(area) == -1)
			m_aAreas.Insert(area);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadHeaderSettings();
	
	//------------------------------------------------------------------------------------------------
	protected bool Init()
	{
		if (!IsMaster())
			return false;

		LoadHeaderSettings();
		
		if (m_aDebugAreas.IsEmpty())
		{
			foreach (SCR_ScenarioFrameworkArea area : m_aAreas)
			{
				if (area.GetDynamicDespawnEnabled())
					continue;
				
				area.Init();
			}
		}
		else
		{
			SCR_ScenarioFrameworkArea area;
			SCR_ScenarioFrameworkLayerTask layerTask;
			foreach (SCR_ScenarioFrameworkDebugArea debugArea : m_aDebugAreas)
			{
				area = debugArea.GetForcedArea();
				if (!area)
					continue;
				
				if (!m_aAreasTasksToSpawn.Contains(area.GetName()))
					m_aAreasTasksToSpawn.Insert(area.GetName());
				
				if (!area.GetDynamicDespawnEnabled())
					area.Init();
				
				layerTask = debugArea.GetForcedLayerTask();
				if (layerTask && !m_aLayersTaskToSpawn.Contains(layerTask.GetName()))
					m_aLayersTaskToSpawn.Insert(layerTask.GetName());
			}
			
			IEntity entity;
			foreach (string coreArea : m_aCoreAreas)
			{
				if (SCR_StringHelper.IsEmptyOrWhiteSpace(coreArea))
					continue;
			
				entity = GetGame().GetWorld().FindEntityByName(coreArea);
				if (!entity)
					continue;
				
				area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
				if (area && !area.GetDynamicDespawnEnabled())
					area.Init()
			}
		}
		
		SCR_ScenarioFrameworkLayerTask.s_OnTaskSetup.Insert(OnTaskCreated);	
		SCR_BaseTaskManager.s_OnTaskUpdate.Insert(OnTaskUpdate);
		
		//if someone registered for the event, then call it
		if (m_OnAllAreasInitiated)
			m_OnAllAreasInitiated.Invoke();
		
		PostInit();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PostInit()
	{
		GenerateTasks();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	// Spawns random Task based on available tasks and Areas that haven't spawned any
	void SpawnRandomTask()
	{
		// In case of more max tasks to be spawned but with less task types, we need to refill it again
		if (m_aESFTaskTypeForRandomization.IsEmpty())
			m_aESFTaskTypeForRandomization.Copy(m_aESFTaskTypesAvailable);
		
		if (m_aLayerTasksForRandomization.IsEmpty())
			return;
		
		Math.Randomize(-1);
		SCR_ScenarioFrameworkLayerTask layerTask = m_aLayerTasksForRandomization.GetRandomElement();
		if (!layerTask)
			return;
		
		// This filters out task types that were already spawned
		SCR_ESFTaskType taskType = layerTask.GetTaskType();
		if (!m_aESFTaskTypeForRandomization.Contains(taskType))
		{
			for (int i = m_aLayerTasksForRandomization.Count() - 1; i >= 0; i--)
			{
			    if (m_aLayerTasksForRandomization[i].GetTaskType() == taskType)
			        m_aLayerTasksForRandomization.Remove(i);
			}
			
			if (m_aESFTaskTypesAvailable.Contains(taskType))
				SpawnRandomTask();
		}
		m_aESFTaskTypeForRandomization.RemoveItem(taskType);
		
		//Spawning and setting necessary states
		SCR_ScenarioFrameworkArea area = layerTask.GetParentArea();
		m_aLayerTasksToBeInitialized.Insert(layerTask);
		layerTask.SetParentLayer(layerTask.GetParentLayer());
		layerTask.Init(area, SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT);
		area.SetLayerTask(layerTask);
		area.SetAreaSelected(true);
		m_aSelectedAreas.Insert(area);
		
		// Removing all Layer Tasks that have the same Task Type
		for (int i = m_aLayerTasksForRandomization.Count() - 1; i >= 0; i--)
		{
		    if (m_aLayerTasksForRandomization[i].GetTaskType() == taskType)
		        m_aLayerTasksForRandomization.Remove(i);
		}
		
		// Removing all Layer Tasks that are from the same Area of randomly selected Layer Task
		for (int i = m_aLayerTasksForRandomization.Count() - 1; i >= 0; i--)
		{
		    if (m_aLayerTasksForRandomization[i].GetParentArea() == area)
		        m_aLayerTasksForRandomization.Remove(i);
		}
		
		Print(string.Format("ScenarioFramework: Creating area %1 with Layer Task %2", area.GetOwner().GetName(), layerTask.GetOwner().GetName()), LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	// Main function responsible for selecting available tasks and spawning the areas related to the tasks
	void GenerateTasks()
	{
		if (m_aTaskTypesAvailable.IsEmpty())
		{
			Print("ScenarioFramework: Available tasks are empty, no new tasks will be generated.", LogLevel.NORMAL);
			return;
		}
		
		if (m_aAreas.IsEmpty())
		{
			Print("ScenarioFramework: There are no Areas to generate tasks from", LogLevel.NORMAL);
			return;
		}
		
		Print("ScenarioFramework: ---------------------- Generating tasks -------------------", LogLevel.NORMAL);
		
		if (!m_aLayersTaskToSpawn.IsEmpty())
		{
			for (int i = 0; i < m_aLayersTaskToSpawn.Count(); i++)
			{
				GenerateSingleTask(i);
			}
			
			PrepareLayerTasksAfterInit();
			Print("ScenarioFramework: ---------------------- Generation of tasks completed -------------------", LogLevel.NORMAL);
			
			//If counts are not the same, we want randomization to occur
			if (m_aAreasTasksToSpawn.Count() == m_aLayersTaskToSpawn.Count())
				return;
		}
		
		//Fetching all Layer Tasks from Areas
		array<SCR_ScenarioFrameworkLayerTask> layerTasksToRandomize = {};
		array<SCR_ScenarioFrameworkLayerTask> layerTasks = {};
		if (m_aDebugAreas.IsEmpty())
		{
			foreach (SCR_ScenarioFrameworkArea area : m_aAreas)
			{
				if (!area)
					continue;
				
				area.GetAllLayerTasks(layerTasks);
				layerTasksToRandomize.InsertAll(layerTasks);
			}
		}
		else
		{
			SCR_ScenarioFrameworkArea forcedArea;
			SCR_ScenarioFrameworkArea forcedLayerTask;
			foreach (SCR_ScenarioFrameworkDebugArea debugArea : m_aDebugAreas)
			{
				forcedArea = debugArea.GetForcedArea();
				if (!forcedArea)
					continue;
				
				//If debug area has LayerTask set, we don't want to put it for randomization
				if (debugArea.GetForcedLayerTask())
					continue;
				
				forcedArea.GetAllLayerTasks(layerTasks);
				layerTasksToRandomize.InsertAll(layerTasks);
			}
		}
		
		//Fetching available Task Types for generation based on type
		foreach (SCR_ScenarioFrameworkTaskType taskTypeClass : m_aTaskTypesAvailable)
		{
			m_aESFTaskTypesAvailable.Insert(taskTypeClass.GetTaskType());
		}
		
		//Removing Layer Tasks that don't have Task Type set in Available Task Types
		for (int i = layerTasksToRandomize.Count() - 1; i >= 0; i--)
		{
			if (!m_aESFTaskTypesAvailable.Contains(layerTasksToRandomize[i].GetTaskType()))
		        layerTasksToRandomize.Remove(i);
		}
		
		//Removing Layer Tasks that don't have activation type set to ON_TASKS_INIT
		for (int i = layerTasksToRandomize.Count() - 1; i >= 0; i--)
		{
		    if (layerTasksToRandomize[i].GetActivationType() != SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT)
		        layerTasksToRandomize.Remove(i);
		}
		
		//Creating a copy so we can work with these in loops
		m_aESFTaskTypeForRandomization.Copy(m_aESFTaskTypesAvailable);
		m_aLayerTasksForRandomization.Copy(layerTasksToRandomize);
		
		//Spawning desired number of tasks
		for (int i = 0; i < m_iMaxNumberOfTasks; i++)
		{
			//Layers Tasks are shrinked down and after certain number of generations, we need to refill it
			if (m_aLayerTasksForRandomization.IsEmpty())
			{
				m_aLayerTasksForRandomization.Copy(layerTasksToRandomize);
			
				foreach (SCR_ScenarioFrameworkArea selectedArea : m_aSelectedAreas)
				{
					for (int j = m_aLayerTasksForRandomization.Count() - 1; j >= 0; j--)
					{
					    if (m_aLayerTasksForRandomization[j].GetParentArea() == selectedArea)
						{
							m_aLayerTasksForRandomization.Remove(j);
							continue;
						}
					}
				}
			}
			
			SpawnRandomTask();
		}

		if (m_aLayerTasksToBeInitialized.Count() < m_iMaxNumberOfTasks)
			Print(string.Format("ScenarioFramework: Available areas do not have any other tasks to generate. Only %1 out of %2 was generated", m_aLayerTasksToBeInitialized, m_iMaxNumberOfTasks), LogLevel.NORMAL);

		PrepareLayerTasksAfterInit();
		Print("ScenarioFramework: ---------------------- Generation of tasks completed -------------------", LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create a new global variable at the scenario
	void CreateVariableValue(string key, string value)
	{
		m_VariableMap.Insert(key, value);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set a value to global variable at the scenario
	void SetVariableValue(string key, string value)
	{
		m_VariableMap.Set(key, value);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get value of given variable
	bool GetVariable(string key, out string value)
	{
		if(key.IsEmpty())
		{
			Print(string.Format("Variable %1 is not set in this scenario", key), LogLevel.NORMAL);
			return false;
		}
		
		return m_VariableMap.Find(key, value);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepares Layer Tasks that were selected by ON_TASK_INIT activation for invoking AfterTasksInitActions
	protected void PrepareLayerTasksAfterInit()
	{
		foreach (SCR_ScenarioFrameworkLayerTask layerTask : m_aLayerTasksToBeInitialized)
		{
			if (!layerTask)
				continue;
			
			layerTask.GetOnAllChildrenSpawned().Insert(CheckLayerTasksAfterInit);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if all Layer Tasks that were selected by ON_TASK_INIT activation for invoking AfterTasksInitActions are finished with spawning
	protected void CheckLayerTasksAfterInit(SCR_ScenarioFrameworkLayerBase layer)
	{
		m_iCurrentlySpawnedLayerTasks++;
		if (m_iCurrentlySpawnedLayerTasks == m_aLayerTasksToBeInitialized.Count())
			//Due to how Task System sometimes works, not everything is initialized right after the Layer Task so we need to wait a bit
			GetGame().GetCallqueue().CallLater(AfterLayerTasksInit, 1000);
	
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ProcessVoicelineEnumAndString(typename targetEnum, string targetString)
	{
		targetEnum = SCR_ECombatOps_Everon_Tasks;
		array<string> stringValues = {};
		SCR_Enum.GetEnumNames(targetEnum, stringValues);
	
		int index = stringValues.Find(targetString);
		if (index != -1)
			m_aIntroVoicelineIndexes.Insert(index)
	}
	
	//------------------------------------------------------------------------------------------------
	//! Executes AfterTasksInitActions after all Layer Tasks are finished spawning
	protected void AfterLayerTasksInit()
	{
		foreach (SCR_ScenarioFrameworkLayerTask layerTask : m_aLayerTasksToBeInitialized)
		{
			if (!layerTask)
				continue;
			
			layerTask.GetOnAllChildrenSpawned().Remove(CheckLayerTasksAfterInit);
			if (m_bDynamicDespawn && !layerTask.GetDynamicDespawnExcluded())
				layerTask.DynamicDespawn(null);
		}
		
		foreach (SCR_ScenarioFrameworkActionBase afterTasksInitActions : m_aAfterTasksInitActions)
		{
			afterTasksInitActions.Init(m_aAreas.GetRandomElement().GetOwner());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateSingleTask(int index)
	{
		if (!m_aLayersTaskToSpawn.IsIndexValid(index) || !m_aAreasTasksToSpawn.IsIndexValid(index))
			return;
		
		string targetLayer = m_aLayersTaskToSpawn[index];
		string targetArea = m_aAreasTasksToSpawn[index];
		
		IEntity layerEntity = GetGame().GetWorld().FindEntityByName(targetLayer);
		if (!layerEntity)
			return;
		
		SCR_ScenarioFrameworkLayerTask taskComponent = SCR_ScenarioFrameworkLayerTask.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!taskComponent)
			return;
		
		m_aLayerTasksToBeInitialized.Insert(taskComponent);
		
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
		Print(string.Format("ScenarioFramework: Creating area %1", area.GetOwner().GetName()), LogLevel.NORMAL);
		Print("ScenarioFramework: ---------------------------------------------------------------", LogLevel.NORMAL);
		m_iMaxNumberOfTasks--;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] eTaskType
	//! \return
	SCR_ScenarioFrameworkArea SelectRandomAreaByTaskType(SCR_ESFTaskType eTaskType)
	{
		if (m_aAreas.IsEmpty())
			return null;
		
		SCR_ScenarioFrameworkArea selectedArea;
		array<SCR_ScenarioFrameworkArea> aAreasCopy = {};
		aAreasCopy.Copy(m_aAreas);
		for (int i = 0, count = m_aAreas.Count(); i < count; i++)
		{
			Math.Randomize(-1);
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
	//! \param[in] child
	//! \return
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
	//! Prepares dynamic spawn/despawn for specific area (Intended for runtime usage)
	//! \param[in] area
	//! \param[in] staySpawned
	void PrepareAreaSpecificDynamicDespawn(SCR_ScenarioFrameworkArea area, bool staySpawned = false)
	{
		int despawnRange = area.GetDynamicDespawnRange();
			
		//If this method is called with staySpawned = false, area will be added to m_aDespawnedAreas and gets despawned
		if (!staySpawned)
		{
			m_aDespawnedAreas.Insert(new Tuple3<SCR_ScenarioFrameworkArea, vector, int>(area, area.GetOwner().GetOrigin(), (despawnRange * despawnRange)));
			area.DynamicDespawn(null);
		}
		else
		{
			m_aSpawnedAreas.Insert(new Tuple3<SCR_ScenarioFrameworkArea, vector, int>(area, area.GetOwner().GetOrigin(), (despawnRange * despawnRange)));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes dynamic spawn/despawn for specific area (Intended for runtime usage)
	//! \param[in] area
	//! \param[in] staySpawned
	void RemoveAreaSpecificDynamicDespawn(SCR_ScenarioFrameworkArea area, bool staySpawned = false)
	{
		int despawnRange = area.GetDynamicDespawnRange();
			
		//If this method is called with staySpawned = false, area will be despawned
		if (!staySpawned)
			area.DynamicDespawn(null);
		
		for (int i = m_aDespawnedAreas.Count() - 1; i >= 0; i--)
		{
			Tuple3<SCR_ScenarioFrameworkArea, vector, int> areaInfo = m_aDespawnedAreas[i];
			if (area == areaInfo.param1)
				m_aDespawnedAreas.Remove(i);
		}
		
		for (int i = m_aSpawnedAreas.Count() - 1; i >= 0; i--)
		{
			Tuple3<SCR_ScenarioFrameworkArea, vector, int> areaInfo = m_aSpawnedAreas[i];
			if (area == areaInfo.param1)
				m_aSpawnedAreas.Remove(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepares dynamic spawn/despawn
	protected void PrepareDynamicDespawn()
	{
		if (!m_bDynamicDespawn)
			return;
		
		GetOnAllAreasInitiated().Remove(PrepareDynamicDespawn);
		
		foreach (SCR_ScenarioFrameworkArea area : m_aAreas)
		{
			if (!area.GetDynamicDespawnEnabled())
				continue;
			
			int despawnRange = area.GetDynamicDespawnRange();
			m_aDespawnedAreas.Insert(new Tuple3<SCR_ScenarioFrameworkArea, vector, int>(area, area.GetOwner().GetOrigin(), (despawnRange * despawnRange)));
			
			area.DynamicDespawn(null);
		}
		
		GetGame().GetCallqueue().CallLater(CheckDistance, 1000 * m_iUpdateRate, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Refreshes array of player characters and checks which areas should spawn/despawn
	protected void CheckDistance()
	{
		m_aObservers.Clear();
		array<int> playerIds = {};
		PlayerManager playerManager = GetGame().GetPlayerManager();
		IEntity player;
		SCR_DamageManagerComponent damageManager;
		playerManager.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			player = playerManager.GetPlayerControlledEntity(playerId);
			if (!player)
				continue;
			
			damageManager = SCR_DamageManagerComponent.GetDamageManager(player);
			if (damageManager && damageManager.GetState() != EDamageState.DESTROYED)
				m_aObservers.Insert(player.GetOrigin());
		}
		
		DynamicSpawn();
		DynamicDespawn();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Goes over despawned areas and checks whether or not said area should spawn
	protected void DynamicSpawn()
	{
		for (int i = m_aDespawnedAreas.Count() - 1; i >= 0; i--)
		{
			Tuple3<SCR_ScenarioFrameworkArea, vector, int> areaInfo = m_aDespawnedAreas[i];
			foreach (vector observerPos : m_aObservers)
			{
				if (vector.DistanceSqXZ(observerPos, areaInfo.param2) < areaInfo.param3)
				{
					areaInfo.param1.DynamicReinit();
					m_aSpawnedAreas.Insert(areaInfo);
					m_aDespawnedAreas.Remove(i);
					break;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Goes over spawned areas and checks whether or not said area should despawn
	protected void DynamicDespawn()
	{
		for (int i = m_aSpawnedAreas.Count() - 1; i >= 0; i--)
		{
			Tuple3<SCR_ScenarioFrameworkArea, vector, int> areaInfo = m_aSpawnedAreas[i];
			bool observerInRange;
			foreach (vector observerPos : m_aObservers)
			{
				if (vector.DistanceSqXZ(observerPos, areaInfo.param2) < areaInfo.param3)
				{
					observerInRange = true;
					break;
				}
			}

			if (!observerInRange)
			{
				areaInfo.param1.DynamicDespawn(null);
				m_aDespawnedAreas.Insert(areaInfo);
				m_aSpawnedAreas.Remove(i);
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		GetOnAllAreasInitiated().Insert(PrepareDynamicDespawn);
		GetGame().GetCallqueue().CallLater(Init,1000,false); //TODO: make the init order properly (the init should start after all Areas are registered)
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int count;
		reader.ReadInt(count);
		
		EntityID id;
		bool draw;
		float radius;
		for (int i = 0; i < count; i++)
		{
			reader.ReadEntityId(id);
			reader.ReadBool(draw);
			reader.ReadFloat(radius);
			
			ManageLayerDebugShape(id, draw, radius, false);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		int count = m_aDebugShapesLayers.Count();
		writer.WriteInt(count);
		
		for (int i = 0; i < count; i++)
		{
			Tuple3<EntityID, bool, float> debugLayer = m_aDebugShapesLayers[i];
			writer.WriteEntityId(debugLayer.param1);
			writer.WriteBool(debugLayer.param2);
			writer.WriteFloat(debugLayer.param3);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] sTitle
	//! \param[in] sSubtitle
	//! \param[in] timeOut
	//! \param[in] factionKey
	//! \param[in] playerID
	void ShowHint(string sTitle, string sSubtitle, int timeOut, FactionKey factionKey = "", int playerID = -1)
	{
		Rpc(RpcDo_ShowHint, sTitle, sSubtitle, timeOut, factionKey, playerID);
		RpcDo_ShowHint(sTitle, sSubtitle, timeOut, factionKey, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] sTitle
	//! \param[in] sSubtitle
	//! \param[in] timeOut
	//! \param[in] factionKey
	//! \param[in] playerID
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_ShowHint(string sTitle, string sSubtitle, int timeOut, FactionKey factionKey, int playerID)
	{
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(factionKey))
		{
			if (SCR_FactionManager.SGetLocalPlayerFaction() != GetGame().GetFactionManager().GetFactionByKey(factionKey))
				return;
		}
		
		if (playerID != -1)
		{
			PlayerController playerController = GetGame().GetPlayerController();
			if (!playerController)
				return;
			
			if (playerID != playerController.GetPlayerId())
				return;
		}
		
		SCR_HintUIInfo info = SCR_HintUIInfo.CreateInfo(WidgetManager.Translate(sTitle), WidgetManager.Translate(sSubtitle), timeOut, 0, 0, true);
		if (info)
			SCR_HintManagerComponent.ShowHint(info);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] sTitle
	//! \param[in] sSubtitle
	//! \param[in] factionKey
	//! \param[in] playerID
	void PopUpMessage(string sTitle, string sSubtitle, FactionKey factionKey = "", int playerID = -1)
	{
		Rpc(RpcDo_PopUpMessage, sTitle, sSubtitle, factionKey, playerID);
		RpcDo_PopUpMessage(sTitle, sSubtitle, factionKey, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] sTitle
	//! \param[in] sSubtitle
	//! \param[in] factionKey
	//! \param[in] playerID
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PopUpMessage(string sTitle, string sSubtitle, FactionKey factionKey, int playerID)
	{
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(factionKey))
		{
			if (SCR_FactionManager.SGetLocalPlayerFaction() != GetGame().GetFactionManager().GetFactionByKey(factionKey))
				return;
		}
		
		if (playerID != -1)
		{
			PlayerController playerController = GetGame().GetPlayerController();
			if (!playerController)
				return;
			
			if (playerID != playerController.GetPlayerId())
				return;
		}
		
		SCR_PopUpNotification.GetInstance().PopupMsg(sTitle, text2: sSubtitle);
	}
	
	//------------------------------------------------------------------------------------------------
	void ManageLayerDebugShape(EntityID id, bool draw, float radius, bool runtime)
	{
		IEntity entity = GetGame().GetWorld().FindEntityByID(id);
		if (!entity)
			return;
			
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return;
		
		layer.m_bShowDebugShapesDuringRuntime = draw;
		layer.m_fDebugShapeRadius = radius;
		
		if (draw)
			layer.SetEventMask(layer.GetOwner(), EntityEvent.INIT | EntityEvent.FRAME);
		
		if (RplSession.Mode() != RplMode.Client)
			m_aDebugShapesLayers.Insert(new Tuple3<EntityID, bool, float>(id, draw, radius));
		
		if (runtime)
			Rpc(RpcDo_ManageLayerDebugShape, id, draw, radius);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_ManageLayerDebugShape(EntityID id, bool draw, float radius)
	{
		IEntity entity = GetGame().GetWorld().FindEntityByID(id);
		if (!entity)
			return;
			
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return;
		
		layer.m_bShowDebugShapesDuringRuntime = draw;
		layer.m_fDebugShapeRadius = radius;
		
		if (draw)
			layer.SetEventMask(layer.GetOwner(), EntityEvent.INIT | EntityEvent.FRAME);
	}
}
