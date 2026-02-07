//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_UICore: SCR_GameCoreBase
{
	[Attribute("", UIWidgets.ResourceNamePicker, "UI sounds entity", "et")]
	private ResourceName m_UISoundEntityPrefab;
	
	[Attribute("5")]
	private int m_iMaxRecentGames;
	
	private static SCR_UICore s_Instance;
	
	//------------------------------------------------------------------------------------------------
	//! Spawn entities for WidgetAnimator and SCR_UISoundsEntity
	override void OnGameStart()
	{
		if (!s_Instance)
			s_Instance = this;
		
		if (!AnimateWidgetEntity.GetInstance())
			GetGame().SpawnEntity(AnimateWidgetEntity);

		if (!SCR_UISoundEntity.GetInstance())
			GetGame().SpawnEntityPrefab(Resource.Load(m_UISoundEntityPrefab));
		
		if (!SCR_TooltipManagerEntity.GetInstance())
			GetGame().SpawnEntity(SCR_TooltipManagerEntity);
		
#ifdef PLATFORM_CONSOLE
		// Console-only debug option to show video settings
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_UI_SHOW_ALL_SETTINGS, "", "Show all settings", "UI");
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		if (s_Instance == this)
			s_Instance = null;
	}
	
	//------------------------------------------------------------------------------------------------
	static void UpdateSplashScreen(bool firstUpdate = true)
	{
		if (!SplashScreenSequence.s_Sequence)
			return;
		
		// Calculate actual delta time
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		float time = world.GetWorldTime();
		if (firstUpdate)
			SplashScreenSequence.m_fWorldTime = time;
		
		float delta = (time - SplashScreenSequence.m_fWorldTime) / 1000;
		SplashScreenSequence.m_fWorldTime = time;
		bool finished = SplashScreenSequence.s_Sequence.Update(delta);
		
		if (!finished)
			GetGame().GetCallqueue().CallLater(UpdateSplashScreen, SplashScreenSequence.UPDATE_TIME, false, false);
	}
};