class SCR_CampaignTutorialStage69Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage69 : SCR_BaseCampaignTutorialStage
{
	protected bool m_bTaskMenuOpened;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_ObjectivesListOpen" + CreateString("#AR-Keybind_Tasks","TasksOpen"), duration: -1);
		GetGame().GetInputManager().AddActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_bTaskMenuOpened;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckCheckpointPos"));
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterTasksShown()
	{
		m_bTaskMenuOpened = true;
		GetGame().GetInputManager().RemoveActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
	}
};