class SCR_WeatherForecastUIComponent: ScriptedWidgetComponent
{
	[Attribute("#AR-Weather_Forecast_Looping")]
 	protected LocalizedString m_sLoopingWeatherText;
	
	[Attribute("#AR-Weather_Forecast_NoTimeProgression")]
	protected LocalizedString m_sNoTimeProgressionText;
	
	[Attribute("WeatherIcon")]
	protected string m_sCurrentWeatherIconName;
	
	[Attribute("CurrentWeather")] 
	protected string m_sCurrentWeatherTextName;
	
	[Attribute("NextWeather")] 
	protected string m_sNextWeatherTextName;
		
	[Attribute("#AR-Date_Format_CurrentWeather")] 
	protected LocalizedString m_sCurrentWeatherFormat;
	
	[Attribute("#AR-Date_Format_NextWeather")] 
	protected LocalizedString m_sNextWeatherFormat;
	
	protected TimeAndWeatherManagerEntity m_TimeAndWeatherEntity;
	protected WeatherStateTransitionManager m_WeatherStateManager;
	protected ref array<ref WeatherState> m_WeatherStates = new ref array<ref WeatherState>;
	
	protected ImageWidget m_wCurrentWeatherIcon;
	protected TextWidget m_wCurrentWeatherText;
	protected TextWidget m_wNextWeatherText;
	protected Widget m_wRoot;
	
	//~Todo: Update using TimeAndWeather callback once implemented
	protected void CheckWeatherUpdate()
	{	
		if (m_WeatherStateManager.IsPreviewingState())
			return;
		
		WeatherState currentWeather = m_WeatherStateManager.GetCurrentState();		
		m_wCurrentWeatherIcon.LoadImageTexture(0, currentWeather.GetIconPath());
		m_wCurrentWeatherText.SetTextFormat(m_sCurrentWeatherFormat, currentWeather.GetLocalizedName());
		
		WeatherState nextWeather = m_WeatherStateManager.GetNextState();
		
		//~ Time progression disabled
		if (!m_TimeAndWeatherEntity.GetIsDayAutoAdvanced())
		{
			m_wNextWeatherText.SetTextFormat(m_sNoTimeProgressionText);
		}
		//~ Is looping
		else if (m_TimeAndWeatherEntity.IsWeatherLooping())
		{
			m_wNextWeatherText.SetTextFormat(m_sLoopingWeatherText);
		}
		else 
		{		
			float duration = m_WeatherStateManager.GetTimeLeftUntilNextState();
			
			float changeTime = duration + m_TimeAndWeatherEntity.GetTimeOfTheDay();
			if (changeTime >= 24)
				changeTime -= 24;
			int nextWeatherHour = Math.Floor(changeTime);
			int nextWeatherMinutes = (changeTime - nextWeatherHour) * 60;

			m_wNextWeatherText.SetTextFormat(m_sNextWeatherFormat, nextWeather.GetLocalizedName(), SCR_Global.GetTimeFormattingHoursMinutes(nextWeatherHour, nextWeatherMinutes));
		}
	}
	
	protected void ShowWeatherUI(bool show)
	{
		m_wRoot.SetVisible(show);
	}
	
	
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_wCurrentWeatherIcon = ImageWidget.Cast(w.FindAnyWidget(m_sCurrentWeatherIconName));
		m_wCurrentWeatherText = TextWidget.Cast(w.FindAnyWidget(m_sCurrentWeatherTextName));
		m_wNextWeatherText = TextWidget.Cast(w.FindAnyWidget(m_sNextWeatherTextName));
		
		if (!m_wCurrentWeatherIcon || !m_wCurrentWeatherText || !m_wNextWeatherText)
		{
			Print("SCR_WeatherForecastUIComponent is missing weather widgets and won't work.", LogLevel.WARNING);
			return;
		}
			
		
		m_TimeAndWeatherEntity = GetGame().GetTimeAndWeatherManager();
		
		if (!m_TimeAndWeatherEntity)
		{
			ShowWeatherUI(false);
			return;
		}
			
		
		m_TimeAndWeatherEntity.GetWeatherStatesList(m_WeatherStates);
		if (m_WeatherStates.IsEmpty())
		{
			ShowWeatherUI(false);
			return;
		}
		
		m_WeatherStateManager = m_TimeAndWeatherEntity.GetTransitionManager();
		
		if (!m_WeatherStateManager)
		{
			ShowWeatherUI(false);
			return;
		}
		
		CheckWeatherUpdate();
		GetGame().GetCallqueue().CallLater(CheckWeatherUpdate, 2000, true);
		
	}
	override void HandlerDeattached(Widget w)
	{
		if (!m_WeatherStateManager || m_WeatherStates.IsEmpty() || !m_wCurrentWeatherIcon || !m_wCurrentWeatherText || !m_wNextWeatherText)
			return;
		
		GetGame().GetCallqueue().Remove(CheckWeatherUpdate);
	}
};
