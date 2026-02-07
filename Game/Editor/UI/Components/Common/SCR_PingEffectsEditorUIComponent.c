class SCR_PingEffectsEditorUIComponent: ScriptedWidgetComponent
{
	[Attribute("PingBorder")]
	protected string m_sPingBorderName;
	
	[Attribute("PulseWidget")]
	protected string m_sPulseWidgetName;
	
	[Attribute("2")]
	protected float m_fPingBorderAnimationSpeed;
	
	[Attribute("1")]
	protected float m_fPulsateAnimationSpeed;
	
	[Attribute("5", desc: "The deletion is done with a seperate timer in SCR_PingEditorComponent")]
	protected int m_iFadeOutDelaySeconds;
	
	protected Widget m_Root;

	//State
	protected bool m_bWaitingForFade = true;
	
	override void HandlerAttached(Widget w)
	{
		m_Root = w;
		Widget pingBorder = w.FindAnyWidget(m_sPingBorderName);
		Widget pulseWidget = w.FindAnyWidget(m_sPulseWidgetName);
		
		if (!pulseWidget || !pingBorder)
			return;
		
		WidgetAnimator.PlayAnimation(new WidgetAnimationOpacity(pingBorder, m_fPingBorderAnimationSpeed, 0));
		WidgetAnimator.PlayAnimation(new WidgetAnimationFrameSize(pingBorder, m_fPingBorderAnimationSpeed, 150, 150));
		
		WidgetAnimator.PlayAnimation(new WidgetAnimationOpacity(pulseWidget, m_fPulsateAnimationSpeed, 0, true));
		WidgetAnimator.PlayAnimation(new WidgetAnimationFrameSize(pulseWidget, m_fPulsateAnimationSpeed, 90, 90, true));
		
		GetGame().GetCallqueue().CallLater(FadeOut, m_iFadeOutDelaySeconds * 1000);
	}
	
	//~Todo: Should delete the widget once fading is done rather then having a seperate delete timer
	protected void FadeOut()
	{
		m_bWaitingForFade = false;
		WidgetAnimator.PlayAnimation(new WidgetAnimationOpacity(m_Root, 1, 0));
	
	}
	
	override void HandlerDeattached(Widget w)
	{
		if (m_bWaitingForFade)
			GetGame().GetCallqueue().Remove(FadeOut);
	}
};



