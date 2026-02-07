/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup System
* @{
*/

/**
@code
FileHandle file = FileIO.OpenFile("$profile:testiik.txt", FileMode.WRITE);
if (file != 0)
{
file.FPrint("A");
file.FPrint(file, "B");
file.FPrint(file, "C");
file.CloseFile();
}
@endcode
*/
sealed class FileHandle: pointer
{
	/**
	\brief Reads raw data from file.
	\param param_array Receiving array for the data
	\param length Length of data
	\returns number of read bytes
	*/
	proto external int ReadFile(void param_array, int length);
	/**
	\brief Close the File
	*/
	proto external void CloseFile();
	/**
	\brief Get line from file, every next call of this function returns next line
	@param file File handle ID of a opened file
	@param var Value to write
	\return int Count of chars or -1 if is not any for read (end of file is EMPTY line)
	*/
	proto external int FGets(string var);
	/**
	\brief Write to file
	@param var Value to write
	@endcode
	*/
	proto external void FPrint(void var);
	/**
	\brief Write to file and add new line
	@param var Value to write
	*/
	proto external void FPrintln(void var);
};

/** @}*/
