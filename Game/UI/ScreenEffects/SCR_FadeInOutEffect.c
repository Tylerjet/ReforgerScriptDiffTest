class SCR_FadeInOutEffect : SCR_BaseScreenEffect
{
	//Widget
	private ImageWidget 							m_wFadeInOut;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wFadeInOut = ImageWidget.Cast(m_wRoot.FindAnyWidget("FadeOut"));
	}

	//------------------------------------------------------------------------------------------------
	//! Fade the screen in/out
	//! \param fadeDirection fades out if true, in if false 
	void FadeOutEffect(bool fadeDirection, float seconds = 0.35)
	{
		if (seconds <= 0)
			return;
		
		float targetVal;
		if (fadeDirection)
		{
			m_wFadeInOut.SetOpacity(0);
			targetVal = 1;
		}
		else
		{
			m_wFadeInOut.SetOpacity(1);
		}
		
		AnimateWidget.Opacity(m_wFadeInOut, targetVal, 1/seconds, true);
	}
};