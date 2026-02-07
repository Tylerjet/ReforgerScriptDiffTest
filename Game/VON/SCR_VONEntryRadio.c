//------------------------------------------------------------------------------------------------
//! VONEntry class for radio entries
class SCR_VONEntryRadio : SCR_VONEntry
{	
	const string LABEL_FREQUENCY_UNITS = "#AR-VON_FrequencyUnits_MHz";
	const string LABEL_DEACTIVATED = "#AR-Radio_TurnedOff";
	
	bool m_bIsLongRange;			// whether this is personal or backpack radio
	protected int m_iFrequency;		// current frequency
	
	//------------------------------------------------------------------------------------------------
	override void InitEntry()
	{
		m_bIsEnabled = m_RadioTransceiver.GetRadio().IsPowered();
		
		if (m_bIsEnabled)
			AdjustEntry(0);
		else 
			m_sText = LABEL_DEACTIVATED;
		
		if (m_GadgetComp.GetType() == EGadgetType.RADIO_BACKPACK)
			m_bIsLongRange = true;
	}
	
	//------------------------------------------------------------------------------------------------ 
	override void ActivateEntry()
	{
		// This check shouldn't be necessary if entries are assured to be notnull
		if (!m_VONComp)
			return;

		m_VONComp.SetCommMethod(ECommMethod.SQUAD_RADIO);
		m_VONComp.SetTransmitRadio(m_RadioTransceiver);
		m_VONController.SetEntryActive(this);
	}
	
	//------------------------------------------------------------------------------------------------ 
	override void AdjustEntry(int modifier)
	{		
		if (!m_bIsEnabled)
			return;
		
		if (!m_RadioTransceiver)
			return;
		
		// Get & adjust frequency
		m_iFrequency = m_RadioTransceiver.GetFrequency();
		int minFreq = m_RadioTransceiver.GetMinFrequency();
		int maxFreq = m_RadioTransceiver.GetMaxFrequency();
		
		// TODO sound temp here until moved to radio level
		if ( (modifier > 0  && m_iFrequency == maxFreq) || (modifier < 0 && m_iFrequency == minFreq) )
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_CHANGEFREQUENCY_ERROR);
		else if (modifier != 0)
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_CHANGEFREQUENCY);
		
		m_iFrequency = m_iFrequency + (modifier * m_RadioTransceiver.GetFrequencyResolution());
		m_iFrequency = Math.ClampInt(m_iFrequency, minFreq, maxFreq);
		
		//FIXME: Do not even try to change frequency without PlayerController
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
		{
			RadioHandlerComponent rhc = RadioHandlerComponent.Cast(pc.FindComponent(RadioHandlerComponent));
			if (rhc)
			{
				rhc.SetFrequency(m_RadioTransceiver, m_iFrequency); // Set new frequency
			}
		}

		float fFrequency = Math.Round(m_iFrequency / 10); // Format the frequency text
		fFrequency = fFrequency / 100;			
		m_sText = fFrequency.ToString(3, 1) + " " + LABEL_FREQUENCY_UNITS;		
	}
	
	//------------------------------------------------------------------------------------------------ 
	override void AdjustEntryModif(int modifier)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_GroupsManagerComponent));
		SCR_Faction playerFaction = SCR_Faction.Cast(SCR_RespawnSystemComponent.GetInstance().GetPlayerFaction(GetGame().GetPlayerController().GetPlayerId()));
		
		if (!groupManager || !playerFaction)
			return;

		if (!m_RadioTransceiver)
			return;

		int factionFreq = playerFaction.GetFactionRadioFrequency();
		int currentFreq = m_RadioTransceiver.GetFrequency();
		
		array<SCR_AIGroup> groups = groupManager.GetPlayableGroupsByFaction(playerFaction);

		int newFreq;
		
		if (currentFreq == factionFreq)		// if platoon frequency
		{
			if (modifier == -1)
				newFreq = groups[0].GetRadioFrequency();	// go to first squad freq
			else 
				newFreq = groups[groups.Count() - 1].GetRadioFrequency();	// go to last squad freq
		}
		else 
		{
			int count = groups.Count();
			if (modifier == 1 && currentFreq == groups[0].GetRadioFrequency())
				newFreq = factionFreq;
			else if (modifier == -1 && currentFreq == groups[groups.Count() - 1].GetRadioFrequency())
				newFreq = factionFreq;
			else 
			{
				bool isMatched;
				
				for (int i = 0; i < count; i++)
				{
					if (currentFreq == groups[i].GetRadioFrequency())
					{
						if (modifier == 1)
							newFreq = groups[i-1].GetRadioFrequency();
						else 
							newFreq = groups[i+1].GetRadioFrequency();
						
						isMatched = true;
						break;
					}
				}	
				
				if (!isMatched)
					newFreq = groups[0].GetRadioFrequency();
			}
		}
		
		m_iFrequency = newFreq;
		m_RadioTransceiver.SetFrequency(m_iFrequency);
		
		float fFrequency = Math.Round(m_iFrequency / 10); // Format the frequency text
		fFrequency = fFrequency / 100;			
		m_sText = fFrequency.ToString(3, 1) + " " + LABEL_FREQUENCY_UNITS;		
		
	}
	
	//------------------------------------------------------------------------------------------------ 
	override void ToggleEntry()
	{
		// Constructor allows the entry to not have m_RadioTransceiver assigned
		if (!m_RadioTransceiver)
			return;
		
		BaseRadioComponent radio = m_RadioTransceiver.GetRadio();

		m_bIsEnabled = !radio.IsPowered();
		
		radio.SetPower(m_bIsEnabled);
		
		if (!m_bIsEnabled)
			m_sText = LABEL_DEACTIVATED;
		else 
			AdjustEntry(0);	
	}
	
	//------------------------------------------------------------------------------------------------ 
	override bool CanBeAdjusted()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------ 
	override bool CanBeToggled()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetIconResource()
	{
		if (!m_RadioTransceiver)
			return string.Empty;
		

		IEntity entity = m_RadioTransceiver.GetRadio().GetOwner();
		// Item will be available, since Transceiver should not exist without RadioComponent
		
		InventoryItemComponent pInvItemComponent = InventoryItemComponent.Cast(entity.FindComponent(InventoryItemComponent));
		if (!pInvItemComponent)
			return string.Empty;			

		ItemAttributeCollection pInvItemAttributes = pInvItemComponent.GetAttributes();
		if (!pInvItemAttributes)
			return string.Empty;
		
		UIInfo uiInfo = pInvItemAttributes.GetUIInfo();
		if (!uiInfo)
			return string.Empty;
		
		return uiInfo.GetIconPath();
	}	
	
	//------------------------------------------------------------------------------------------------
	override ECommMethod GetVONMethod()
	{
		return ECommMethod.SQUAD_RADIO;
	}
};
