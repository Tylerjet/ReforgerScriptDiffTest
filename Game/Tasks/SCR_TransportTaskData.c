//------------------------------------------------------------------------------------------------
class SCR_TransportTaskData : SCR_RequestedTaskData
{
	protected static const int TRANSPORT_TASK_DATA_SIZE = 13;
	protected vector m_vTargetPosition;
	protected bool m_bVolunteerMet;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskSupportClass GetSupportClass()
	{
		return GetTaskManager().GetSupportedTaskByTaskType(SCR_TransportTask);
	}
	
	//------------------------------------------------------------------------------------------------
	override void LoadDataFromTask(SCR_BaseTask task)
	{
		super.LoadDataFromTask(task);
		
		SCR_TransportTask transportTask = SCR_TransportTask.Cast(task);
		if (!transportTask)
			return;
		
		m_vTargetPosition = transportTask.GetTargetPosition();
		m_bVolunteerMet = transportTask.GetVolunteerMet();
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetTargetPosition()
	{
		return m_vTargetPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetupTask(SCR_BaseTask task)
	{
		super.SetupTask(task);
		
		SCR_TransportTask transportTask = SCR_TransportTask.Cast(task);
		if (!transportTask)
			return;
		
		transportTask.SetTargetPosition(GetTargetPosition());
		transportTask.SetVolunteerMet(m_bVolunteerMet);
	}

	//------------------------------------------------------------------------------------------------
	override void Deserialize(ScriptBitReader reader)
	{
		super.Deserialize(reader);
		
		vector origin = vector.Zero;
		
		reader.ReadVector(m_vTargetPosition);
		reader.ReadBool(m_bVolunteerMet);
	}
	
	//------------------------------------------------------------------------------------------------
	static override int GetDataSize()
	{
		return SCR_RequestedTaskData.GetDataSize() + TRANSPORT_TASK_DATA_SIZE;
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
	static bool PropCompare(SCR_TransportTaskData prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return SCR_RequestedTaskData.PropCompare(prop, snapshot, ctx)
			&& snapshot.Compare(prop.m_vTargetPosition[0], 4)
			&& snapshot.Compare(prop.m_vTargetPosition[1], 4)
			&& snapshot.Compare(prop.m_vTargetPosition[2], 4)
			&& snapshot.Compare(prop.m_bVolunteerMet, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_TransportTaskData prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		SCR_RequestedTaskData.Extract(prop, ctx, snapshot);
		snapshot.SerializeBytes(prop.m_vTargetPosition[0], 4);
		snapshot.SerializeBytes(prop.m_vTargetPosition[1], 4);
		snapshot.SerializeBytes(prop.m_vTargetPosition[2], 4);
		snapshot.SerializeBytes(prop.m_bVolunteerMet, 1);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_TransportTaskData prop) 
	{
		SCR_RequestedTaskData.Inject(snapshot, ctx, prop);
		
		float temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vTargetPosition[0] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vTargetPosition[1] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vTargetPosition[2] = temp;
		
		snapshot.SerializeBytes(prop.m_bVolunteerMet, 1);
		
		return true;
	}
};
