class SCR_BrowserLinkComponent : ScriptedWidgetComponent
{
	[Attribute("Link_Bohemia")]
	protected string m_sAddress;
	
	[Attribute("false")]
	protected bool m_bIsURL;
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		return OpenBrowser();
	}
	
	//------------------------------------------------------------------------------------------------
	bool OpenBrowser()
	{
		PlatformService ps = GetGame().GetPlatformService();
		
		if (!ps)
			return false;

		BackendApi backendAPI = GetGame().GetBackendApi();
		string url = backendAPI.GetLinkItem(m_sAddress);

		if (m_bIsURL)
			ps.OpenBrowser(m_sAddress);
		else
			ps.OpenBrowser(url);
		
		return true;
	}
};