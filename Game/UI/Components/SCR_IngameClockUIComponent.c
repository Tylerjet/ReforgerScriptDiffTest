/**
Gets the Ingame time and displays it on an UI text element.
*/
class SCR_IngameClockUIComponent : MenuRootSubComponent 
{
	//References
	protected TextWidget m_ClockTimeText;
	protected TextWidget m_DateText;
	protected ImageWidget m_DayTimeIcon;
	protected TimeAndWeatherManagerEntity m_TimeAndWeatherManagerEntity;
	
	[Attribute(defvalue: "1")]
	protected bool m_bShowDate;
	[Attribute()]
	protected string m_sTimeTextName;
	[Attribute()]
	protected string m_sDateTextName;
	[Attribute()]
	protected string m_sSeperatorName;
	[Attribute()]
	protected string m_sDayTimeIconName;
	
	
	//~Todo: Update using TimeAndWeather callback once implemented
	//======================== CLOCK UPDATE ========================\\
	//Called everyframe
	protected void OnIngameClockUpdate()
	{
		//Get time
		int hour, minute, sec;
		
		if (!m_TimeAndWeatherManagerEntity) return;
		m_TimeAndWeatherManagerEntity.GetHoursMinutesSeconds(hour, minute, sec);
		
		//Set text
		m_ClockTimeText.SetText(SCR_Global.GetTimeFormattingHoursMinutes(hour, minute));
		
		if (m_DayTimeIcon)
		{
			SCR_UIInfo dayTimeUIInfo;
			m_TimeAndWeatherManagerEntity.GetCurrentDayTimeUIInfo(dayTimeUIInfo);
			dayTimeUIInfo.SetIconTo(m_DayTimeIcon);
		}
			
	}
	
	protected void OnIngameClockAndDateUpdate()
	{
		if (!m_TimeAndWeatherManagerEntity) 
			return;
		
		OnIngameClockUpdate();
		
		//Get date
		string dateString, monthString;
		int day, month, year;
		m_TimeAndWeatherManagerEntity.GetDate(year, month, day);
		
		//Set date
		m_DateText.SetTextFormat("#AR-Date_Format_MonthFull", day, SCR_Global.GetMonthString(month), year);
	}
	
	//======================== ATTACH/DEATTACH HANDLER ========================\\
	//On Init
	override void HandlerAttachedScripted(Widget w)
	{
		m_ClockTimeText = TextWidget.Cast(w.FindAnyWidget(m_sTimeTextName));
		m_DateText = TextWidget.Cast(w.FindAnyWidget(m_sDateTextName));
		m_DayTimeIcon = ImageWidget.Cast(w.FindWidget(m_sDayTimeIconName));

		if (m_ClockTimeText) 
		{
			m_TimeAndWeatherManagerEntity = GetGame().GetTimeAndWeatherManager();
			
			//Only add update if there is a 'TimeAndWeatherManagerEntity' availible
			if (m_TimeAndWeatherManagerEntity) 
			{
				if (m_bShowDate)
				{
					OnIngameClockAndDateUpdate();
					GetGame().GetCallqueue().CallLater(OnIngameClockAndDateUpdate, 1000, true);
				}
				else {
					if (m_DateText)
						m_DateText.SetVisible(false);
					
					Widget seperator = w.FindAnyWidget(m_sSeperatorName);
					if (seperator)
						seperator.SetVisible(false);

					OnIngameClockUpdate();
					GetGame().GetCallqueue().CallLater(OnIngameClockUpdate, 1000, true);
				}
				
				if (m_DayTimeIcon)
				{
					SCR_UIInfo dayTimeUIInfo;
					m_TimeAndWeatherManagerEntity.GetCurrentDayTimeUIInfo(dayTimeUIInfo);
					dayTimeUIInfo.SetIconTo(m_DayTimeIcon);
				}
			}
			else 
			{
				if (m_DateText)
					m_DateText.SetVisible(false);
				
				Widget seperator = w.FindAnyWidget(m_sSeperatorName);
				if (seperator)
					seperator.SetVisible(false);
				
				if (m_DayTimeIcon)
					m_DayTimeIcon.SetVisible(false);
			}
		}
	}
	
	
	//On Destroy
	override void HandlerDeattached(Widget w)
	{
		if (m_ClockTimeText)
		{
			//Remove frame update InGame clock
			MenuRootBase menu = GetMenu();
			
			if (menu)
			{
				if (m_bShowDate)
					menu.GetOnMenuUpdate().Remove(OnIngameClockAndDateUpdate);
				else
					menu.GetOnMenuUpdate().Remove(OnIngameClockUpdate);
			}
				
		
		}
	}

	
};