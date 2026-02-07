class SCR_BrowserLinkComponent : ScriptedWidgetComponent
{
	[Attribute("https://www.bohemia.net")]
	protected string m_sAddress;
	
	[Attribute("false")]
	protected bool m_bIsNews;
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		PlatformService ps = GetGame().GetPlatformService();
		if (!ps)
			return true;

		if (m_bIsNews)
			ps.OpenBrowser(m_sAddress); // TODO: Get correct address for newsletter
		else
			ps.OpenBrowser(m_sAddress);
		return true;
	}
};