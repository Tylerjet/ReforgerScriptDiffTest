class SCR_BrowserLinkComponent : ScriptedWidgetComponent
{
	[Attribute("www.bohemia.net")]
	protected string m_sAddress;
	
	[Attribute("false")]
	protected bool m_bIsNews;
	
	//protected string m_sPrefix = "https:/";
	//protected string m_sSecondPrefix = "/";
	protected string m_sSecondPrefix;
	protected string m_sPrefix;
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		PlatformService ps = GetGame().GetPlatformService();
		if (!ps)
			return true;

		if (m_bIsNews)
			ps.OpenBrowser(m_sPrefix + m_sSecondPrefix + m_sAddress); // TODO: Get correct address for newsletter
		else
			ps.OpenBrowser(m_sPrefix + m_sSecondPrefix + m_sAddress);
		return true;
	}
};