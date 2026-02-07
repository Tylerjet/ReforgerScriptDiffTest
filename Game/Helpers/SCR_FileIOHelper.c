class SCR_FileIOHelper
{
	protected static const string DELIMITER = "/";

	//------------------------------------------------------------------------------------------------
	//! Create sub-directories in the proper order, circumventing a FileIO.MakeDirectory limitation
	//! \param absoluteDirectory e.g C:/Arma4/Data/scripts/My/Sub/Directory
	//! \return true if the whole directory structure was created or already exists, false otherwise
	static bool CreateDirectory(string absoluteDirectory)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(absoluteDirectory))
			return false;

		if (FileIO.FileExists(absoluteDirectory))
			return true;

		absoluteDirectory.Replace("\\", DELIMITER);
		absoluteDirectory = SCR_StringHelper.ReplaceRecursive(absoluteDirectory, DELIMITER + DELIMITER, DELIMITER);

		if (FileIO.FileExists(absoluteDirectory))
			return true;

		array<string> pieces = {};
		absoluteDirectory.Split(DELIMITER, pieces, true);

		if (pieces.Count() < 2)
		{
			Print("Cannot create directory " + absoluteDirectory + "; not enough directories", LogLevel.DEBUG);
			return false;
		}

		string path = pieces[0]; // C: part on Windows
		for (int i = 1, count = pieces.Count(); i < count; i++)
		{
			path += DELIMITER + pieces[i];

			if (FileIO.FileExists(path))
				continue;

			if (!FileIO.MakeDirectory(path))
			{
				Print("Cannot create directory " + absoluteDirectory + "; blocked at " + path, LogLevel.DEBUG);
				return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Copy source file to destination
	//! \param source file to copy
	//! \param destination copy destination
	//! \param overwrite set to false to prevent an accidental overwrite
	//! \return
	static bool Copy(string source, string destination, bool overwrite = true)
	{
		array<string> content = ReadFileContent(source);
		if (!content)
		{
			Print("Cannot get file content - " + source, LogLevel.ERROR);
			return false;
		}

#ifdef WORKBENCH
		string absolutePath;
		if (!FilePath.IsAbsolutePath(destination))
		{
			if (!Workbench.GetAbsolutePath(destination, absolutePath, false))
			{
				Print("Cannot get destination's absolute file path - " + destination, LogLevel.WARNING);
				return false;
			}
		}
#endif // WORKBENCH

		if (!overwrite && FileIO.FileExists(destination))
		{
			Print("Not allowed to overwrite file - " + destination, LogLevel.WARNING);
			return false;
		}

		if (!WriteFileContent(destination, content))
		{
			Print("Cannot write file - " + destination, LogLevel.ERROR);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get file content as array of lines
	//! \param filePath relative or absolute
	//! \return array of lines or null if file does not exist or cannot be opened
	static array<string> ReadFileContent(string filePath)
	{
		FileHandle fileHandle = FileIO.OpenFile(filePath, FileMode.READ);
		if (!fileHandle)
		{
			Print("Could not open " + filePath, LogLevel.WARNING);
			return null;
		}

		string lineContent;
		array<string> result = {};
		while (!fileHandle.IsEOF())
		{
			fileHandle.ReadLine(lineContent);
			result.Insert(lineContent);
		}

		fileHandle.Close();

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Write all lines in the file, replacing all its content
	//! Overwrites the file if it exists
	//! \param filePath relative or absolute
	//! \param lines
	//! \return true on success, false otherwise
	static bool WriteFileContent(string filePath, notnull array<string> lines)
	{
		FileHandle fileHandle = FileIO.OpenFile(filePath, FileMode.WRITE);
		if (!fileHandle)
		{
			Print("Could not open " + filePath, LogLevel.WARNING);
			return false;
		}

		int linesCountMinusOne = lines.Count() - 1;
		if (linesCountMinusOne == -1)
		{
			fileHandle.Write(string.Empty); // needed?
		}
		else
		{
			foreach (int lineNumber, string line : lines)
			{
				if (lineNumber < linesCountMinusOne)
					fileHandle.WriteLine(line);
			}

			// avoid final line return due to WriteLine
			fileHandle.Write(lines[linesCountMinusOne]);
		}

		fileHandle.Close();

		return true;
	}
}
