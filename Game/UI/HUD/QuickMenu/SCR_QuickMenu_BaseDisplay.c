//------------------------------------------------------------------------------------------------
//! VON quickmenu, attached to PlayerController's HUDManagerComponent
class SCR_QuickMenu_BaseDisplay: SCR_InfoDisplayExtended
{				
	const string ENTRY_TURN_ON 	= "#AR-UserAction_TurnOn";
	const string ENTRY_TURN_OFF = "#AR-UserAction_TurnOff";
	
	const string W_ADJUST_ICON_PC 	= "AdjustIcon_PC";
	const string W_ADJUST_LABEL_PC 	= "AdjustLabel_PC";
	const string W_ADJUST_ICON 		= "AdjustIcon";
	const string W_ADJUST_LABEL 	= "AdjustLabel";
	const string W_TOGGLE_ICON_PC 	= "ToggleIcon_PC";
	const string W_TOGGLE_LABEL_PC 	= "ToggleLabel_PC";
	const string W_TOGGLE_ICON 		= "ToggleIcon";
	const string W_TOGGLE_LABEL 	= "ToggleLabel";
	
	const float DISABLED_ALPHA = 0.75;							// percent
	const float AUTO_CLOSE_TIMEOUT = 2.0;						// seconds
	const ref Color ICON_ORANGE = Color.FromSRGBA(255, 182, 95, 255);
	
	protected bool m_bIsMenuOpen;
	protected int m_iSelected; 									// selected entry index
	protected float m_fInputTimer = 0;							// selection timeout counter

	protected InputManager m_InputManager;
	protected SCR_VONController m_VONController;
	protected SCR_VONEntry m_pEntrySelected;					// menu entry that is currently selected
	protected SCR_QuickMenu_DisplayElement m_pElementSelected;	// element connected to selected menu entry
	
	protected ref array<ref SCR_VONEntry> m_aEntries = {};					// array of VON entries
	protected ref array<ref SCR_QuickMenu_DisplayElement> m_aElements = {}; // array of all existing quick menu elements, unused elements are not removed, just hidden to be re-used later

	//------------------------------------------------------------------------------------------------
	//! Switch widgets color scheme to visualize whether it is disabled (normal/greyed out)
	//! \param widgetName is the subject widget
	//! \param enabled determines target state, true for enabled
	//! \param isIcon differentiates setting between text/icon
	void SetWidgetState(string widgetName, bool enabled, bool isIcon = false)
	{
		Widget w = m_wRoot.FindAnyWidget(widgetName);
		if (!w)
			return;
		
		if (enabled)
		{
			w.SetOpacity(1);
			
			if (isIcon)
				w.SetColor(ICON_ORANGE);
			else
				w.SetColor(Color.White);
		}
		else
		{
			w.SetOpacity(DISABLED_ALPHA);
			w.SetColor(Color.Gray75);
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Set text
	//! \param widgetName is the subject widget
	//! \param text is the target text
	void SetWidgetText(string widgetName, string text)
	{
		TextWidget w = TextWidget.Cast(m_wRoot.FindAnyWidget(widgetName));
		if (!w)
			return;
		
		w.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create/disable entry widgets
	void PrepareWidgets()
	{
		int iWidgetsCount = m_aElements.Count();
		int iWidgetsDelta = iWidgetsCount - m_VONController.GetVONEntries().Count();
		int iWidgetsDeltaAbs = Math.AbsInt(iWidgetsDelta);

		if (iWidgetsDelta < 0)	// Element widgets missing
		{
			for (int i = 0; i < iWidgetsDeltaAbs; i++)
			{
				SCR_QuickMenu_DisplayElement pElement = new SCR_QuickMenu_DisplayElement(m_wRoot);
				m_aElements.Insert(pElement);
			}
		}
		else if (iWidgetsDelta > 0)	// Too many element widgets
		{
			for (int i = iWidgetsCount - 1; i >= iWidgetsCount - iWidgetsDeltaAbs; i--)
			{
				SCR_QuickMenu_DisplayElement pElement = m_aElements[i];
				if (pElement) // Hide un-needed element
					pElement.SetVisible(false);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Display entry widgets
	void DisplayEntries()
	{
		if (m_aElements.IsEmpty())
			return;
		
		array<ref SCR_VONEntry> entries = m_VONController.GetVONEntries(); // Fetch menu entries data
		int entriesCount = entries.Count();
		
		// Set element widget data
		for (int i = 0; i < entriesCount; i++)
		{
			SCR_VONEntry pEntry = entries[i];
			SCR_QuickMenu_DisplayElement pElement = m_aElements[i];
			pElement.Init(pEntry);
			pElement.SetVisible(true);
			
			if (i == m_iSelected)
			{
				m_pElementSelected = pElement;
				m_pElementSelected.Update(true);
				SCR_UISoundEntity.SoundEvent(UISounds.ITEM_SELECTED);
			}
			else
				pElement.Update(false);
		}
		
		// Show the menu
		WidgetAnimator.PlayAnimation(m_wRoot, WidgetAnimationType.Opacity, 1, WidgetAnimator.FADE_RATE_DEFAULT, false, true);
		
		m_bIsMenuOpen = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update hints
	void UpdateHints()
	{		
		bool bCanChange = m_pEntrySelected.CanBeAdjusted() && m_pEntrySelected.m_bIsEnabled;
		SetWidgetState(W_ADJUST_ICON_PC, bCanChange, true);
		SetWidgetState(W_ADJUST_LABEL_PC, bCanChange);
		SetWidgetState(W_ADJUST_ICON, bCanChange, true);
		SetWidgetState(W_ADJUST_LABEL, bCanChange);		
		
		bCanChange = m_pEntrySelected.CanBeToggled();
		SetWidgetState(W_TOGGLE_ICON, bCanChange, true);
		SetWidgetState(W_TOGGLE_LABEL, bCanChange);
		SetWidgetState(W_TOGGLE_ICON_PC, bCanChange, true);
		SetWidgetState(W_TOGGLE_LABEL_PC, bCanChange);
		
		if (bCanChange)		// on/off text
		{
			if (m_pEntrySelected.m_bIsEnabled)
			{
				SetWidgetText(W_TOGGLE_LABEL_PC, ENTRY_TURN_OFF);
				SetWidgetText(W_TOGGLE_LABEL, ENTRY_TURN_OFF);
			}
			else 
			{
				SetWidgetText(W_TOGGLE_LABEL_PC, ENTRY_TURN_ON);
				SetWidgetText(W_TOGGLE_LABEL, ENTRY_TURN_ON);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Initialize InfoDisplay, create or reuse menu entries
	void OnOpen()
	{		
		if (!m_VONController.m_bIsInitEntries)
		{
			if(!m_VONController.InitEntries())
				return;
		}
		
		m_aEntries = m_VONController.GetVONEntries();
		if (m_aEntries.IsEmpty())
			return;
		
		foreach (SCR_VONEntry pEntry : m_aEntries)
		{
			pEntry.InitEntry(); // Init menu entries
		}
		
		if (!m_pEntrySelected)	// Select entry
		{		
			m_iSelected = 0;
			m_pEntrySelected = m_aEntries[m_iSelected];
		}

		m_VONController.SetEntryActive(m_pEntrySelected);

		PrepareWidgets();
		DisplayEntries();
		UpdateHints();
		UpdateGroupHints();
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		groupManager.GetOnPlayableGroupRemoved().Insert(UpdateGroupHints);
		groupManager.GetOnPlayableGroupCreated().Insert(UpdateGroupHints);
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Menu entry was changed, update entry
	//! \param input is any integer which is interepreted later in AdjustEntry according to its needs
	void OnAdjust(int input)
	{
		if (!m_pEntrySelected)
			return;
		
		m_pEntrySelected.AdjustEntry(input);	
		m_pElementSelected.SetText(m_pEntrySelected.GetDisplayText());
		UpdateGroupHints();
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Menu entry was toggled
	void OnToggle()
	{
		if (!m_pEntrySelected)
			return;
		
		m_pEntrySelected.ToggleEntry();
		m_pElementSelected.SetText(m_pEntrySelected.GetDisplayText());
		UpdateHints();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Menu entry was selected, update selection
	//! \param input is the selection step, 1 = next / -1 = previous
	void OnSelect(int input)
	{
		int count = m_aEntries.Count();
		if (count == 0)
			return;
		
		// Visually un-select last selected
		m_pElementSelected = m_aElements[m_iSelected];
		m_pElementSelected.Update(false);
		
		m_iSelected = m_iSelected + input;
		m_iSelected = m_iSelected % count;
		
		if (m_iSelected < 0)
			m_iSelected = m_iSelected + count;
		
		// Select new entry
		m_pElementSelected = m_aElements[m_iSelected];
		m_pElementSelected.Update(true);
		m_pEntrySelected = m_aEntries[m_iSelected];
		m_VONController.SetEntryActive(m_pEntrySelected);

		SCR_UISoundEntity.SoundEvent(UISounds.ITEM_SELECTED);
		
		UpdateHints();
		UpdateGroupHints();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Menu was closed, hide the InfoDisplay
	void OnClose()
	{
		if (m_pElementSelected)
			m_pElementSelected.Update(false);
		
		WidgetAnimator.PlayAnimation(m_wRoot, WidgetAnimationType.Opacity, 0, WidgetAnimator.FADE_RATE_DEFAULT, false, true); // fade out
		m_bIsMenuOpen = false;
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		groupManager.GetOnPlayableGroupRemoved().Remove(UpdateGroupHints);
		groupManager.GetOnPlayableGroupCreated().Remove(UpdateGroupHints);
	}
		
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		if (m_bIsMenuOpen)
			OnMenuClose(0,0);
		
		Cleanup();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	protected void OnDestroyed(IEntity killer)
	{
		if (m_bIsMenuOpen)
			OnMenuClose(0,0);
		
		Cleanup();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_VONController Event
	protected void OnVONEntriesChanged(SCR_VONEntry entry, bool state)
	{
		if (m_bIsMenuOpen)
			OnMenuClose(0,0);
	}
	
	//------------------------------------------------------------------------------------------------
	// ACTION LISTENER CALLBACKS
	//------------------------------------------------------------------------------------------------
	//! Menu item configuration callback
	void OnMenuAdjustUp(float value, EActionTrigger reason)
	{
		m_fInputTimer = 0;
		OnAdjust(1);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Menu item configuration callback
	void OnMenuAdjustDown(float value, EActionTrigger reason)
	{
		m_fInputTimer = 0;
		OnAdjust(-1);
	}	
		
	//------------------------------------------------------------------------------------------------
	//! Menu item configuration callback: 1 = up / 2 = down
	protected void OnMenuAdjust(float value, EActionTrigger reason)
	{
		m_fInputTimer = 0;
		
		if (value == 1)
			OnAdjust(1);
		else 
			OnAdjust(-1);
	}

	//------------------------------------------------------------------------------------------------
	//! Open menu callback
	protected void OnMenuOpen(float value, EActionTrigger reason) 
	{	
		m_fInputTimer = 0;
		
		// TODO this check can be removed, leaving this here so it doesnt spawn some input conflict in GM
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager && editorManager.IsOpened())
			return;
		
		OnOpen();	
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Close menu callback
	protected void OnMenuClose(float value, EActionTrigger reason)
	{
		OnClose();	
	}
			
	//------------------------------------------------------------------------------------------------	
	//! Cycle menu callback: 1 = next / 2 = previous
	protected void OnMenuCycle(float value, EActionTrigger reason) 
	{
		m_fInputTimer = 0;
		if (value > 1) 		// direction switch
			value = -1;

		OnSelect(value);		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Entry toggle
	protected void OnMenuToggle(float value, EActionTrigger reason)
	{
		m_fInputTimer = 0;
		OnToggle();
	}
		
	//------------------------------------------------------------------------------------------------
	protected void AddActionListeners()
	{		
		m_InputManager = GetGame().GetInputManager();
		
		// QuickMenuContext (exlusive)
		m_InputManager.AddActionListener("QuickMenuNext", EActionTrigger.DOWN, OnMenuCycle);				
		m_InputManager.AddActionListener("QuickMenuPrev", EActionTrigger.DOWN, OnMenuCycle);				
		m_InputManager.AddActionListener("QuickMenuWheelUp", EActionTrigger.DOWN, OnMenuAdjustUp);	
		m_InputManager.AddActionListener("QuickMenuWheelDown", EActionTrigger.DOWN, OnMenuAdjustDown);
		m_InputManager.AddActionListener("QuickMenuUp", EActionTrigger.DOWN, OnMenuAdjust);				
		m_InputManager.AddActionListener("QuickMenuDown", EActionTrigger.DOWN, OnMenuAdjust);			
		m_InputManager.AddActionListener("QuickMenuAction", EActionTrigger.DOWN, OnMenuToggle);
		
		m_InputManager.AddActionListener("QuickMenuClose", EActionTrigger.DOWN, OnMenuClose);	// QuickMenuContext (non-exclusive)
		m_InputManager.AddActionListener("VONMenu", EActionTrigger.DOWN, OnMenuOpen); 			// VONContext
		
		// events
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		playerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);	
		playerController.m_OnDestroyed.Insert(OnDestroyed);
		
		m_VONController = SCR_VONController.Cast(playerController.FindComponent(SCR_VONController));
		m_VONController.m_OnEntriesChanged.Insert(OnVONEntriesChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveActionListeners()
	{		
		if (!m_InputManager)
			return;
		
		// QuickMenuContext (exlusive)
		m_InputManager.RemoveActionListener("QuickMenuNext", EActionTrigger.DOWN, OnMenuCycle);
		m_InputManager.RemoveActionListener("QuickMenuPrev", EActionTrigger.DOWN, OnMenuCycle);
		m_InputManager.RemoveActionListener("QuickMenuWheelUp", EActionTrigger.DOWN, OnMenuAdjustUp);
		m_InputManager.RemoveActionListener("QuickMenuWheelDown", EActionTrigger.DOWN, OnMenuAdjustDown);
		m_InputManager.RemoveActionListener("QuickMenuUp", EActionTrigger.DOWN, OnMenuAdjust);
		m_InputManager.RemoveActionListener("QuickMenuDown", EActionTrigger.DOWN, OnMenuAdjust);
		m_InputManager.RemoveActionListener("QuickMenuAction", EActionTrigger.DOWN, OnMenuToggle);
					
		m_InputManager.RemoveActionListener("QuickMenuClose", EActionTrigger.DOWN, OnMenuClose);	// QuickMenuContext (non-exclusive)
		m_InputManager.RemoveActionListener("VONMenu", EActionTrigger.DOWN, OnMenuOpen);			// VONContext
		
		// events
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
		{
			playerController.m_OnControlledEntityChanged.Remove(OnControlledEntityChanged);
			playerController.m_OnDestroyed.Remove(OnDestroyed);
		}
		
		if (m_VONController)
			m_VONController.m_OnEntriesChanged.Remove(OnVONEntriesChanged);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup
	protected void Cleanup()
	{
		m_iSelected = 0;
		m_pEntrySelected = null;
		m_pElementSelected = null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (m_bIsMenuOpen)
		{
			m_InputManager.ActivateContext("QuickMenuContextExclusive");
			m_InputManager.ActivateContext("QuickMenuContext");
					
			m_fInputTimer += timeSlice;
			if (m_fInputTimer > AUTO_CLOSE_TIMEOUT)		// auto-close		
				OnMenuClose(0,0);	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		AddActionListeners();
		
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wRoot.SetVisible(false); // Make menu invisible at start
		m_wRoot.SetOpacity(0);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		RemoveActionListeners();
		Cleanup();

		if (m_aElements)
			m_aElements.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateGroupHints()
	{
		SCR_RespawnSystemComponent respawnComponent = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnComponent)
			return;
		string playerEncryption, radioEncryption;
		SCR_MilitaryFaction playerFaction = SCR_MilitaryFaction.Cast(respawnComponent.GetPlayerFaction(GetGame().GetPlayerController().GetPlayerId()));
		if (!playerFaction)
			return;
		playerEncryption = playerFaction.GetFactionRadioEncryptionKey();
		radioEncryption = m_pEntrySelected.m_RadioComp.GetEncryptionKey();
				
		Widget groupList = m_wRoot.FindAnyWidget("GroupListLayout");
		SCR_GroupVONList listComponent = SCR_GroupVONList.Cast(groupList.FindHandler(SCR_GroupVONList));
		listComponent.ClearList();
		if (playerEncryption != radioEncryption)
			return;
		listComponent.InitiateList(m_pEntrySelected.m_RadioComp);
	}
};