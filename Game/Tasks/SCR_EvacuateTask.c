[EntityEditorProps(category: "GameScripted/Tasks", description: "A support task.", color: "0 0 255 255")]
class SCR_EvacuateTaskClass: SCR_RequestedTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_EvacuateTask : SCR_RequestedTask
{
	static const string SUPPORT_TASK_DESCRIPTION_TEXT = "#AR-CampaignTasks_TitleEvac";
	static const CampaignBaseType BASES_FILTER = CampaignBaseType.MAJOR | CampaignBaseType.MAIN;
	
	protected vector m_vStartOrigin;
	
	//------------------------------------------------------------------------------------------------
	static float GetMinDistanceFromStart()
	{
		SCR_EvacuateTaskSupportClass supportClass = SCR_EvacuateTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskBySupportClassType(SCR_EvacuateTaskSupportClass));
		if (supportClass)
			return supportClass.GetMinDistanceFromStart();
		
		return 1000;
	}
	
	//------------------------------------------------------------------------------------------------
	static float GetMaxDistanceFromRequester()
	{
		SCR_EvacuateTaskSupportClass supportClass = SCR_EvacuateTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskBySupportClassType(SCR_EvacuateTaskSupportClass));
		if (supportClass)
			return supportClass.GetMaxDistanceFromRequester();
		
		return 50;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetMapDescriptorText()
	{
		return GetTaskListTaskText();
	}
	
	//------------------------------------------------------------------------------------------------
	void PeriodicalCheck()
	{
		bool assigneeCloseEnough = true;
		bool requesterInBase = false;
		GenericEntity requesterEntity = GenericEntity.Cast(m_Requester.GetControlledEntity());
		
		if (!requesterEntity)
		{
			GetTaskManager().FailTask(this);
			return;
		}
		
		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(requesterEntity.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliationComponent)
			return;
		
		Faction requesterFaction = factionAffiliationComponent.GetAffiliatedFaction();
		
		vector requesterOrigin = requesterEntity.GetOrigin();
		
		array<SCR_CampaignBase> bases = new array<SCR_CampaignBase>();
		
		SCR_CampaignBaseManager.GetInstance().GetFilteredBases(SCR_EvacuateTask.BASES_FILTER, bases);
		
		for (int i = 0; i < bases.Count(); i++)
		{
			if (requesterFaction != bases[i].GetOwningFaction())
				continue;
			
			vector baseOrigin = bases[i].GetOrigin();
			float baseToStartDistance = vector.Distance(baseOrigin, m_vStartOrigin);
			
			if (baseToStartDistance < SCR_EvacuateTask.GetMinDistanceFromStart())
				continue;
			
			SCR_CampaignTriggerEntity trigger = bases[i].GetTrigger();
			if (!trigger.QueryEntityInside(requesterEntity))
				continue;
			
			requesterInBase = true;
			break;
		}
		
		SCR_BaseTaskExecutor assignee = GetAssignee();
		if (!assignee)
			assigneeCloseEnough = false;
		else
		{
			IEntity assigneeEntity = assignee.GetControlledEntity();
			vector assigneePosition = vector.Zero;
			if (assigneeEntity)
				assigneePosition = assigneeEntity.GetOrigin();
			float distance = vector.Distance(requesterOrigin, assigneePosition);
			
			if (distance > SCR_EvacuateTask.GetMaxDistanceFromRequester())
				assigneeCloseEnough = false;
		}
		
		if (requesterInBase)
		{
			if (assigneeCloseEnough)
				GetTaskManager().FinishTask(this);
			else
				GetTaskManager().FailTask(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetStartOrigin(vector startOrigin)
	{
		if (m_vStartOrigin != vector.Zero)
		{
			m_vStartOrigin = startOrigin;
			return;
		}
		
		if (!m_Requester)
			return;
		
		GenericEntity requesterEntity = GenericEntity.Cast(m_Requester.GetControlledEntity());
		if (!requesterEntity)
			return;
		
		m_vStartOrigin = requesterEntity.GetOrigin();
		
		ShowAvailableTask();
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetStartOrigin()
	{
		return m_vStartOrigin;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Serialize(ScriptBitWriter writer)
	{
		super.Serialize(writer);
		
		vector startOrigin = GetStartOrigin();
		writer.WriteVector(startOrigin);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_EvacuateTask(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode(this))
			return;
		
		SetFlags(EntityFlags.ACTIVE, true);
		SetEventMask(EntityEvent.FRAME);
		SetIndividual(true);
		
		if (!GetTaskManager().IsProxy())
			SCR_BaseTaskManager.s_OnPeriodicalCheck5Second.Insert(PeriodicalCheck);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_EvacuateTask()
	{
		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
			return;
		
		if (!GetTaskManager().IsProxy())
			SCR_BaseTaskManager.s_OnPeriodicalCheck5Second.Remove(PeriodicalCheck);
	}

};
