//------------------------------------------------------------------------------------------------
class SplashScreen: BaseLoadingAnim
{
	const string SPLASH_LAYOUT = "{A1CE9D1EC16DA9BE}UI/layouts/Menus/MainMenu/SplashScreen.layout";
	const string BASIC_LAYOUT = "{C925ADF957A9670A}UI/layouts/Menus/MainMenu/PreloadScreen.layout";
	protected SCR_LoadingSpinner m_Spinner;
	static bool s_SplashShowed = false;
	
	//------------------------------------------------------------------------------------------------
	override void Load()
	{
		m_wRoot = m_WorkspaceWidget.CreateWidgets(BASIC_LAYOUT,m_WorkspaceWidget);
		
		if (BaseLoadingAnim.s_NumReloads > 0 || s_SplashShowed)
		{
			// Show basic loading screen
			Widget w = m_wRoot.FindAnyWidget("Spinner");
			if (w)
				m_Spinner = SCR_LoadingSpinner.Cast(w.FindHandler(SCR_LoadingSpinner));
		}
		else
		{
			// Show splash screen
			SplashScreenSequence.s_wLayout = m_WorkspaceWidget.CreateWidgets(SPLASH_LAYOUT);
			SplashScreenSequence.s_Sequence = new SplashScreenSequence(SplashScreenSequence.s_wLayout);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice, float progress, float minDurationRatio)
	{
		super.Update(timeSlice, progress, minDurationRatio);
		if (SplashScreenSequence.s_Sequence)
			SplashScreenSequence.s_Sequence.Update(timeSlice, progress);
		else if (m_Spinner)
			m_Spinner.Update(timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsFirstLoadingScreen()
	{
		return BaseLoadingAnim.s_NumReloads == 0 && !s_SplashShowed;
	}
	
	static void SplashShowed()
	{
		s_SplashShowed = true;
	}
};

//---------------------------------------------------------------------------------------------
class SplashScreenSequence
{
	const float UPDATE_TIME = 100;
	static float m_fWorldTime;
	static ref Widget s_wLayout;
	static ref SplashScreenSequence s_Sequence;
	static bool m_bShown = false;

	protected BaseWorld m_World;
	protected float m_fTime;
	protected ref array<ref WidgetAnimationBase> m_aAnimations;
	
	protected int m_iPhase;
	protected Widget m_wLogoBI;
	protected Widget m_wFadeImage;
	protected Widget m_wMap;
	protected Widget m_wDust1;
	protected Widget m_wDust2;
	
	protected const float m_fFadeoutTime = 0.25;
	protected const float m_fPhase1Time = 3;
	protected const float m_fPhase2Time = 3.5;
	protected const float m_fPhase3Time = 4;
	protected const float m_fMapWidth = 3840;
	protected const float m_fMapHeight = 2160;
	
	Widget m_wRoot;
	protected SCR_LoadingSpinner m_Spinner;
	
	//---------------------------------------------------------------------------------------------
	void SplashScreenSequence(Widget root)
	{
		m_wRoot = root;
		
		m_aAnimations = {};
		m_fTime = 0;
		m_wRoot.SetZOrder(100000);
		
		m_wMap = m_wRoot.FindAnyWidget("Map");
		m_wLogoBI = m_wRoot.FindAnyWidget("BILogo");
		m_wFadeImage = m_wRoot.FindAnyWidget("FadeImage");
		m_wDust1 = m_wRoot.FindAnyWidget("Dust1");
		m_wDust2 = m_wRoot.FindAnyWidget("Dust2");
		
		Widget spinner = m_wRoot.FindAnyWidget("Spinner");
		if (spinner)
			m_Spinner = SCR_LoadingSpinner.Cast(spinner.FindHandler(SCR_LoadingSpinner));
		
		// Setup first phase
		m_wLogoBI.SetVisible(false);
		m_wMap.SetVisible(false);
		
		m_wFadeImage.SetOpacity(1);
		m_wFadeImage.SetVisible(true);
		
 		Fade(m_wFadeImage, false, 0.35);
		ShowBohemia();
		
		// Skip splash screen if custom world is present, or nosplash parameter
		string world;
		if (System.IsCLIParam("nosplash") || System.GetCLIParam("world", world) /*|| Game().IsDev()*/)
			m_iPhase = 1;
	}
	
	// Update from loading anim
	//---------------------------------------------------------------------------------------------
	void Update(float timeSlice, float progress)
	{
		m_fTime += timeSlice;
		UpdateAnimations(timeSlice);
		
		if (m_Spinner)
			m_Spinner.Update(timeSlice);
	}

	// Update from callqueue
	//---------------------------------------------------------------------------------------------
	bool Update(float timeSlice)
	{
		m_fTime += timeSlice;
		UpdateAnimations(timeSlice);
		
		if (m_Spinner)
			m_Spinner.Update(timeSlice);
		
		// Check for starting a new sequence
		switch (m_iPhase)
		{
			case 0:
				if (m_fTime > m_fPhase1Time)
				{
					// Fade out the logo screen
					m_iPhase++;
					Fade(m_wFadeImage, true, m_fFadeoutTime);
				}
			break;
			case 1:
				if (m_fTime > m_fPhase2Time)
				{
					m_iPhase++;
					FadeOutSplashScreen();
				}
			break;
			case 2:
				if (m_fTime > m_fPhase3Time)
				{
					DeleteSequence();
					return true;
				}
			break;
		}
		
		return false;
	}
	
	//---------------------------------------------------------------------------------------------
	void DeleteSequence()
	{
		if (s_wLayout)
		{
			s_wLayout.SetVisible(false);
			s_wLayout.RemoveFromHierarchy();
			s_wLayout = null;
		}
		
		SplashScreenSequence.s_Sequence = null;
	}
	
	//---------------------------------------------------------------------------------------------
	protected void Fade(Widget w, bool show, float time)
	{
		// Prevent null division
		if (time <= 0)
			time = 0.0001;
		
		AnimateOpacity(w, 1 / time, show);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void FadeOutSplashScreen()
	{
		m_wLogoBI.SetVisible(false);
		m_wMap.SetVisible(false);
		m_wDust2.SetVisible(false);
		m_wDust1.SetVisible(false);
		
		AnimateOpacity(m_wRoot, 1 / m_fFadeoutTime, 0);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void ShowBohemia()
	{
		ResetMapAnimation();
		ResetDustAnimation();

		// Show animated logo
		m_wLogoBI.SetVisible(true);
		m_wLogoBI.SetOpacity(0);
		AnimateOpacity(m_wLogoBI, 1, 1);
	}
	
	// These methods are replacement of widget animator methods, since it's unavailable during the loading screen
	//---------------------------------------------------------------------------------------------
	protected void AnimateOpacity(Widget w, float speed, float targetValue)
	{
		if (!w || speed <= 0)
			return;
		
		StopAnimation(w, WidgetAnimationType.Opacity);
		m_aAnimations.Insert(new WidgetAnimationOpacity(w, speed, targetValue));
	}
	
	//---------------------------------------------------------------------------------------------
	protected void AnimateFrameSize(Widget w, float speed, float width, float height)
	{
		if (!w || speed <= 0)
			return;
		
		StopAnimation(w, WidgetAnimationType.FrameSize);
		m_aAnimations.Insert(new WidgetAnimationFrameSize(w, speed, width, height));
	}
	
	//---------------------------------------------------------------------------------------------
	protected void AnimatePosition(Widget w, float speed, float posX, float posY)
	{
		if (!w || speed <= 0)
			return;
		
		StopAnimation(w, WidgetAnimationType.Position);
		m_aAnimations.Insert(new WidgetAnimationPosition(w, speed, posX, posY));
	}
	
	//---------------------------------------------------------------------------------------------
	protected void AnimateAlphaMask(Widget w, float speed, float targetValue)
	{
		if (!w || speed <= 0)
			return;
		
		StopAnimation(w, WidgetAnimationType.AlphaMask);
		m_aAnimations.Insert(new WidgetAnimationOpacity(w, speed, targetValue));
	}
	
	//---------------------------------------------------------------------------------------------
	protected void StopAnimation(Widget w, WidgetAnimationType type)
	{
		foreach (int i, WidgetAnimationBase animation : m_aAnimations)
		{
			if (w != animation.m_wWidget || animation.m_eAnimationType != type)
				continue;
			
			m_aAnimations.Remove(i);
			return;
		}
	}
	
	//---------------------------------------------------------------------------------------------
	protected void UpdateAnimations(float timeSlice)
	{
		int count = m_aAnimations.Count();
		for (int i = count - 1; i >=0; i--)
		{
			bool finished = m_aAnimations[i].OnUpdate(timeSlice);
			if (finished && !m_aAnimations[i].Repeat())
				m_aAnimations.Remove(i);
		}
	}
	
	//---------------------------------------------------------------------------------------------
	protected void ResetMapAnimation()
	{
		m_wMap.SetVisible(true);
		m_wMap.SetOpacity(1);
		AnimateFrameSize(m_wMap, 0.025, m_fMapWidth * 0.45, m_fMapHeight * 0.45);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void ResetDustAnimation()
	{
		m_wDust1.SetVisible(true);
		m_wDust1.SetOpacity(1);
		m_wDust2.SetVisible(true);
		m_wDust2.SetOpacity(1);

		AnimatePosition(m_wDust1, 0.0025, 10000,0);
		AnimatePosition(m_wDust2, 0.0005, 10000,0);
	}
};