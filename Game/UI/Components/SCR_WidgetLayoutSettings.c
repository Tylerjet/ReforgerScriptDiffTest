class SCR_WidgetLayoutSettings: ScriptedWidgetComponent
{
	[Attribute()]
	protected float m_iLeftPadding;
	[Attribute()]
	protected float m_iTopPadding;
	[Attribute()]
	protected float m_iRightPadding;
	[Attribute()]
	protected float m_iBottomPadding;
	
	[Attribute("0", UIWidgets.ComboBox, "LayoutHorizontalAlign", "", ParamEnumArray.FromEnum(LayoutHorizontalAlign))]
	protected LayoutHorizontalAlign m_LayoutHorizontalAlign;
	[Attribute("0", UIWidgets.ComboBox, "LayoutVerticalAlign", "", ParamEnumArray.FromEnum(LayoutHorizontalAlign))]
	protected LayoutHorizontalAlign m_LayoutVerticalAlign;
	
	
	override void HandlerAttached(Widget w)
	{
		AlignableSlot.SetPadding(w, m_iLeftPadding, m_iTopPadding, m_iRightPadding, m_iBottomPadding);
		AlignableSlot.SetHorizontalAlign(w, m_LayoutHorizontalAlign);
		AlignableSlot.SetVerticalAlign(w, m_LayoutVerticalAlign);
	}
};