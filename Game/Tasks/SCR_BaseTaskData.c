//------------------------------------------------------------------------------------------------
class SCR_BaseTaskData
{
	static const int BASE_TASK_DATA_SIZE = 32;
	protected int m_iFactionIndex;
	protected int m_iTaskID;
	protected bool m_bIndividual = false;
	protected float m_fLastAssigneeAddedTimestamp = -1;
	protected bool m_bNewTask = false;
	protected ref array<int> m_aAssignees = new ref array<int>();
	protected vector m_vPosition = vector.Zero;
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTaskSupportClass GetSupportClass()
	{
		return GetTaskManager().GetSupportedTaskByTaskType(SCR_BaseTask);
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyAboutTask(notnull SCR_BaseTask task)
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetNewTask()
	{
		return m_bNewTask;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetLastAssigneeAddedTimestamp()
	{
		return m_fLastAssigneeAddedTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIndividual()
	{
		return m_bIndividual;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTaskID()
	{
		return m_iTaskID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFactionIndex()
	{
		return m_iFactionIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetNew()
	{
		m_bNewTask = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadDataFromTask(SCR_BaseTask task)
	{
		m_iFactionIndex = SCR_BaseTaskManager.GetFactionIndex(task.GetTargetFaction());
		m_iTaskID = task.GetTaskID();
		m_bIndividual = task.IsIndividual();
		m_fLastAssigneeAddedTimestamp = task.GetLastAssigneeAddedTimestamp();
		m_vPosition = task.GetOrigin();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetupTask(SCR_BaseTask task)
	{
		if (!task)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Faction targetFaction = factionManager.GetFactionByIndex(GetFactionIndex());
		//if (!targetFaction) //--- Don't exit, unassigned faction is valid situation in editor where task may be prepared for later use
		//	return;
		
		task.SetTaskID(GetTaskID());
		task.SetTargetFaction(targetFaction);
		task.SetIndividual(GetIndividual());
		
		if (m_vPosition != vector.Zero)
			GetTaskManager().MoveTask(m_vPosition, task.GetTaskID());
		
		if (m_aAssignees != null && m_aAssignees.Count() > 0)
			GetTaskManager().CacheAssignment(this, m_aAssignees);
		
		task.Create();
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_BaseTask SpawnTask(SCR_BaseTaskSupportClass supportClass, typename taskType)
	{
		if (!supportClass)
			return SCR_BaseTask.Cast(GetGame().SpawnEntity(taskType));
		else
			return SCR_BaseTask.Cast(GetGame().SpawnEntityPrefab(supportClass.GetTaskPrefab()));
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTask CreateTask()
	{
		if (!GetGame().GetWorld())
			return null;
		
		SCR_BaseTask task = SpawnTask(GetSupportClass(), SCR_BaseTask);
		
		if (!task)
			return task;
		
		SetupTask(task);
		return task;
	}
	
	//------------------------------------------------------------------------------------------------
	void Deserialize(ScriptBitReader reader)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		// Reading factionIndex
		m_iFactionIndex = 0;
		reader.Read(m_iFactionIndex, 4);

		// Reading m_iTaskID
		reader.ReadInt(m_iTaskID);

		// Reading m_bIndividualTask
		reader.ReadBool(m_bIndividual);

		// Reading m_fLastAssigneeAddedTimestamp
		reader.ReadFloat(m_fLastAssigneeAddedTimestamp);
		
		// Reading target m_aAssignees.Count()
		int assigneesCount;
		reader.Read(assigneesCount, 7);
		
		if (!m_aAssignees)
			m_aAssignees = new ref array<int>();
		
		int assigneeID;
		for (int i = 0; i < assigneesCount; i++)
		{
			// Reading assignee ID
			reader.ReadInt(assigneeID);
			//SCR_BaseTaskExecutor assignee = SCR_BaseTaskExecutor.GetTaskExecutorByID(assigneeID);
			m_aAssignees.Insert(assigneeID);
		}
		
		// Reading position
		reader.ReadVector(m_vPosition);
	}
	
	//------------------------------------------------------------------------------------------------
	static int GetDataSize()
	{
		return BASE_TASK_DATA_SIZE;
	}
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, GetDataSize());
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, GetDataSize());
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx) 
	{
		return lhs.CompareSnapshots(rhs, GetDataSize());
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_BaseTaskData prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return snapshot.Compare(prop.m_iFactionIndex, 4)
			&& snapshot.Compare(prop.m_iTaskID, 4)
			&& snapshot.Compare(prop.m_bIndividual, 4)
			&& snapshot.Compare(prop.m_fLastAssigneeAddedTimestamp, 4)
			&& snapshot.Compare(prop.m_bNewTask, 4)
			&& snapshot.Compare(prop.m_vPosition[0], 4)
			&& snapshot.Compare(prop.m_vPosition[1], 4)
			&& snapshot.Compare(prop.m_vPosition[2], 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_BaseTaskData prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(prop.m_iFactionIndex, 4);
		snapshot.SerializeBytes(prop.m_iTaskID, 4);
		snapshot.SerializeBytes(prop.m_bIndividual, 4);
		snapshot.SerializeBytes(prop.m_fLastAssigneeAddedTimestamp, 4);
		snapshot.SerializeBytes(prop.m_bNewTask, 4);
		snapshot.SerializeBytes(prop.m_vPosition[0], 4);
		snapshot.SerializeBytes(prop.m_vPosition[1], 4);
		snapshot.SerializeBytes(prop.m_vPosition[2], 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_BaseTaskData prop) 
	{
		snapshot.SerializeBytes(prop.m_iFactionIndex, 4);
		snapshot.SerializeBytes(prop.m_iTaskID, 4);
		snapshot.SerializeBytes(prop.m_bIndividual, 4);
		snapshot.SerializeBytes(prop.m_fLastAssigneeAddedTimestamp, 4);
		snapshot.SerializeBytes(prop.m_bNewTask, 4);
		float temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vPosition[0] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vPosition[1] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vPosition[2] = temp;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BaseTaskData()
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_BaseTaskData()
	{
		
	}
};
