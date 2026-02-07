/*!
Detects hover condition. When mouse is over this widget for more than threshold amound of time,
hover event handler is fired.
This component relies on SCR_TooltipManagerEntity.
*/

class SCR_HoverDetectorComponent : ScriptedWidgetComponent
{
	ref ScriptInvoker m_OnHoverDetected = new ScriptInvoker();
	ref ScriptInvoker m_OnMouseLeave = new ScriptInvoker();
	
	[Attribute("200", desc: "Time between mouse entering widget and event activation, in milliseconds")]
	protected float m_fHoverDelayMs;
	
	protected bool m_bMouseOver;
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_bMouseOver = true;
		auto callQueue = GetGame().GetCallqueue();
		callQueue.Remove(OnTimer);
		callQueue.CallLater(OnTimer, m_fHoverDelayMs);
		return false;
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_bMouseOver = false;
		auto callQueue = GetGame().GetCallqueue();
		callQueue.Remove(OnTimer);
		m_OnMouseLeave.Invoke();
		return false;
	}
	
	void OnTimer()
	{
		// Invoke event if cursor is still over this widget
		if (m_bMouseOver)
			m_OnHoverDetected.Invoke();
	}
};