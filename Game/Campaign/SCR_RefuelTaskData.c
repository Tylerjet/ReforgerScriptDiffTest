//------------------------------------------------------------------------------------------------
class SCR_RefuelTaskData : SCR_RequestedTaskData
{
	static const int REFUEL_TASK_DATA_SIZE = 0;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskSupportClass GetSupportClass()
	{
		return GetTaskManager().GetSupportedTaskByTaskType(SCR_RefuelTask);
	}
	
	//------------------------------------------------------------------------------------------------
	override void LoadDataFromTask(SCR_BaseTask task)
	{
		super.LoadDataFromTask(task);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetupTask(SCR_BaseTask task)
	{
		if (!task)
			return;
		
		super.SetupTask(task);
	}

	//------------------------------------------------------------------------------------------------
	override void Deserialize(ScriptBitReader reader)
	{
		super.Deserialize(reader);
	}
	
	//------------------------------------------------------------------------------------------------
	static override int GetDataSize()
	{
		return SCR_RequestedTaskData.GetDataSize() + REFUEL_TASK_DATA_SIZE;
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
	static bool PropCompare(SCR_RefuelTaskData prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return SCR_RequestedTaskData.PropCompare(prop, snapshot, ctx);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_RefuelTaskData prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		return SCR_RequestedTaskData.Extract(prop, ctx, snapshot);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_RefuelTaskData prop) 
	{
		SCR_RequestedTaskData.Inject(snapshot, ctx, prop);
		
		return true;
	}
};
