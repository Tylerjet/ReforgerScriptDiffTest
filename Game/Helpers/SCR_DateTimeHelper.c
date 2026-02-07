class SCR_DateTimeHelper
{
	//------------------------------------------------------------------------------------------------
	//! \return local datetime in format "yyyy-mm-dd hh:ii:ss"
	static string GetDateTimeLocal()
	{
		int year, month, day, hour, minute, second;
		System.GetYearMonthDay(year, month, day);
		System.GetHourMinuteSecond(hour, minute, second);
		return SCR_FormatHelper.FormatDateTime(year, month, day, hour, minute, second);
	}

	//------------------------------------------------------------------------------------------------
	//! \return UTC datetime in format "yyyy-mm-dd hh:ii:ss"
	static string GetDateTimeUTC()
	{
		int year, month, day, hour, minute, second;
		System.GetYearMonthDayUTC(year, month, day);
		System.GetHourMinuteSecondUTC(hour, minute, second);
		return SCR_FormatHelper.FormatDateTime(year, month, day, hour, minute, second);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Returns translated date expression, either in short or verbose style
	\param day number
	\param month number (from 1 to 12)
	\param year number
	\param verbose bool
	\return translated date expression
	*/
	static string GetDateString(int day, int month, int year, bool verbose = true)
	{
		string date;
		
		if (verbose)
			date = WidgetManager.Translate("#AR-Date_Format_MonthFull", day, SCR_DateTimeHelper.GetMonthString(month), year);
		else 
			date = WidgetManager.Translate("#AR-Date_Format", day, month, year);
		
		return date;
	}	
	
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns translation key for the month.
	\param month number (from 1 to 12)
	\param standalone bool - is the month name going to be displayed as standalone string or part of the date (default: false)
	\return month's translation key
	*/
	static string GetMonthString(int month, bool standalone = false)
	{
		if (month < 1 || month > 12)
			return string.Empty;

		string months[12];

		if (standalone)
		{
			months = {
				"#AR-Date_January_Standalone",
				"#AR-Date_February_Standalone",
				"#AR-Date_March_Standalone",
				"#AR-Date_April_Standalone",
				"#AR-Date_May_Standalone",
				"#AR-Date_June_Standalone",
				"#AR-Date_July_Standalone",
				"#AR-Date_August_Standalone",
				"#AR-Date_September_Standalone",
				"#AR-Date_October_Standalone",
				"#AR-Date_November_Standalone",
				"#AR-Date_December_Standalone",
			};		
		}
		else
		{
			months = {
				"#AR-Date_January",
				"#AR-Date_February",
				"#AR-Date_March",
				"#AR-Date_April",
				"#AR-Date_May",
				"#AR-Date_June",
				"#AR-Date_July",
				"#AR-Date_August",
				"#AR-Date_September",
				"#AR-Date_October",
				"#AR-Date_November",
				"#AR-Date_December",
			};		
		}
		
		return months[month -1];
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Returns translation key for the abbrev. month.
	\param month number (from 1 to 12)
	\return month's short translation key
	*/
	static string GetAbbreviatedMonthString(int month)
	{
		if (month < 1 || month > 12)
			return string.Empty;

		// if it gets called often, cache it as a static array
		array<string> months = {
			"#AR-Date_January_StandaloneShort",
			"#AR-Date_February_StandaloneShort",
			"#AR-Date_March_StandaloneShort",
			"#AR-Date_April_StandaloneShort",
			"#AR-Date_May_StandaloneShort",
			"#AR-Date_June_StandaloneShort",
			"#AR-Date_July_StandaloneShort",
			"#AR-Date_August_StandaloneShort",
			"#AR-Date_September_StandaloneShort",
			"#AR-Date_October_StandaloneShort",
			"#AR-Date_November_StandaloneShort",
			"#AR-Date_December_StandaloneShort",
		};

		return months[month -1];
	}

	//------------------------------------------------------------------------------------------------
	//! \return the absolute difference in seconds
	static int GetTimeDifference(int hour0, int minute0, int second0, int hour1, int minute1, int second1, out int hour = 0, out int minute = 0, out int second = 0)
	{
		int time0 = GetSecondsFromHourMinuteSecond(hour0, minute0, second0);
		int time1 = GetSecondsFromHourMinuteSecond(hour1, minute1, second1);

		int result = time0 - time1;
		if (result < 0)
			result *= -1;

		GetHourMinuteSecondFromSeconds(result, hour, minute, second);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return total absolute difference in hh:ii:ss format
	static string GetTimeDifferenceFormatted(int hour0, int minute0, int second0, int hour1, int minute1, int second1)
	{
		int hour, minute, second;
		GetTimeDifference(hour0, minute0, second0, hour1, minute1, second1, hour, minute, second);
		return SCR_FormatHelper.FormatTime(hour, minute, second);
	}

	//------------------------------------------------------------------------------------------------
	//! \return local time in format "hh:ii:ss"
	static string GetTimeLocal()
	{
		int hour, minute, second;
		System.GetHourMinuteSecond(hour, minute, second);
		return SCR_FormatHelper.FormatTime(hour, minute, second);
	}

	//------------------------------------------------------------------------------------------------
	//! \return UTC time in format "hh:ii:ss"
	static string GetTimeUTC()
	{
		int hour, minute, second;
		System.GetHourMinuteSecondUTC(hour, minute, second);
		return SCR_FormatHelper.FormatTime(hour, minute, second);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Splits given seconds to days, hours, minutes and remaining seconds
	\param totalSeconds total seconds to be converted - absolute value will be used
	\param[out] outDays Returns amount of Days
	\param[out] outHours Returns amount of Hours 0..23
	\param[out] outMinutes Returns amount of Minutes 0..59
	\param[out] outSeconds Returns remaining amount of Seconds 0..59
	*/
	static void GetDayHourMinuteSecondFromSeconds(int totalSeconds, out int outDays, out int outHours, out int outMinutes, out int outSeconds)
	{
		if (totalSeconds < 0)
			totalSeconds *= -1;

		outDays = totalSeconds / 86400;
		outHours = (totalSeconds % 86400) / 3600;
		outMinutes = (totalSeconds % 3600) / 60;
		outSeconds = totalSeconds % 60;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Splits given seconds to hours, minutes and remaining seconds
	\param totalSeconds total seconds to be converted - absolute value will be used
	\param[out] outHours Returns amount of Hours 0..x (can be above 24)
	\param[out] outMinutes Returns amount of Minutes 0..59
	\param[out] outSeconds Returns remaining amount of Seconds 0..59
	*/
	static void GetHourMinuteSecondFromSeconds(int totalSeconds, out int outHours, out int outMinutes, out int outSeconds)
	{
		if (totalSeconds < 0)
			totalSeconds *= -1;

		outHours = totalSeconds / 3600;
		outMinutes = (totalSeconds % 3600) / 60;
		outSeconds = totalSeconds % 60;
	}

	//------------------------------------------------------------------------------------------------
	//! \param hour
	//! \param minute
	//! \param second
	//! \return seconds
	static int GetSecondsFromHourMinuteSecond(int hour = 0, int minute = 0, int second = 0)
	{
		return hour * 3600 + minute * 60 + second;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Grabs all given values and convert it to minutes.
	\param[out] year Years to convert into minutes. Note that max years is around 4000
	\param[out] month Months to convert into minutes.
	\param[out] day Days to convert into minutes.
	\param[out] hour Hours to convert into minutes.
	\param[out] minutes To add to the total minutes
	*/
	static int ConvertDateIntoMinutes(int year = 0, int month = 0, int day = 0, int hour = 0, int minutes = 0)
	{
		return (year * 525600) + (month * 43800) + (day * 1440) + (hour * 60) + minutes;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Grabs the given total minutes and converts it into years, months, days, hours and minutes
	\param totalDateMinutes total minutes to be converted - absolute value will be used
	\param[out] year Returns amount of years 0..x
	\param[out] month Returns amount of months 0..12
	\param[out] day Returns amount of days 0..31
	\param[out] hour Returns amount of hours 0..23
	\param[out] minutes Returns amount of minutes 0..59
	*/
	static void ConvertMinutesIntoDate(int totalDateMinutes, out int year, out int month, out int day, out int hour, out int minutes)
	{
		year = totalDateMinutes / 525600;
		totalDateMinutes -= year * 525600;

		month = totalDateMinutes / 43800;
		totalDateMinutes -= month * 43800;

		day = totalDateMinutes / 1440;
		totalDateMinutes -= day * 1440;

		hour = totalDateMinutes / 60;
		totalDateMinutes -= hour * 60;

		minutes = totalDateMinutes;
	}
};
