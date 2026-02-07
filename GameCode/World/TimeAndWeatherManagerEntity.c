class TimeAndWeatherManagerEntityClass: BaseTimeAndWeatherManagerEntityClass
{
};

/*!
Manager entity responsible for managing in-game time and weather,
providing the script and gamecode with usable in-game API.
*/
class TimeAndWeatherManagerEntity : BaseTimeAndWeatherManagerEntity
{	
	[Attribute(desc: "An ordered array of daytime info. Starting just after midnight and ending at midnight, the daytime must be ordered!")]
	protected ref array<ref SCR_DayTimeInfoBase> m_aOrderedDaytimeInfo;
	
	[Attribute(desc: "An ordered array of winddirection info, starting at north and having set steps between each wind direction. (default 45)")]
	protected ref array<ref SCR_WindDirectionInfo> m_OrderedWindDirectionInfo;
	
	[Attribute(desc: "An array of monephase naming in order from New moon to Full moon. Check GetMoonPhaseNameForDate before changing this array!")]
	protected ref array<ref SCR_MoonPhaseInfo> m_aMoonPhasesInfo;
	
	protected bool m_bDelayedWindOverride = false;
	protected int m_iDelayedPlayerChangingWind = -1;
	protected float m_fDelayedWindSpeedOverride = -1;
	protected float m_fDelayedWindDirectionOverride = -1;
	protected bool m_bListiningToWindOverrideDelay = false;
	
	//Replicated
	protected bool m_bWeatherIsLooping = false;
	
	/*!
	Sets current weather to looping true or false.  (server only)
	Use ForceWeatherTo to have more control over transition time and state duration
	\param setLooping Set looping true or false
	\param playerChangedLooping ID of player that set weather to looping
	*/
	void SetCurrentWeatherLooping(bool setLooping, int playerChangedLooping = 0)
	{
		ForceWeatherTo(setLooping, playerThatChangedWeather: playerChangedLooping);
	}
	
	/*!
	Forces weather to the given state. Does not care if the transition is valid or not. (Server only)
	Use this if you want to set weather to looping or change the weather on the fly
	Do not change weather from looping true/false outside of this function
	\param setLooping Set looping true or false
	\param weatherID weather to set. Leave empty if you want to change the current weather to looping true/false
	\param transitionDuration transition duration of weather
	\param stateDuration state duration of weather
	\param playerThatChangedWeather ID of player that changed the weather for notifications. Leave empty if no notifications should be called
	*/
	void ForceWeatherTo(bool setLooping, string weatherID = string.Empty, float transitionDuration = 0, float stateDuration = 0.001, int playerThatChangedWeather = 0)
	{
		if (!Replication.IsServer())
		{
			Print("'ForceWeatherTo' was called in 'TimeAndWeatherManagerEntity' but only server has authority to change weather!", LogLevel.WARNING);
			return;
		}
		
		WeatherStateTransitionManager transitionManager = GetTransitionManager();	
		if (!transitionManager)
		{
			Print("'ForceWeatherTo' was called in 'TimeAndWeatherManagerEntity' but TransitionManager was not found!", LogLevel.WARNING);
			return;
		}
		
		if (weatherID.IsEmpty())
		{
			WeatherState currentWeather = transitionManager.GetCurrentState();
			weatherID = currentWeather.GetStateName();
		}
			
		WeatherStateTransitionNode transitionNode = transitionManager.CreateStateTransition(weatherID, transitionDuration, stateDuration);
		if (!transitionNode)
		{
			Print("'ForceWeatherTo' in 'TimeAndWeatherManagerEntity' could not create a new WeatherStateTransitionNode", LogLevel.WARNING);
			return;
		}
		
		transitionNode.SetLooping(setLooping);
		
		//~ Remove Preview if any
		if (transitionManager.IsPreviewingState())
			transitionManager.SetStatePreview(false);
		
		transitionManager.EnqueueStateTransition(transitionNode, false);
		transitionManager.RequestStateTransitionImmediately(transitionNode);
		
		//~ Send to clients if weather is looping or not
		if (setLooping != m_bWeatherIsLooping)
		{
			UpdateWeatherLooping(setLooping);
			Rpc(UpdateWeatherLooping, setLooping);
		}
		
		//~ Send notification if player ID was given
		if (playerThatChangedWeather > 0)
		{
			if (!setLooping)
			{
				SCR_NotificationsComponent.SendToGameMasters(ENotification.EDITOR_ATTRIBUTES_WEATHER_AUTO, playerThatChangedWeather);
			}
			else 
			{
				BaseGameMode gameMode = GetGame().GetGameMode();
				if (gameMode)
				{
					SCR_NotificationSenderComponent notificationSender = SCR_NotificationSenderComponent.Cast(gameMode.FindComponent(SCR_NotificationSenderComponent));
					
					if (notificationSender)
						notificationSender.OnWeatherChangedNotification(playerThatChangedWeather);
				}
			}
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void UpdateWeatherLooping(bool isLooping)
	{	
		m_bWeatherIsLooping = isLooping;
	}
	
	/*!
	Get if weather is looping
	\return true if weather currently is looping
	*/
	bool IsWeatherLooping()
	{
		return m_bWeatherIsLooping;
	}
	
	/*!
	Gets moon phase name for supplied date, geo location is not needed but timezone yes in order to calculate UTC correctly.
	\param year Year.
	\param month Month in range <1, 12>
	\param day Day in range <1, 31>
	\param timeOfTheDay24 Time of the day in 24 hour floating point format.
	\param timezone TimeZone Offset in hours ranging <-12, +14>
	\param dstOffset DST (daylight savings time) offset, must be 0.0 or positive value.
	\return SCR_MoonPhaseUIInfo moonphase info. This includes the full name of the moonphase as well as a simplified name (eg: first and Third quarter will be called half moon)
	*/
	SCR_MoonPhaseUIInfo GetMoonPhaseInfoForDate(int year, int month, int day, float timeOfTheDay24, float timezone, float dstOffset, float Latitude)
	{
		if (m_aMoonPhasesInfo.IsEmpty())
		{
			Print("Could not get moon phase name as m_aMoonPhasesInfo is empty!", LogLevel.ERROR);
			return null;
		}
		
		float moonPhaseAmount = GetMoonPhaseForDate(year, month, day, timeOfTheDay24, timezone, dstOffset);		
		
		int nextPhaseYearCheck = year;
		int nextPhaseMonthCheck = month;
		int nextPhaseDayCheck = day + 1;
		
		//Calculate if the next day is a valid date
		if (!CheckValidDate(nextPhaseYearCheck, nextPhaseMonthCheck, nextPhaseDayCheck))
		{
			if (nextPhaseMonthCheck != 12)
			{
				nextPhaseMonthCheck += 1;
			}	
			else
			{
				nextPhaseMonthCheck = 1;
				nextPhaseYearCheck += 1;
			}
			
			nextPhaseDayCheck = 1;
		}
		
		//Check the next days moon phase
		float nextPhaseAmount = GetMoonPhaseForDate(nextPhaseYearCheck, nextPhaseMonthCheck, nextPhaseDayCheck, timeOfTheDay24, timezone, dstOffset);
		EMoonPhaseEnums beforeOrAfterFullMoon;
		
		//Check if current moonphase is before or after full moon by checking if next days moonphase amount is higher or lower then current moon phase amount
		if (nextPhaseAmount > moonPhaseAmount)
			beforeOrAfterFullMoon = EMoonPhaseEnums.BEFORE_FULL_MOON;
		else 
			beforeOrAfterFullMoon = EMoonPhaseEnums.AFTER_FULL_MOON;
			
		int count = m_aMoonPhasesInfo.Count();
		
		SCR_MoonPhaseUIInfo uiInfo;
		
		for(int i = 0; i < count; i++)
        {
			if (i == count -1)
			{
				uiInfo = m_aMoonPhasesInfo[i].GetUIInfo();
				break;
			}

			if (m_aMoonPhasesInfo[i].IsMoonPhase(moonPhaseAmount, beforeOrAfterFullMoon))
			{
				uiInfo = m_aMoonPhasesInfo[i].GetUIInfo();
				break;
			}
				
        }
		
		if (uiInfo)
			uiInfo.SetMoonphaseImageRotation(Latitude >= 0);
		
		return uiInfo;
	}
	
	/*!
		Sets the current weather.
		Only issuable by the authority.
		Automatically broadcast to all clients.	
		\param hours Current weather as a normalized value <0.0, 1.0>
		\param immediateChange Whether change should be applied immediately, forcing recomputation. This should be true only in case of editor and similar items.
		\return Returns true when command is issued successfully, false otherwise.
	*/
	//proto native bool SetCurrentWeather(float weather01, bool immediateChange = false);
	
	/*!
		Retrieves the current weather.
		\return Current weather as a normalized value <0.0, 1.0>
	*/
	//proto native float GetCurrentWeather();
	


	/*!
		Sets the current preset.
		Only issuable by the authority.
		Automatically broadcast to all clients.	
		\param hours Current preset as a normalized value <0.0, 1.0>
		\param immediateChange Whether change should be applied immediately, forcing recomputation. This should be true only in case of editor and similar items.
		\return Returns true when command is issued successfully, false otherwise.
	*/
	//proto native bool SetCurrentPreset(float preset01, bool immediateChange = false);
	
	/*!
		Retrieves the current preset.
		\return Current preset as a normalized value <0.0, 1.0>
	*/
	//proto native float GetCurrentPreset();

	
	/*!
		Retrieves current time of the day and returns it as a script wrapper for hours, minutes and seconds.
		\return Returns script wrapper for time
	*/
	ref TimeContainer GetTime()
	{
		ref TimeContainer cont = new TimeContainer();
		GetHoursMinutesSeconds(cont.m_iHours, cont.m_iMinutes, cont.m_iSeconds);
		return cont;
	}
	
	protected void CreateDayTimeInfoArray(int year = -1, int month = -1, int day = -1)
	{
		foreach(SCR_DayTimeInfoBase dayTimeInfo: m_aOrderedDaytimeInfo)
			dayTimeInfo.Clear();
		
		foreach(SCR_DayTimeInfoBase dayTimeInfo: m_aOrderedDaytimeInfo)
			dayTimeInfo.SetDayTime(m_aOrderedDaytimeInfo, this, year, month, day);
	}
	
	/*!
	Gets the UI info of the current time of day. Also returns the index of the current time of day info
	\param[out] uiInfo ui info of time of day
	/return int time of day info
	*/
	int GetCurrentDayTimeUIInfo(out SCR_UIInfo uiInfo)
	{
		CreateDayTimeInfoArray();
		
		return GetDayTimeUIInfo(GetTimeOfTheDay(), uiInfo);		
	}
	
	/*!
	Gets an array of daytime info which holds the UIinfo of specific times of the day
	\param TimeOfDay the time of day to get time info of
	\param[out] uiInfo time info of the given day time
	\param year date to get time info of
	\param month date to get time info of
	\param day date to get time info of
	*/
	int GetDayTimeUIInfo(float TimeOfDay, out SCR_UIInfo uiInfo, int year = -1, int month = -1, int day = -1)
	{
		CreateDayTimeInfoArray(year, month, day);
		
		int count = m_aOrderedDaytimeInfo.Count();
		
		//If before early morning but after midnight
		if (TimeOfDay < m_aOrderedDaytimeInfo[0].GetDayTime())
		{
			uiInfo = m_aOrderedDaytimeInfo[count -1].GetDayTimeUIInfo();
			return count -1;
		}
		
		for (int i = 0; i < count -1; i++)
		{
			if (TimeOfDay > m_aOrderedDaytimeInfo[i].GetDayTime() && TimeOfDay < m_aOrderedDaytimeInfo[i +1].GetDayTime())
			{
				uiInfo = m_aOrderedDaytimeInfo[i].GetDayTimeUIInfo();
				return i;
			} 
		}
		
		//Just before Midnight
		uiInfo = m_aOrderedDaytimeInfo[count -2].GetDayTimeUIInfo();
		return count -2;
	}
	
	/*!
	Gets an array of daytime info which holds the UIinfo of specific times of the day
	\param[out] dayTimeInfoArray array with time of day infos
	\param year date to get time info of
	\param month date to get time info of
	\param day date to get time info of
	*/
	int GetDayTimeInfoArray(out notnull array<SCR_DayTimeInfoBase> dayTimeInfoArray, int year = -1, int month = -1, int day = -1)
	{
		CreateDayTimeInfoArray(year, month, day);
		
		dayTimeInfoArray.Clear();
		
		foreach (SCR_DayTimeInfoBase info: m_aOrderedDaytimeInfo)
			dayTimeInfoArray.Insert(info);
		
		return dayTimeInfoArray.Count();
	}
	
	/*!
	Gets array of ordered Wind Direction info
	\param[out] orderedWindDirectionInfo array of winddirection info
	\return int count of array
	*/
	int GetOrderedWindDirectionInfoArray(notnull array<SCR_WindDirectionInfo> orderedWindDirectionInfo)
	{
		foreach (SCR_WindDirectionInfo windInfo: m_OrderedWindDirectionInfo)
			orderedWindDirectionInfo.Insert(windInfo);
		
		return m_OrderedWindDirectionInfo.Count();
	}
	
	/*!
	Gets wind direction info using index of array
	\param index index to find winddirection info
	\param[out] windDirectionInfo found winddirection info
	\return bool returns true if winddirection info found
	*/
	bool GetWindDirectionInfoFromIndex(int index, out SCR_WindDirectionInfo windDirectionInfo)
	{
		if (m_OrderedWindDirectionInfo.IsEmpty() || index < 0 || index >= m_OrderedWindDirectionInfo.Count())
			return false;
		
		windDirectionInfo = m_OrderedWindDirectionInfo[index];
		return true;
	}
	
	/*!
	Gets wind direction info from winddirection float
	\param windDirectionFloat winddirection of which to find the info
	\param[out] index index of winddirection info
	\param[out] windDirectionInfo found winddirection info
	\return bool returns true if winddirection info found
	*/
	bool GetWindDirectionInfoFromFloat(float windDirectionFloat, out int index, out SCR_WindDirectionInfo windDirectionInfo)
	{
		if (m_OrderedWindDirectionInfo.IsEmpty() || m_OrderedWindDirectionInfo.Count() < 2)
			return false;
		
		int count = m_OrderedWindDirectionInfo.Count();
		float checkRange = (m_OrderedWindDirectionInfo[1].GetWindDirectionValue() - m_OrderedWindDirectionInfo[0].GetWindDirectionValue()) / 2;
		
		for (int i = 0; i < count; i++)
		{
			float firstCheck = m_OrderedWindDirectionInfo[i].GetWindDirectionValue() - checkRange;
			float secondCheck = m_OrderedWindDirectionInfo[i].GetWindDirectionValue() + checkRange;
	
			if (firstCheck < 0)
			{			
				firstCheck = firstCheck + 360;	
				if ((windDirectionFloat <= m_OrderedWindDirectionInfo[i].GetWindDirectionValue() || windDirectionFloat >= firstCheck) || (windDirectionFloat < secondCheck))
				{
					index = i;
					windDirectionInfo = m_OrderedWindDirectionInfo[i];
					return true;
				}
				
			}
			else if (secondCheck > 360)
			{
				secondCheck = secondCheck - 360;
				if ((windDirectionFloat >= firstCheck) && (windDirectionFloat > m_OrderedWindDirectionInfo[i].GetWindDirectionValue() || windDirectionFloat <= secondCheck))
				{
					index = i;
					windDirectionInfo = m_OrderedWindDirectionInfo[i];
					return true;
				}
			}
			else 
			{
				if (windDirectionFloat >= firstCheck && windDirectionFloat < secondCheck)
				{
					index = i;
					windDirectionInfo = m_OrderedWindDirectionInfo[i];
					return true;
				}
			}
		}
		return false;
	}

	/*!
	Sets wind override true or false. Will wait a frame before it is applied so wind direction change and wind speed change can be executed
	\param overrideWind if true wind will be set to override using m_fDelayedWindSpeedOverride and m_fDelayedWindDirectionOverride
	\param playerChangingWind player who changed the wind override to show a notification
	*/
	void DelayedSetWindOverride(bool overrideWind, int playerChangingWind = -1)
	{
		m_bDelayedWindOverride = overrideWind;
		
		if (playerChangingWind > -1)
			m_iDelayedPlayerChangingWind = playerChangingWind;
		
		StartListeningToWindApplyDelay();
	}
	
	/*!
	Sets wind speed override. Will wait a frame before it is applied. Will do nothing if m_bDelayedWindOverride is false
	\params windSpeed new windspeed
	\param playerChangingWind player who changed the wind override to show a notification
	*/
	void DelayedOverrideWindSpeed(float windSpeed, int playerChangingWind = -1)
	{
		m_fDelayedWindSpeedOverride = windSpeed;
		
		if (playerChangingWind > -1)
			m_iDelayedPlayerChangingWind = playerChangingWind;
		
		StartListeningToWindApplyDelay();
	}
	
	/*!
	Sets wind direction override. Will wait a frame before it is applied. Will do nothing if m_bDelayedWindOverride is false
	\params windDirection new winddirection
	\param playerChangingWind player who changed the wind override to show a notification
	*/
	void DelayedOverrideWindDirection(float windDirection, int playerChangingWind = -1)
	{
		m_fDelayedWindDirectionOverride = windDirection;
		
		if (playerChangingWind > -1)
			m_iDelayedPlayerChangingWind = playerChangingWind;
		
		StartListeningToWindApplyDelay();
	}
	
	//Start listening to the frame delay to execute all wind changed in one go
	protected void StartListeningToWindApplyDelay()
	{
		if (m_bListiningToWindOverrideDelay)
			return;
		
		m_bListiningToWindOverrideDelay = true;
		GetGame().GetCallqueue().CallLater(DelayedApplyWindOverride, 1);
	}
	
	//Applies all wind override changes in one go instead of separately setting wind speed and wind direction override.
	//Also shows noptification if m_iDelayedPlayerChangingWind is set 
	protected void DelayedApplyWindOverride()
	{		
		//Set wind to auto
		if (!m_bDelayedWindOverride)
		{
			SetWindSpeedOverride(false);
			SetWindDirectionOverride(false);
			
			//Notification wind default
			if (m_iDelayedPlayerChangingWind > -1)
				SCR_NotificationsComponent.SendToGameMasters(ENotification.EDITOR_ATTRIBUTES_WIND_DEFAULT, m_iDelayedPlayerChangingWind);
		}
		//Override wind
		else 
		{
			if (m_fDelayedWindSpeedOverride == -1)
				m_fDelayedWindSpeedOverride = GetWindSpeed();
			
			SetWindSpeedOverride(true, m_fDelayedWindSpeedOverride);
			
			if (m_fDelayedWindDirectionOverride == -1)
				m_fDelayedWindDirectionOverride =  GetWindDirection();
			
			SetWindDirectionOverride(true, m_fDelayedWindDirectionOverride);	
			
			//Notification wind override
			if (m_iDelayedPlayerChangingWind > -1)
			{
				SCR_WindDirectionInfo windDirectionInfo;
				int windDirectionIndex;
				GetWindDirectionInfoFromFloat(m_fDelayedWindDirectionOverride, windDirectionIndex, windDirectionInfo);
				SCR_NotificationsComponent.SendToGameMasters(ENotification.EDITOR_ATTRIBUTES_WIND_CHANGED, m_iDelayedPlayerChangingWind, m_fDelayedWindSpeedOverride * 1000, windDirectionIndex);
			}
		}
		
		ClearDelayedWindOverrideVars();
	}
	
	//Reset all changed wind varriables
	protected void ClearDelayedWindOverrideVars()
	{
		m_bListiningToWindOverrideDelay = false;
		m_bDelayedWindOverride = IsWindSpeedOverridden() || IsWindDirectionOverridden();
		m_iDelayedPlayerChangingWind = -1;
		m_fDelayedWindSpeedOverride = -1;
		m_fDelayedWindDirectionOverride = -1;
	}
	
	/*!
	Sets current time of the day via a script wrapper.
	\see SetHoursMinutesSeconds for more detailed description.
	\params cont Script wrapper for time in hours, minutes and seconds
	\return Returns true when command is issued successfully, false otherwise.
	*/
	bool SetTime(TimeContainer cont)
	{
		return SetHoursMinutesSeconds(cont.m_iHours, cont.m_iMinutes, cont.m_iSeconds);
	}
	
	
	//======================================== RPL ========================================\\
	override bool RplSave(ScriptBitWriter writer)
    {	
        writer.WriteBool(m_bWeatherIsLooping); 
        return true;
    }
     
    override bool RplLoad(ScriptBitReader reader)
    {
		int isLooping;
		
        reader.ReadBool(isLooping);

		
		UpdateWeatherLooping(isLooping);
		
        return true;
    }
	
	/*!
	Forces weather to supplied state. 
	Also computes a new state transition and variant transitions.
	
	Can only be set by the authority (server, singleplayer...).
	
	\param stateName Name of the state
	\param setToStartVariant Set state to start variant (recommend = true), true by default.
	\return False if not authority or state does not exist within state machine. True otherwise.
	*/
	 //proto native bool SetWeatherState(string stateName, bool setToStartVariant = true);
	 
	/*!
	Forces state to supplied variant.
	Also computes a new variant transition.
	
	Can only be set by the authority (server, singleplayer...).
	
	\return False if not authority or variant does not exist within current state. True otherwise.
	*/
	//proto native bool SetWeatherVariant(string variantName);

	/*!
	Forces weather to supplied state and variant.
	Can only be set by the authority (server, singleplayer...)
	\return False if not authority or state does not exist or variant doest not exist within selected state. True otherwise.
	*/
	//proto native bool SetWeatherStateAndVariant(string stateName, string variantName);
	
	/*!
	Tries to request a state transition.
	Can only be set by the authority (server, singleplayer...)
	\param stateName destination state name
	\param durationHrs Transition duration (if greater than 0). Otherwise random duration between min/max limits.
	\return WeatherTransitionRequestResponse with result of call
	*/
	//proto native WeatherTransitionRequestResponse RequestWeatherStateTransition(string stateName, float durationHrs = 0);
	
	/*!
	Tries to request a variant transition.
	Can only be set by the authority (server, singleplayer...)
	\param variatName destination variant name
	\param durationHrs Transition duration (if greater than 0). Otherwise random duration between min/max limits.
	\return WeatherTransitionRequestResponse with result of call
	*/
	//proto native WeatherTransitionRequestResponse RequestWeatherVariantTransition(string variantName, float durationHrs = 0);
};

/*!
	Simple container holding in-game time data in hours, minutes, seconds format.
*/
class TimeContainer
{
	//!< In-game hours <0, 24>
	int m_iHours;
	//!< In-game minutes <0, 60>
	int m_iMinutes;
	//!< In-game seconds <0, 60>
	int m_iSeconds;
	
	/*!
		Creates new time container from provided params.
		Actual values can be retrieved via TimeAndWeatherManagerEntity.
		\param hours Hours <0,24>
		\param minutes Minutes <0,60>
		\param seconds Seconds <0,60>
	*/
	void TimeContainer(int hours = 0, int minutes = 0, int seconds = 0)
	{
		m_iHours = hours;
		m_iMinutes = minutes;
		m_iSeconds = seconds;
	}
	
	/*
		Creates new time container from hours, minutes and seconds.
	*/
	static TimeContainer FromHoursMinutesSeconds(int hours, int minutes, int seconds)
	{
		return new TimeContainer(hours, minutes, seconds);
	}
	
	/*
		Creates new time container from time of  the day <0.0, 0.24>
	*/
	static TimeContainer FromTimeOfTheDay(float hours24)
	{
		int h, m, s;
		TimeAndWeatherManagerEntity.TimeToHoursMinutesSeconds(hours24, h, m, s);
		return new TimeContainer(h, m, s);
	}
	
	/*
		Converts the content of this container into in-game time as a fraction of day <0.0, 0.24>.
	*/
	float ToTimeOfTheDay()
	{
		return TimeAndWeatherManagerEntity.HoursMinutesSecondsToTime(m_iHours, m_iMinutes, m_iSeconds);
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EDayTimeEnums, "m_dayTimeEnum")]
class SCR_DayTimeInfoBase
{
	[Attribute("0", UIWidgets.ComboBox, "Day Time Enum", "", ParamEnumArray.FromEnum(EDayTimeEnums))]
	protected EDayTimeEnums m_DayTimeEnum;
		
	[Attribute()]
	protected ref SCR_UIInfo m_UIInfo;
	
	protected float m_fTimeOfDay = -1;
	
	/*!
	Gets time of day UI info
	/return SCR_UIInfo ui info of time of day
	*/
	SCR_UIInfo GetDayTimeUIInfo()
	{
		return m_UIInfo;
	}
	
	/*!
	Gets time of day enum
	/return EDayTimeEnums time of day enum
	*/
	EDayTimeEnums GetDaytimeEnum()
	{
		return m_DayTimeEnum;
	}
	
	/*!
	Gets time of day in seconds. This is when the time of day starts
	/return seconds of starting time of time of day
	*/
	float GetDayTime()
	{
		return m_fTimeOfDay;
	}
	
	/*!
	Clear the data
	*/
	void Clear()
	{
		m_fTimeOfDay = -1;
	}
	
	/*!
	Gets time of day in seconds. This is when the time of day starts. Used when init other times of day
	/param daytimeInfo array of other time of days to get info from
	/return seconds of starting time of time of day
	*/
	float GetDayTime(notnull array<ref SCR_DayTimeInfoBase> daytimeInfo, TimeAndWeatherManagerEntity timeAndWeatherEntity)
	{
		if (m_fTimeOfDay < 0)
			SetDayTime(daytimeInfo, timeAndWeatherEntity);
		
		return m_fTimeOfDay;
	}
	
	/*!
	Sets the starting time of the time of day in seconds
	/param daytimeInfo array of other time of days to get info from
	*/
	void SetDayTime(notnull array<ref SCR_DayTimeInfoBase> daytimeInfo, TimeAndWeatherManagerEntity timeAndWeatherEntity, int year = -1, int month = -1, int day = -1)
	{		
		Print(string.Format("%1 Uses the SCR_DayTimeInfoBase use one of the inherented classes instead!", typename.EnumToString(EDayTimeEnums, m_DayTimeEnum)), LogLevel.WARNING);
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EDayTimeEnums, "m_dayTimeEnum")]
class SCR_DayTimeInfoStatic: SCR_DayTimeInfoBase
{
	[Attribute(desc: "Hour which this daytime starts, is a value of 0 to 24.0")]
	protected int m_iHour;
	
	override void SetDayTime(notnull array<ref SCR_DayTimeInfoBase> daytimeInfo, TimeAndWeatherManagerEntity timeAndWeatherEntity, int year = -1, int month = -1, int day = -1)
	{
		if (m_fTimeOfDay > -1)
			return;
		
		m_fTimeOfDay = m_iHour;
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EDayTimeEnums, "m_dayTimeEnum")]
class SCR_DayTimeInfoAuto: SCR_DayTimeInfoBase
{
	override void SetDayTime(notnull array<ref SCR_DayTimeInfoBase> daytimeInfo, TimeAndWeatherManagerEntity timeAndWeatherEntity, int year = -1, int month = -1, int day = -1)
	{
		if (m_fTimeOfDay > -1)
			return;
		
		if (year < 0)
		{
			timeAndWeatherEntity.GetDate(year, month, day);
		}
		
		bool hasSunSetandRise;
		
		switch (m_DayTimeEnum)
		{
			//~Todo: Needs to get sunset/sun rise of given date
			case EDayTimeEnums.DAYTIME_DAWN:
			{	
				hasSunSetandRise = timeAndWeatherEntity.GetSunriseHourForDate(year, month, day, timeAndWeatherEntity.GetCurrentLatitude(), timeAndWeatherEntity.GetCurrentLongitude(), timeAndWeatherEntity.GetTimeZoneOffset(),  timeAndWeatherEntity.GetDSTOffset(),  m_fTimeOfDay);
				
				//If has no sun rise
				if (!hasSunSetandRise)
					m_fTimeOfDay = 5;

				return;
			}
			//~Todo: Get sunset time
			case EDayTimeEnums.DAYTIME_DUSK:
			{	
				hasSunSetandRise = timeAndWeatherEntity.GetSunsetHourForDate(year, month, day, timeAndWeatherEntity.GetCurrentLatitude(), timeAndWeatherEntity.GetCurrentLongitude(), timeAndWeatherEntity.GetTimeZoneOffset(),  timeAndWeatherEntity.GetDSTOffset(),  m_fTimeOfDay);
				
				//If has no sun set
				if (!hasSunSetandRise)
					m_fTimeOfDay = 19;
				
				return;
			}
		}
			
		Print(string.Format("%1 is not supported with autoTime!", typename.EnumToString(EDayTimeEnums, m_DayTimeEnum)), LogLevel.WARNING);
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EDayTimeEnums, "m_dayTimeEnum")]
class SCR_DayTimeInfoBetween: SCR_DayTimeInfoBase
{
	[Attribute("0", UIWidgets.ComboBox, "Day Time X", "", ParamEnumArray.FromEnum(EDayTimeEnums))]
	protected EDayTimeEnums m_DaytimeAfter;
	
	[Attribute("0", UIWidgets.ComboBox, "Day Time Y", "", ParamEnumArray.FromEnum(EDayTimeEnums))]
	protected EDayTimeEnums m_DaytimeBefore;
	
	override void SetDayTime(notnull array<ref SCR_DayTimeInfoBase> daytimeInfo, TimeAndWeatherManagerEntity timeAndWeatherEntity, int year = -1, int month = -1, int day = -1)
	{
		if (m_DayTimeEnum == m_DaytimeAfter || m_DayTimeEnum == m_DaytimeBefore)
		{	
			Print(string.Format("SCR_DayTimeInfoBetween %1 has a reference to itself!", typename.EnumToString(EDayTimeEnums, m_DayTimeEnum)), LogLevel.WARNING);	
			return;
		}
		
		if (m_fTimeOfDay > -1)
			return;
		
		float dayTimeAfterSeconds, DayTimeBeforeSeconds;
		bool dayTimeAfterSet = false;
		bool dayTimeBeforeSet = false;
		
		foreach(SCR_DayTimeInfoBase daytime: daytimeInfo)
		{
			if (daytime.GetDaytimeEnum() == m_DaytimeAfter)
			{
				dayTimeAfterSeconds = daytime.GetDayTime(daytimeInfo, timeAndWeatherEntity);
				dayTimeAfterSet = true;
				
				if (dayTimeAfterSeconds == 24)
					dayTimeAfterSeconds = 0;
				
				if (dayTimeAfterSet && dayTimeBeforeSet)
					break;
			}
			else if (daytime.GetDaytimeEnum() == m_DaytimeBefore)
			{
				DayTimeBeforeSeconds = daytime.GetDayTime(daytimeInfo, timeAndWeatherEntity);
				dayTimeBeforeSet = true;
				
				if (DayTimeBeforeSeconds == 0)
					DayTimeBeforeSeconds = 24;
				
				if (dayTimeAfterSet && dayTimeBeforeSet)
					break;
			}
		}
		
		if (dayTimeAfterSeconds < 0 || !dayTimeAfterSet)
		{
			Print(string.Format("Could not set %1 as %2 daytime after was not correctly found! This might be because %2 is also depened on %1 or that it doesn't exist in the array!", typename.EnumToString(EDayTimeEnums, m_DayTimeEnum), typename.EnumToString(EDayTimeEnums, m_DaytimeAfter)), LogLevel.WARNING);
			m_fTimeOfDay = 0;
			return;
		}
			
		if (DayTimeBeforeSeconds < 0 || !dayTimeBeforeSet)
		{
			Print(string.Format("Could not set %1 as %2 daytime before was not correctly found! This might be because %2 is also depened on %1 or that it doesn't exist in the array!", typename.EnumToString(EDayTimeEnums, m_DayTimeEnum), typename.EnumToString(EDayTimeEnums, m_DaytimeBefore)), LogLevel.WARNING);  
			m_fTimeOfDay = 0;
			return;
		}
		
		m_fTimeOfDay = (dayTimeAfterSeconds + DayTimeBeforeSeconds) / 2;
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_UIInfo")]
class SCR_WindDirectionInfo
{
	[Attribute()]
	ref SCR_UIInfo m_UIInfo;
	
	[Attribute()]
	protected int m_iWindDirection;
	
	SCR_UIInfo GetUIInfo()
	{
		return m_UIInfo;
	}
	
	int GetWindDirectionValue()
	{
		return m_iWindDirection;
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_UIInfo")]
class SCR_MoonPhaseInfo
{
	[Attribute()]
	ref SCR_MoonPhaseUIInfo m_UIInfo;
	
	[Attribute(desc: "IsMoonPhase will return true if moonPhaseAmount is equal or greater then m_fPhaseEqualOrGreater and less then m_fPhaseLessThen")]
	protected float m_fPhaseEqualOrGreater;
	
	[Attribute(desc: "IsMoonPhase will return true if moonPhaseAmount is equal or greater then m_fPhaseEqualOrGreater and less then m_fPhaseLessThen")]
	protected float m_fPhaseLessThen;
	
	//Before full moon meaning first quarter and waxing Crescent/Gibbous. After full moon meaning it is third Quarter and Waning Crescent/Gibbous. And doesn't matter means full moon or new moon
	[Attribute("0", UIWidgets.ComboBox, "Before Or AfterFull Moon", "", ParamEnumArray.FromEnum(EMoonPhaseEnums) )]	
	protected EMoonPhaseEnums m_iBeforeOrAfterFullMoon;
	
	SCR_MoonPhaseUIInfo GetUIInfo()
	{
		return m_UIInfo;
	}
	
	bool IsMoonPhase(float moonPhaseAmount, EMoonPhaseEnums beforeOrAfterFullMoon)
	{
		if (m_iBeforeOrAfterFullMoon != EMoonPhaseEnums.IGNORE && beforeOrAfterFullMoon != EMoonPhaseEnums.IGNORE)
		{
			if (m_iBeforeOrAfterFullMoon != beforeOrAfterFullMoon)
				return false;
		}
		
		return moonPhaseAmount >= m_fPhaseEqualOrGreater && moonPhaseAmount < m_fPhaseLessThen;
	}
	
	
};

enum EMoonPhaseEnums
{
	IGNORE,
	BEFORE_FULL_MOON,
	AFTER_FULL_MOON
};

enum EDayTimeEnums
{
	DAYTIME_DAWN,
	DAYTIME_MORNING,
	DAYTIME_AFTERNOON,
	DAYTIME_DUSK,
	DAYTIME_EVENING,
	DAYTIME_NIGHT
};
