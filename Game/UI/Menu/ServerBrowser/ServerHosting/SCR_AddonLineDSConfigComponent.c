//----------------------------------------------------------------------------------------------
class SCR_AddonLineDSConfigComponent : SCR_AddonLineBaseComponent
{
	protected const string BUTTON_UP = "m_UpButton";
	protected const string BUTTON_DOWN = "m_DownButton";
	protected const string BUTTON_SORT_CONFIRM = "m_SortConfirmButton";
	
	protected bool m_bWidgetEnabled = false; // Doesn't reflect enable state of item
	
	// Sort buttons
	protected SCR_ModularButtonComponent m_ButtonUp;
	protected SCR_ModularButtonComponent m_ButtonDown;
	protected SCR_ModularButtonComponent m_ButtonSortConfirm;
	
	protected bool m_bOnBottom;
	protected bool m_bIsSorting;
	
	protected ref ScriptInvokerScriptedWidgetComponent Event_OnButtonUp;
	protected ref ScriptInvokerScriptedWidgetComponent Event_OnButtonDown;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnSortConfirm;

	//----------------------------------------------------------------------------------------------
	override void Init(SCR_WorkshopItem item)
	{
		// Sorting buttons
		// Up
		Widget wButtonUp = m_wRoot.FindAnyWidget(BUTTON_UP);
		if (wButtonUp)
			m_ButtonUp = SCR_ModularButtonComponent.FindComponent(wButtonUp);
		
		if (m_ButtonUp)
		{
			m_ButtonUp.m_OnClicked.Insert(InvokeEventOnButtonUp);
			m_aMouseButtons.Insert(m_ButtonUp);
		}

		// Down
		Widget wButtonDown = m_wRoot.FindAnyWidget(BUTTON_DOWN);
		if (wButtonDown)
			m_ButtonDown = SCR_ModularButtonComponent.FindComponent(wButtonDown);
		
		if (m_ButtonDown)
		{
			m_ButtonDown.m_OnClicked.Insert(InvokeEventOnButtonDown);
			m_aMouseButtons.Insert(m_ButtonDown);
		}

		// Confirm
		Widget sortConfirm = m_wRoot.FindAnyWidget(BUTTON_SORT_CONFIRM);
		if (sortConfirm)
			m_ButtonSortConfirm = SCR_ModularButtonComponent.FindComponent(sortConfirm);
			
		if (m_ButtonSortConfirm)
		{
			m_ButtonSortConfirm.m_OnClicked.Insert(OnSortConfirm);
			m_aMouseButtons.Insert(m_ButtonSortConfirm);
		}
		
		// Setup rest 
		super.Init(item);
		
		HandleEnableButtons(m_bWidgetEnabled, m_bIsSorting);
	}
	
	//----------------------------------------------------------------------------------------------
	override void UpdateAllWidgets()
	{
		super.UpdateAllWidgets();
		
		HandleEnableButtons(m_bWidgetEnabled, m_bIsSorting);
		HandleSortingButtons();
		
		// Hide action buttons
		m_Widgets.m_wDeleteButton.SetVisible(false);
		
		m_Widgets.m_wHorizontalState.SetVisible(false);
		m_Widgets.m_wUpdateButton.SetVisible(false);
	}
	
	// Display oredering buttons if enabled
	//------------------------------------------------------------------------------------------------
	protected void HandleSortingButtons()
	{
		bool onTop = m_wRoot.GetZOrder() == 0;
		bool visible = m_bIsSorting || (m_bFocused && GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE);
		
		if (m_ButtonUp)
			m_ButtonUp.SetVisible(!onTop && m_bWidgetEnabled && visible);
		
		if (m_ButtonDown)
			m_ButtonDown.SetVisible(!m_bOnBottom && m_bWidgetEnabled && visible);
		
		if (m_ButtonSortConfirm)
			m_ButtonSortConfirm.SetVisible(m_bIsSorting);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSortConfirm()
	{
		if (m_OnSortConfirm)
			m_OnSortConfirm.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnButtonUp()
	{
		if (Event_OnButtonUp)
			Event_OnButtonUp.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnButtonDown()
	{
		if (Event_OnButtonDown)
			Event_OnButtonDown.Invoke(this);
	}
	
	//----------------------------------------------------------------------------------------------
	// API
	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	void SetWidgetEnabled(bool enabled)
	{
		m_bWidgetEnabled = enabled;	
		UpdateAllWidgets();
	}
	
	//----------------------------------------------------------------------------------------------
	bool GetWidgetEnabled()
	{
		return m_bWidgetEnabled;
	}
	
	//----------------------------------------------------------------------------------------------
	void SetOnBottom(bool onBottom)
	{
		m_bOnBottom = onBottom;
	}
	
	//----------------------------------------------------------------------------------------------
	void NotifySorting(bool sorting)
	{
		m_bIsSorting = sorting;
		HandleSortingButtons();
		HandleEnableButtons(m_bWidgetEnabled, sorting);
	}
		
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetEventOnButtonUp()
	{
		if (!Event_OnButtonUp)
			Event_OnButtonUp = new ScriptInvokerScriptedWidgetComponent();

		return Event_OnButtonUp;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetEventOnButtonDown()
	{
		if (!Event_OnButtonDown)
			Event_OnButtonDown = new ScriptInvokerScriptedWidgetComponent();

		return Event_OnButtonDown;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnSortConfirm()
	{
		if (!m_OnSortConfirm)
			m_OnSortConfirm = new ScriptInvokerScriptedWidgetComponent();

		return m_OnSortConfirm;
	}
}