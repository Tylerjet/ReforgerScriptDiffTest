//------------------------------------------------------------------------------------------------
//! Main menu entity
[EntityEditorProps(category: "GameScripted/Menu", description:"When put into a level will make sure that the main menu is open.")]
class SCR_MainMenuEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_MainMenuEntity : GenericEntity
{	
	[Attribute("ChimeraMenuPreset.MainMenu", UIWidgets.SearchComboBox, "Menu to launch at start of the world", "", ParamEnumArray.FromEnum(ChimeraMenuPreset) )]
	ChimeraMenuPreset m_eMenu;
	
	[Attribute("{6D74D089BD372587}UI/layouts/Menus/MainMenu/BetaWarningScreen.layout")]
	private ResourceName m_sSplashScreenLayout;
	
	//------------------------------------------------------------------------------------------------
	void SCR_MainMenuEntity(IEntitySource src, IEntity parent)
	{
		// Enable
		SetEventMask(EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MainMenuEntity()
	{
		if (m_eMenu == ChimeraMenuPreset.MainMenu)
			GetGame().m_bIsMainMenuOpen = false;
	}

	// Wait until first update - all systems, like MenuManager should be initialized
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		bool isDevVersion = Game.IsDev();
		bool isFirstLoad = SplashScreen.IsFirstLoadingScreen();
		
		Print("Loading main menu. Dev mode:" + isDevVersion + " First load: " + isFirstLoad);
		if (!isFirstLoad || isDevVersion)
			ShowMainMenu();
		else
			ShowSplashScreen();
		
		// Disable all events
		SetEventMask(~EntityEvent.ALL);
		ClearFlags(EntityFlags.ACTIVE, true);
	}
	
	//------------------------------------------------------------------------------------------------
	private void ShowSplashScreen()
	{
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sSplashScreenLayout, GetGame().GetWorkspace());
		SCR_SplashScreenComponent comp = SCR_SplashScreenComponent.Cast(w.FindHandler(SCR_SplashScreenComponent));
		if (!comp)
			return;
		
		SplashScreen.SplashShowed();
		
		comp.ShowEAScreen();
		comp.m_OnFinished.Insert(ShowMainMenu);		
	}
	
	//------------------------------------------------------------------------------------------------
	private void ShowMainMenu()
	{
		GetGame().GetMenuManager().OpenMenu(m_eMenu);
		if (m_eMenu == ChimeraMenuPreset.MainMenu)
			GetGame().m_bIsMainMenuOpen = true;
	}
};
