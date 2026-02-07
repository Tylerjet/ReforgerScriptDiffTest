#ifdef WORKBENCH

typedef func WorkbenchSearchResourcesCallback;
void WorkbenchSearchResourcesCallback(ResourceName resName, string filePath = "");

typedef int DateTimeUtcAsInt;
class DateTimeUtcAsInt
{
	int GetSecond()
	{
		return value & 0x3f;
	}

	int GetMinute()
	{
		return (value >> 6) & 0x3f;
	}

	int GetHour()
	{
		return (value >> 12) & 0x1f;
	}

	int GetDay()
	{
		return (value >> 17) & 0x1f;
	}

	int GetMonth()
	{
		return (value >> 22) & 0xf;
	}

	int GetYear()
	{
		return ((value >> 26) & 0x3f) + 2000;
	}
}

class Workbench
{
	private void Workbench();
	private void ~Workbench();
	static proto native WBModuleDef GetModule(typename type);
	static proto native bool OpenModule(typename type);
	static proto native bool CloseModule(typename type);
	static proto native void Dialog(string caption, string text, string detailedText = "");
	static proto int ScriptDialog(string caption, string text, Class data);
	//! Search for all resources by filer and call callback method for each. `rootPath` must be in "exact path" format e.g. `"$addonName:Prefabs"`.
	static proto bool SearchResources(WorkbenchSearchResourcesCallback callback, array<string> fileExtensions = null, array<string> searchStrArray = null, string rootPath = "", bool recursive = true);
	static proto native int RunCmd(string command, bool wait = false);
	static proto native ProcessHandle RunProcess(string command);
	static proto native bool KillProcess(ProcessHandle handle);
	static proto void GetCwd(out string currentDir);
	static proto void GetUserName(out string userName);
	static proto bool GetAbsolutePath(string relativePath, out string absPath, bool mustExist = true);
	//! Returns absolute path to game project settings.
	static proto native string GetCurrentGameProjectFile();
	//! Returns game project settings.
	static proto native BaseContainer GetGameProjectSettings();
	static proto string GenerateGloballyUniqueID64();
	static proto native void Exit(int exitCode);
	static proto native DateTimeUtcAsInt GetPackedUtcTime();
	static proto ResourceName GetResourceName(string path);
	static proto native bool OpenResource(string filename);
}

class WBModuleDef: pointer
{
	private void WBModuleDef();
	private void ~WBModuleDef();
	proto native external bool SetOpenedResource(string filename);
	proto native external int GetNumContainers();
	proto native external BaseContainer GetContainer(int index = 0);
	proto external bool GetCmdLine(string name, out string value);
	proto native external bool Save();
	proto native external bool Close();
}

/*!
\addtogroup WorkbenchEditors Workbench Editors
\{
*/

class ParticleEditor: WBModuleDef
{
}

class AnimEditor: WBModuleDef
{
}

class AudioEditor: WBModuleDef
{
}

class BehaviorEditor: WBModuleDef
{
}

class NavmeshGeneratorMain: WBModuleDef
{
}

class ProcAnimEditor: WBModuleDef
{
}

class ScriptEditor: WBModuleDef
{
	proto external bool GetCurrentFile(out string filename);
	proto native external int GetCurrentLine();
	proto native external int GetLinesCount();
	//! Gets line text (if line is -1, current line is used).
	proto external bool GetLineText(out string text, int line = -1);
	//! Sets line text (if line is -1, current line is used).
	proto native external void SetLineText(string text, int line = -1);
	//! Insert line before line (if line is -1, current line is used).
	proto native external void InsertLine(string text, int line = -1);
	//! Removes line (if line is -1, current line is used).
	proto native external void RemoveLine(int line = -1);
}

class ResourceManager: WBModuleDef
{
	proto native external MetaFile GetMetaFile(string absFilePath);

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
	proto native external MetaFile RegisterResourceFile(string absFilePath);

	/*!
	Rebuild already registered resource.
	Usage:
	\code
		rm.RebuildResourceFiles({"UI\\Imagesets\\Test\\Test.tga"}, "PC");
	\endcode
	*/
	proto native external void RebuildResourceFile(string filePath, string configuration, bool selectFiles);

	/*!
	Rebuild already registered resources.
	Usage:
	\code
		rm.RebuildResourceFiles({"UI\\Imagesets\\Test\\Test.tga"}, "PC");
	\endcode
	*/
	proto native external void RebuildResourceFiles(notnull array<string> filePaths, string configuration);
	proto native external void WaitForFile(string filePath, int maxTimeMs = 1000);

	//! Return exact paths of selected items from ResourceBrowser panel in ResourceManager.
	proto void GetResourceBrowserSelection(WorkbenchSearchResourcesCallback callback, bool recursive = false);
}

class WorldEditor: WBModuleDef
{
	proto native external WorldEditorAPI GetApi();
	proto external bool GetTerrainBounds(out vector min, out vector max);
	proto native void SwitchToGameMode(bool debugMode = false, bool fullScreen = false);
	proto native void SwitchToEditMode();

	//! Return `true` if editor is in prefab edit mode (dedicated mode for editing prefabs).
	proto native bool IsPrefabEditMode();

	//! Return exact paths of selected items from ResourceBrowser panel in WorldEditor.
	proto void GetResourceBrowserSelection(WorkbenchSearchResourcesCallback callback, bool recursive = false);

	bool WaitForGameMode(int timeout = 120000 /*msec*/)
	{
		while (value.GetApi() && !value.GetApi().IsGameMode())
		{
			Sleep(50);
			timeout -= 50;
			if (timeout < 0)
				return false;
		}

		if (value.GetApi() == null)
			return false;
		return true;
	}
}

class LocalizationEditor: WBModuleDef
{
	//! Begins group of undo actions (for user will whole group behave like one action).
	proto native external void BeginModify(string text);
	//! Modifies single StringTableItem property.
	proto native external void ModifyProperty(BaseContainer container, int variable, string value);
	//! Ends group of undo actions.
	proto native external void EndModify();
	//! Refreshes UI.
	proto native external void RefreshUI();
	//! Returns string table container.
	proto native external BaseContainer GetTable();
	//! Returns indexes of rows which are filtered at the moment.
	proto native external void GetFilteredRows(notnull out array<int> rowsIdx);
	//! Returns indexes of rows which are selected at the moment.
	proto native external void GetSelectedRows(notnull out array<int> rowsIdx);
	//! Filters just rows given in `rowsIdx` array.
	proto native external void AddUserFilter(notnull array<int> rowsIdx, string caption);
}

/*!
\}
*/

enum WETMouseButtonFlag
{
	LEFT = 1,
	RIGHT = 2,
	MIDDLE = 4
}

enum ModifierKey
{
	SHIFT = 0x02000000,
	CONTROL = 0x04000000,
	ALT = 0x08000000
}

class WorldEditorTool
{
	//! Filled by workbench
	WorldEditorAPI m_API;

	void OnKeyPressEvent(KeyCode key, bool isAutoRepeat) {}
	void OnKeyReleaseEvent(KeyCode key, bool isAutoRepeat) {}
	void OnEnterEvent() {}
	void OnLeaveEvent() {}
	void OnMouseMoveEvent(float x, float y) {}
	void OnMouseDoubleClickEvent(float x, float y, WETMouseButtonFlag buttons) {}
	void OnMousePressEvent(float x, float y, WETMouseButtonFlag buttons) {}
	void OnMouseReleaseEvent(float x, float y, WETMouseButtonFlag buttons) {}
	void OnWheelEvent(int delta) {}
	void OnActivate() {}
	void OnDeActivate() {}
	void OnAfterLoadWorld() {}
	void OnBeforeUnloadWorld() {}

	static proto native bool GetModifierKeyState(ModifierKey modifierKey);
	static proto native void UpdatePropertyPanel();

	private void WorldEditorTool() {}
	private void ~WorldEditorTool() {}
}

class GeneratedResources
{
	proto bool RegisterResource(string absPath, out ResourceName resourceName);

	private void GeneratedResources();
	private void ~GeneratedResources();
}

class WBProgressDialog: Managed
{
	void WBProgressDialog(string title, WBModuleDef parentWindow);
	proto native external void SetProgress(float progress);
}

class WorkbenchPlugin: Managed
{
	void Run() {}
	void RunCommandline() {}
	void Configure() {}

	private void WorkbenchPlugin();
	private void ~WorkbenchPlugin();
}

class ResourceManagerPlugin: WorkbenchPlugin
{
	void OnRegisterResource(string absFileName, BaseContainer metaFile);
	void OnBuildResource(string absFileName, BaseContainer metaFile, GeneratedResources generatedResources);
	void OnRenameResource(string absFileNameOld, string absFileNameNew, BaseContainer metaFile);

	//! Returns a directory where new default materials may be generated for given mesh object model (`absModelPath`).
	string OnGetMaterialGenerateDir(string absModelPath);

	//! Returns suggested MaterialClass name for given material path.
	string OnGetMaterialClassName(string absMaterialPath, GeneratedResources generatedResources);
	void OnMaterialCreated(string absMaterialPath, BaseContainer materialSrc, GeneratedResources generatedResources);
}

class WorldEditorPlugin: WorkbenchPlugin
{
	void OnGameModeStarted(string worldName, string gameMode, bool playFromCameraPos, vector cameraPosition, vector cameraAngles);
	void OnGameModeEnded();
	
	/*!
	Called when user dropped some kind of data into a world edit window. Plugins can completely reimplement default editor funtionality using WorldEditorAPI
	\param windowType Type of a window where data were dropped. Values represent Perpective, Top, Right, Back views
	\param posX Horizontal window position
	\param posY Vertical window position
	\param dataType Type of dropped data. At the moment we support "WorldEditor/EntityType" and "Workbench/ResourceFiles" values
	\param data Depends on dataType. If dataType is "WorldEditor/EntityType" then data[0] contains a className. If dataType is "Workbench/ResourceFiles" then it contains one or more registered resource names
	*/
	bool OnWorldEditWindowDataDropped(int windowType, int posX, int posY, string dataType, array<string> data);
}

class LocalizationEditorPlugin: WorkbenchPlugin
{
	void OnSave(BaseContainer stringTable, string stringTableItemClassName, string stringTableAbsPath);
	void OnChange(BaseContainer stringTableItem, string propName, string propValue);
	void OnImport(BaseContainer newItem, BaseContainer oldItem);
	void OnExport(BaseContainer item);
	void OnSelectionChanged();
	bool IsReadOnly(BaseContainer item, bool isImporting);
	//! Called for each item during building runtime table, expected column name to export for given language.
	string GetExportColumn(BaseContainer item, string languageCode);
}

class ButtonAttribute
{
	string m_Label;
	bool m_Focused;

	void ButtonAttribute(string label = "ScriptButton", bool focused = false)
	{
		m_Label = label;
		m_Focused = focused;
	}
}

/*!
Attribute for Workbench plugin definition:
- `name` - ui name in Script Tools menu
- `description` - tooltip
- `shortcut` - shortcut in simple text form e.g. "ctrl+g"
- `icon` - relative path to icon file (32x32 png)
- `wbModules` - list of strings representing Workbench modules where this tool should be avalaible (e.g. {"ResourceManager", "ScriptEditor"}). Leave null or empty array for any module.
*/
class WorkbenchPluginAttribute
{
	string m_Name;
	string m_Icon;
	string m_Shortcut;
	string m_Description;
	string m_Category;
	int m_AwesomeFontCode; //! https://fontawesome.com/cheatsheet/

	ref array<string> m_WBModules;

	void WorkbenchPluginAttribute(string name, string description = "", string shortcut = "", string icon = "", array<string> wbModules = null, string category = "", int awesomeFontCode = 0)
	{
		m_Name = name;
		m_Icon = icon;
		m_Shortcut = shortcut;
		m_Description = description;
		m_WBModules = wbModules;
		m_Category = category;
		m_AwesomeFontCode = awesomeFontCode;
	}
}

//! Attribute for Workbench tool definition.
class WorkbenchToolAttribute: WorkbenchPluginAttribute
{
}

class MetaFile: BaseContainer
{
	proto external ResourceName GetResourceID();
	proto external void Save();
}

class TexTools
{
	private void TexTools();
	private void ~TexTools();

	/*!
	Save raw pixels (ARGB stored in int) to dds file.
	Usage:
	\code
		string filePath = "c:\\textures\\test.dds";
		int data[256];

		// generate same gradient
		for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
		{
			int clr = y * 16 + 15;
			data[x * 16 + y] = ARGB(255, clr, clr, clr);
		}

		// save dds to file
		if (TexTools.SaveImageData(filePath, 16, 16, data) == false)
		{
			Print("Can't save image", LogLevel.ERROR);
			return;
		}
	\endcode
	*/
	static proto native bool SaveImageData(string filePath, int width, int height, notnull array<int> data);

	/*!
	Repair borders. Real data pictures are from `3,3` to `sizeX-3,sizeY-3`.
	Rest of the image must be copied due to DXT compression from the border
	lines -> in 4x4 DXT block must be just four colors, the other reason is
	the mip-mapping!
	*/
	static proto native void RepairTerrainTextureBorders(int width, int height, notnull inout array<int> data);
}

#endif