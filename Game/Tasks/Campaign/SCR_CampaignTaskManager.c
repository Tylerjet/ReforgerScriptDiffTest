[EntityEditorProps(category: "GameScripted/Campaign", description: "Campaign specific task manager.", color: "0 0 255 255")]
class SCR_CampaignTaskManagerClass: SCR_BaseTaskManagerClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTaskManager : SCR_BaseTaskManager
{
	protected bool m_bInitialTasksGenerated = false;
	protected ref array<SCR_CampaignBase> m_aCheckedBases = {};
	
	//*********************//
	//PUBLIC STATIC METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	//! Returns the instance of this entity, also can create it when there's none. 
	static SCR_CampaignTaskManager GetCampaignTaskManagerInstance()
	{
		if (!GetTaskManager())
		{
			if (GetGame())
			{
				BaseWorld world = GetGame().GetWorld();
				if (world)
					SCR_CampaignTaskManager.Cast(GetGame().SpawnEntity(SCR_CampaignTaskManager, world));
			}
		}
		
		SCR_CampaignTaskManager castInstance = SCR_CampaignTaskManager.Cast(GetTaskManager());
		
		if (!castInstance)
			return null;
		
		return castInstance;
	}
	
	//******************************//
	//PUBLIC OVERRIDE MEMBER METHODS//
	//******************************//
	
	//------------------------------------------------------------------------------------------------
	override void OnTaskUpdate(SCR_BaseTask task)
	{
		super.OnTaskUpdate(task);
		
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (campaign && GetWorld().GetWorldTime() > 2000)
			campaign.RefreshAIWaypoints(task, task.GetTaskState());
	}
	
	//*********************//
	//PUBLIC MEMBER METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	void CreateCampaignDefendTask(notnull SCR_CampaignDefendTaskData taskData)
	{
		Rpc(RPC_CreateDefendTask, taskData);
	}
	
	//------------------------------------------------------------------------------------------------
	//!	This method is called when a new base has been captured.
	//! Creates recon task around a base + creates capture tasks for bases in the newly gained radio signal.
	void GenerateNewTask(SCR_CampaignBase capturedBase)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EXECUTE_TASKS))
			return;
#endif
		
		GenerateCaptureTasks(capturedBase);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates a new campaign task.
	SCR_CampaignTask CreateNewCampaignTask(SCR_CampaignTaskType type, SCR_CampaignBase base, Faction faction, bool newTask = false)
	{
		if (!base || !faction)
			return null;
		
		if (base.GetOwningFaction() == faction && type == SCR_CampaignTaskType.CAPTURE)
			return null;
		
		SCR_CampaignTask task = SCR_CampaignTask.Cast(SCR_BaseTaskData.SpawnTask(GetSupportedTaskByTaskType(SCR_CampaignTask), SCR_CampaignTask));
		if (!task)
			return null;
		
		task.SetTargetBase(base);
		task.SetTargetFaction(faction);
		task.SetType(type);
		
		/*SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (campaign && newTask && SCR_RespawnSystemComponent.GetLocalPlayerFaction() == faction)
		{
			string text = task.TASK_AVAILABLE_TEXT + " " + task.GetTitle();
			string baseName;
			
			if (task.GetTargetBase())
				baseName = task.GetTargetBase().GetBaseNameUpperCase();

			SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, param1: baseName, text2: task.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT, sound: UISounds.TASK_CREATED);
		}*/
		
		if (!IsProxy())
		{
			array<int> assignees = {};
			GetTaskAssigneeIDs(task, assignees);
			
			SCR_CampaignTaskData taskData = new SCR_CampaignTaskData();
			taskData.LoadDataFromTask(task);
			taskData.SetNew();
			Rpc(RPC_CreateCampaignTask, taskData, assignees);
		}
		
		return task;
	}
	
	//****************************//
	//PROTECTED MEMBER RPC METHODS//
	//****************************//
	
	//------------------------------------------------------------------------------------------------
	//! RPC executed on clients, creating a task based on given parameters
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_CreateDefendTask(SCR_CampaignDefendTaskData taskData)
	{
		if (!taskData)
			return;
		
		taskData.CreateTask();
	}
	
	//------------------------------------------------------------------------------------------------
	//! RPC executed on clients, creating a task based on given parameters
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_CreateCampaignTask(SCR_CampaignTaskData taskData , array<int> assignees)
	{
		if (!taskData)
			return;
		
		if (GetTask(taskData.GetTaskID()))
			return;
		
		taskData.CreateTask();
	}
	
	//------------------------------------------------------------------------------------------------
	//! RPC executed on clients, creating a task based on given parameters
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_CreateCampaignBuildingTask(SCR_CampaignBuildingTaskData taskData , array<int> assignees)
	{
		if (!taskData)
			return;
		
		if (GetTask(taskData.GetTaskID()))
			return;
		
		taskData.CreateTask();
	}
	
	//************************//
	//PROTECTED MEMBER METHODS//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateRecaptureTask(notnull SCR_CampaignBase capturedBase)
	{
		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		SCR_CampaignFaction owningFaction = capturedBase.GetOwningFaction();
		if (!owningFaction)
			return;
		
		SCR_CampaignFaction opposingFaction = factionManager.GetEnemyFaction(owningFaction);
		if (!opposingFaction)
			return;
		
		if (GetTask(capturedBase, opposingFaction, SCR_CampaignTaskType.CAPTURE))
			return;
		
		if (!capturedBase.IsBaseInFactionRadioSignal(opposingFaction))
			return;
		
		CreateNewCampaignTask(SCR_CampaignTaskType.CAPTURE, capturedBase, opposingFaction, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateInitialTasksForAllBases()
	{
		if (!m_bInitialized || m_bInitialTasksGenerated)
			return;
		
		m_bInitialTasksGenerated = true;
		
		RplComponent rplComponent = RplComponent.Cast(FindComponent(RplComponent));
		
		//On proxy we don't create any tasks, we skip the creation
		if (!rplComponent || rplComponent.IsProxy())
			return;
		
		//Get bases manager
		SCR_CampaignBaseManager manager = SCR_CampaignBaseManager.GetInstance();
		
		//Get all bases
		array<SCR_CampaignBase> bases = manager.GetBases();
		
		//Create start tasks for each base
		for (int i = bases.Count() - 1; i >= 0; i--)
		{
			// Create tasks for this base
			if (bases[i].GetOwningFaction())
				GenerateNewTask(bases[i]);
		}
		
		//Let the game mode know, the tasks are ready
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gamemode)
			gamemode.HandleOnTasksInitialized();
	}
	
	//***************************//
	//PUBLIC MEMBER EVENT METHODS//
	//***************************//
	
	//------------------------------------------------------------------------------------------------
	void OnAllBasesInitialized()
	{
		// Timeout added to allow for backend load process
		GetGame().GetCallqueue().CallLater(GenerateInitialTasksForAllBases, 2000);
	}
	
	//------------------------------------------------------------------------------------------------
	//! An event called when a base has been captured.
	void OnBaseCaptured(notnull SCR_CampaignBase capturedBase)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EXECUTE_TASKS))
			return;
#endif
		
		m_aCheckedBases.Clear();
		GenerateNewTask(capturedBase);
		
		if (capturedBase.GetType() != CampaignBaseType.SMALL)
			return;
		
		if (capturedBase.GetAllBaseServices(builtOnly: true) < 3) 
			GenerateNewBuildingTask(capturedBase);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBuildingTask GenerateNewBuildingTask(notnull SCR_CampaignBase base)
	{
		SCR_CampaignFaction faction = base.GetOwningFaction();
		if (!faction)
			return null;
				
		SCR_CampaignBuildingTask task = SCR_CampaignBuildingTask.Cast(SCR_BaseTaskData.SpawnTask(GetSupportedTaskByTaskType(SCR_CampaignBuildingTask), SCR_CampaignBuildingTask));
		if (!task)
			return null;
		
		task.SetTargetBase(base);
		task.SetTargetFaction(faction);
		
		/*SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (campaign && SCR_RespawnSystemComponent.GetLocalPlayerFaction() == faction)
		{
			string text = task.TASK_AVAILABLE_TEXT + " " + task.GetTitle();
			string baseName;
			
			if (task.GetTargetBase())
				baseName = task.GetTargetBase().GetBaseNameUpperCase();

			SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, param1: baseName, text2: task.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT, sound: UISounds.TASK_CREATED);
		}*/
		
		if (!IsProxy())
		{
			array<int> assignees = {};
			GetTaskAssigneeIDs(task, assignees);
			
			SCR_CampaignBuildingTaskData taskData = new SCR_CampaignBuildingTaskData();
			taskData.LoadDataFromTask(task);
			taskData.SetNew();
			Rpc(RPC_CreateCampaignBuildingTask, taskData, assignees);
		}
		
		return task;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the found task if it exists.
	//TODO: Remove this method after no longer needed.
	SCR_CampaignTask GetTask(SCR_CampaignBase targetBase, Faction targetFaction, SCR_CampaignTaskType type)
	{
		foreach (SCR_BaseTask task: m_aTaskList)
		{
			SCR_CampaignTask castTask = SCR_CampaignTask.Cast(task);
			
			if (!castTask)
				continue;
			
			if (castTask.GetType() == type && castTask.GetTargetBase() == targetBase && castTask.GetTargetFaction() == targetFaction)
				return castTask;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server side
	//! Called after a new base has been captured or a mobile HQ deployed
	//! Creates capture tasks for small, major and main bases in newly gained range
	void GenerateCaptureTasks(notnull IEntity entity)
	{
		SCR_CampaignTaskType type = SCR_CampaignTaskType.CAPTURE;
		array<SCR_CampaignBase> bases = new array<SCR_CampaignBase>();
		SCR_CampaignFaction faction;
		
		if (entity.Type() == SCR_CampaignBase)
		{
			// It's a base
			SCR_CampaignBase capturedBase = SCR_CampaignBase.Cast(entity);
			m_aCheckedBases.Insert(capturedBase);
			GenerateRecaptureTask(capturedBase);
			faction = capturedBase.GetOwningFaction();
			
			if (!faction)
				return;
			
			if (!capturedBase.IsBaseInFactionRadioSignal(faction))
				return;
			
			capturedBase.GetBasesInRange(CampaignBaseType.SMALL | CampaignBaseType.MAJOR | CampaignBaseType.MAIN | CampaignBaseType.RELAY, bases);
			array<SCR_CampaignBase> otherBases = {};
			
			foreach (SCR_CampaignBase base: bases)
			{
				if (GetTask(base, faction, type))
					continue;
				
				if (base.GetOwningFaction() == faction)
				{
					if (m_aCheckedBases.Contains(base))
						continue;
					
					GenerateCaptureTasks(base);
					continue;
				}
				
				otherBases.Clear();
				base.GetBasesInRange(capturedBase.GetType(), otherBases);
				
				if (!otherBases.Contains(capturedBase))
					continue;
				
				CreateNewCampaignTask(type, base, faction, true);
			}
		}
		else
		{
			// It's a mobile HQ
			SCR_CampaignMobileAssemblyComponent comp = SCR_CampaignMobileAssemblyComponent.Cast(entity.FindComponent(SCR_CampaignMobileAssemblyComponent));
			
			if (comp)
			{
				faction = comp.GetParentFaction();
				bases = comp.GetBasesInRange();
				
				foreach (SCR_CampaignBase base: bases)
				{
					if (GetTask(base, faction, type))
						continue;
					
					if (base.GetOwningFaction() == faction)
					{
						if (m_aCheckedBases.Contains(base))
							continue;
						
						GenerateCaptureTasks(base);
						continue;
					}
					
					CreateNewCampaignTask(type, base, faction, true);
				}
			}
		}
	}

	//************************//
	//CONSTRUCTOR & DESTRUCTOR//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		if (baseManager.AllBasesInitialized())
		{
			// Timeout added to allow for backend load process
			GetGame().GetCallqueue().CallLater(GenerateInitialTasksForAllBases, 2000);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignTaskManager(IEntitySource src, IEntity parent)
	{
		if (SCR_GameModeCampaignMP.NotPlaying())
			return;
		
		if (!g_TaskManagerInstance)
			g_TaskManagerInstance = this;
		
		//Register to script invokers
		SCR_GameModeCampaignMP.s_OnBaseCaptured.Insert(OnBaseCaptured);
		SCR_CampaignBaseManager.s_OnAllBasesInitialized.Insert(OnAllBasesInitialized);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTaskManager()
	{
		//Unregister from script invokers
		SCR_GameModeCampaignMP.s_OnBaseCaptured.Remove(OnBaseCaptured);
		SCR_CampaignBaseManager.s_OnAllBasesInitialized.Remove(OnAllBasesInitialized);
	}

};
