//------------------------------------------------------------------------------------------------
class SCR_RequestedTaskData : SCR_BaseTaskData
{
	static const int REQUESTED_TASK_DATA_SIZE = 4;
	protected int m_iRequesterID;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskSupportClass GetSupportClass()
	{
		return GetTaskManager().GetSupportedTaskByTaskType(SCR_RequestedTask);
	}
	
	//------------------------------------------------------------------------------------------------
	override void LoadDataFromTask(SCR_BaseTask task)
	{
		super.LoadDataFromTask(task);
		
		SCR_RequestedTask requestedTask = SCR_RequestedTask.Cast(task);
		if (!requestedTask)
			return;
		
		SCR_BaseTaskExecutor requester = requestedTask.GetRequester();
		if (!requester)
			return;
		
		m_iRequesterID = SCR_BaseTaskExecutor.GetTaskExecutorID(requester);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRequesterID()
	{
		return m_iRequesterID;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetupTask(SCR_BaseTask task)
	{
		super.SetupTask(task);
		
		SCR_RequestedTask requestedTask = SCR_RequestedTask.Cast(task);
		if (!requestedTask)
			return;
		
		SCR_BaseTaskExecutor requester = SCR_BaseTaskExecutor.GetTaskExecutorByID(GetRequesterID());
		
		if (requester)
			requestedTask.SetRequester(requester);
		else
			requestedTask.SetRequesterID(GetRequesterID());
	}

	//------------------------------------------------------------------------------------------------
	override void Deserialize(ScriptBitReader reader)
	{
		super.Deserialize(reader);
		
		reader.Read(m_iRequesterID, 32);
	}
	
	//------------------------------------------------------------------------------------------------
	static override int GetDataSize()
	{
		return SCR_BaseTaskData.GetDataSize() + REQUESTED_TASK_DATA_SIZE;
	}
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static override void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, GetDataSize());
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, GetDataSize());
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx) 
	{
		return lhs.CompareSnapshots(rhs, GetDataSize());
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_RequestedTaskData prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return SCR_BaseTaskData.PropCompare(prop, snapshot, ctx)
			&& snapshot.Compare(prop.m_iRequesterID, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_RequestedTaskData prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		SCR_BaseTaskData.Extract(prop, ctx, snapshot);
		snapshot.SerializeBytes(prop.m_iRequesterID, 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_RequestedTaskData prop) 
	{
		SCR_BaseTaskData.Inject(snapshot, ctx, prop);
		snapshot.SerializeBytes(prop.m_iRequesterID, 4);
		
		return true;
	}
};
