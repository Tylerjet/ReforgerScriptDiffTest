/*
===========================================
Do not modify, this script is generated
===========================================
*/

#ifdef WORKBENCH

/*!
\addtogroup WorkbenchAPI_Modules
\{
*/

sealed class ResourceManager: WBModuleDef
{
	proto external MetaFile GetMetaFile(string absFilePath);
	/*!
	Register register resource (create meta file).
	Usage:
	\code
		// register resource
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		MetaFile meta = rm.RegisterResourceFile("c:\\DATA\\UI\\Imagesets\\Test\\Test.tga");
		meta.Save();

		// build resource
		rm.RebuildResourceFiles({"UI\\Imagesets\\Test\\Test.tga"}, "PC");
	\endcode
	*/
	proto external MetaFile RegisterResourceFile(string absFilePath);
	/*!
	Rebuild already registered resource.
	Usage:
	\code
		rm.RebuildResourceFiles({"UI\\Imagesets\\Test\\Test.tga"}, "PC");
	\endcode
	*/
	proto external void RebuildResourceFile(string filePath, string configuration, bool selectFiles);
	/*!
	Rebuild already registered resources.
	Usage:
	\code
		rm.RebuildResourceFiles({"UI\\Imagesets\\Test\\Test.tga"}, "PC");
	\endcode
	*/
	proto external void RebuildResourceFiles(notnull array<string> filePaths, string configuration);
	proto external void WaitForFile(string filePath, int maxTimeMs = 1000);
	//! Return exact paths of selected items from ResourceBrowser panel in ResourceManager.
	proto external void GetResourceBrowserSelection(WorkbenchSearchResourcesCallback callback, bool recursive = false);
}

/*!
\}
*/

#endif // WORKBENCH
