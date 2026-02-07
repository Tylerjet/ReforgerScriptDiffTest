class SCR_PlatformHelper
{
	//------------------------------------------------------------------------------------------------
	static bool IsWindows()
	{
		if (System.GetPlatform() == EPlatform.WINDOWS)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsLinux()
	{
		if (System.GetPlatform() == EPlatform.LINUX)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsXbox()
	{
		EPlatform platform = System.GetPlatform();

		if (platform == EPlatform.XBOX_ONE || platform == EPlatform.XBOX_ONE_X || platform == EPlatform.XBOX_ONE_S || platform == EPlatform.XBOX_SERIES_X || platform == EPlatform.XBOX_SERIES_S)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsPlaystation()
	{
		EPlatform platform = System.GetPlatform();

		if (platform == EPlatform.PS4 || platform == EPlatform.PS5 || platform == EPlatform.PS5_PRO)
			return true;

		return false;
	}
}
