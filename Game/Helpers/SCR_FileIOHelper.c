class SCR_FileIOHelper
{
	protected static const string DELIMITER = "/";

	//------------------------------------------------------------------------------------------------
	//! Create sub-directories in the proper order, circumventing a FileIO.MakeDirectory limitation
	//! \param absoluteDirectory e.g C:/Arma4/Data/scripts/My/Sub/Directory
	//! \return true if the whole directory structure was created, false otherwise
	static bool CreateDirectory(string absoluteDirectory)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(absoluteDirectory))
			return false;

		absoluteDirectory.Replace("\\", DELIMITER);
		while (absoluteDirectory.IndexOf(DELIMITER + DELIMITER) > -1)
		{
			absoluteDirectory.Replace(DELIMITER + DELIMITER, DELIMITER);
		}

		if (FileIO.FileExists(absoluteDirectory))
			return true;

		array<string> pieces = {};
		absoluteDirectory.Split(DELIMITER, pieces, true);

		if (pieces.Count() < 2)
			return false;

		string path = pieces[0]; // C: part on Windows
		for (int i = 1, count = pieces.Count(); i < count; i++)
		{
			if (!path.IsEmpty())
				path += DELIMITER;

			path += pieces[i];

			if (FileIO.FileExists(path))
				continue;

			if (!FileIO.MakeDirectory(path))
			{
				Print("Could NOT create directory " + absoluteDirectory + "; blocked at " + path, LogLevel.DEBUG);
				return false;
			}
		}

		return true;
	}
}
