//------------------------------------------------------------------------------------------------
class SCR_CampaignDefendTaskData : SCR_BaseTaskData
{
	static const int CAMPAIGN_DEFEND_TASK_DATA_SIZE = 4;
	protected int m_iBaseID;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskSupportClass GetSupportClass()
	{
		return GetTaskManager().GetSupportedTaskByTaskType(SCR_CampaignDefendTask);
	}
	
	//------------------------------------------------------------------------------------------------
	override void LoadDataFromTask(SCR_BaseTask task)
	{
		super.LoadDataFromTask(task);
		
		SCR_CampaignDefendTask campaignDefendTask = SCR_CampaignDefendTask.Cast(task);
		if (!campaignDefendTask)
			return;
		
		SCR_CampaignBase base = campaignDefendTask.GetTargetBase();
		int id = base.GetBaseID();
		
		m_iBaseID = id;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetupTask(SCR_BaseTask task)
	{
		SCR_CampaignDefendTask defendTask = SCR_CampaignDefendTask.Cast(task);
		if (!defendTask)
			return;
		
		super.SetupTask(defendTask);
		
		SCR_CampaignTaskManager taskManager = SCR_CampaignTaskManager.GetCampaignTaskManagerInstance();
		if (!taskManager)
			return;
		
		SCR_CampaignBase base = SCR_CampaignBaseManager.GetInstance().FindBaseByID(m_iBaseID);
		if (!base)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Faction faction = factionManager.GetFactionByIndex(m_iFactionIndex);
		
		defendTask.SetTargetFaction(faction);
		defendTask.SetTargetBase(base);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Deserialize(ScriptBitReader reader)
	{
		super.Deserialize(reader);
		
		reader.ReadInt(m_iBaseID);
	}
	
	//------------------------------------------------------------------------------------------------
	static override int GetDataSize()
	{
		return SCR_BaseTaskData.GetDataSize() + CAMPAIGN_DEFEND_TASK_DATA_SIZE;
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
	static bool PropCompare(SCR_CampaignDefendTaskData prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return SCR_BaseTaskData.PropCompare(prop, snapshot, ctx)
			&& snapshot.Compare(prop.m_iBaseID, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_CampaignDefendTaskData prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		SCR_BaseTaskData.Extract(prop, ctx, snapshot);
		snapshot.SerializeBytes(prop.m_iBaseID, 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_CampaignDefendTaskData prop) 
	{
		SCR_BaseTaskData.Inject(snapshot, ctx, prop);
		snapshot.SerializeBytes(prop.m_iBaseID, 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignDefendTaskData()
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignDefendTaskData()
	{
		
	}
};
