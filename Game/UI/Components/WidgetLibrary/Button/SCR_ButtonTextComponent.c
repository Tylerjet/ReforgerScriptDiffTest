//------------------------------------------------------------------------------------------------
class SCR_ButtonTextComponent : SCR_ButtonBaseComponent 
{
	[Attribute()]
	protected LocalizedString m_sText;
	
	protected TextWidget m_wText;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wText = TextWidget.Cast(w.FindAnyWidget("Text"));
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