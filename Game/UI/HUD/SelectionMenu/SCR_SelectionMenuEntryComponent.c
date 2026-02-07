/*!
Base widget component for selection menu entries
Handles mainly mouse interaction with the entry layout
*/
//------------------------------------------------------------------------------------------------
class SCR_SelectionMenuEntryComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("0.4")]
	protected float m_fDisabledOpacity;

	protected bool m_bEnabled = true;

	protected SCR_SelectionMenuEntry m_Entry;

	// Invokers
	protected ref ScriptInvoker<SCR_SelectionMenuEntryComponent> m_OnMouseEnter;
	protected ref ScriptInvoker<SCR_SelectionMenuEntryComponent> m_OnMouseLeave;
	protected ref ScriptInvoker<SCR_SelectionMenuEntryComponent> m_OnClick;

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnMouseEnter()
	{
		if (m_OnMouseEnter)
			m_OnMouseEnter.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnMouseEnter()
	{
		if (!m_OnMouseEnter)
			m_OnMouseEnter = new ScriptInvoker();

		return m_OnMouseEnter;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnMouseLeave()
	{
		if (m_OnMouseLeave)
			m_OnMouseLeave.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnMouseLeave()
	{
		if (!m_OnMouseLeave)
			m_OnMouseLeave = new ScriptInvoker();

		return m_OnMouseLeave;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnClick()
	{
		if (m_OnClick)
			m_OnClick.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnClick()
	{
		if (!m_OnClick)
			m_OnClick = new ScriptInvoker();

		return m_OnClick;
	}

	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		SetEnabled(m_bEnabled);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		InvokeEventOnMouseEnter();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		InvokeEventOnMouseLeave();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button == 0)
			InvokeEventOnClick();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	// Custom
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void SetEnabled(bool enabled)
	{
		m_bEnabled = enabled;

		if (!m_wRoot)
			return;

		// Set visuals
		if (m_bEnabled)
			m_wRoot.SetOpacity(1);
		else
			m_wRoot.SetOpacity(m_fDisabledOpacity);
	}

	//------------------------------------------------------------------------------------------------
	//! Set entry holding data driving this visuals
	void SetEntry(SCR_SelectionMenuEntry entry)
	{
		m_Entry = entry;
	}
};
