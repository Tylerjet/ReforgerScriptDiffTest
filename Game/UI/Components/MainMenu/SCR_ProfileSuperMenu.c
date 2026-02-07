//------------------------------------------------------------------------------------------------
class SCR_ProfileSuperMenu : SCR_SuperMenuBase
{
	const string TERMS_OF_SERVICE_URL = "https:\/\/www.bohemia.net/privacy-policy";
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		SCR_NavigationButtonComponent comp = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Back", GetRootWidget());
		if (comp)
			comp.m_OnActivated.Insert(OnBack);

		SCR_NavigationButtonComponent tos = SCR_NavigationButtonComponent.GetNavigationButtonComponent("ToS", GetRootWidget());
		if (tos)
			tos.m_OnActivated.Insert(OnTos);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPage(int page)
	{
		if (!m_TabViewComponent)
			return;

		m_TabViewComponent.ShowTab(page);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBack()
	{
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTos()
	{
		GetGame().GetPlatformService().OpenBrowser(TERMS_OF_SERVICE_URL);
	}
};