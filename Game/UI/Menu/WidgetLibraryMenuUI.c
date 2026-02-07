//------------------------------------------------------------------------------------------------
class WidgetLibraryMenuUI: ChimeraMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		SCR_NavigationButtonComponent back = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Back",GetRootWidget());
		if (back)
			back.m_OnActivated.Insert(OnBack);
	}

	//------------------------------------------------------------------------------------------------
	void OnBack()
	{
		GameStateTransitions.RequestGameplayEndTransition();
	}
};