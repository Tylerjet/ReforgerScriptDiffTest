class SCR_RestrictionZoneWarningHUDComponent : SCR_InfoDisplay
{
	override void Show(bool show, float speed = WidgetAnimator.FADE_RATE_DEFAULT, bool force = false)
	{			
		if (m_wRoot && !m_wRoot.IsVisible() && show)
			m_wRoot.SetVisible(true);
		
		super.Show(show, speed, force);
	}
	
	
};