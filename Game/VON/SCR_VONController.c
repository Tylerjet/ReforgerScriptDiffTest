//------------------------------------------------------------------------------------------------
class SCR_VONControllerClass: ScriptGameComponentClass
{}

//------------------------------------------------------------------------------------------------
//! Scripted VON input and control, attached to SCR_PlayerController
class SCR_VONController : ScriptComponent
{
	const string VON_DIRECT_HOLD = "VONDirect";
	const string VON_CHANNEL_HOLD = "VONChannel";
	
	static bool s_bIsInit;					// component init done, static so its only done once
	bool m_bIsInitEntries;					// entry init done
	protected bool m_bIsDisabled; 			// VON control is disabled
	protected bool m_bIsActive;				// VON is active
	protected bool m_bIsToggledDirect;		// VON direct speech toggle is active
	protected bool m_bIsToggledChannel;		// VON channel speech toggle is active
	protected bool m_bIsActiveModeDirect;	// used for controller, switch between direct VON = true / channel VON = false
	protected string m_sActiveHoldAction;	// tracker for ending the hold action VON 
	
	protected IEntity m_VONEntity;							// cached ent, compared to currently controlled for update
	protected SCR_VoNComponent m_VONComp;					// vonComp of controlled entity
	protected SCR_GadgetManagerComponent m_GadgetMgr;		// gadget manager of controlled entity
	protected SCR_PlayerController m_PlayerController;		// downcasted owner
	protected InputManager m_InputManager;					// input manager
	protected SCR_VonDisplay m_VONDisplay;					// VON transmission display
	protected SCR_VONEntry m_ActiveEntry;					// active entry (non direct speech)
	protected ref SCR_VONEntry m_DirectSpeechEntry;			// separate direct speech entry
	protected ref array<ref SCR_VONEntry> m_aEntries = {};
	
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
		m_VONEntity = null;
		
		if (!entity)
			return;
		
		// must be owner
		RplComponent rplComponent = RplComponent.Cast(entity.FindComponent(RplComponent));
		if (!rplComponent || !rplComponent.IsOwner())
			return;					
		
		m_VONComp = SCR_VoNComponent.Cast(entity.FindComponent(SCR_VoNComponent));
		if (!m_VONComp)
			return;

		m_VONEntity = entity;
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
	void SetEntryActive(SCR_VONEntry entry)
	{
		if (entry == m_ActiveEntry)
			return;
			
		m_ActiveEntry = entry;

		//m_OnActiveDeviceChanged.Invoke(entry);	// commented out since it is not used yet
	}
		
	//------------------------------------------------------------------------------------------------
	//! Add new VON entry
	//! \param entry is subject entry
	void AddEntry(SCR_VONEntry entry)
	{
		m_aEntries.Insert(entry);
		m_OnEntriesChanged.Invoke(entry, true);
		entry.InitEntry();
		
		if (!m_ActiveEntry)
			SetEntryActive(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove existing VON entry
	//! \param entry is subject entry
	void RemoveEntry(SCR_VONEntry entry)
	{
		m_OnEntriesChanged.Invoke(entry, false);
		m_aEntries.RemoveItem(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON direct speech listener callback
	protected void OnVONDirect(float value, EActionTrigger reason)
	{		
		if (!m_bIsInitEntries)
		{
			if (!InitEntries())
			{
				m_sActiveHoldAction = string.Empty;
				return;
			}
		}
		
		if (m_bIsToggledDirect)	// direct speech toggle is active, do nothing
			return;
		
		m_sActiveHoldAction = VON_DIRECT_HOLD;
		
		if (reason == EActionTrigger.DOWN)
			ActivateVON(m_DirectSpeechEntry, true);
		else if (reason == EActionTrigger.UP)
			ActivateVON(m_DirectSpeechEntry, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON channel speech listener callback
 	protected void OnVONChannel(float value, EActionTrigger reason)
	{
		if (!m_bIsInitEntries)
		{
			if (!InitEntries())
			{
				m_sActiveHoldAction = string.Empty;
				return;
			}
		}
		
		if (!m_ActiveEntry || !m_ActiveEntry.m_bIsEnabled || m_bIsToggledChannel)
			return;
		
		m_sActiveHoldAction = VON_CHANNEL_HOLD;
		
		if (reason == EActionTrigger.DOWN)
			ActivateVON(m_ActiveEntry, true);
		else if (reason == EActionTrigger.UP)
			ActivateVON(m_ActiveEntry, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON channel speech listener callback
 	protected void OnVONGamepad(float value, EActionTrigger reason)
	{
		if (m_bIsActiveModeDirect)
			OnVONDirect(0, reason);
		else 
			OnVONChannel(0, reason);
			
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
			m_bIsToggledDirect = false;
			m_bIsToggledChannel = false;
			ActivateVON(m_DirectSpeechEntry, false);
		}
		else if (value == 1)
		{
			m_bIsToggledDirect = !m_bIsToggledDirect;
			m_bIsToggledChannel = false;
			ActivateVON(m_DirectSpeechEntry, m_bIsToggledDirect);
		}
		else 
		{
			if (!m_ActiveEntry)
				return;
			
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
			OnVONToggleGamepad(0, EActionTrigger.DOWN);
		else
		{
			OnVONGamepad(0, EActionTrigger.DOWN); 
			GetGame().GetCallqueue().CallLater(OnVONGamepad, 100, false, 0, EActionTrigger.UP);	// visualisation of switching from direct to radio VON when using controller
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
		
		entry.ActivateEntry();
					
		if (!state)
		{			
			m_bIsActive = false;
			m_VONComp.SetCapture(false);
			
			foreach (SCR_VONEntry vEntry : m_aEntries)	// TODO temporary hack to avoid sound loopback
			{
				if (vEntry != entry && vEntry.m_bIsSuspended )
				{
					vEntry.m_bIsSuspended = false;
					vEntry.ToggleEntry();
				}
			}
			
			if (m_bIsToggledDirect) // direct toggle is active so ending VON should not end capture
			{
				ActivateVON(m_DirectSpeechEntry, m_bIsToggledDirect);
				m_sActiveHoldAction = string.Empty;
			}
			else if (m_bIsToggledChannel) // channel toggle is active so ending VON should not end capture
			{
				ActivateVON(m_ActiveEntry, m_bIsToggledChannel);
				m_sActiveHoldAction = string.Empty;
			}
		}
		else
		{
			m_VONComp.SetCapture(true);
			m_bIsActive = true;
			
			FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast( m_VONEntity.FindComponent( FactionAffiliationComponent ) );	// TODO temp hack against sound loopback
			if (factionComp && SCR_VONEntryRadio.Cast(entry))
			{
				SCR_MilitaryFaction playerFaction = SCR_MilitaryFaction.Cast(factionComp.GetAffiliatedFaction());
				if (playerFaction)
				{
					int factionHQFrequency = playerFaction.GetFactionRadioFrequency();
					if (factionHQFrequency == entry.m_RadioComp.GetFrequency())
					{
						foreach (SCR_VONEntry vEntry : m_aEntries)
						{
							if (vEntry != entry && vEntry.m_bIsEnabled )
							{
								vEntry.ToggleEntry();
								vEntry.m_bIsSuspended = true;
							}
						}
					}
				}
			}
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
	//! Used to deactivate VON for dead players
	protected void OnDestroyed(IEntity killer)
	{		
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggle(0,0);
		
		CleanupEntries();
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
		if (radioComp)
		{
			SCR_VONEntryRadio radioEntry = new SCR_VONEntryRadio(this, m_VONComp, radioComp);
			AddEntry(radioEntry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! GadgetManager Event
	protected void OnGadgetRemoved(SCR_GadgetComponent gadgetComp)
	{
		BaseRadioComponent radioComp = BaseRadioComponent.Cast(gadgetComp.GetOwner().FindComponent(BaseRadioComponent));
		
		int count = m_aEntries.Count();
		for (int i; i < count; i++)
		{
			if (m_aEntries[i].m_RadioComp == radioComp)
			{
				if (m_aEntries[i] == m_ActiveEntry)	// if the radio being removed is active 
				{
					if (m_bIsToggledChannel)
						OnVONToggle(2, 0);
					else 
						ActivateVON(m_ActiveEntry, false);
					
					RemoveEntry(m_aEntries[i]);
					
					if (!m_aEntries.IsEmpty())
						m_ActiveEntry = m_aEntries[0]; // cycle active entry
					else 
						m_ActiveEntry = null;
				}
				else 
					RemoveEntry(m_aEntries[i]);
				
				return;
			}
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
		
		if (m_VONEntity)			
		{			
			if (m_aEntries)
				m_aEntries.Clear();
			
			m_DirectSpeechEntry = new SCR_VONEntry(this, m_VONComp); // Init direct speech entry
			
			// Init radio entries
			m_GadgetMgr = SCR_GadgetManagerComponent.GetGadgetManager(m_VONEntity);
			if (m_GadgetMgr)
			{
				array<SCR_GadgetComponent> radiosArray = m_GadgetMgr.GetGadgetsByType(EGadgetType.RADIO_BACKPACK); // backpack radio
				array<SCR_GadgetComponent> radiosArray2 = m_GadgetMgr.GetGadgetsByType(EGadgetType.RADIO); // squad radios
				
				if (!radiosArray.IsEmpty())
					radiosArray.InsertAll(radiosArray2);
				else 
					radiosArray = radiosArray2;
				
				foreach (SCR_GadgetComponent radio : radiosArray)
				{
					BaseRadioComponent radioComp = BaseRadioComponent.Cast(radio.GetOwner().FindComponent(BaseRadioComponent));
					SCR_VONEntryRadio radioEntry = new SCR_VONEntryRadio(this, m_VONComp, radioComp);
					AddEntry(radioEntry);
					radioEntry.InitEntry();
					
					if (!m_ActiveEntry)
						SetEntryActive(radioEntry);
				}
				
				m_GadgetMgr.m_OnGadgetAdded.Insert(OnGadgetAdded);
				m_GadgetMgr.m_OnGadgetRemoved.Insert(OnGadgetRemoved);
			}	
			
			m_bIsInitEntries = true;
			return true;			
		}
	
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup VON entries, done when controlled entity changes
	protected void CleanupEntries()
	{
		m_aEntries.Clear();
		m_VONEntity = null;
		
		if (m_GadgetMgr)
		{
			m_GadgetMgr.m_OnGadgetAdded.Remove(OnGadgetAdded);
			m_GadgetMgr.m_OnGadgetRemoved.Remove(OnGadgetRemoved);
		}
		
		m_bIsInitEntries = false;
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
		m_InputManager.AddActionListener("VONGamepad", EActionTrigger.DOWN, OnVONGamepad);
		m_InputManager.AddActionListener("VONGamepad", EActionTrigger.UP, OnVONGamepad);
		m_InputManager.AddActionListener("VONToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.AddActionListener("VONToggleGamepad", EActionTrigger.DOWN, OnVONToggleGamepad);
		m_InputManager.AddActionListener("VONSwitch", EActionTrigger.DOWN, OnVONSwitch);
		
		m_PlayerController = SCR_PlayerController.Cast(GetOwner());
		m_PlayerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		m_PlayerController.m_OnDestroyed.Insert(OnDestroyed);
		PauseMenuUI.m_OnPauseMenuOpened.Insert(OnPauseMenuOpened);
		PauseMenuUI.m_OnPauseMenuClosed.Insert(OnPauseMenuClosed);
		
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
		m_InputManager.RemoveActionListener("VONGamepad", EActionTrigger.DOWN, OnVONGamepad);
		m_InputManager.RemoveActionListener("VONGamepad", EActionTrigger.UP, OnVONGamepad);
		m_InputManager.RemoveActionListener("VONToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.RemoveActionListener("VONToggleGamepad", EActionTrigger.DOWN, OnVONToggleGamepad);
		m_InputManager.RemoveActionListener("VONSwitch", EActionTrigger.DOWN, OnVONSwitch);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Debug
	protected void UpdateDebug()
	{
		DbgUI.Begin("VON debug");
		string dbg = "VONentity: %1 | VONcomp: %2 | Entries init done: %3 | entries: %4";
		DbgUI.Text( string.Format( dbg, m_VONEntity, m_VONComp, m_bIsInitEntries, m_aEntries.Count() ) );
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
		if (!m_bIsDisabled)
		{
			m_InputManager.ActivateContext("VONContext");
			
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