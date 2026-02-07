//------------------------------------------------------------------------------------------------
class SCR_EvacuateTaskData : SCR_RequestedTaskData
{
	static const int EVACUATE_TASK_DATA_SIZE = 12;
	protected vector m_vStartOrigin;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskSupportClass GetSupportClass()
	{
		return GetTaskManager().GetSupportedTaskByTaskType(SCR_EvacuateTask);
	}
	
	//------------------------------------------------------------------------------------------------
	override void LoadDataFromTask(SCR_BaseTask task)
	{
		super.LoadDataFromTask(task);
		
		SCR_EvacuateTask evacuateTask = SCR_EvacuateTask.Cast(task);
		if (!evacuateTask)
			return;
		
		m_vStartOrigin = evacuateTask.GetStartOrigin();
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetStartOrigin()
	{
		return m_vStartOrigin;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetupTask(SCR_BaseTask task)
	{
		super.SetupTask(task);
		
		SCR_EvacuateTask evacuateTask = SCR_EvacuateTask.Cast(task);
		if (!evacuateTask)
			return;
		
		evacuateTask.SetStartOrigin(GetStartOrigin());
	}

	//------------------------------------------------------------------------------------------------
	override void Deserialize(ScriptBitReader reader)
	{
		super.Deserialize(reader);
		
		vector origin = vector.Zero;
		reader.ReadVector(m_vStartOrigin);
	}
	
	//------------------------------------------------------------------------------------------------
	static override int GetDataSize()
	{
		return SCR_RequestedTaskData.GetDataSize() + EVACUATE_TASK_DATA_SIZE;
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
	static bool PropCompare(SCR_EvacuateTaskData prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return SCR_RequestedTaskData.PropCompare(prop, snapshot, ctx)
			&& snapshot.Compare(prop.m_vStartOrigin[0], 4)
			&& snapshot.Compare(prop.m_vStartOrigin[1], 4)
			&& snapshot.Compare(prop.m_vStartOrigin[2], 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_EvacuateTaskData prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		SCR_RequestedTaskData.Extract(prop, ctx, snapshot);
		snapshot.SerializeBytes(prop.m_vStartOrigin[0], 4);
		snapshot.SerializeBytes(prop.m_vStartOrigin[1], 4);
		snapshot.SerializeBytes(prop.m_vStartOrigin[2], 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_EvacuateTaskData prop) 
	{
		SCR_RequestedTaskData.Inject(snapshot, ctx, prop);
		
		float temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vStartOrigin[0] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vStartOrigin[1] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vStartOrigin[2] = temp;
		
		return true;
	}
};
