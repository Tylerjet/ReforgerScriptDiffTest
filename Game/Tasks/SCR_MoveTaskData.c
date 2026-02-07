//------------------------------------------------------------------------------------------------
class SCR_MoveTaskData : SCR_EditorTaskData
{
	static const int MOVE_TASK_DATA_SIZE = 12;
	protected vector m_vTargetPosition;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskSupportClass GetSupportClass()
	{
		return GetTaskManager().GetSupportedTaskByTaskType(SCR_MoveTask);
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetTargetPosition()
	{
		return m_vTargetPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	override void LoadDataFromTask(SCR_BaseTask task)
	{
		super.LoadDataFromTask(task);
		
		SCR_MoveTask moveTask = SCR_MoveTask.Cast(task);
		if (!moveTask)
			return;
		
		m_vTargetPosition = moveTask.GetOrigin();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetupTask(SCR_BaseTask task)
	{
		if (!task)
			return;
		
		super.SetupTask(task);
		
		SCR_MoveTask moveTask = SCR_MoveTask.Cast(task);
		if (m_vTargetPosition != vector.Zero)
			moveTask.SetOrigin(m_vTargetPosition);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Deserialize(ScriptBitReader reader)
	{
		super.Deserialize(reader);
		
		reader.ReadVector(m_vTargetPosition);
	}
	
	//------------------------------------------------------------------------------------------------
	static override int GetDataSize()
	{
		return SCR_BaseTaskData.GetDataSize() + MOVE_TASK_DATA_SIZE;
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
	static bool PropCompare(SCR_MoveTaskData prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return SCR_BaseTaskData.PropCompare(prop, snapshot, ctx)
			&& snapshot.Compare(prop.m_vTargetPosition, 12);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_MoveTaskData prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		SCR_BaseTaskData.Extract(prop, ctx, snapshot);
		snapshot.SerializeBytes(prop.m_vTargetPosition, 12);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_MoveTaskData prop) 
	{
		SCR_BaseTaskData.Inject(snapshot, ctx, prop);
		snapshot.SerializeBytes(prop.m_vTargetPosition, 12);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MoveTaskData()
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MoveTaskData()
	{
		
	}
};
