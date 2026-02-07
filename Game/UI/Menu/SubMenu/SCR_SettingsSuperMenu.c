//------------------------------------------------------------------------------------------------
class SCR_SettingsSuperMenu : SCR_SuperMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		SCR_NavigationButtonComponent comp = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Back",GetRootWidget());
		if (comp)
			comp.m_OnActivated.Insert(OnBack);

		bool showBlur = !GetGame().m_bIsMainMenuOpen;

		Widget img = GetRootWidget().FindAnyWidget("MenuBackground");
		if (img)
			img.SetVisible(!showBlur);

		BlurWidget blur = BlurWidget.Cast(GetRootWidget().FindAnyWidget("Blur0"));
		if (blur)
		{
			blur.SetVisible(showBlur);
			if (showBlur)
			{
				float w, h;
				GetGame().GetWorkspace().GetScreenSize(w, h);
				blur.SetSmoothBorder(0, 0, w * 0.8, 0);
			}
		}

// Remove video settings and keybinds for consoles, if not in settings debug mode

#ifdef PLATFORM_CONSOLE
		bool isDebug = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_UI_SHOW_ALL_SETTINGS, false);
		if (isDebug)
			return;

		Widget w = GetRootWidget().FindAnyWidget("TabViewRoot0");
		if (!w)
			return;

		SCR_TabViewComponent tab = SCR_TabViewComponent.Cast(w.FindHandler(SCR_TabViewComponent));
		if (!tab)
			return;

		//TODO: remake this to still work in case of switching tab order or adding/removing tabs from settings
		tab.RemoveTab(0); // remove Video
		// remain Audio, Gameplay and Game Master
		tab.ShowTab(0);		// select Audio
#endif
	}

	void OnBack()
	{
		Close();
	}
};
