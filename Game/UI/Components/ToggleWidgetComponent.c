/*!
When any of child buttons is clicked, toggle visibility of the widget specified in the attribute.
*/
class ToggleWidgetComponent: ScriptedWidgetComponent
{
	[Attribute(desc: "Name of the widget whose visibility whill be toggled.")]
	private string m_WidgetName;
	
	private Widget m_Widget;
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_Widget)
			m_Widget.SetVisible(!m_Widget.IsVisible());
		else
			Print(string.Format("Widget '%1' not found!"), LogLevel.ERROR);
		return false;
	}
	
	override void HandlerAttached(Widget w)
	{
		m_Widget = w.FindAnyWidget(m_WidgetName);
	}
};