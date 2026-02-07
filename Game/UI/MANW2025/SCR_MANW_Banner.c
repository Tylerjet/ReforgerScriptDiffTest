class SCR_MANW_Banner : SCR_ScriptedWidgetComponent
{
	protected Widget m_wBannerButtonMain;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{	
		super.HandlerAttached(w);
		
		m_wBannerButtonMain = m_wRoot.FindAnyWidget("m_wBannerButtonMain");
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{	
		bool result = super.OnClick(w, x, y, button);
		
		if (w != m_wBannerButtonMain)
			return result;
		
		SCR_MANW_Dialogs.CreateBannerDialog();
		
		return result;
	}
}