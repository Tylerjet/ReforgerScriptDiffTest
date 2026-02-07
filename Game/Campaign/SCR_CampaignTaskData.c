//------------------------------------------------------------------------------------------------
//! Used when sending bigger RPCs for task synchronization
class SCR_CampaignTaskData : SCR_BaseTaskData
{
	static const int CAMPAIGN_TASK_DATA_SIZE = 8;
	protected int m_iBaseID;
	protected SCR_CampaignTaskType m_eType;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskSupportClass GetSupportClass()
	{
		return GetTaskManager().GetSupportedTaskByTaskType(SCR_CampaignTask);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignTaskType GetTaskType()
	{
		return m_eType;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBaseID()
	{
		return m_iBaseID;
	}
	
	//------------------------------------------------------------------------------------------------
	override void NotifyAboutTask(notnull SCR_BaseTask task)
	{
		SCR_CampaignTask campaignTask = SCR_CampaignTask.Cast(task);
		if (!campaignTask)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (campaign && m_bNewTask && SCR_RespawnSystemComponent.GetLocalPlayerFaction() == campaignTask.GetTargetFaction())
		{
			string text = task.TASK_AVAILABLE_TEXT + " " + task.GetTitle();
			string baseName = "";
			
			if (campaignTask.GetTargetBase())
				baseName = campaignTask.GetTargetBase().GetBaseNameUpperCase();
			
			SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, param1: baseName, text2: task.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT, sound: UISounds.TASK_CREATED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void LoadDataFromTask(SCR_BaseTask task)
	{
		super.LoadDataFromTask(task);
		
		SCR_CampaignTask campaignTask = SCR_CampaignTask.Cast(task);
		if (!campaignTask)
			return;
		
		m_iBaseID = campaignTask.GetTargetBase().GetBaseID();
		m_eType = campaignTask.GetType();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetupTask(SCR_BaseTask task)
	{
		SCR_CampaignTask campaignTask = SCR_CampaignTask.Cast(task);
		if (!campaignTask)
			return;
		
		super.SetupTask(campaignTask);
		
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
		
		if (base.GetOwningFaction() == faction)
			return;
		
		campaignTask.SetTargetBase(base);
		campaignTask.SetTargetFaction(faction);
		campaignTask.SetType(m_eType);
		
		NotifyAboutTask(campaignTask);
	}

	//------------------------------------------------------------------------------------------------
	override void Deserialize(ScriptBitReader reader)
	{
		super.Deserialize(reader);
		
		reader.ReadInt(m_iBaseID);
		reader.ReadIntRange(m_eType, 0, SCR_CampaignTaskType.LAST-1);
	}
	
	//------------------------------------------------------------------------------------------------
	static override int GetDataSize()
	{
		return SCR_BaseTaskData.GetDataSize() + CAMPAIGN_TASK_DATA_SIZE;
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
	static bool PropCompare(SCR_CampaignTaskData prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return SCR_BaseTaskData.PropCompare(prop, snapshot, ctx)
			&& snapshot.Compare(prop.m_iBaseID, 4)
			&& snapshot.Compare(prop.m_eType, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_CampaignTaskData prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		SCR_BaseTaskData.Extract(prop, ctx, snapshot);
		snapshot.SerializeBytes(prop.m_iBaseID, 4);
		snapshot.SerializeBytes(prop.m_eType, 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_CampaignTaskData prop) 
	{
		SCR_BaseTaskData.Inject(snapshot, ctx, prop);
		snapshot.SerializeBytes(prop.m_iBaseID, 4);
		snapshot.SerializeBytes(prop.m_eType, 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignTaskData()
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTaskData()
	{
		
	}
};
