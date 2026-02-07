class SCR_CampaignTutorialStage86Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage86 : SCR_BaseCampaignTutorialStage
{
	protected float m_fSavedTime;
	protected AudioHandle m_PlayedRadio;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fConditionCheckPeriod = 0.1;
		string custommsg;
		
		if(GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			custommsg = CreateString("#AR-Keybind_VONChannel","VONChannel");
		else
			custommsg = CreateString("#AR-Keybind_VONChannel","VONGamepad");
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Transmission" + custommsg, duration: -1);
		InputManager inputMan = GetGame().GetInputManager();
		inputMan.AddActionListener("VONChannel", EActionTrigger.DOWN, OnVOIPPress);
		inputMan.AddActionListener("VONChannel", EActionTrigger.UP, OnVOIPRelease);
		inputMan.AddActionListener("VONGamepad", EActionTrigger.DOWN, OnVOIPPress);
		inputMan.AddActionListener("VONGamepad", EActionTrigger.UP, OnVOIPRelease);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		BaseRadioComponent radio = m_TutorialComponent.GetPlayerRadio();
		
		if (!radio)
			return false;
		
		if (radio.GetFrequency() != DESIRED_FREQUENCY)
			m_fSavedTime = 0;
		
		bool done = (radio.GetFrequency() == DESIRED_FREQUENCY && m_fSavedTime != 0 && m_fSavedTime + 6000 <= GetGame().GetWorld().GetWorldTime());
		
		if (done)
		{
			InputManager inputMan = GetGame().GetInputManager();
			inputMan.RemoveActionListener("VONChannel", EActionTrigger.DOWN, OnVOIPPress);
			inputMan.RemoveActionListener("VONChannel", EActionTrigger.UP, OnVOIPRelease);
			inputMan.RemoveActionListener("VONGamepad", EActionTrigger.DOWN, OnVOIPPress);
			inputMan.RemoveActionListener("VONGamepad", EActionTrigger.UP, OnVOIPRelease);
			return true;
		}
		else
		{
			return false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_CaptureLaruns();
		m_TutorialComponent.StageReset_HandleRespawnRadios();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInputDeviceChanged(bool switchedToKeyboard)
	{
		string custommsg;
				
		if (switchedToKeyboard)
			custommsg = CreateString("#AR-Keybind_VONChannel","VONChannel");
		else
			custommsg = CreateString("#AR-Keybind_VONChannel","VONGamepad");
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Transmission" + custommsg, duration: -1, isSilent: true);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnVOIPPress()
	{
		BaseRadioComponent radio = m_TutorialComponent.GetPlayerRadio();
		
		if (!radio)
			return;
		
		IEntity unit = radio.GetOwner().GetParent();
		
		if (unit == m_Player && radio.GetFrequency() == DESIRED_FREQUENCY && radio.IsPowered())
		{
			m_fSavedTime = GetGame().GetWorld().GetWorldTime();
			PlayRadioMsg();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnVOIPRelease()
	{
		PlayRadioMsg(true);
		m_fSavedTime = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayRadioMsg(bool stop = false)
	{
		AudioSystem.TerminateSound(m_PlayedRadio);
		
		if (stop)
			return;
		
		SCR_CommunicationSoundComponent soundComp = SCR_CommunicationSoundComponent.Cast(m_Player.FindComponent(SCR_CommunicationSoundComponent));
		
		if (!soundComp)
			return;
		
		SignalsManagerComponent signalComp = SignalsManagerComponent.Cast(m_Player.FindComponent(SignalsManagerComponent));
		
		if (!signalComp)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		SCR_CallsignManagerComponent callsignManager = SCR_CallsignManagerComponent.Cast(campaign.FindComponent(SCR_CallsignManagerComponent));
		
		if (!callsignManager)
			return;
		
		int baseCallsign = SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("MainBaseLevie")).GetCallsign();
		
		int signalBase = signalComp.AddOrFindSignal("Base");
		int signalCompanyCaller = signalComp.AddOrFindSignal("CompanyCaller");
		int signalPlatoonCaller = signalComp.AddOrFindSignal("PlatoonCaller");
		int signalSquadCaller = signalComp.AddOrFindSignal("SquadCaller");
		
		int callerCallsignCompany, callerCallsignPlatoon, callerCallsignSquad, callerCallsignCharacter;
		callsignManager.GetEntityCallsignIndexes(m_Player, callerCallsignCompany, callerCallsignPlatoon, callerCallsignSquad, callerCallsignCharacter);
		
		signalComp.SetSignalValue(signalBase, baseCallsign);
		signalComp.SetSignalValue(signalCompanyCaller, callerCallsignCompany);
		signalComp.SetSignalValue(signalPlatoonCaller, callerCallsignPlatoon);
		signalComp.SetSignalValue(signalSquadCaller, callerCallsignSquad);
		
		m_PlayedRadio = soundComp.SoundEvent(SCR_SoundEvent.SOUND_SL_SRT);
	}
};