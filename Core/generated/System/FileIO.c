/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup System
* @{
*/

sealed class FileIO
{
	private void FileIO();
	private void ~FileIO();
	
	/**
	\brief Opens File
	@param name of a file to open, (you can use filesystem prefixes. For accessing profile dir use '$profile', e.g. '$profile:myFileName.txt')
	@param mode constants FileMode.WRITE, FileMode.READ or FileMode.APPEND flag can be used
	\return file handle ID or 0 if fails
	@endcode
	*/
	static proto FileHandle OpenFile(string name, FileMode mode);
	/**
	\brief Creates a parser for a given `filename`
	It can be then used with `ParseLine` and EndParse.
	*/
	static proto ParseHandle BeginParse(string filename);
	//!Check existence of file
	static proto bool FileExist(string name);
	//!Makes a directory
	static proto bool MakeDirectory(string name);
	//!delete file. Works only on "$profile:" and "$saves:" locations
	static proto bool DeleteFile(string name);
	//! copy file. destName must be "$profile:" or "$saves:" location
	static proto bool CopyFile(string sourceName, string destName);
};

/** @}*/
