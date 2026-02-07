//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Tasks", description: "Campaign building task support entity.", color: "0 0 255 255")]
class SCR_CampaignBuildingTaskSupportEntityClass: SCR_CampaignBaseTaskSupportEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingTaskSupportEntity : SCR_CampaignBaseTaskSupportEntity
{
	//------------------------------------------------------------------------------------------------
	//! An event called when a base has been captured.
	void OnBaseCaptured(notnull SCR_CampaignBase capturedBase)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EXECUTE_TASKS))
			return;
#endif
		
		if (capturedBase.GetType() != CampaignBaseType.BASE)
			return;
		
		if (capturedBase.GetAllBaseServices() == 0) 
			GenerateNewBuildingTask(capturedBase);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBuildingTask GenerateNewBuildingTask(notnull SCR_CampaignBase base)
	{
		if (!GetTaskManager())
			return null;
		
		SCR_CampaignFaction faction = base.GetOwningFaction();
		if (!faction)
			return null;
				
		SCR_CampaignBuildingTask task = SCR_CampaignBuildingTask.Cast(CreateTask());
		if (!task)
			return null;
		
		SetTargetBase(task, base); // Replicated internally
		SetTargetFaction(task, faction); // Replicated internally
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (campaign && SCR_RespawnSystemComponent.GetLocalPlayerFaction() == faction)
		{
			string text = task.TASK_AVAILABLE_TEXT + " " + task.GetTitle();
			string baseName;
			
			if (task.GetTargetBase())
				baseName = task.GetTargetBase().GetBaseNameUpperCase();

			SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, param1: baseName, text2: task.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT, sound: SCR_SoundEvent.TASK_CREATED);
		}
		
		return task;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignBuildingTaskSupportEntity(IEntitySource src, IEntity parent)
	{
		//Register to script invokers TASK DISSABLED FOR NOW
		//SCR_GameModeCampaignMP.s_OnBaseCaptured.Insert(OnBaseCaptured);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignBuildingTaskSupportEntity()
	{
		//Unregister from script invokers
		//SCR_GameModeCampaignMP.s_OnBaseCaptured.Remove(OnBaseCaptured);
	}
}