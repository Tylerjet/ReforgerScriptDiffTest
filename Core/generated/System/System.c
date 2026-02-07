/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup System
* @{
*/

sealed class System
{
	private void System();
	private void ~System();
	
	/**
	\brief Switches memory validation (huge slowdown! Use with care only for certain section of code!)
	\param enable \p bool Enable
	\return \p void
	@code
	???
	@endcode
	*/
	static proto void MemoryValidation(bool enable);
	//!number of allocations of physical memory
	static proto int MemoryAllocationCount();
	//!total allocation of physical memory in kilobytes
	static proto int MemoryAllocationKB();
	/**
	\brief Returns command line argument
	\param name of a command line argument
	\param val \p string value of the param or empty string if the param hasn't been found
	\return True if param is present, False if hasn't been found
	@code
	string param;
	System.GetCLIParam("world", param); // return a value when program executed with param -world something
	@endcode
	*/
	static proto bool GetCLIParam(string param, out string val);
	/**
	\brief Returns if command line argument is present
	\param name of a command line argument
	\return True if param is present, False if hasn't been found
	@code
	if (System.IsCLIParam("verbose")) // Prints "something" when program executed with param -verbose
	{
	Print("something");
	}
	@endcode
	*/
	static proto bool IsCLIParam(string param);
	//! Copy text from clipboard (works only on PC)
	static proto string ImportFromClipboard();
	//! Set text to clipboard (works only on PC)
	static proto void ExportToClipboard(string text);
	static proto string GetProfileName();
	static proto string GetMachineName();
	/**
	\brief Returns world time
	\param[out] hour \p int Hour
	\param[out] minute \p int Minute
	\param[out] second \p int Second
	\return \p void
	@code
	int hour = 0;
	int minute = 0;
	int second = 0;
	
	System.GetHourMinuteSecondUTC(hour, minute, second);
	
	Print(hour);
	Print(minute);
	Print(second);
	
	>> hour = 16
	>> minute = 38
	>> second = 7
	@endcode
	*/
	static proto void GetHourMinuteSecond(out int hour, out int minute, out int second);
	/**
	\brief Returns UTC world time
	\param[out] hour \p int Hour
	\param[out] minute \p int Minute
	\param[out] second \p int Second
	\return \p void
	@code
	int hour = 0;
	int minute = 0;
	int second = 0;
	
	System.GetHourMinuteSecondUTC(hour, minute, second);
	
	Print(hour);
	Print(minute);
	Print(second);
	
	>> hour = 15
	>> minute = 38
	>> second = 7
	@endcode
	*/
	static proto void GetHourMinuteSecondUTC(out int hour, out int minute, out int second);
	/**
	\brief Returns world date
	\param[out] year \p int Year
	\param[out] month \p int Month
	\param[out] day \p int Day
	\return \p void
	@code
	int year = 0;
	int month = 0;
	int day = 0;
	
	System.GetYearMonthDay(year, month, day);
	
	Print(year);
	Print(month);
	Print(day);
	
	>> year = 2015
	>> month = 3
	>> day = 24
	@endcode
	*/
	static proto void GetYearMonthDay(out int year, out int month, out int day);
	/**
	\brief Returns UTC world date
	\param[out] year \p int Year
	\param[out] month \p int Month
	\param[out] day \p int Day
	\return \p void
	@code
	int year = 0;
	int month = 0;
	int day = 0;
	
	System.GetYearMonthDayUTC(year, month, day);
	
	Print(year);
	Print(month);
	Print(day);
	
	>> year = 2015
	>> month = 3
	>> day = 24
	@endcode
	*/
	static proto void GetYearMonthDayUTC(out int year, out int month, out int day);
	static proto ProfileData GetProfileData();
	/**
	\brief Returns number of milliseconds that have elapsed since the game was started.
	*/
	static proto int GetTickCount(int prev = 0);
	static proto string GetAdapterName();
	static proto void GetNativeResolution(out int width, out int height);
	static proto void GetRenderingResolution(out int width, out int height);
	static proto void GetSupportedResolutions(out notnull array<int> widths, out notnull array<int> heights);
	/*!
	Takes screenshot and stores it in a BMP format at specified path.
	\param path Path where screenshot should be written. If you use absolute path or $NamedFileSystem:
	path, it will be used directly without any changes. If you use relative path, it will be
	relative to $profile: file system. Finally, if path is empty string, screenshot will be
	saved to "$profile:ScreenShots/DATETIME.bmp" where DATETIME will be replaced by current
	date and time.
	*/
	static proto void MakeScreenshot(string path);
	/*!
	Returns actual fps (average in last 10 frames)
	*/
	static proto float GetFPS();
	/*!
	Returns average time a frame took in seconds (average over last 10 frames)
	*/
	static proto float GetFrameTimeS();
	/*!
	\brief Find files with extension in given path on all FileSystems accessible for game
	@code
	array<string> files = {};
	System.FindFiles(files.Insert, "configs/", ".conf");
	files.Debug();
	@endcode
	*/
	static proto bool FindFiles(FindFilesCallback callback, string path, string ext);
	/*!
	Return current platform
	*/
	static proto EPlatform GetPlatform();
	/*!
	Checks if the app runs in console mode (no rendering, audio or input)
	@return True if the app runs in console mode. False otherwise.
	*/
	static proto bool IsConsoleApp();
};

/** @}*/
