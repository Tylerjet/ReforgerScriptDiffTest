class SCR_PulsateUIComponent: ScriptedWidgetComponent
{
	[Attribute("Pulse")]
	protected string m_sPulseWidgetName;
	
	[Attribute("2")]
	protected float m_fPulsateAnimationSpeed;
	
	override void HandlerAttached(Widget w)
	{
		Widget pulse = w.FindAnyWidget(m_sPulseWidgetName);
		
		WidgetAnimator.PlayAnimation(new WidgetAnimationOpacity(pulse, m_fPulsateAnimationSpeed, 0, true));
		//WidgetAnimator.PlayAnimation(new WidgetAnimationFrameSize(pulse, m_fPulsateAnimationSpeed, 90, 90, true));
	}
	override void HandlerDeattached(Widget w)
	{
	}
};
