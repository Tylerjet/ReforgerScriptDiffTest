class SCR_CampaignTutorialStage70Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage70 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 12;
		string custommsg;
				
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			custommsg = CreateString("#AR-Tutorial_ExpandTask", "MouseLeft");
		else
			custommsg = CreateString("#AR-Tutorial_ExpandTask", "CharacterAction");
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_ObjectivesListInfo" + custommsg, duration: 12, isTimerVisible: true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckCheckpointPos"));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInputDeviceChanged(bool switchedToKeyboard)
	{
		string custommsg;
				
		if (switchedToKeyboard)
			custommsg = CreateString("#AR-Tutorial_ExpandTask", "MouseLeft");
		else
			custommsg = CreateString("#AR-Tutorial_ExpandTask", "CharacterAction");
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_ObjectivesListInfo" + custommsg, duration: 12, isTimerVisible: true);
	}
};