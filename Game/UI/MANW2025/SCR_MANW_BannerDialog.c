class SCR_MANW_BannerDialog : SCR_ConfigurableDialogUi
{
	protected SCR_InputButtonComponent m_Confirm;
	
	const string LINK_ITEM = "Link_MakeArma";
	const string LINK_FALLBACK = "https://www.makearmanotwar.com";

	const string DESCRIPTION = "#MANW-AnnouncementAfterSubmissions_Description";
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		m_Confirm = FindButton(BUTTON_CONFIRM);
		m_Confirm.SetEnabled(SCR_ServicesStatusHelper.IsBackendConnectionAvailable());
		
		string url = GetGame().GetBackendApi().GetLinkItem(LINK_ITEM);
		if (url.IsEmpty())
			url = LINK_FALLBACK;
		
		TextWidget description = TextWidget.Cast(m_wRoot.FindAnyWidget("m_wDescription"));
		description.SetTextFormat(DESCRIPTION, url);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		super.OnConfirm();
	
		string url = GetGame().GetBackendApi().GetLinkItem(LINK_ITEM);
		if (url.IsEmpty())
			url = LINK_FALLBACK;
			
		GetGame().GetPlatformService().OpenBrowser(url);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		m_Confirm.SetEnabled(SCR_ServicesStatusHelper.IsBackendConnectionAvailable());
	}
}