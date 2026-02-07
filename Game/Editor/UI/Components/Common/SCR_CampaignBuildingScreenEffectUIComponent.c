class SCR_CampaignBuildingScreenEffectUIComponent : ScriptedWidgetComponent
{
	protected ImageWidget m_wScrenEffectColor;
	protected ImageWidget m_wScrenEffectVignette;
	protected BlurWidget m_wBlurEffect;
	protected WidgetAnimationBase m_WidgetAnimation;
	
	protected const float DURATION_IN_ALPHA = 0.5;
	protected const float DURATION_IN_BLUR = 0.75;
	protected const float DURATION_OUT_ALPHA = 0.5;
	protected const float DURATION_OUT_BLUR = 0.5;
	
	protected const float VALUE_ALPHA = 0.5;
	protected const float VALUE_BLUR = 0.8;
	protected const float VALUE_DISABLED = 0.0;

	//------------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		m_wScrenEffectColor = ImageWidget.Cast(w.FindAnyWidget("ScreenEffectColor"));
		m_wScrenEffectVignette = ImageWidget.Cast(w.FindAnyWidget("ScreenEffectVignette"));
		m_wBlurEffect = BlurWidget.Cast(w.FindAnyWidget("ScreenEffectBlur"));
		
		SCR_CampaignBuildingEditorComponent CampaignBuildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!CampaignBuildingEditorComponent)
			return;
		
		CampaignBuildingEditorComponent.GetOnObstructionEventTriggered().Insert(AreaTriggerChange);	
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] activated
	void AreaTriggerChange(bool activated)
	{
		if (activated)
			StartObstructionAnimation();
		else
			FinishObstructionAnimation();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Start screen obstruction effect.
	void StartObstructionAnimation()
	{
		m_WidgetAnimation = AnimateWidget.AlphaMask(m_wScrenEffectVignette, VALUE_ALPHA, DURATION_IN_ALPHA);
		m_WidgetAnimation.SetCurve(EAnimationCurve.EASE_OUT_CUBIC);

		AnimateWidget.AddAnimation(new SCR_WidgetAnimationBlurIntensity(m_wBlurEffect, DURATION_IN_BLUR, VALUE_BLUR));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Ends screen obstruction effect.
	void FinishObstructionAnimation()
	{
		m_WidgetAnimation = AnimateWidget.AlphaMask(m_wScrenEffectVignette, VALUE_DISABLED, DURATION_OUT_ALPHA);
		m_WidgetAnimation.SetCurve(EAnimationCurve.EASE_IN_OUT_CUBIC);

		AnimateWidget.AddAnimation(new SCR_WidgetAnimationBlurIntensity(m_wBlurEffect, DURATION_OUT_BLUR, VALUE_DISABLED));
	}
};