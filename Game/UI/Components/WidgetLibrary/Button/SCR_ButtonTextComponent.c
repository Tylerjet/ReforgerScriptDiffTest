//------------------------------------------------------------------------------------------------
class SCR_ButtonTextComponent : SCR_ButtonBaseComponent 
{
	[Attribute("Text", desc: "Text widget name within the button")]
	protected LocalizedString m_sTextWidgetName;
	
	[Attribute()]
	protected LocalizedString m_sText;
	
	protected TextWidget m_wText;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wText = TextWidget.Cast(w.FindAnyWidget(m_sTextWidgetName));
		if (m_wText)
			m_wText.SetText(m_sText);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		if (m_sText == text)
			return;
		
		m_sText = text;
		if (m_wText)
			m_wText.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTextWithParam(string text, string param1, string param2)
	{
		m_sText = text;
		if (m_wText)
			m_wText.SetTextFormat(text, param1, param2);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetText()
	{
		return m_sText;
	}
	
	//------------------------------------------------------------------------------------------------
	TextWidget GetTextWidget()
	{
		return m_wText;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ButtonTextComponent GetButtonText(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_ButtonTextComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_ButtonTextComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
};