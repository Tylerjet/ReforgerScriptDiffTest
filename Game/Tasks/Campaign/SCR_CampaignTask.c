//------------------------------------------------------------------------------------------------
class SCR_CampaignTaskClass: SCR_CampaignBaseTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTask : SCR_CampaignBaseTask
{
	static const string CAMPAIGN_TASK_RECONFIGURED_TEXT = "#AR-CampaignTasks_Reconfigured-UC";
	static const string CAMPAIGN_TASK_RECONFIGURED_BY_TEXT = "#AR-CampaignTasks_ReconfiguredBy-UC";
	
	//*****************//
	//MEMBER ATTRIBUTES//
	//*****************//
	
	[Attribute(defvalue: "Task title for reconfigure.", desc: "The task title visible to the player.")]
	protected string m_sNameReconfigure;
	[Attribute(defvalue: "Task description.", desc: "The task description visible to the player.")]
	protected string m_sDescriptionReconfigure;
	
	//**************************//
	//PROTECTED MEMBER VARIABLES//
	//**************************//
	
	protected SCR_CampaignTaskType m_eType;
	
	//*********************************//
	//PROTECTED OVERRIDE MEMBER METHODS//
	//*********************************//
	
	//------------------------------------------------------------------------------------------------
	protected override bool DoneByAssignee()
	{
		SCR_BaseTaskExecutor assignee = GetAssignee();
		if (assignee && m_TargetBase)
		{
			int assigneeID = SCR_BaseTaskExecutor.GetTaskExecutorID(assignee);
			int reconfigurerID = m_TargetBase.GetReconfiguredByID();
			if (assigneeID == reconfigurerID)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Shows a message about this task being available again
	protected override void ShowAvailableTask(bool afterAssigneeRemoved = false)
	{
		SCR_ECannotAssignReasons reason;
		if (CanBeAssigned(reason))
		{
			SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
			if (campaign)
			{
				string text = TASK_AVAILABLE_TEXT + " " + GetTitle();
				string baseName;
				
				if (m_TargetBase)
					baseName = GetBaseNameWithCallsign();
				
				SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, param1: baseName, text2: TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT, sound: UISounds.TASK_CREATED);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ShowTaskProgress(bool showMsg = true)
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (campaign && showMsg)
		{
			string baseName;
			
			if (m_TargetBase)
				baseName = GetBaseNameWithCallsign();
			
			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_PROGRESS_TEXT + " " + GetTitle() + " " + TASK_AMOUNT_COMPLETED_TEXT, prio: SCR_ECampaignPopupPriority.TASK_PROGRESS, param1: baseName);
		}
	}
	
	//******************************//
	//PUBLIC OVERRIDE MEMBER METHODS//
	//******************************//
	
	//------------------------------------------------------------------------------------------------
	override string GetIconSuffix()
	{
		if (m_TargetBase && m_TargetBase.GetType() == CampaignBaseType.RELAY)
			return "_Relay";
		
		return "";
	}
	
	//------------------------------------------------------------------------------------------------
	override void DoNotifyAssignment(int assigneeID)
	{
		SCR_CampaignNetworkComponent assigneeNetworkComponent = SCR_CampaignNetworkComponent.GetCampaignNetworkComponent(assigneeID);
		if (!assigneeNetworkComponent)
			return;
		
		assigneeNetworkComponent.SendPlayerMessage(GetAssignMessage(), m_TargetBase.GetCallsign());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return the title of this task.
	override string GetTitle()
	{
		if (m_TargetBase && m_TargetBase.GetType() == CampaignBaseType.RELAY)
			return m_sNameReconfigure;
		
		return m_sName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return the description of this task.
	override string GetDescription()
	{
		if (m_TargetBase && m_TargetBase.GetType() == CampaignBaseType.RELAY)
			return m_sDescriptionReconfigure;
		
		return m_sDescription;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Finish(bool showMsg = true)
	{
		showMsg = SCR_RespawnSystemComponent.GetLocalPlayerFaction() == m_TargetFaction;
		super.Finish(showMsg);
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		// Reward XP for reconfiguring a relay
		if (!GetTaskManager().IsProxy() && GetType() == SCR_CampaignTaskType.CAPTURE && m_TargetBase.GetType() == CampaignBaseType.RELAY)
		{
			int playerID = m_TargetBase.GetReconfiguredByID();
			PlayerController player = GetGame().GetPlayerManager().GetPlayerController(playerID);
			campaign.AwardXP(player, CampaignXPRewards.RELAY_RECONFIGURED, 1.0, DoneByAssignee());
		};
		
		string baseName;
		
		if (m_TargetBase)
			baseName = GetBaseNameWithCallsign();
		
		if (campaign && showMsg)
		{
			// TODO make this nicer
			if (m_bIndividualTask)
			{
				if (IsAssignedToLocalPlayer())
				{
					if (DoneByAssignee())
					{
						SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: UISounds.TASK_SUCCEED, text2: SCR_BaseTask.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
					}
					else
					{
						SCR_PopUpNotification.GetInstance().PopupMsg(TASK_FAILED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: UISounds.TASK_FAILED);
					}
				}
				else
				{
					if (m_aAssignees && m_aAssignees.Count() > 0)
					{
						string text;
						string par1;
						string par2;
						string par3;
						string par4;
						string par5;
						GetFinishTextData(text, par1, par2, par3, par4, par5);
						SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: par1, param2: par2, param3: par3, param4: par4, param5: par5, sound: UISounds.TASK_SUCCEED);
					}
					else
						SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: UISounds.TASK_SUCCEED, text2: SCR_BaseTask.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
				}
			}
			else
				SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: UISounds.TASK_SUCCEED, text2: SCR_BaseTask.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fails the task.
	override void Fail(bool showMsg = true)
	{
		showMsg = SCR_RespawnSystemComponent.GetLocalPlayerFaction() == m_TargetFaction;
		super.Fail(showMsg);
		
		/*SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (campaign && showMsg)
		{
			string baseName;
		
			if (m_TargetBase)
				baseName = GetBaseNameWithCallsign();
			
			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_FAILED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: UISounds.TASK_FAILED);
		}*/
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns a text informing about the task being finished
	override string GetFinishText()
	{
		// TODO Add other types of tasks here once needed...
		string text = "";
		
		switch (m_eType)
		{
			case SCR_CampaignTaskType.CAPTURE:
			{
				if (m_TargetBase)
					if (m_bIndividualTask && DoneByAssignee())
						text = string.Format(CAMPAIGN_TASK_RECONFIGURED_BY_TEXT, GetBaseNameWithCallsign(), GetAllAssigneeNamesString());
					else
						text = string.Format(CAMPAIGN_TASK_RECONFIGURED_TEXT, GetBaseNameWithCallsign());
				
				break;
			};
		}
			
		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns text data informing about the task being finished
	void GetFinishTextData(out string text = "", out string param1 = "", out string param2 = "", out string param3 = "", out string param4 = "", out string param5 = "")
	{
		switch (m_eType)
		{
			case SCR_CampaignTaskType.CAPTURE:
			{
				if (m_TargetBase)
					if (m_bIndividualTask && DoneByAssignee())
					{
						text = CAMPAIGN_TASK_RECONFIGURED_BY_TEXT;
						param1 = GetBaseNameWithCallsign();
						param2 = GetAllAssigneeNamesString();
					}
					else
					{
						text = CAMPAIGN_TASK_RECONFIGURED_TEXT;
						param1 = GetBaseNameWithCallsign();
					}

				break;
			};
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetMapDescriptorText()
	{
		return GetTaskListTaskText();
	}
	
	//*********************//
	//PUBLIC MEMBER METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	override void SetTitleWidgetText(notnull TextWidget textWidget, string taskText)
	{
		string baseName;
		
		if (m_TargetBase)
			baseName = GetBaseNameWithCallsign();
		
		textWidget.SetTextFormat(taskText, baseName);
	}

	//------------------------------------------------------------------------------------------------
	override string GetTitleText()
	{
		string baseName;

		if (m_TargetBase)
			baseName = GetBaseNameWithCallsign();

		return string.Format("%1 %2", m_sName, baseName);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetDescriptionWidgetText(notnull TextWidget textWidget, string taskText)
	{
		string baseName;
		
		if (m_TargetBase)
			baseName = GetBaseNameWithCallsign();
		
		textWidget.SetTextFormat(taskText, baseName);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the type of this task.
	SCR_CampaignTaskType GetType()
	{
		return m_eType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the type of this task.
	void SetType(SCR_CampaignTaskType type)
	{
		m_eType = type;
		
		// Assign proper waypoint unless it's a relay 
		SetAIWaypoint();
	}
	
	//***************************//
	//PUBLIC MEMBER EVENT METHODS//
	//***************************//
	
	//------------------------------------------------------------------------------------------------
	override void Serialize(ScriptBitWriter writer)
	{
		super.Serialize(writer);
		
		int baseID = -1;
		SCR_CampaignBase base = GetTargetBase();
		if (base)
			baseID = base.GetBaseID();
		
		writer.WriteInt(baseID);
		writer.WriteIntRange(GetType(), 0, SCR_CampaignTaskType.LAST-1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! An event triggered from task manager when a base has been captured.
	void OnCampaignBaseCaptured(SCR_CampaignBase capturedBase)
	{
		if (!m_TargetBase || !capturedBase || !m_TargetFaction || m_eType != SCR_CampaignTaskType.CAPTURE)
			return;
		
		SCR_CampaignFaction castFaction = SCR_CampaignFaction.Cast(m_TargetFaction);
		
		if (capturedBase != m_TargetBase)
		{
			if (!castFaction)
				return;
			
			if (!m_TargetBase.IsBaseInFactionRadioSignal(castFaction))
				GetTaskManager().FailTask(this);
			
			return;
		}
		
		if (castFaction && !m_TargetBase.IsBaseInFactionRadioSignal(castFaction))
		{
			if (capturedBase.GetType() != CampaignBaseType.RELAY)
				GetTaskManager().FailTask(this);
		}
		
		if (capturedBase.GetOwningFaction() == m_TargetFaction)
			GetTaskManager().FinishTask(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignTask(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode(this))
			return;
		
		if (!GetTaskManager().IsProxy())
			SCR_GameModeCampaignMP.s_OnBaseCaptured.Insert(OnCampaignBaseCaptured);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTask()
	{
		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
			return;
		
		if (!GetTaskManager().IsProxy())
			SCR_GameModeCampaignMP.s_OnBaseCaptured.Remove(OnCampaignBaseCaptured);
	}
};
