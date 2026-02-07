class SCR_WidgetAnimationBlurIntensity : WidgetAnimationBase
{
	protected float m_fValueDefault;
	protected float m_fValueTarget;
	protected float m_fValueCurrent;
	protected ImageWidget m_wImage;
		
	//------------------------------------------------------------------------------------------------
	protected override void Animate(bool finished)
	{
		if (!m_wWidget)
			return;
		
		BlurWidget blurWidget = BlurWidget.Cast(m_wWidget);
		if (!blurWidget)
			return;
		
		m_fValueCurrent = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fValue);
		blurWidget.SetIntensity(m_fValueCurrent);
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] w
	//! \param[in] speed
	//! \param[in] targetValue
	void SCR_WidgetAnimationBlurIntensity(Widget w, float speed, float targetValue)
	{
		m_fValueTarget = targetValue;
		m_fValueDefault = 0;
	}
};