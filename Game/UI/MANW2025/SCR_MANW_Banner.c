class SCR_MANW_Banner : SCR_ScriptedWidgetComponent
{
	protected string NAME_BUTTON 	= "m_wBannerButtonMain";
	protected string NAME_TEXT 		= "BannerText";
	
	protected Widget m_wBannerButtonMain;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{	
		super.HandlerAttached(w);
		
		m_wRoot.SetVisible(false);
		Init();
		
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Init()
	{
		if (GetGame().GetBackendApi().GetPopupCount() < 1)
			return;
		
		PopupFeedItem popupItem = GetGame().GetBackendApi().GetPopupItem(0);
		if (!popupItem)
			return;
		
		m_wRoot.SetVisible(true);
		
		TextWidget textTitle = TextWidget.Cast(m_wRoot.FindAnyWidget(NAME_TEXT));
		textTitle.SetText(popupItem.BannerText());
		
		m_wBannerButtonMain = m_wRoot.FindAnyWidget(NAME_BUTTON);
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
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		if (!m_wRoot.IsVisible() && SCR_ServicesStatusHelper.IsBackendConnectionAvailable())
			Init();
	}
}