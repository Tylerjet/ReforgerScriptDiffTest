//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingTaskData : SCR_BaseTaskData
{
	static const int BUILDING_TASK_DATA_SIZE = 4;
	protected int m_iBaseID;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskSupportClass GetSupportClass()
	{
		return GetTaskManager().GetSupportedTaskByTaskType(SCR_CampaignBuildingTask);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBaseID()
	{
		return m_iBaseID;
	}
	
	//------------------------------------------------------------------------------------------------
	override void LoadDataFromTask(SCR_BaseTask task)
	{
		super.LoadDataFromTask(task);
		
		SCR_CampaignBuildingTask buildingTask = SCR_CampaignBuildingTask.Cast(task);
		if (!buildingTask)
			return;
		
		m_iBaseID = buildingTask.GetTargetBase().GetBaseID();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetupTask(SCR_BaseTask task)
	{
		SCR_CampaignBuildingTask buildingTask = SCR_CampaignBuildingTask.Cast(task);	
		if (!buildingTask)
			return;
		
		super.SetupTask(buildingTask);
		
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
		
		if (base.GetOwningFaction() != faction)
			return;
		
		buildingTask.SetTargetBase(base);
		buildingTask.SetTargetFaction(faction);
		
		NotifyAboutTask(buildingTask);
	}
	
	//------------------------------------------------------------------------------------------------
	override void NotifyAboutTask(notnull SCR_BaseTask task)
	{
		SCR_CampaignBuildingTask buildingTask = SCR_CampaignBuildingTask.Cast(task);
		if (!m_bNewTask || !buildingTask)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (!campaign)
			return;
		
		if (SCR_RespawnSystemComponent.GetLocalPlayerFaction() == buildingTask.GetTargetFaction())
		{
			string text = task.TASK_AVAILABLE_TEXT + " " + task.GetTitle();
			string baseName;
			
			if (buildingTask.GetTargetBase())
				baseName = buildingTask.GetTargetBase().GetBaseNameUpperCase();
			
			SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, param1: baseName, text2: task.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT, sound: UISounds.TASK_CREATED);
		}
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
		return SCR_BaseTaskData.GetDataSize() + BUILDING_TASK_DATA_SIZE;
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
	static bool PropCompare(SCR_CampaignBuildingTaskData prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
	   return  SCR_BaseTaskData.PropCompare(prop, snapshot, ctx)
	       && snapshot.Compare(prop.m_iBaseID, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_CampaignBuildingTaskData prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
	   SCR_BaseTaskData.Extract(prop, ctx, snapshot);
	   snapshot.SerializeBytes(prop.m_iBaseID, 4);
	   
	   return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_CampaignBuildingTaskData prop) 
	{
	   SCR_BaseTaskData.Inject(snapshot, ctx, prop);
	   snapshot.SerializeBytes(prop.m_iBaseID, 4);
	   
	   return true;
	}
}
