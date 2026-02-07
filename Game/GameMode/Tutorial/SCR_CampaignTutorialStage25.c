class SCR_CampaignTutorialStage25Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage25 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_FIREPOZ_2");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		m_fDelay = 1;
		m_TutorialComponent.SetCheckLeaning(true);
				
		foreach (SCR_FiringRangeTarget target : m_TutorialComponent.GetAllTargets())
		{		
			if (target.GetSetDistance() == 10)
			{
				target.SetState(ETargetState.TARGET_UP);
				target.SetAutoResetTarget(true);
				target.Event_TargetChangeState.Remove(m_TutorialComponent.CountTargetHit);
				target.Event_TargetChangeState.Insert(m_TutorialComponent.CountTargetHit);
			}
		}
		
		string custommsg;
		
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			custommsg = CreateString("#AR-Keybind_Lean","CharacterLean");
		else
			custommsg = CreateString("#AR-Keybind_Lean","CharacterLeanVariable");
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Leaning" + custommsg, duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return (m_TutorialComponent.GetTargetHits() > 4);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_RifleRespawn();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInputDeviceChanged(bool switchedToKeyboard)
	{
		string custommsg;
		
		if (switchedToKeyboard)
			custommsg = CreateString("#AR-Keybind_Lean","CharacterLean");
		else
			custommsg = CreateString("#AR-Keybind_Lean","CharacterLeanVariable");
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Leaning" + custommsg, duration: -1, isSilent: true);
	}
};