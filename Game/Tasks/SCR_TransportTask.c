[EntityEditorProps(category: "GameScripted/Tasks", description: "Transport task.")]
class SCR_TransportTaskClass : SCR_RequestedTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TransportTask : SCR_RequestedTask
{
	[Attribute("")]
	protected LocalizedString m_sAssigneeMetText;
	
	[Attribute("")]
	protected LocalizedString m_sRequesterMetText;
	
	protected vector m_vTargetPosition = vector.Zero;
	protected bool m_bVolunteerMet = false;
	
	//------------------------------------------------------------------------------------------------
	bool GetVolunteerMet()
	{
		return m_bVolunteerMet;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVolunteerMet(bool volunteerMet)
	{
		m_bVolunteerMet = volunteerMet;
		
		if (volunteerMet)
		{
			if (IsAssignedToLocalPlayer())
				SCR_PopUpNotification.GetInstance().PopupMsg(m_sRequesterMetText, prio: SCR_ECampaignPopupPriority.TASK_DONE);
			if (this == GetTaskManager().GetLocallyRequestedTask())
				SCR_PopUpNotification.GetInstance().PopupMsg(m_sAssigneeMetText, prio: SCR_ECampaignPopupPriority.TASK_DONE);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTargetPosition(vector targetPosition)
	{
		m_vTargetPosition = targetPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetTargetPosition()
	{
		return m_vTargetPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAssigneeKilled()
	{
		if (GetTaskManager().IsProxy())
			return;
		
		if (GetVolunteerMet())
			GetTaskManager().FailTask(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void PeriodicalCheck()
	{
		SCR_BaseTaskExecutor assignee = GetAssignee();
		if (!assignee)
			return;
		
		IEntity assigneeEntity = assignee.GetControlledEntity();
		IEntity requesterEntity = m_Requester.GetControlledEntity();
		
		SCR_TransportTaskSupportClass supportClass = SCR_TransportTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskByTaskType(Type()));
		if (!supportClass)
			return;
		
		if (!GetVolunteerMet())
		{
			if (vector.DistanceSq(assigneeEntity.GetOrigin(), requesterEntity.GetOrigin()) > supportClass.GetMaxDistanceAssigneeSq())
				return;
			
			GetTaskManager().TransportTaskNextPhase(this);
			return;
		}
		else if (vector.DistanceSq(requesterEntity.GetOrigin(), GetOrigin()) > supportClass.GetMaxDistanceDestinationSq())
			return;
		
		GetTaskManager().FinishTask(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Serialize(ScriptBitWriter writer)
	{
		super.Serialize(writer);
		
		vector targetPosition = GetTargetPosition();
		writer.WriteVector(targetPosition);
		writer.WriteBool(m_bVolunteerMet);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (taskManager && taskManager.IsProxy())
			return;
		
		SCR_BaseTaskManager.s_OnPeriodicalCheck2Second.Insert(PeriodicalCheck);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_TransportTask(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_TransportTask()
	{
	}

};
