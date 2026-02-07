class SCR_CampaignTutorialStage30Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage30 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_1");
		m_fWaypointCompletionRadius = 10;
		m_fWaypointHeightOffset = 0;
		string custommsg;
		
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			custommsg = CreateString("#AR-Keybind_Freelook","Freelook");
		else
			custommsg = CreateString("#AR-Keybind_Freelook","MenuEnable");
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SwitchingSeats" + custommsg, duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_MoveInJeep(m_WaypointEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInputDeviceChanged(bool switchedToKeyboard)
	{
		string custommsg;
		
		if (switchedToKeyboard)
			custommsg = CreateString("#AR-Keybind_Freelook","Freelook");
		else
			custommsg = CreateString("#AR-Keybind_Freelook","ManualCameraAttach");
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SwitchingSeats" + custommsg, duration: -1);
	}
};