//------------------------------------------------------------------------------------------------
//! Type of a players current VON transmission
enum EVONTransmitType
{
	NONE,
	DIRECT,
	CHANNEL,
	LONG_RANGE
}

//------------------------------------------------------------------------------------------------
// invoker typedefs
void OnVONActiveToggled(bool isToggledDirect, bool isToggledChannel);	// when direct toggle activates
typedef func OnVONActiveToggled;

void OnEntriesChanged(SCR_VONEntry entry, bool state);			
typedef func OnEntriesChanged;

//------------------------------------------------------------------------------------------------
class SCR_VONControllerClass: ScriptGameComponentClass
{}

//------------------------------------------------------------------------------------------------
//! Scripted VON input and control, attached to SCR_PlayerController
class SCR_VONController : ScriptComponent
{
	protected const string VON_MENU_OPENING_CONTEXT = "VONMenuOpeningContext";
	
	[Attribute("", UIWidgets.Object)]
	protected ref SCR_VONMenu m_VONMenu;
		
	protected const string VON_DIRECT_HOLD = "VONDirect";
	protected const string VON_CHANNEL_HOLD = "VONChannel";
	protected const string VON_LONG_RANGE_HOLD = "VONLongRange";
	protected const float TOGGLE_OFF_DELAY = 0.5;		// seconds, delay before toggle state can get canceled, to avoid collision of double click and press
	
	static bool s_bIsInit;					// component init done, static so its only done once
	protected bool m_bIsDisabled; 			// VON control is disabled
	protected bool m_bIsPauseDisabled;		// disabled by pause menu
	protected bool m_bIsUnconscious; 		// Character is unconscious --> VON control is disabled
	protected bool m_bIsActive;				// VON is active
	protected bool m_bIsToggledDirect;		// VON direct speech toggle is active
	protected bool m_bIsToggledChannel;		// VON channel speech toggle is active
	protected bool m_bIsUsingGamepad;		// is using gamepad / KBM
	protected bool m_bIsActiveModeDirect;	// used for controller, switch between direct VON = true / channel VON = false
	protected bool m_bIsActiveModeLong;		// used for controller, switch between long range VON = true / channel VON = false
	protected float m_fToggleOffDelay;		// used to track delay before toggle can be cancelled
	protected string m_sActiveHoldAction;	// tracker for ending the hold action VON 
	protected string m_sLocalEncryptionKey;	// local players faction encryption key
	protected EVONTransmitType m_eVONType;	// currently active VON type
	protected InputManager m_InputManager;

	protected SCR_VoNComponent m_VONComp;					// VON component used for transmission
	protected SCR_VonDisplay m_VONDisplay;					// VON transmission display
	protected SCR_VONEntry m_ActiveEntry;					// active entry (non direct speech)
	protected SCR_VONEntry m_LongRangeEntry;				// entry for long range radio, if available
	protected SCR_VONEntry m_SavedEntry;					// entry used for switching to different active entry after current one is done, f.e. after VONLongRange
	protected ref SCR_VONEntry m_DirectSpeechEntry;			// separate direct speech entry
	protected ref array<ref SCR_VONEntry> m_aEntries = {};	// all entries except direct
	
	
	protected ref ScriptInvokerBase<OnEntriesChanged> 	m_OnEntriesChanged = new ScriptInvokerBase<OnEntriesChanged>();			// when entry array is modified (SCR_VONEntry entry, bool isAdded)
	protected ref ScriptInvokerBase<OnEntriesChanged> 	m_OnEntriesActiveChanged = new ScriptInvokerBase<OnEntriesChanged>();	// when entry array is modified (SCR_VONEntry entry, bool isActive)
	protected ref ScriptInvokerBase<OnVONActiveToggled> m_OnVONActiveToggled = new ScriptInvokerBase<OnVONActiveToggled>();		// when direct toggle activates (bool isToggledDirect, bool isToggledChannel)
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<OnEntriesChanged> GetOnEntriesChangedInvoker()
	{
		return m_OnEntriesChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<OnEntriesChanged> GetOnEntriesActiveChangedInvoker()
	{
		return m_OnEntriesActiveChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<OnVONActiveToggled> GetOnVONActiveToggledInvoker()
	{
		return m_OnVONActiveToggled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDisplay(SCR_VonDisplay display)
	{ 
		m_VONDisplay = display; 
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_VonDisplay GetDisplay()
	{ 
		return m_VONDisplay; 
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_VONMenu GetVONMenu()
	{ 
		return m_VONMenu; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disables ability to transmit and related VON UI
	//! \param state is the target state
	//! \return true if this call changed the state to target, false if the state was target state already
	bool SetVONDisabled(bool state)
	{
		if (m_bIsDisabled == state)
			return false;
		
		m_bIsDisabled = state;
		UpdateSystemState();
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is VON disabled
	bool IsVONDisabled()
	{
		return m_bIsDisabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get active VON component for transmit
	SCR_VoNComponent GetVONComponent()
	{ 
		return m_VONComp; 
	}
		
	//------------------------------------------------------------------------------------------------
	//! Set component for transmit of VON
	//! \param VONComp is the subject
	void SetVONComponent(SCR_VoNComponent VONComp)
	{			
		m_VONComp = VONComp;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Get current entries
	//! \return count of entries
	int GetVONEntries(inout array<ref SCR_VONEntry> entries)
	{
		int count = m_aEntries.Count();
		for (int i = 0; i < count; i++)
		{
			entries.Insert(m_aEntries[i]);
		}

		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get number of entries
	int GetVONEntryCount()
	{
		return m_aEntries.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get active entry for outgoing transmission
	//! \return active entry
	SCR_VONEntry GetActiveEntry()
	{
		return m_ActiveEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Using long range radio
	bool IsUsingLRR()
	{
		if (!m_ActiveEntry)
			return false;
		
		return m_LongRangeEntry == m_ActiveEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Long range raido entry is available
	bool IsLRRAvailable()
	{
		return m_LongRangeEntry != null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set entry as active for outgoing transmission
	//! \param entry is subject entry
	//! \param setFromMenu determines whether this was performed by menu selection
	void SetEntryActive(SCR_VONEntry entry, bool setFromMenu = false)
	{
		if (entry == m_ActiveEntry)
			return;
				
		if (m_ActiveEntry)
		{
			m_OnEntriesActiveChanged.Invoke(m_ActiveEntry, false);
			m_ActiveEntry.SetActive(false);
		}
		
		entry.SetActive(true);
		m_ActiveEntry = entry;
		m_OnEntriesActiveChanged.Invoke(entry, true);
		
		if (m_ActiveEntry && !m_ActiveEntry.IsUsable())	// turn off toggle when switching to disabled entry
			OnVONToggle(0,0);
		
		if (m_VONDisplay)
			m_VONDisplay.ShowSelectedVONHint(m_ActiveEntry);
		
		if (m_bIsUsingGamepad && setFromMenu)	// cancel long range state if switching to personal radio
		{
			SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
			if (radioEntry && !radioEntry.IsLongRange())
				SetGamepadLRRMode(false, true);			
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Add new VON entry
	//! Set as active entry if m_ActiveEntry is null
	//! \param entry is subject entry
	void AddEntry(SCR_VONEntry entry)
	{
		m_aEntries.Insert(entry);
		entry.InitEntry();
		
		if (SCR_VONEntryRadio.Cast(entry) && SCR_VONEntryRadio.Cast(entry).IsLongRange())	// if long range entry, set it here
			m_LongRangeEntry = entry;
		
		m_OnEntriesChanged.Invoke(entry, true);
		
		if (!m_ActiveEntry)
			SetEntryActive(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove existing VON entry.
	//! Sets m_ActiveEntry to null, when removing active entry
	//! \param entry is subject entry
	void RemoveEntry(SCR_VONEntry entry)
	{
		m_OnEntriesChanged.Invoke(entry, false);
		
		//Deactivate if the entry being removed is active
		if (entry == m_ActiveEntry)
		{
			if (m_bIsToggledChannel)
				OnVONToggle(2, 0);
			else 
				DeactivateVON();
			
			m_ActiveEntry = null;
		}
		
		if (entry == m_LongRangeEntry)	// unset long range entry
			m_LongRangeEntry = null;
				
		if (m_VONMenu)
		{
			SCR_RadialMenu vonRadial = m_VONMenu.GetRadialMenu();
			if (vonRadial && vonRadial.IsOpened())
				vonRadial.Close();
		}
		
		m_aEntries.RemoveItem(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets long rang radio mode for gamepad input scheme
	protected void SetGamepadLRRMode(bool state, bool isSwitch = false)
	{
		m_bIsActiveModeLong = state;
		
		if (m_SavedEntry && !isSwitch)	// restore last saved entry if there is one
			SetActiveTransmit(m_SavedEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON direct speech listener callback
	protected void OnVONDirect(float value, EActionTrigger reason)
	{		
		if (reason == EActionTrigger.DOWN)
			VONDirect(true);
		else
			VONDirect(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON channel speech listener callback
 	protected void OnVONChannel(float value, EActionTrigger reason)
	{
		if (reason == EActionTrigger.DOWN)
			VONChannel(true);
		else
			VONChannel(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON channel speech listener callback for long range radios
 	protected void OnVONLongRange(float value, EActionTrigger reason)
	{
		if (reason == EActionTrigger.DOWN)
			VONLongRange(true);
		else
			VONLongRange(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON channel speech listener callback
 	protected void OnVONGamepad(float value, EActionTrigger reason)
	{				
		if (m_bIsActiveModeLong && !m_bIsActiveModeDirect)
		{
			if (reason == EActionTrigger.PRESSED && m_eVONType == EVONTransmitType.LONG_RANGE)
				return;
		
			if (reason == EActionTrigger.PRESSED)
				VONLongRange(true);
			else 
				VONLongRange(false);
		}
		else if (m_bIsActiveModeDirect)
		{
			if (reason == EActionTrigger.PRESSED && m_eVONType == EVONTransmitType.DIRECT)
				return;
			
			if (reason == EActionTrigger.PRESSED)
				VONDirect(true);
			else 
				VONDirect(false);
		}
		else
		{
			if (reason == EActionTrigger.PRESSED && m_eVONType == EVONTransmitType.CHANNEL)
				return;
			
			if (reason == EActionTrigger.PRESSED)
				VONChannel(true);
			else 
				VONChannel(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
 	protected void OnVONLongRangeGamepadToggle(float value, EActionTrigger reason)
	{				
		if (!m_bIsActiveModeLong && m_LongRangeEntry && m_LongRangeEntry.IsUsable())
			SetGamepadLRRMode(true);
		else
			SetGamepadLRRMode(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON toggle   val 0 = off / 1 = direct / 2 = channel
 	protected void OnVONToggle(float value, EActionTrigger reason)
	{
		if (!m_VONComp)
			return;
		
		if (value == 0)
		{
			m_fToggleOffDelay = 0;
			m_bIsToggledDirect = false;
			m_bIsToggledChannel = false;
			DeactivateVON();
		}
		else if (value == 1)
		{
			m_fToggleOffDelay = TOGGLE_OFF_DELAY;
			m_bIsToggledDirect = !m_bIsToggledDirect;
			m_bIsToggledChannel = false;
			
			if (m_bIsToggledDirect)
				ActivateVON(EVONTransmitType.DIRECT);
			else
				DeactivateVON();
		}
		else 
		{
			SetGamepadLRRMode(false);
			
			if (!m_ActiveEntry || !m_ActiveEntry.IsUsable())
				return;
			
			m_fToggleOffDelay = TOGGLE_OFF_DELAY;
			m_bIsToggledDirect = false;
			m_bIsToggledChannel = !m_bIsToggledChannel;
			
			if (m_bIsToggledChannel)
				ActivateVON(EVONTransmitType.CHANNEL);
			else 
				DeactivateVON();
		}
						
		m_OnVONActiveToggled.Invoke(m_bIsToggledDirect, m_bIsToggledChannel);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON toggle controller
 	protected void OnVONToggleGamepad(float value, EActionTrigger reason)
	{
		if (m_bIsActiveModeDirect)
			OnVONToggle(1, EActionTrigger.DOWN);	// direct toggle
		else 
			OnVONToggle(2, EActionTrigger.DOWN);	// channel toggle
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON switch between direct/channel mode
 	protected void OnVONSwitch(float value, EActionTrigger reason)
	{
		m_bIsActiveModeDirect = !m_bIsActiveModeDirect;
		
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggleGamepad(0, EActionTrigger.UP);
		else if (m_VONDisplay)
		{
			if (m_bIsActiveModeDirect)
				m_VONDisplay.ShowSelectedVONHint(m_DirectSpeechEntry);
			else if (m_ActiveEntry)
				m_VONDisplay.ShowSelectedVONHint(m_ActiveEntry);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	protected void VONDirect(bool activate)
	{
		if (!m_VONComp)
			return;
		
		if (m_bIsToggledDirect && m_fToggleOffDelay <= 0)	// direct speech toggle is active, cancel it
			OnVONToggle(0,0);
		
		if (!m_DirectSpeechEntry.IsUsable())
			return;
		
		m_sActiveHoldAction = VON_DIRECT_HOLD;
		
		if (activate)
			ActivateVON(EVONTransmitType.DIRECT);
		else
			DeactivateVON(EVONTransmitType.DIRECT);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void VONChannel(bool activate)
	{
		if (!m_VONComp)
			return;
			
		if (m_bIsToggledChannel && m_fToggleOffDelay <= 0) // channel speech toggle is active, cancel it
			OnVONToggle(0,0);
		
		if (!m_ActiveEntry || !m_ActiveEntry.IsUsable())	// if active entry disabled, use direct instead
		{
			VONDirect(activate);
			m_VONDisplay.ShowSelectedVONDisabledHint();
			return;
		}
		
		m_sActiveHoldAction = VON_CHANNEL_HOLD;
		
		if (activate)
			ActivateVON(EVONTransmitType.CHANNEL);
		else
			DeactivateVON(EVONTransmitType.CHANNEL);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void VONLongRange(bool activate)
	{
		if (!m_VONComp)
			return;
		
		if ((m_bIsToggledChannel || m_bIsToggledDirect) && m_fToggleOffDelay <= 0) 
			OnVONToggle(0,0);
		
		if (!m_LongRangeEntry || !m_LongRangeEntry.IsUsable())	// if active entry disabled, use direct instead
		{
			VONDirect(activate);
			m_VONDisplay.ShowSelectedVONDisabledHint();
			return;
		}
		
		m_sActiveHoldAction = VON_LONG_RANGE_HOLD;
		
		if (activate)
		{
			if (!m_SavedEntry)	// saved entry to switch back to after using long range mode
				m_SavedEntry = m_ActiveEntry;
			
			m_bIsActiveModeDirect = false;
			
			ActivateVON(EVONTransmitType.LONG_RANGE);
		}
		else
		{
			DeactivateVON(EVONTransmitType.LONG_RANGE);	
			
			if (m_SavedEntry && !m_bIsUsingGamepad)	// restore last saved entry if there is one
				SetActiveTransmit(m_SavedEntry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON activation
	//! \param entry is the subject VON entry
 	protected void ActivateVON(EVONTransmitType transmitType)
	{				
		if (!m_VONComp)
			return;
		
		m_eVONType = transmitType;
		SCR_VONEntry entry = GetEntryByTransmitType(transmitType);
				
		if (!GetGame().GetVONCanTransmitCrossFaction())		// is cross faction transmit disabled
		{
			if (!m_sLocalEncryptionKey)
				InitEncryptionKey();
		
			SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
			if (radioEntry && m_sLocalEncryptionKey != string.Empty && radioEntry.GetTransceiver().GetRadio().GetEncryptionKey() != m_sLocalEncryptionKey)
			{
				m_VONDisplay.ShowSelectedVONDisabledHint(true);
				return;
			}
		}
					
		SetActiveTransmit(entry);
		m_VONComp.SetCapture(true);
		m_bIsActive = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Current VON deactivation
	protected void DeactivateVON(EVONTransmitType transmitType = EVONTransmitType.NONE)
	{
		if (!m_VONComp)
			return; 
		
		if (transmitType != EVONTransmitType.NONE && transmitType != m_eVONType)	// only deactivate target type, in case other type was activated already
			return;
	
		m_eVONType = EVONTransmitType.NONE;
		m_bIsActive = false;
		m_VONComp.SetCapture(false);
		m_sActiveHoldAction = string.Empty;
					
		if (m_bIsToggledDirect) 		// direct toggle is active so ending VON should not end capture
			ActivateVON(EVONTransmitType.DIRECT);
		else if (m_bIsToggledChannel)	// channel toggle is active so ending VON should not end capture
			ActivateVON(EVONTransmitType.CHANNEL);
	}
		
	//------------------------------------------------------------------------------------------------
	protected SCR_VONEntry GetEntryByTransmitType(EVONTransmitType type)
	{
		switch (type)
		{
			case EVONTransmitType.CHANNEL:
				return m_ActiveEntry;
			
			case EVONTransmitType.LONG_RANGE:
				return m_LongRangeEntry;
			
			default:
				return m_DirectSpeechEntry;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set transmission method depending on entry type when starting VON transmit
	protected void SetActiveTransmit(notnull SCR_VONEntry entry)
	{		
		if (entry.GetVONMethod() == ECommMethod.SQUAD_RADIO)
		{
			m_VONComp.SetCommMethod(ECommMethod.SQUAD_RADIO);
			m_VONComp.SetTransmitRadio(SCR_VONEntryRadio.Cast(entry).GetTransceiver());
			SetEntryActive(entry);
		}
		else 
		{
			m_VONComp.SetCommMethod(ECommMethod.DIRECT);
			m_VONComp.SetTransmitRadio(null);
		}
		
		if (entry == m_SavedEntry)
			m_SavedEntry = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON hold timeout, stops active von transmission in case of specific condition which would prevent for it to end normally
	protected void TimeoutVON()
	{
		DeactivateVON();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	//! Used to reinit VON when new entity is controlled
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{				
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggle(0,0);
		
		m_sLocalEncryptionKey = string.Empty;
		
		if (to)
			SetVONComponent(SCR_VoNComponent.Cast(to.FindComponent(SCR_VoNComponent)));
		else 
			SetVONComponent(null);	
		
		if (from)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(from);
			if (character)
			{	
				SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
				if (controller)
					controller.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
			}
		}
		
		bool unconsciousVONEnabled;
		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!baseGameMode)
			return;
		
		SCR_GameModeHealthSettings healthSettingsComp = baseGameMode.GetGameModeHealthSettings();
		if (healthSettingsComp)
			unconsciousVONEnabled = healthSettingsComp.IsUnconsciousVONPermitted();
		
		if (unconsciousVONEnabled)
			return;
		
		if (to)		
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(to);
			if (!character)
				return;
			
			SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
			if (controller)
			{
				m_bIsUnconscious = controller.IsUnconscious();
				controller.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
			}
			
			UpdateSystemState();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	//! Used to deactivate VON for dead players
	protected void OnDestroyed(Instigator killer, IEntity killerEntity)
	{		
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggle(0,0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	//! Used to unregister VON for deleted players
	protected void OnPlayerDeleted(int playerId, IEntity player)
	{		
		if (playerId != GetGame().GetPlayerController().GetPlayerId())
			return;
		
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggle(0,0);
		
		m_ActiveEntry = null;
		m_LongRangeEntry = null;
		
		m_aEntries.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Game Event
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		m_bIsUsingGamepad = isGamepad;
		
		if (!isGamepad)
		{
			m_bIsActiveModeDirect = false;
			m_bIsActiveModeLong = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! PauseMenuUI Event
	protected void OnPauseMenuOpened()
	{
		if (SetVONDisabled(true))
			m_bIsPauseDisabled = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! PauseMenuUI Event
	protected void OnPauseMenuClosed()
	{
		if (m_bIsPauseDisabled)
			SetVONDisabled(false);
		
		m_bIsPauseDisabled = false;
	}
			
	//------------------------------------------------------------------------------------------------
	//! Assign VON comp by fetching it from controlled entity
	bool AssignVONComponent()
	{
		IEntity controlledEnt = SCR_PlayerController.Cast(GetOwner()).GetControlledEntity();
		if (controlledEnt)
			SetVONComponent(SCR_VoNComponent.Cast(controlledEnt.FindComponent(SCR_VoNComponent)));
		
		if(!m_VONComp)
			return false;
		
		return true;			
	}
	
	//------------------------------------------------------------------------------------------------
	void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		m_bIsUnconscious = newLifeState == ECharacterLifeState.INCAPACITATED;
		
		UpdateSystemState();
		
		if (newLifeState == ECharacterLifeState.ALIVE)
			return;
		
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggle(0,0);
			
		TimeoutVON();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitEncryptionKey()
	{
		SCR_FactionManager fManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (fManager)
		{
			SCR_Faction faction = SCR_Faction.Cast(fManager.GetLocalPlayerFaction());
			if (faction)
				m_sLocalEncryptionKey = faction.GetFactionRadioEncryptionKey();
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Initialize component, done once per controller
	protected void Init(IEntity owner)
	{	
		if (s_bIsInit || System.IsConsoleApp())	// hosted server will have multiple controllers, init just the first one // dont init on dedicated server
		{
			Deactivate(owner);
			return;
		}
		
		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
			return;
		
		m_InputManager.AddActionListener(VON_DIRECT_HOLD, EActionTrigger.DOWN, OnVONDirect);
		m_InputManager.AddActionListener(VON_DIRECT_HOLD, EActionTrigger.UP, OnVONDirect);
		m_InputManager.AddActionListener(VON_CHANNEL_HOLD, EActionTrigger.DOWN, OnVONChannel);
		m_InputManager.AddActionListener(VON_CHANNEL_HOLD, EActionTrigger.UP, OnVONChannel);
		m_InputManager.AddActionListener(VON_LONG_RANGE_HOLD, EActionTrigger.DOWN, OnVONLongRange);
		m_InputManager.AddActionListener(VON_LONG_RANGE_HOLD, EActionTrigger.UP, OnVONLongRange);
		m_InputManager.AddActionListener("VONGamepad", EActionTrigger.PRESSED, OnVONGamepad);
		m_InputManager.AddActionListener("VONGamepad", EActionTrigger.UP, OnVONGamepad);
		m_InputManager.AddActionListener("VONGamepadLongRange", EActionTrigger.DOWN, OnVONLongRangeGamepadToggle);
		m_InputManager.AddActionListener("VONDirectToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.AddActionListener("VONChannelToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.AddActionListener("VONToggleGamepad", EActionTrigger.DOWN, OnVONToggleGamepad);
		m_InputManager.AddActionListener("VONSwitch", EActionTrigger.DOWN, OnVONSwitch);
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		playerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		playerController.m_OnDestroyed.Insert(OnDestroyed);
		PauseMenuUI.m_OnPauseMenuOpened.Insert(OnPauseMenuOpened);
		PauseMenuUI.m_OnPauseMenuClosed.Insert(OnPauseMenuClosed);
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerDeleted().Insert(OnPlayerDeleted);
		
		m_DirectSpeechEntry = new SCR_VONEntry(); // Init direct speech entry
		
		ConnectToHandleUpdateVONControllersSystem();
		
		s_bIsInit = true;
		
		if (m_VONMenu)
			m_VONMenu.Init(this);
		
		UpdateSystemState();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup component
	protected void Cleanup()
	{
		m_VONMenu = null;
		
		UpdateSystemState();
		
		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
			return;
		
		m_InputManager.RemoveActionListener(VON_DIRECT_HOLD, EActionTrigger.DOWN, OnVONDirect);
		m_InputManager.RemoveActionListener(VON_DIRECT_HOLD, EActionTrigger.UP, OnVONDirect);
		m_InputManager.RemoveActionListener(VON_CHANNEL_HOLD, EActionTrigger.DOWN, OnVONChannel);
		m_InputManager.RemoveActionListener(VON_CHANNEL_HOLD, EActionTrigger.UP, OnVONChannel);
		m_InputManager.RemoveActionListener(VON_LONG_RANGE_HOLD, EActionTrigger.DOWN, OnVONLongRange);
		m_InputManager.RemoveActionListener(VON_LONG_RANGE_HOLD, EActionTrigger.UP, OnVONLongRange);
		m_InputManager.RemoveActionListener("VONGamepad", EActionTrigger.PRESSED, OnVONGamepad);
		m_InputManager.RemoveActionListener("VONGamepad", EActionTrigger.UP, OnVONGamepad);
		m_InputManager.RemoveActionListener("VONGamepadLongRange", EActionTrigger.DOWN, OnVONLongRangeGamepadToggle);
		m_InputManager.RemoveActionListener("VONDirectToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.RemoveActionListener("VONDChannelToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.RemoveActionListener("VONToggleGamepad", EActionTrigger.DOWN, OnVONToggleGamepad);
		m_InputManager.RemoveActionListener("VONSwitch", EActionTrigger.DOWN, OnVONSwitch);
	
		IEntity ent = PlayerController.Cast(GetOwner()).GetControlledEntity();
		if (ent)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(ent);
			if (character)
			{	
				SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
				if (controller)
					controller.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
			}
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerDeleted().Remove(OnPlayerDeleted);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Debug
	protected void UpdateDebug()
	{
		DbgUI.Begin("VON debug");
		string dbg = "VONcomp: %1 | entries: %2";
		DbgUI.Text( string.Format( dbg, m_VONComp, m_aEntries.Count() ) );
		string dbg2 = "Gpad Direct mode: %1 | Gpad LRR mode: %2";
		DbgUI.Text( string.Format( dbg2, m_bIsActiveModeDirect, m_bIsActiveModeLong ) );
		string dbg3 = "Activ: %1";
		DbgUI.Text( string.Format( dbg3, m_ActiveEntry ) );
		string dbg4 = "LRRad: %1";
		DbgUI.Text( string.Format( dbg4, m_LongRangeEntry ) );
		string dbg5 = "Saved: %1";
		DbgUI.Text( string.Format( dbg5, m_SavedEntry ) );
		
		string line =  "freq: %2 | active: %3 | class: %4 | type: %1 ";
		foreach ( SCR_VONEntry entry : m_aEntries )
		{
			SCR_VONEntryRadio radio = SCR_VONEntryRadio.Cast(entry);
			DbgUI.Text( string.Format( line, SCR_Enum.GetEnumName(EGadgetType, radio.GetGadget().GetType()), radio.GetEntryFrequency(), radio.IsActive(), radio.ToString()));
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		Init(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnDeactivate(IEntity owner)
	{
		Cleanup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSystemState()
	{
		if ((!m_bIsDisabled && !m_bIsUnconscious) || m_VONMenu)
			ConnectToHandleUpdateVONControllersSystem();
		else
			DisconnectFromHandleUpdateVONControllersSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ConnectToHandleUpdateVONControllersSystem()
	{
		World world = GetOwner().GetWorld();
		HandleUpdateVONControllersSystem updateSystem = HandleUpdateVONControllersSystem.Cast(world.FindSystem(HandleUpdateVONControllersSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Register(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromHandleUpdateVONControllersSystem()
	{
		World world = GetOwner().GetWorld();
		HandleUpdateVONControllersSystem updateSystem = HandleUpdateVONControllersSystem.Cast(world.FindSystem(HandleUpdateVONControllersSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Unregister(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		if (!m_bIsDisabled && !m_bIsUnconscious)
		{
			m_InputManager.ActivateContext("VONContext");
			m_InputManager.ActivateContext(VON_MENU_OPENING_CONTEXT);
			
			if (m_fToggleOffDelay > 0)
				m_fToggleOffDelay -= timeSlice;
			
			/* 	When non overlaying context such as chat is activated during hold action, we will no longer receive the EActionTrigger.UP callback from the previous context, which is currently intended
				This timeouts it in such case so the VON will not be stuck in an active state	*/
			if (m_sActiveHoldAction != string.Empty)
			{
				if (!m_InputManager.IsActionActive(m_sActiveHoldAction))
					TimeoutVON();
			}
		}
		
		if (m_VONMenu)
			m_VONMenu.Update(timeSlice);
		
		#ifdef VON_DEBUG
			UpdateDebug();
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		s_bIsInit = false;
		Cleanup();
		DisconnectFromHandleUpdateVONControllersSystem();
		
		super.OnDelete(owner);
	}
};
