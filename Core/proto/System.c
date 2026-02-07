/*!
\defgroup System System
Enforce script system methods
\{
*/

typedef func FindFilesCallback;
void FindFilesCallback(string fileName, FileAttribute attributes = 0, string filesystem = string.Empty);

class MemoryStatsSnapshot: Managed
{
	proto native static int GetStatsCount();
	proto native static string GetStatName(int idx);
	proto native int GetStatValue(int idx);
}

//! Handle to a running process.
class ProcessHandle: pointer
{
}

/*!
\}
*/
