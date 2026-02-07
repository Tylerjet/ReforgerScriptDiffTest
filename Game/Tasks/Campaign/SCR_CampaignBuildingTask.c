//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingTaskClass: SCR_CampaignBaseTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingTask : SCR_CampaignBaseTask
{
	override void SetTargetBase(notnull SCR_CampaignBase targetBase)
	{
		super.SetTargetBase(targetBase);
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		campaign.s_OnServiceBuild.Insert(FinishTask);
		campaign.s_OnBaseCaptured.Insert(CancelTask);
	}
	
	//------------------------------------------------------------------------------------------------
	void FinishTask(SCR_CampaignBase base, SCR_CampaignNetworkComponent networkComponent)
	{
		if (base && base == GetTargetBase())
			GetTaskManager().FinishTask(this);
		
		if (networkComponent)
			networkComponent.AddPlayerXP(CampaignXPRewards.SERVICE_BUILD);
	}
	
	//------------------------------------------------------------------------------------------------
	void CancelTask(SCR_CampaignBase base)
	{
		if (base && base == GetTargetBase() && m_TargetFaction != base.GetOwningFaction())
			GetTaskManager().FailTask(this);	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return the title of this task.
	override string GetTitle()
	{
		string baseName;

		if (m_TargetBase)
			baseName = GetBaseNameWithCallsign();

		return string.Format("%1 %2", m_sName, baseName);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Finish(bool showMsg = true)
	{
		showMsg = SCR_RespawnSystemComponent.GetLocalPlayerFaction() == m_TargetFaction;
		super.Finish(showMsg);
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
				
		string baseName;
		
		if (m_TargetBase)
			baseName = GetBaseNameWithCallsign();
		
		if (!campaign || !showMsg)
			return;
		
		// TODO make this nicer
		if (m_bIndividualTask)
		{
			if (IsAssignedToLocalPlayer())
			{
				if (DoneByAssignee())
				{
					SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: UISounds.TASK_SUCCEED);
				}
				else
				{
					SCR_PopUpNotification.GetInstance().PopupMsg(TASK_FAILED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: UISounds.TASK_FAILED);
				}
			}
			else
			{
				SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: UISounds.TASK_SUCCEED);
			}
		}
		else
			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: UISounds.TASK_SUCCEED);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fails the task.
	override void Fail(bool showMsg = true)
	{
		showMsg = SCR_RespawnSystemComponent.GetLocalPlayerFaction() == m_TargetFaction;
		super.Fail(showMsg);
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (campaign && showMsg)
		{
			string baseName;
		
			if (m_TargetBase)
				baseName = GetBaseNameWithCallsign();
			
			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_FAILED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: UISounds.TASK_FAILED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Serialize(ScriptBitWriter writer)
	{
		super.Serialize(writer);
		
		int baseID = -1;
		SCR_CampaignBase base = GetTargetBase();
		if (base)
			baseID = base.GetBaseID();
		
		writer.WriteInt(baseID);
	}
}
