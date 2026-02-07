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
		m_bIsEnabled = m_RadioComp.IsPowered();
		
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
		m_VONComp.SetCommMethod(ECommMethod.SQUAD_RADIO);
		m_VONComp.SetTransmitRadio(m_RadioComp);
		m_VONController.SetEntryActive(this);
	}
	
	//------------------------------------------------------------------------------------------------ 
	override void AdjustEntry(int modifier)
	{		
		if (!m_bIsEnabled)
			return;
		
		// Get & adjust frequency
		m_iFrequency = m_RadioComp.GetFrequency();
		int minFreq = m_RadioComp.GetMinFrequency();
		int maxFreq = m_RadioComp.GetMaxFrequency();
		
		// TODO sound temp here until moved to radio level
		if ( (modifier > 0  && m_iFrequency == maxFreq) || (modifier < 0 && m_iFrequency == minFreq) )
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_CHANGEFREQUENCY_ERROR);
		else if (modifier != 0)
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_CHANGEFREQUENCY);
		
		m_iFrequency = m_iFrequency + (modifier * m_RadioComp.GetFrequencyResolution());
		m_iFrequency = Math.ClampInt(m_iFrequency, minFreq, maxFreq);
		
		m_RadioComp.SetFrequency(m_iFrequency); // Set new frequency

		float fFrequency = Math.Round(m_iFrequency / 10); // Format the frequency text
		fFrequency = fFrequency / 100;			
		m_sText = fFrequency.ToString(3, 1) + " " + LABEL_FREQUENCY_UNITS;		
	}
	
	//------------------------------------------------------------------------------------------------ 
	override void AdjustEntryModif(int modifier)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_GroupsManagerComponent));
		SCR_MilitaryFaction playerFaction = SCR_MilitaryFaction.Cast(SCR_RespawnSystemComponent.GetInstance().GetPlayerFaction(GetGame().GetPlayerController().GetPlayerId()));
		
		if (!groupManager || !playerFaction)
			return;
		
		int factionFreq = playerFaction.GetFactionRadioFrequency();
		int currentFreq = m_RadioComp.GetFrequency();
		
		array<SCR_AIGroup> groups = groupManager.GetPlayableGroupsByFaction(playerFaction);

		int newFreq;
		
		if (currentFreq == factionFreq)		// if platoon frequency
		{
			if (modifier == -1)
				newFreq = groups[0].GetGroupFrequency();	// go to first squad freq
			else 
				newFreq = groups[groups.Count() - 1].GetGroupFrequency();	// go to last squad freq
		}
		else 
		{
			int count = groups.Count();
			if (modifier == 1 && currentFreq == groups[0].GetGroupFrequency())
				newFreq = factionFreq;
			else if (modifier == -1 && currentFreq == groups[groups.Count() - 1].GetGroupFrequency())
				newFreq = factionFreq;
			else 
			{
				bool isMatched;
				
				for (int i = 0; i < count; i++)
				{
					if (currentFreq == groups[i].GetGroupFrequency())
					{
						if (modifier == 1)
							newFreq = groups[i-1].GetGroupFrequency();
						else 
							newFreq = groups[i+1].GetGroupFrequency();
						
						isMatched = true;
						break;
					}
				}	
				
				if (!isMatched)
					newFreq = groups[0].GetGroupFrequency();
			}
		}
		
		m_iFrequency = newFreq;
		m_RadioComp.SetFrequency(m_iFrequency);
		
		float fFrequency = Math.Round(m_iFrequency / 10); // Format the frequency text
		fFrequency = fFrequency / 100;			
		m_sText = fFrequency.ToString(3, 1) + " " + LABEL_FREQUENCY_UNITS;		
		
	}
	
	//------------------------------------------------------------------------------------------------ 
	override void ToggleEntry()
	{
		m_bIsEnabled = !m_RadioComp.IsPowered();
		
		m_RadioComp.TogglePower(m_bIsEnabled);
		
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
		IEntity item = m_RadioComp.GetOwner();
		if (!item)
			return string.Empty;
		
		InventoryItemComponent pInvItemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
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
