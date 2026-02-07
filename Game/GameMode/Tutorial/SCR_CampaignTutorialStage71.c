class SCR_CampaignTutorialStage71Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage71 : SCR_BaseCampaignTutorialStage
{
	SCR_CampaignTask m_Task;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Volunteer", duration: -1);
		
		if (!GetTaskManager())
			return;
	
		SCR_CampaignBase baseLaruns = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseLaruns"));
	
		if (!baseLaruns)
			return;
	
		SCR_CampaignTaskSupportEntity supportClass = SCR_CampaignTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignTaskSupportEntity));
	
		if (!supportClass)
			return;
		
		m_Task = supportClass.GetTask(baseLaruns, GetGame().GetFactionManager().GetFactionByKey(SCR_GameModeCampaignMP.FACTION_BLUFOR), SCR_CampaignTaskType.CAPTURE);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (m_Task)
			return (m_Task.GetAssignee() == SCR_BaseTaskExecutor.GetLocalExecutor());
		else
			return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckCheckpointPos"));
	}
};