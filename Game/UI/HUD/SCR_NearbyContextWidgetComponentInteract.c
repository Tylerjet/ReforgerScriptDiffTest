class SCR_NearbyContextWidgetComponentInteract : SCR_NearbyContextWidgetComponentBase
{
	[Attribute()]
	protected string m_sIconWidget;

	protected ImageWidget m_wIcon;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (!m_sIconWidget)
			return;

		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sIconWidget));
	}

	//------------------------------------------------------------------------------------------------
	override void OnAssigned(SCR_ActionContextUIInfo info, UserActionContext context)
	{
		super.OnAssigned(info, context);

		if (info && m_wIcon && !SetIconFromAction(info))
			info.SetIconTo(m_wIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the Icon defined in the UIInfo from action
	//! \param[in] SCR_ActionContextUIInfo containing all the information for the widget Icon, etc. Can be null
	//! return True if icon was able to be set from Action, false otherwise
	protected bool SetIconFromAction(SCR_ActionContextUIInfo info)
	{	
		if (!m_FirstAction || !m_wIcon || !info)
			return false;
	
		SCR_ActionUIInfo actionUIInfo = SCR_ActionUIInfo.Cast(m_FirstAction.GetUIInfo());
		
		if (!actionUIInfo)
			return false;
			
		if (!actionUIInfo.SetIconTo(m_wIcon))
			return false;
		
		return true;
	}
}
