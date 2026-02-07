class SCR_MultiTextTooltipUIComponent: ScriptedWidgetComponent
{
	[Attribute("")]
	protected ResourceName m_TextPrefab;
	
	
	protected Widget m_wRoot;;
	
	void ClearAllText()
	{
		if (!m_wRoot)
			return; 
		
		Widget child = m_wRoot.GetChildren();
		Widget childtemp;
		while (child)
		{
			childtemp = child;
			child = child.GetSibling();
			childtemp.RemoveFromHierarchy();
		}
	}
	
	void AddText(string text, string param1 = string.Empty)
	{
		if (!m_wRoot)
			return;
		
		Widget newWidget = GetGame().GetWorkspace().CreateWidgets(m_TextPrefab, m_wRoot);
		if (!newWidget)
			return;
		
		TextWidget textWidget = TextWidget.Cast(newWidget);
		if (textWidget)
			textWidget.SetTextFormat(text, param1);
	}
	
	
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
	}
};
