[EntityEditorProps(category: "GameScripted/Tasks", description: "Entity that takes care of managing tasks.", color: "0 0 255 255")]
class SCR_BaseTaskManagerClass: GenericEntityClass
{
};

SCR_BaseTaskManager g_TaskManagerInstance;
SCR_BaseTaskManager GetTaskManager()
{
	return g_TaskManagerInstance;
};

//------------------------------------------------------------------------------------------------
class SCR_BaseTaskManager : GenericEntity
{

#ifdef ENABLE_DIAG
	protected bool m_bPrintedTasks = false;
#endif
	static const float DEFAULT_ABANDON_WAITING_TIME = 15;
	
	//**************************//
	//PROTECTED MEMBER VARIABLES//
	//**************************//
	[Attribute()]
	ref array<ref SCR_BaseTaskSupportClass> m_aSupportedTaskTypes;
	
	[RplProp()]
	protected float m_fTimestamp = 0;
	protected float m_fTimestampTimer = 0;
	protected ref array<SCR_BaseTask> m_aTaskList = new ref array<SCR_BaseTask>();
	protected ref array<ref SCR_TaskAssignmentData> m_aCachedTaskAssignments = new ref array<ref SCR_TaskAssignmentData>();
	
	[Attribute("1")]
	protected float m_fAssigneeCacheTimer;
	
	[Attribute("#AR-Tasks_TitleLocalRequest-UC")]
	string m_sLocalRequestTitle;
	
	[Attribute("#AR-Tasks_ReassignPopup")]
	string m_sReassignPopup;
	
	[Attribute("#AR-Tasks_AssignPopupGM")]
	string m_sAssignPopupGM;
	
	protected float m_fAssigneeCacheCheckTimestamp;
	protected ref map<SCR_BaseTaskExecutor, float> m_mAssigneesAbandoned = new ref map<SCR_BaseTaskExecutor, float>();
	
	protected bool m_bInitialized = false;
	
	protected static SCR_FactionManager s_FactionManager;
	protected RplComponent m_RplComponent;
	
	protected SCR_RequestedTask m_LocallyRequestedTask;
	
	//*****************************//
	//PUBLIC STATIC SCRIPT INVOKERS//
	//*****************************//
	
	static ref ScriptInvoker s_OnTaskUpdate = new ref ScriptInvoker();
	static ref ScriptInvoker s_OnTaskFinished = new ref ScriptInvoker();
	static ref ScriptInvoker s_OnTaskFailed = new ref ScriptInvoker();
	static ref ScriptInvoker s_OnTaskRemoved = new ref ScriptInvoker();
	static ref ScriptInvoker s_OnTaskCancelled = new ref ScriptInvoker();
	
	static ref ScriptInvoker s_OnTaskAssigned = new ref ScriptInvoker();
	static ref ScriptInvoker s_OnTaskUnassigned = new ref ScriptInvoker();
	
	static ref ScriptInvoker s_OnTaskFactionAssigned = new ref ScriptInvoker();
	
	static ref ScriptInvoker s_OnTaskCreated = new ref ScriptInvoker();
	static ref ScriptInvoker s_OnTaskDeleted = new ref ScriptInvoker();
	
	static ref ScriptInvoker s_OnPeriodicalCheck2Second = new ref ScriptInvoker();
	static ref ScriptInvoker s_OnPeriodicalCheck5Second = new ref ScriptInvoker();
	static ref ScriptInvoker s_OnPeriodicalCheck60Second = new ref ScriptInvoker();
	
	//*********************//
	//PUBLIC STATIC METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	static bool HasRequestedTask(int playerID)
	{
		SCR_BaseTaskExecutor executor = SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID);
		if (!executor)
			return true;
		
		return GetTaskManager().FindRequestedTask(executor) != null;
	}
	
	//------------------------------------------------------------------------------------------------
	static int GetFactionIndex(Faction faction)
	{
		if (!s_FactionManager || !faction)
			return -1;
		
		return s_FactionManager.GetFactionIndex(faction);
	}
	
	//***************************//
	//PUBLIC EVENT MEMBER METHODS//
	//***************************//
	
	//------------------------------------------------------------------------------------------------
	//! An event called when a player disconnects
	//! Server only
	void OnPlayerDisconnected(int id)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EXECUTE_TASKS))
			return;
#endif
		
		SCR_BaseTaskExecutor executor = SCR_BaseTaskExecutor.GetTaskExecutorByID(id);
		SCR_BaseTask task = null;
		if (executor)
			task = executor.GetAssignedTask();
		
		if (task)
			UnassignTask(task, executor, SCR_EUnassignReason.ASSIGNEE_DISCONNECT);
		
		task = FindRequestedTask(executor);
		
		if (task)
			CancelTask(task.GetTaskID());
	}
	
	//------------------------------------------------------------------------------------------------
	//! An event called when a task is deleted.
	void OnTaskDeleted(SCR_BaseTask task)
	{
		if (m_aTaskList.Find(task) != -1)
			m_aTaskList.RemoveItem(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//! An event called when a new task is created.
	void OnTaskCreated(SCR_BaseTask task)
	{
		if (m_aTaskList.Find(task) == -1)
			m_aTaskList.Insert(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//! An event called whenever any task has been changed, updated, created.
	void OnTaskUpdate(SCR_BaseTask task)
	{
		s_OnTaskUpdate.Invoke(task);
	}
	
	//************************//
	//PROTECTED MEMBER METHODS//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	//! Checks if this entity is locally owned
	//! Public because it's also used by tasks as they don't have any RplComponents themselves
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PeriodicalCheck2Second()
	{
		s_OnPeriodicalCheck2Second.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PeriodicalCheck5Second()
	{
		s_OnPeriodicalCheck5Second.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PeriodicalCheck60Second()
	{
		s_OnPeriodicalCheck60Second.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitializeSupportedTasks()
	{
		if (!m_aSupportedTaskTypes)
			return;
		
		for (int i = m_aSupportedTaskTypes.Count() - 1; i >= 0; i--)
		{
			m_aSupportedTaskTypes[i].Initialize();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_RefuelTask CreateNewRefuelTask(int requesterID, IEntity targetVehicle = null)
	{
		SCR_BaseTaskSupportClass supportClass = GetTaskManager().GetSupportedTaskByTaskType(SCR_RefuelTask);
		if (!supportClass)
			return null;
		
		SCR_RefuelTask refuelTask = SCR_RefuelTask.Cast(GetGame().SpawnEntityPrefab(supportClass.GetTaskPrefab(), GetWorld()));
		if (!refuelTask)
			return null;
		
		SCR_BaseTaskExecutor requester = SCR_BaseTaskExecutor.GetTaskExecutorByID(requesterID);
		
		refuelTask.SetRequester(requester);
		refuelTask.SetTargetVehicle(targetVehicle);
		
		return refuelTask;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_EvacuateTask CreateNewEvacuationTask(int requesterID)
	{
		SCR_BaseTaskSupportClass supportClass = GetTaskManager().GetSupportedTaskByTaskType(SCR_EvacuateTask);
		if (!supportClass)
			return null;
		
		SCR_EvacuateTask evacuateTask = SCR_EvacuateTask.Cast(GetGame().SpawnEntityPrefab(supportClass.GetTaskPrefab(), GetWorld()));
		if (!evacuateTask)
			return null;
		
		SCR_BaseTaskExecutor requester = SCR_BaseTaskExecutor.GetTaskExecutorByID(requesterID);
		
		evacuateTask.SetRequester(requester);
		
		return evacuateTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks every task for assignee timeout
	protected void CheckAssigneeTimeout()
	{
		foreach (SCR_BaseTask task : m_aTaskList)
		{
			if (task && task.IsIndividual())
				task.CheckAssigneeTimeout();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Assigns cached tasks if any
	protected void AssignCachedTasks()
	{
		for (int i = m_aCachedTaskAssignments.Count() - 1; i >= 0; i--)
		{
			SCR_TaskAssignmentData assignmentData = m_aCachedTaskAssignments[i];
			SCR_BaseTask task = GetTask(assignmentData.m_iTaskID);
			
			if (!task)
				return;
			
			foreach (int assignee : assignmentData.m_aAssignees)
			{
				SCR_BaseTaskExecutor taskExecutor = SCR_BaseTaskExecutor.GetTaskExecutorByID(assignee);
				if (!taskExecutor)
					continue;
				
				LocalAssignTask(task, taskExecutor, assignmentData.m_fLastAssigneeAddedTimestamp);
			}
			
			m_aCachedTaskAssignments.Remove(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void MoveTask(vector newPosition, int taskID)
	{
		SCR_BaseTask task = GetTask(taskID);
		if (!task)
			return;
		
		task.SetOrigin(newPosition);
		
		if (IsProxy())
			return;
		
		Rpc(RPC_MoveTask, taskID, newPosition);
	}
	
	//------------------------------------------------------------------------------------------------
	//Caches a task assignment for later
	void CacheAssignment(SCR_BaseTaskData taskData, array<int> assignees)
	{
		if (!m_aCachedTaskAssignments)
			return;
		
		SCR_TaskAssignmentData assignmentData = new SCR_TaskAssignmentData();
		assignmentData.m_fLastAssigneeAddedTimestamp = taskData.GetLastAssigneeAddedTimestamp();
		assignmentData.m_iTaskID = taskData.GetTaskID();
		assignmentData.m_aAssignees.Copy(assignees);
		
		m_aCachedTaskAssignments.Insert(assignmentData);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Outs an array with all assignee IDs of the given task
	protected int GetTaskAssigneeIDs(SCR_BaseTask task, out array<int> assignees)
	{
		if (!assignees)
			assignees = new array<int>();
		
		int count = 0;
		array<SCR_BaseTaskExecutor> taskAssignees = new array<SCR_BaseTaskExecutor>();
		task.GetAssignees(taskAssignees);
		foreach (SCR_BaseTaskExecutor assignee : taskAssignees)
		{
			assignees.Insert(SCR_BaseTaskExecutor.GetTaskExecutorID(assignee));
			count++;
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTaskTypeIndex(notnull SCR_BaseTask task)
	{
		typename taskType = task.Type();
		for (int i = m_aSupportedTaskTypes.Count() - 1; i >= 0; i--)
		{
			if (m_aSupportedTaskTypes[i].GetTypename() == taskType)
				return i;
		}
		
		return -1;
	}
	//------------------------------------------------------------------------------------------------
	/*!
	Check if task of given class has configuration in task manager.
	\param typeName Task class
	\return True if the task has configuration
	*/
	bool HasTaskData(typename typeName)
	{
		for (int i = m_aSupportedTaskTypes.Count() - 1; i >= 0; i--)
		{
			if (m_aSupportedTaskTypes[i].GetTypename() == typeName)
				return true;
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the found task if it exists.
	SCR_BaseTask GetTask(int taskID)
	{
		foreach (SCR_BaseTask task: m_aTaskList)
		{
			if (task.GetTaskID() == taskID)
			{
				return task;
			}
		}
		return null;
	}
	
	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	protected bool CheckMasterOnlyMethod(string methodName)
	{
		if (IsProxy())
		{
		    Print("Master-only method (SCR_BaseTaskManager." + methodName + ") called on proxy, some functionality might be broken!", LogLevel.WARNING);
			return false;
		}
		
		return true;
	}
	#endif
	
	//*********************//
	//PUBLIC MEMBER METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	void DeleteTask(SCR_BaseTask task)
	{
		m_aTaskList.RemoveItem(task);
		task.OnDelete();
		delete task;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLocallyRequestedTask(notnull SCR_RequestedTask requestedTask)
	{
		m_LocallyRequestedTask = requestedTask;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_RequestedTask GetLocallyRequestedTask()
	{
		return m_LocallyRequestedTask;
	}
	
	//------------------------------------------------------------------------------------------------
	// For use where we can get task.Type()
	SCR_BaseTaskSupportClass GetSupportedTaskByTaskType(typename type)
	{
		if (!m_aSupportedTaskTypes)
			return null;
		
		for (int i = m_aSupportedTaskTypes.Count() - 1; i >= 0; i--)
		{
			if (type == m_aSupportedTaskTypes[i].GetTypename())
				return m_aSupportedTaskTypes[i];
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	// For use in static methods, where we cannot access .Type() on tasks
	SCR_BaseTaskSupportClass GetSupportedTaskBySupportClassType(typename type)
	{
		if (!m_aSupportedTaskTypes)
			return null;
		
		for (int i = m_aSupportedTaskTypes.Count() - 1; i >= 0; i--)
		{
			if (type == m_aSupportedTaskTypes[i].Type())
				return m_aSupportedTaskTypes[i];
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//Returns the list of all supported task types
	array<ref SCR_BaseTaskSupportClass> GetSupportedTasks()
	{
		return m_aSupportedTaskTypes;
	}
	
	//------------------------------------------------------------------------------------------------
	notnull SCR_BaseTaskExecutor LocalCreateTaskExecutor(int playerID)
	{
		SCR_BaseTaskExecutor executor = SCR_BaseTaskExecutor.FindTaskExecutorByID(playerID);
		if (executor)
			return executor;
		
		executor = SCR_BaseTaskExecutor.Cast(GetGame().SpawnEntity(SCR_BaseTaskExecutor, GetGame().GetWorld()));
		executor.SetPlayerID(playerID);
		
		return executor;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_RequestedTask FindRequestedTask(SCR_BaseTaskExecutor requester)
	{
		for (int i = 0; i < m_aTaskList.Count(); i++)
		{
			SCR_RequestedTask requestedTask = SCR_RequestedTask.Cast(m_aTaskList[i]);
			if (!requestedTask)
				continue;
			
			SCR_BaseTaskExecutor taskRequester = requestedTask.GetRequester();
			if (requester == taskRequester)
				return requestedTask;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Outs an array of tasks filtered by faction.
	//! filterFaction param is the allowed faction!
	int GetFilteredTasks(notnull out array<SCR_BaseTask> tasks, Faction filterFaction = null)
	{
		int count = 0;
		tasks.Clear();
		
		if (!filterFaction)
			return GetTasks(tasks);
		
		foreach (SCR_BaseTask task: m_aTaskList)
		{
			if (!task)
				continue;
			
			if (task.GetTargetFaction() == filterFaction)
			{
				tasks.Insert(task);
				count++;
			}
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	//This should only be called on the server!
	void RequestTransport(int requesterID, vector fromPosition, vector toPosition)
	{
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("RequestTransport()"))
			return;
		#endif
		
		SCR_TransportTaskSupportClass supportClass = SCR_TransportTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskByTaskType(SCR_TransportTask));
		if (!supportClass)
			return;
		
		SCR_TransportTask transportTask = supportClass.CreateNewTransportTask(requesterID, fromPosition, toPosition);
		
		if (transportTask)
		{
			SCR_TransportTaskData data = new SCR_TransportTaskData();
			data.LoadDataFromTask(transportTask);
			Rpc(RPC_CreateTransportTaskFromData, data);
			MoveTask(fromPosition, transportTask.GetTaskID());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//This should only be called on the server!
	void RequestRefuel(int requesterID, IEntity vehicle, vector position)
	{
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("RequestRefuel()"))
			return;
		#endif
		
		SCR_RefuelTask refuelTask = CreateNewRefuelTask(requesterID, vehicle);
		
		if (refuelTask)
		{
			SCR_RefuelTaskData data = new SCR_RefuelTaskData();
			data.LoadDataFromTask(refuelTask);
			Rpc(RPC_CreateRefuelTaskFromData, data);
			MoveTask(position, refuelTask.GetTaskID());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//This should only be called on the server!
	void RequestEvacuation(int requesterID, vector position)
	{
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("RequestEvacuation()"))
			return;
		#endif
		
		SCR_EvacuateTask evacuateTask = CreateNewEvacuationTask(requesterID);
		
		if (evacuateTask)
		{
			SCR_EvacuateTaskData data = new SCR_EvacuateTaskData();
			data.LoadDataFromTask(evacuateTask);
			Rpc(RPC_CreateEvacuationTaskFromData, data);
			MoveTask(position, evacuateTask.GetTaskID());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetAssigneeAbandoned(SCR_BaseTaskExecutor assignee)
	{
		if (!m_mAssigneesAbandoned || !assignee)
			return false;
		
		float timestamp = m_mAssigneesAbandoned.Get(assignee);
		
		if (m_fTimestamp < timestamp)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void LocalCancelTask(SCR_BaseTask task)
	{
		if (!task)
			return;
		
		task.Cancel();
		DeleteTask(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//This should only be called on the server!
	void CancelTask(int taskID)
	{
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("CancelTask()"))
			return;
		#endif
		
		Rpc(RPC_CancelTask, taskID);
		RPC_CancelTask(taskID);
	}
	
	//------------------------------------------------------------------------------------------------
	//This should only be called on the server!
	void AbandonTask(int taskID, notnull SCR_BaseTaskExecutor assignee)
	{
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("AbandonTask()"))
			return;
		#endif
		
		SCR_BaseTask task = GetTask(taskID);
		
		if (!task)
			return;
		
		if (task.NotifyUnassign())
			task.DoNotifyUnassign(SCR_BaseTaskExecutor.GetTaskExecutorID(assignee));
		
		UnassignTask(task, assignee, SCR_EUnassignReason.ASSIGNEE_ABANDON);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used for assigning the task with given task ID to the assignee in parameter
	void RequestAssignment(int taskID, notnull SCR_BaseTaskExecutor assignee)
	{
		if (!assignee || IsProxy())
			return;
		
		SCR_BaseTask task = GetTask(taskID);
		
		if (!task)
			return;
		
		if (task.NotifyAssignment())
			task.DoNotifyAssignment(SCR_BaseTaskExecutor.GetTaskExecutorID(assignee));
		
		AssignTask(task, assignee);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerRegistered(int registeredPlayerID)
	{
		if (registeredPlayerID == SCR_PlayerController.GetLocalPlayerId())
			CreateTaskExecutors();
		else
			LocalCreateTaskExecutor(registeredPlayerID);
		
		AssignCachedTasks();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates a task executor for each connected player
	void CreateTaskExecutors()
	{
		array<int> players = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(players);
		
		foreach (int playerID : players)
		{
			if (SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID))
				continue;
			
			LocalCreateTaskExecutor(playerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_FactionManager GetFactionManager()
	{
		return s_FactionManager;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the current timestamp
	float GetTimestamp()
	{
		return m_fTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Outs an array of all of the tasks.
	int GetTasks(notnull out array<SCR_BaseTask> tasks)
	{
		int count = 0;
		
		foreach (SCR_BaseTask task: m_aTaskList)
		{
			tasks.Insert(task);
			count++;
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates a new task.
	SCR_BaseTask CreateNewTask()
	{
		IEntity taskEntity = GetGame().SpawnEntity(SCR_BaseTask);
		SCR_BaseTask task = SCR_BaseTask.Cast(taskEntity);
		
		return task;
	}
	
	//------------------------------------------------------------------------------------------------
	void LocalFinishTask(notnull SCR_BaseTask task)
	{
		if (!task)
			return;
		
		task.Finish();
		DeleteTask(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//This should only be called on the server!
	void FinishTask(notnull SCR_BaseTask task)
	{
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("FinishTask()"))
			return;
		#endif
		
		Rpc(RPC_FinishTask, task.GetTaskID());
		LocalFinishTask(task);
	}
	
	//------------------------------------------------------------------------------------------------
	void LocalFailTask(notnull SCR_BaseTask task)
	{
		task.Fail();
		DeleteTask(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//This should only be called on the server!
	void FailTask(notnull SCR_BaseTask task)
	{
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("FailTask()"))
			return;
		#endif
		
		Rpc(RPC_FailTask, task.GetTaskID());
		LocalFailTask(task);
	}
	
	//------------------------------------------------------------------------------------------------
	void LocalSetTaskFaction(notnull SCR_BaseTask task, Faction faction)
	{
		task.SetTargetFaction(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	//This should only be called on the server!
	void SetTaskFaction(notnull SCR_BaseTask task, Faction faction)
	{
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("SetTaskFaction()"))
			return;
		#endif
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Rpc(RPC_SetTaskFaction, task.GetTaskID(), factionManager.GetFactionIndex(faction));
		LocalSetTaskFaction(task, faction);
	}
	
	//------------------------------------------------------------------------------------------------
	//Todo: Implement tasks removal with delay
	//! Removes a task.
	void RemoveTask(SCR_BaseTask task)
	{
		if (!task)
			return;
		
		int index = m_aTaskList.Find(task);
		
		if (index == -1)
			return;
 		else
		{
			task.Remove();
			DeleteTask(task);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void LocalAssignTask(notnull SCR_BaseTask task, notnull SCR_BaseTaskExecutor assignee, float timestamp = -1, bool forced = false)
	{
		task.AddAssignee(assignee, timestamp);
		s_OnTaskAssigned.Invoke(task);
		
		if (forced && assignee == SCR_BaseTaskExecutor.GetLocalExecutor())
			SCR_PopUpNotification.GetInstance().PopupMsg(m_sAssignPopupGM);
	}

	//------------------------------------------------------------------------------------------------
	//This should only be called on the server!
	void AssignTask(notnull SCR_BaseTask task, notnull SCR_BaseTaskExecutor assignee, bool forced = false)
	{
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("AssignTask()"))
			return;
		#endif
		
		Rpc(RPC_AssignTask, SCR_BaseTaskExecutor.GetTaskExecutorID(assignee), task.GetTaskID(), m_fTimestamp, forced);
		LocalAssignTask(task, assignee, m_fTimestamp, forced);
	}
	
	//------------------------------------------------------------------------------------------------
	void LocalUnassignTask(notnull SCR_BaseTask task, notnull SCR_BaseTaskExecutor assignee, SCR_EUnassignReason reason)
	{
		if (!assignee)
			return;
		
		switch (reason)
		{
			case SCR_EUnassignReason.ASSIGNEE_TIMEOUT:
				break;
			case SCR_EUnassignReason.ASSIGNEE_DISCONNECT:
				break;
			case SCR_EUnassignReason.ASSIGNEE_ABANDON:
			{
				if (m_mAssigneesAbandoned)
					m_mAssigneesAbandoned.Set(assignee, m_fTimestamp + DEFAULT_ABANDON_WAITING_TIME);
				break;
			}
			case SCR_EUnassignReason.GM_REASSIGN:
			{
				if (assignee == SCR_BaseTaskExecutor.GetLocalExecutor())
					SCR_PopUpNotification.GetInstance().PopupMsg(m_sReassignPopup);
				break;
			}
		}
		
		task.RemoveAssignee(assignee, reason);
		s_OnTaskUnassigned.Invoke(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//This should only be called on the server!
	void UnassignTask(notnull SCR_BaseTask task, notnull SCR_BaseTaskExecutor assignee, SCR_EUnassignReason reason)
	{
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("UnassignTask()"))
			return;
		#endif
		
		Rpc(RPC_UnassignTask, task.GetTaskID(), SCR_BaseTaskExecutor.GetTaskExecutorID(assignee), reason);
		LocalUnassignTask(task, assignee, reason);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Create data for given task.
	\param task Task
	\return Task data
	*/
	SCR_BaseTaskData CreateTaskData(SCR_BaseTask task)
	{
		int taskType = GetTaskTypeIndex(task);
		if (taskType == -1 || m_aSupportedTaskTypes.Count() <= taskType)
		{
			string warning = string.Format("Task data for %1 cannot be created. %1 couldn't be found in m_aSupportedTaskTypes.", task.Type().ToString());
			Print(warning, LogLevel.WARNING);
			return null;
		}
		
		return m_aSupportedTaskTypes[taskType].CreateTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	void TransportTaskNextPhase(notnull SCR_TransportTask transportTask)
	{
		if (!transportTask.GetVolunteerMet())
		{
			transportTask.SetVolunteerMet(true);
			MoveTask(transportTask.GetTargetPosition(), transportTask.GetTaskID());
			Rpc(RPC_TransportTaskNextPhase, transportTask.GetTaskID());
		}
		else
			FinishTask(transportTask);
	}
	
	//***********//
	//RPC METHODS//
	//***********//
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_TransportTaskNextPhase(int taskID)
	{
		SCR_TransportTask transportTask = SCR_TransportTask.Cast(GetTask(taskID));
		transportTask.SetVolunteerMet(true);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_CancelTask(int taskID)
	{
		SCR_BaseTask task = GetTask(taskID);
		LocalCancelTask(task);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_FinishTask(int taskID)
	{
		SCR_BaseTask task = GetTask(taskID);
		
		if (task)
			LocalFinishTask(task);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_FailTask(int taskID)
	{
		SCR_BaseTask task = GetTask(taskID);
		
		if (task)
			LocalFailTask(task);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetTaskFaction(int taskID, int factionIndex)
	{
		SCR_BaseTask task = GetTask(taskID);
		if (!task)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		LocalSetTaskFaction(task, factionManager.GetFactionByIndex(factionIndex));
	}
	
	//------------------------------------------------------------------------------------------------
	//! RPC executed on clients, creating a task based on given parameters
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_CreateMoveTask(SCR_MoveTaskData taskData, array<int> assignees)
	{
		ChimeraGame chimera = GetGame();
		if (!chimera || !taskData || !s_FactionManager)
			return;
		
		Faction targetFaction = s_FactionManager.GetFactionByIndex(taskData.GetFactionIndex());
		if (!targetFaction)
			return;
		
		SCR_MoveTask task = SCR_MoveTask.Cast(GetGame().SpawnEntity(SCR_MoveTask));
		if (!task)
			return;
		
		task.SetTaskID(taskData.GetTaskID());
		task.SetTargetFaction(targetFaction);
		task.SetIndividual(taskData.GetIndividual());
		task.SetOrigin(taskData.GetTargetPosition());
		
		if (assignees != null && assignees.Count() > 0)
		{
			CacheAssignment(taskData, assignees);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! RPC executed on clients, creating a task based on given parameters
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_CreateBaseTask(SCR_BaseTaskData taskData, array<int> assignees)
	{
		if (!taskData)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Faction targetFaction = factionManager.GetFactionByIndex(taskData.GetFactionIndex());
		if (!targetFaction)
			return;
		
		SCR_BaseTask task = SCR_BaseTask.Cast(GetGame().SpawnEntity(SCR_BaseTask));
		if (!task)
			return;
		
		task.SetTaskID(taskData.GetTaskID());
		task.SetTargetFaction(targetFaction);
		task.SetIndividual(taskData.GetIndividual());
		
		if (assignees != null && assignees.Count() > 0)
		{
			CacheAssignment(taskData, assignees);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_CreateEvacuationTaskFromData(SCR_EvacuateTaskData data)
	{
		if (data)
			data.CreateTask();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_CreateTransportTaskFromData(SCR_TransportTaskData data)
	{
		if (data)
			data.CreateTask();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_CreateRefuelTaskFromData(SCR_RefuelTaskData data)
	{
		if (data)
			data.CreateTask();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Assigns a task on clients.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_UnassignTask(int taskID, int playerID, SCR_EUnassignReason reason)
	{
		SCR_BaseTask task = GetTask(taskID);
		if (!task)
			return;
		
		SCR_BaseTaskExecutor assignee = SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID);
		if (assignee)
			LocalUnassignTask(task, assignee, reason);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Assigns a task on clients.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_AssignTask(int playerID, int taskID, float timestamp, bool forced)
	{
		SCR_BaseTask task = GetTask(taskID);
		if (!task)
			return;
		
		SCR_BaseTaskExecutor assignee = SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID);
		if (!assignee)
			return;
		
		LocalAssignTask(task, assignee, timestamp, forced);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_MoveTask(int taskID, vector newPosition)
	{
		MoveTask(newPosition, taskID);
	}
	
	//*************//
	//ENTITY EVENTS//
	//*************//

	//-----------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		m_bInitialized = true;
		
		InitializeSupportedTasks();
		
		s_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerRegistered().Insert(OnPlayerRegistered);
		
		// If replicated make sure it never streams out
		auto rplComponent = RplComponent.Cast(FindComponent(RplComponent));
		if (rplComponent && rplComponent.IsSelfInserted())
		{
			auto ctx = new GameRplSchedulerInsertionCtx;
			ctx.CanBeStreamed = false;
			rplComponent.InsertToReplication(ctx);
		}
		
		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		
		GetGame().GetCallqueue().CallLater(PeriodicalCheck2Second, 2000, true);
		GetGame().GetCallqueue().CallLater(PeriodicalCheck5Second, 5000, true);
		GetGame().GetCallqueue().CallLater(PeriodicalCheck60Second, 60000, true);
	}
	
	//-----------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (GetTaskManager() && GetTaskManager() != this)
		{
			delete this;
			return;
		}
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SHOW_ALL_TASKS))
		{
			if (!m_bPrintedTasks)
			{
				m_bPrintedTasks = true;
				for (int i = m_aTaskList.Count() - 1; i >= 0; i--)
				{
					Print(m_aTaskList[i].GetDescription());
				}
			}
		}
		else
			m_bPrintedTasks = false;
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EXECUTE_TASKS))
			return;
#endif
		
		m_fTimestamp += timeSlice;
		m_fTimestampTimer -= timeSlice;
		if (m_fTimestampTimer <= 0)
		{
			Replication.BumpMe();
			m_fTimestampTimer = 1;
		}
		
		if (!IsProxy() && m_fTimestamp > m_fAssigneeCacheCheckTimestamp)
		{
			m_fAssigneeCacheCheckTimestamp = m_fTimestamp + m_fAssigneeCacheTimer;
			CheckAssigneeTimeout();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		int count = m_aTaskList.Count();
		writer.WriteInt(count);
		
		for (int i = 0; i < count; i++)
		{
			SCR_BaseTask task = m_aTaskList[i];
			if (!task || task.FindComponent(RplComponent)) //--- Assume that tasks with their own replication will sync data themselves
			{
				writer.WriteInt(-1);
				continue;
			}
			
			int taskType = GetTaskTypeIndex(task);
			writer.WriteInt(taskType);
			
			task.Serialize(writer);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int count;
		reader.ReadInt(count);
		
		for (int i = 0; i < count; i++)
		{
			int taskType;
			reader.ReadInt(taskType);
			if (taskType == -1)
				continue;
			
			SCR_BaseTaskData taskData = m_aSupportedTaskTypes[taskType].CreateTaskData();
			if (!taskData)
				return false;
						
			taskData.Deserialize(reader);
			taskData.CreateTask();
		}
		
		//Let the game mode know, the tasks are ready
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gamemode)
			gamemode.HandleOnTasksInitialized();
		
		AssignCachedTasks();
		
		return true;
	}
	
	//************************//
	//CONSTRUCTOR & DESTRUCTOR//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	void SCR_BaseTaskManager(IEntitySource src, IEntity parent)
	{
		if (!g_TaskManagerInstance)
			g_TaskManagerInstance = this;
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_TASKS_MENU, "Tasks", "");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EXECUTE_TASKS, "", "Execute tasks", "Tasks", true);
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SHOW_ALL_TASKS, "", "Print all tasks", "Tasks", false);
#endif
		SetEventMask(EntityEvent.FRAME | EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, true);
		SetFlags(EntityFlags.NO_LINK, false);
		
		//Register to Script Invokers
		s_OnTaskCancelled.Insert(OnTaskUpdate);
		s_OnTaskAssigned.Insert(OnTaskUpdate);
		s_OnTaskUnassigned.Insert(OnTaskUpdate);
		s_OnTaskFactionAssigned.Insert(OnTaskUpdate);
		s_OnTaskFinished.Insert(OnTaskUpdate);
		s_OnTaskFailed.Insert(OnTaskUpdate);
		s_OnTaskCreated.Insert(OnTaskUpdate);
		s_OnTaskCreated.Insert(OnTaskCreated);
		s_OnTaskDeleted.Insert(OnTaskDeleted);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_BaseTaskManager()
	{
		if (m_aCachedTaskAssignments)
		{
			m_aCachedTaskAssignments.Clear();
			m_aCachedTaskAssignments = null;
		}
		
		m_aTaskList.Clear();
		m_aTaskList = null;
		
		//Unregister from Script Invokers
		s_OnTaskCancelled.Remove(OnTaskUpdate);
		s_OnTaskAssigned.Remove(OnTaskUpdate);
		s_OnTaskUnassigned.Remove(OnTaskUpdate);
		s_OnTaskFactionAssigned.Remove(OnTaskUpdate);
		s_OnTaskFinished.Remove(OnTaskUpdate);
		s_OnTaskFailed.Remove(OnTaskUpdate);
		s_OnTaskCreated.Remove(OnTaskUpdate);
		s_OnTaskCreated.Remove(OnTaskCreated);
	}
};
