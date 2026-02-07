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
class SCR_VONControllerClass: ScriptGameComponentClass
{}

//------------------------------------------------------------------------------------------------
//! Scripted VON input and control, attached to SCR_PlayerController
class SCR_VONController : ScriptComponent
{
	const string VON_DIRECT_HOLD = "VONDirect";
	const string VON_CHANNEL_HOLD = "VONChannel";
	const string VON_LONG_RANGE_HOLD = "VONLongRange";
	const float TOGGLE_OFF_DELAY = 0.5;		// seconds, delay before toggle state can get canceled, to avoid collision of double click and press
	
	static bool s_bIsInit;					// component init done, static so its only done once
	bool m_bIsInitEntries;					// entry init done
	protected bool m_bIsDisabled; 			// VON control is disabled
	protected bool m_bIsUnconscious; 		// Character is unconscious --> VON control is disabled
	protected bool m_bIsActive;				// VON is active
	protected bool m_bIsToggledDirect;		// VON direct speech toggle is active
	protected bool m_bIsToggledChannel;		// VON channel speech toggle is active
	protected bool m_bIsActiveModeDirect;	// used for controller, switch between direct VON = true / channel VON = false
	protected bool m_bIsActiveModeLong;		// used for controller, switch between long range VON = true / channel VON = false
	protected float m_fToggleOffDelay;		// used to track delay before toggle can be cancelled
	protected string m_sActiveHoldAction;	// tracker for ending the hold action VON 
	protected EVONTransmitType m_eVONType;	// currently active VON type
	protected EventHandlerManagerComponent m_EventHandlerManager;
	protected InputManager m_InputManager;
	
	protected IEntity m_VONEntity;							// cached ent, compared to currently controlled for update
	protected SCR_VoNComponent m_VONComp;					// vonComp of controlled entity
	protected SCR_GadgetManagerComponent m_GadgetMgr;		// gadget manager of controlled entity
	protected SCR_PlayerController m_PlayerController;		// downcasted owner
	protected SCR_VonDisplay m_VONDisplay;					// VON transmission display
	protected SCR_VONEntry m_ActiveEntry;					// active entry (non direct speech)
	protected SCR_VONEntry m_LongRangeEntry;				// entry for long range radio, if available
	protected SCR_VONEntry m_SavedEntry;					// entry used for switching to different active entry after current one is done, f.e. after VONLongRange
	protected ref SCR_VONEntry m_DirectSpeechEntry;			// separate direct speech entry
	protected ref array<ref SCR_VONEntry> m_aEntries = {};	// all entries except direct
	
	ref ScriptInvoker<SCR_VONEntry> 		m_OnActiveDeviceChanged = new ScriptInvoker();	// when non direct entry changes
	ref ScriptInvoker<SCR_VONEntry, bool> 	m_OnEntriesChanged = new ScriptInvoker();		// when entry array is modified, true = added
	ref ScriptInvoker<bool, bool> 			m_OnVONActiveToggled = new ScriptInvoker();		// when direct toggle activates
	
	//------------------------------------------------------------------------------------------------
	void SetDisplay(SCR_VonDisplay display){ m_VONDisplay = display; }
	//------------------------------------------------------------------------------------------------
	SCR_VonDisplay GetDisplay(){ return m_VONDisplay; }
	//------------------------------------------------------------------------------------------------
	SCR_VoNComponent GetVONComponent(){ return m_VONComp; }
		
	//------------------------------------------------------------------------------------------------
	//! Set entity for transmit of VON, must be the owner
	//! \param entity is the target subject
	void SetVoNEntity(IEntity entity)
	{			
		m_VONEntity = entity;
		if (m_EventHandlerManager)
				m_EventHandlerManager.RemoveScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
		
		m_EventHandlerManager = null;
		m_VONComp = null;
		
		if (!entity)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (!character)
			return;
		
		CharacterControllerComponent characterController = character.GetCharacterController();
		if (!characterController)
			return;
		
		m_bIsUnconscious = characterController.IsUnconscious();
		
		m_EventHandlerManager = EventHandlerManagerComponent.Cast(character.FindComponent(EventHandlerManagerComponent));
		if (m_EventHandlerManager)
			m_EventHandlerManager.RegisterScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get current entries
	//! \return array of entries
	array<ref SCR_VONEntry> GetVONEntries()
	{		
		return m_aEntries;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get active entry for outgoing transmission
	//! \return active entry
	SCR_VONEntry GetActiveEntry()
	{
		return m_ActiveEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set entry as active for outgoing transmission
	//! \param entry is subject entry
	//! \param resetLRR determines whether the long range radio state should be reset by this selection
	void SetEntryActive(SCR_VONEntry entry, bool resetLRR = false)
	{
		if (entry == m_ActiveEntry)
			return;
			
		m_ActiveEntry = entry;
		
		if (resetLRR)	// cancel long range state if switching to personal radio
		{
			SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
			if (radioEntry && !radioEntry.m_bIsLongRange)
				m_bIsActiveModeLong = false;			
		}

		m_OnActiveDeviceChanged.Invoke(entry);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Add new VON entry
	//! Set as active entry if m_ActiveEntry is null
	//! \param entry is subject entry
	void AddEntry(SCR_VONEntry entry)
	{
		m_aEntries.Insert(entry);
		entry.InitEntry();
		
		if (SCR_VONEntryRadio.Cast(entry) && SCR_VONEntryRadio.Cast(entry).m_bIsLongRange)	// if long range entry, set it here
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
				ActivateVON(m_ActiveEntry, false);
			
			m_ActiveEntry = null;
		}
		
		if (entry == m_LongRangeEntry)	// unset long range entry
			m_LongRangeEntry = null;
		
		m_aEntries.RemoveItem(entry);
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
		{
			m_bIsActiveModeLong = false;
			VONChannel(true);
		}
		else
			VONChannel(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON channel speech listener callback for long range radios
 	protected void OnVONLongRange(float value, EActionTrigger reason)
	{
		if (reason == EActionTrigger.DOWN)
		{
			m_bIsActiveModeLong = false;
			VONLongRange(true);
		}
		else
			VONLongRange(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON channel speech listener callback
 	protected void OnVONGamepad(float value, EActionTrigger reason)
	{		
		if (reason == EActionTrigger.UP)
			m_eVONType = EVONTransmitType.NONE;
		
		if (m_bIsActiveModeDirect)
		{
			if (reason == EActionTrigger.PRESSED && m_eVONType == EVONTransmitType.DIRECT)
				return;
			
			if (reason == EActionTrigger.PRESSED)
				VONDirect(true);
			else 
				VONDirect(false);
		}
		else if (m_bIsActiveModeLong)
		{
			if (reason == EActionTrigger.PRESSED && m_eVONType == EVONTransmitType.LONG_RANGE)
				return;
		
			if (reason == EActionTrigger.PRESSED)
				VONLongRange(true);
			else 
				VONLongRange(false);
		
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
 	protected void OnVONLongRangeGamepad(float value, EActionTrigger reason)
	{				
		if (m_bIsActiveModeLong || !m_LongRangeEntry || !m_LongRangeEntry.m_bIsEnabled)
		{
			m_bIsActiveModeLong = false;
			if (m_bIsActive)
				VONLongRange(false);
		}
		else 
		{
			m_bIsActiveModeLong = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON toggle   val 0 = off / 1 = direct / 2 = channel
 	protected void OnVONToggle(float value, EActionTrigger reason)
	{
		if (!m_bIsInitEntries)
		{
			if (!InitEntries())
			{
				m_sActiveHoldAction = string.Empty;
				return;
			}
		}
		
		if (value == 0)
		{
			m_fToggleOffDelay = 0;
			m_bIsToggledDirect = false;
			m_bIsToggledChannel = false;
			ActivateVON(m_DirectSpeechEntry, false);
		}
		else if (value == 1)
		{
			m_fToggleOffDelay = TOGGLE_OFF_DELAY;
			m_bIsToggledDirect = !m_bIsToggledDirect;
			m_bIsToggledChannel = false;
			ActivateVON(m_DirectSpeechEntry, m_bIsToggledDirect);
		}
		else 
		{
			if (!m_ActiveEntry)
				return;
			
			m_bIsActiveModeLong = false;
			m_fToggleOffDelay = TOGGLE_OFF_DELAY;
			m_bIsToggledDirect = false;
			m_bIsToggledChannel = !m_bIsToggledChannel;
			ActivateVON(m_ActiveEntry, m_bIsToggledChannel);
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
		else
		{
			OnVONGamepad(0, EActionTrigger.PRESSED); 
			GetGame().GetCallqueue().CallLater(OnVONGamepad, 100, false, 0, EActionTrigger.UP);	// visualisation of switching from direct to radio VON when using controller
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void VONDirect(bool activate)
	{
		if (!m_bIsInitEntries)
		{
			if (!InitEntries())
			{
				m_sActiveHoldAction = string.Empty;
				return;
			}
		}
		
		if (!m_DirectSpeechEntry || !m_DirectSpeechEntry.m_bIsEnabled)
			return;
		
		if (m_bIsToggledDirect && m_fToggleOffDelay <= 0)	// direct speech toggle is active, cancel it
			OnVONToggle(0,0);
		
		m_sActiveHoldAction = VON_DIRECT_HOLD;
		
		if (activate)
		{
			m_eVONType = EVONTransmitType.DIRECT;
			ActivateVON(m_DirectSpeechEntry, true);
		}
		else
			ActivateVON(m_DirectSpeechEntry, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void VONChannel(bool activate)
	{
		if (!m_bIsInitEntries)
		{
			if (!InitEntries())
			{
				m_sActiveHoldAction = string.Empty;
				return;
			}
		}
	
		if (!m_ActiveEntry || !m_ActiveEntry.m_bIsEnabled)
			return;
		
		if (m_bIsToggledChannel && m_fToggleOffDelay <= 0) // channel speech toggle is active, cancel it
			OnVONToggle(0,0);
		
		m_sActiveHoldAction = VON_CHANNEL_HOLD;
		
		if (activate)
		{
			m_eVONType = EVONTransmitType.CHANNEL;
			ActivateVON(m_ActiveEntry, true);
		}
		else
			ActivateVON(m_ActiveEntry, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void VONLongRange(bool activate)
	{
		if (!m_bIsInitEntries)
		{
			if (!InitEntries())
			{
				m_sActiveHoldAction = string.Empty;
				return;
			}
		}
		
		if (!m_LongRangeEntry || !m_LongRangeEntry.m_bIsEnabled)
			return;
		
		if ((m_bIsToggledChannel || m_bIsToggledDirect) && m_fToggleOffDelay <= 0) 
			OnVONToggle(0,0);
		
		m_sActiveHoldAction = VON_LONG_RANGE_HOLD;
		
		if (activate)
		{
			if (!m_SavedEntry)	// saved entry to switch back to after using long range keybind
			{
				if (m_bIsActiveModeDirect)
					m_SavedEntry = m_DirectSpeechEntry;
				else
					m_SavedEntry = m_ActiveEntry;
			}
			
			m_bIsActiveModeDirect = false;
			
			m_eVONType = EVONTransmitType.LONG_RANGE;
			ActivateVON(m_LongRangeEntry, true);
		}
		else
		{
			ActivateVON(m_LongRangeEntry, false);	
			
			if (m_SavedEntry)	// restore last saved entry if there is one
			{
				m_SavedEntry.ActivateEntry();
				if (m_SavedEntry.m_RadioTransceiver == null)
					m_bIsActiveModeDirect = true;
				
				m_SavedEntry = null;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON activation
	//! \param entry is the subject VON entry
	//! \param state determines target state ON/OFF
 	protected void ActivateVON(SCR_VONEntry entry, bool state)
	{				
		if (!entry)
			return;
				
		if (!state)
		{			
			m_bIsActive = false;
			m_VONComp.SetCapture(false);
						
			if (m_bIsToggledDirect) // direct toggle is active so ending VON should not end capture
			{
				entry.ActivateEntry();
				ActivateVON(m_DirectSpeechEntry, m_bIsToggledDirect);
				m_sActiveHoldAction = string.Empty;
			}
			else if (m_bIsToggledChannel) // channel toggle is active so ending VON should not end capture
			{
				entry.ActivateEntry();
				ActivateVON(m_ActiveEntry, m_bIsToggledChannel);
				m_sActiveHoldAction = string.Empty;
			}
		}
		else
		{
			entry.ActivateEntry();
			m_VONComp.SetCapture(true);
			m_bIsActive = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON hold timeout
	protected void TimeoutVON()
	{
		if (m_sActiveHoldAction == VON_DIRECT_HOLD)
			ActivateVON(m_DirectSpeechEntry, false);
		else if (m_sActiveHoldAction == VON_CHANNEL_HOLD)
			ActivateVON(m_ActiveEntry, false);
		else if (m_sActiveHoldAction == VON_LONG_RANGE_HOLD)
			ActivateVON(m_LongRangeEntry, false);
		
		m_sActiveHoldAction = string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	//! Used to reinit VON when new entity is controlled
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{		
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggle(0,0);
		
		CleanupEntries();
		SetVoNEntity(to);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	//! Used to deactivate VON for dead players. Listeners are added back because they are previously 
	//! removed on unconsciousness and if character dies, they need to added back.
	protected void OnDestroyed(IEntity killer)
	{		
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggle(0,0);
		
		CleanupEntries();
		
		//m_bIsUnconscious = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Game Event
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
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
		m_bIsDisabled = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! PauseMenuUI Event
	protected void OnPauseMenuClosed()
	{
		m_bIsDisabled = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! GadgetManager Event
	protected void OnGadgetAdded(SCR_GadgetComponent gadgetComp)
	{
		BaseRadioComponent radioComp = BaseRadioComponent.Cast(gadgetComp.GetOwner().FindComponent(BaseRadioComponent));
		if (!radioComp)
			return;
		
		// Put all transceivers (AKA) channels in the VoN menu
		for (int i = radioComp.TransceiversCount() - 1; i >= 0; --i)
		{
			SCR_VONEntryRadio radioEntry = new SCR_VONEntryRadio(this, m_VONComp, radioComp.GetTransceiver(i), gadgetComp);
			AddEntry(radioEntry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! GadgetManager Event
	protected void OnGadgetRemoved(SCR_GadgetComponent gadgetComp)
	{	
		for (int i = m_aEntries.Count() - 1; i >= 0; --i)
		{
			if (m_aEntries[i].GetGadget() == gadgetComp)
				RemoveEntry(m_aEntries[i]);
			// Do not dare to break or return here, there may be multiple entries per one gadget
		}
		
		// Check if we still have active entry...
		if (!m_ActiveEntry)
		{
			// ...try to fix if we do not
			if (!m_aEntries.IsEmpty())
				m_ActiveEntry = m_aEntries[0];
			else 
				m_ActiveEntry = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init VON entries, done when controlled entity changes
	bool InitEntries()
	{
		if (!m_VONEntity)	//No entity, try to use current controlled entity
		{
			IEntity controlledEnt = m_PlayerController.GetControlledEntity();
			if (controlledEnt)
				SetVoNEntity(controlledEnt);
		}
		
		if (!m_VONEntity)			
			return false;
		
		// must be owner (is this necessary?)
		RplComponent rplComponent = RplComponent.Cast(m_VONEntity.FindComponent(RplComponent));
		if (!rplComponent || !rplComponent.IsOwner())
			return false;
		
		m_VONComp = SCR_VoNComponent.Cast(m_VONEntity.FindComponent(SCR_VoNComponent));
		if (!m_VONComp)
			return false;
		
		if (!m_VONComp)
			return false;
		
		if (m_aEntries)
			m_aEntries.Clear();
		
		m_DirectSpeechEntry = new SCR_VONEntry(this, m_VONComp); // Init direct speech entry
		m_DirectSpeechEntry.m_bIsEnabled = true;
		
		// Init radio entries
		m_GadgetMgr = SCR_GadgetManagerComponent.GetGadgetManager(m_VONEntity);
		if (!m_GadgetMgr)
			return false;
		
		// Buracisko do not like this. We shouldn't treat them in different way
		// TODO: What about Intercom and what about entering/leaving car
		array<SCR_GadgetComponent> radiosArray = new array<SCR_GadgetComponent>;
		radiosArray.Copy(m_GadgetMgr.GetGadgetsByType(EGadgetType.RADIO)); // squad radios
		radiosArray.InsertAll(m_GadgetMgr.GetGadgetsByType(EGadgetType.RADIO_BACKPACK)); // backpack radio

		foreach (SCR_GadgetComponent radio : radiosArray)
		{
			BaseRadioComponent radioComp = BaseRadioComponent.Cast(radio.GetOwner().FindComponent(BaseRadioComponent));
			// Get all individual transceivers (AKA channels) from the radio
			for (int i = 0 ; i < radioComp.TransceiversCount(); ++i)
			{
				BaseTransceiver radioChannel = radioComp.GetTransceiver(i);
				SCR_VONEntryRadio radioEntry = new SCR_VONEntryRadio(this, m_VONComp, radioChannel, radio);
				// This already makes sure its activates entry if no other is activated
				AddEntry(radioEntry);
			}
		}
		
		// Use events from gadget manager to maintain the list of entries
		// Note: This is illformed for non gadget radios, like intercom in vehicle
		// - Redesign needed unless different approach is used
		m_GadgetMgr.m_OnGadgetAdded.Insert(OnGadgetAdded);
		m_GadgetMgr.m_OnGadgetRemoved.Insert(OnGadgetRemoved);
		
		m_bIsInitEntries = true;
		return true;			
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup VON entries, done when controlled entity changes
	protected void CleanupEntries()
	{
		m_aEntries.Clear();
		
		if (m_DirectSpeechEntry)
			m_DirectSpeechEntry.m_bIsEnabled = false;
		
		m_VONEntity = null;
		
		if (m_GadgetMgr)
		{
			m_GadgetMgr.m_OnGadgetAdded.Remove(OnGadgetAdded);
			m_GadgetMgr.m_OnGadgetRemoved.Remove(OnGadgetRemoved);
		}
		
		m_bIsInitEntries = false;
	}
	
	void OnConsciousnessChanged(bool conscious)
	{
		m_bIsUnconscious = !conscious;
		
		if (conscious)
			return;
		
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggle(0,0);
			
		TimeoutVON();
	}
		
	//------------------------------------------------------------------------------------------------
	//! Initialize component, done once per controller
	protected void Init()
	{
		if (s_bIsInit)	// hosted server will have multiple controllers, init just the first one
			return;
		
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
		m_InputManager.AddActionListener("VONGamepadLongRange", EActionTrigger.DOWN, OnVONLongRangeGamepad);
		m_InputManager.AddActionListener("VONDirectToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.AddActionListener("VONChannelToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.AddActionListener("VONToggleGamepad", EActionTrigger.DOWN, OnVONToggleGamepad);
		m_InputManager.AddActionListener("VONSwitch", EActionTrigger.DOWN, OnVONSwitch);
		
		m_PlayerController = SCR_PlayerController.Cast(GetOwner());
		m_PlayerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		m_PlayerController.m_OnDestroyed.Insert(OnDestroyed);
		PauseMenuUI.m_OnPauseMenuOpened.Insert(OnPauseMenuOpened);
		PauseMenuUI.m_OnPauseMenuClosed.Insert(OnPauseMenuClosed);
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		SetEventMask(GetOwner(), EntityEvent.FRAME);
		
		s_bIsInit = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup component
	protected void Cleanup()
	{
		s_bIsInit = false;
		
		if (GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(OnVONGamepad);
		
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
		m_InputManager.RemoveActionListener("VONGamepadLongRange", EActionTrigger.DOWN, OnVONLongRangeGamepad);
		m_InputManager.RemoveActionListener("VONDirectToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.RemoveActionListener("VONDChannelToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.RemoveActionListener("VONToggleGamepad", EActionTrigger.DOWN, OnVONToggleGamepad);
		m_InputManager.RemoveActionListener("VONSwitch", EActionTrigger.DOWN, OnVONSwitch);
	
		if (m_EventHandlerManager)
			m_EventHandlerManager.RemoveScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Debug
	protected void UpdateDebug()
	{
		DbgUI.Begin("VON debug");
		string dbg = "VONentity: %1 | VONcomp: %2 | Entries init done: %3 | entries: %4";
		DbgUI.Text( string.Format( dbg, m_VONEntity, m_VONComp, m_bIsInitEntries, m_aEntries.Count() ) );
		string dbg2 = "Gpad Direct mode: %1 | Gpad LRR mode: %2 | Active entry: %3";
		DbgUI.Text( string.Format( dbg2, m_bIsActiveModeDirect, m_bIsActiveModeLong, m_ActiveEntry ) );
		string dbg3 = "Saved Entry: %1";
		DbgUI.Text( string.Format( dbg3, m_SavedEntry ) );
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		Init();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_bIsDisabled && !m_bIsUnconscious)
		{
			m_InputManager.ActivateContext("VONContext");
			
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
		
		#ifdef VON_DEBUG
			UpdateDebug();
		#endif
	}
	//------------------------------------------------------------------------------------------------
	void ~SCR_VONController()
	{
		Cleanup();
	}
};
