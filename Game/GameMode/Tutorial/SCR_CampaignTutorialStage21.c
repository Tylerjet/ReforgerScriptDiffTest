class SCR_CampaignTutorialStage21Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage21 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDelay = 1;
		
		foreach (SCR_FiringRangeTarget target : m_TutorialComponent.GetAllTargets())
		{
			if (target.GetSetDistance() == 5)
			{
				target.SetState(ETargetState.TARGET_UP);
				target.Event_TargetChangeState.Remove(m_TutorialComponent.CountTargetHit);
				target.Event_TargetChangeState.Insert(m_TutorialComponent.CountTargetHit);
			}
		}
		
		string custommsg;
		
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			custommsg = CreateString("#AR-Keybind_Aim","CharacterWeaponADS");
		else
			custommsg = CreateString("#AR-Keybind_Aim","CharacterWeaponADSHold");
		
		custommsg = custommsg + CreateString("#AR-Keybind_Fire","CharacterFire");
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Firing" + custommsg, duration: -1);
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
			custommsg = CreateString("#AR-Keybind_Aim","CharacterWeaponADS");
		else
			custommsg = CreateString("#AR-Keybind_Aim","CharacterWeaponADSHold");
		
		custommsg = custommsg + CreateString("#AR-Keybind_Fire","CharacterFire");
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Firing" + custommsg, duration: -1, isSilent: true);
	}
};