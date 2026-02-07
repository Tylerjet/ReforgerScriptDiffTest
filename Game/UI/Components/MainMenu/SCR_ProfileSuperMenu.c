enum SCR_EProfileSuperMenuTabId
{
	NEWS = 0,
	COMMUNITY
}

class SCR_ProfileSuperMenu : SCR_SuperMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		SCR_InputButtonComponent comp = m_DynamicFooter.FindButton("Back");
		if (comp)
			comp.m_OnActivated.Insert(Close);

		Widget logoWidget = GetRootWidget().FindAnyWidget("LogoButton");
		if (logoWidget)
		{
			SCR_ModularButtonComponent logo = SCR_ModularButtonComponent.FindComponent(logoWidget);
			if (logo)
				logo.m_OnClicked.Insert(OnLogoClicked);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLogoClicked()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.WelcomeDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPage(SCR_EProfileSuperMenuTabId page)
	{
		m_SuperMenuComponent.GetTabView().ShowTab(page);
	}
}