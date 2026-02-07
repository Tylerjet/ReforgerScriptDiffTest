/*!
Manager of external session save files.
*/
[BaseContainerProps(configRoot: true)]
class SCR_SaveManagerCore: SCR_GameCoreBase
{
	protected const string GAME_SESSION_STORAGE_NAME = "SCR_SaveFileManager_FileNameToLoad";
	
	[Attribute()]
	protected ref array<ref SCR_DSSessionCallback> m_aCallbacks;
	
	protected string m_sMissionSaveFileName;
#ifdef WORKBENCH
	protected ref SCR_MissionHeader m_WorkbenchMissionHeader;
#endif
	
	protected BaseContainer m_LatestSaveContainer;
	protected ref SCR_LatestSaveSettings m_LatestSaveSettings;
	
	protected bool m_bDebugDelete;
	protected ref SCR_ServerSaveSessionCallback m_UploadCallback;
	protected ref SCR_SaveManager_BackendCallback m_DownloadCallback;
	protected ref SCR_SaveManager_PageParams m_DownloadPageParams;
	
	//////////////////////////////////////////////////////////////////////////////////////////
	///@name Actions
	///@{
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/*!
	Create a save of given type.
	\param type Save type
	\param customName Custom addition to file name (optional; applicable only to some save types)
	\return True if save request was initiated
	*/
	bool Save(ESaveType type, string customName = string.Empty)
	{
		SCR_DSSessionCallback callback = FindCallback(type);
		if (!callback)
		{
			Print(string.Format("SCR_SaveManagerCore: Cannot save, no rules found for save type %1! Check configuration of SCR_SaveLoadComponent on game mode.", typename.EnumToString(ESaveType, type)), LogLevel.WARNING);
			return false;
		}
		
		return callback.SaveSession(m_sMissionSaveFileName, customName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Load the given save file.
	This will "insert" it straight to running session, which can lead to issues (especially when loading the save file multiple times).
	Consider restarting the world first.
	\param Save file name
	\return True if load request was initiated
	*/
	bool Load(string fileName)
	{
		SCR_DSSessionCallback callback = FindCallback(fileName);
		if (!callback)
		{
			Print(string.Format("SCR_SaveManagerCore: Cannot load save file '%1', no rules found for it!", fileName), LogLevel.WARNING);
			return false;
		}
		
		return callback.LoadSession(fileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Restart the current world and load the latest save.
	\return True if load request was initiated
	*/
	bool RestartAndLoad()
	{
		string latestSaveFileName;
		return FindLatestSave(m_sMissionSaveFileName, latestSaveFileName) && RestartAndLoad(latestSaveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Restart the current world and load select save file.
	\param Save file name
	\return True if load request was initiated
	*/
	bool RestartAndLoad(string fileName)
	{
		if (!SetFileNameToLoad(fileName))
			return false;
		
		GameStateTransitions.RequestServerReload();
		return true;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check if file of given type exists.
	\param type Save type
	\param customName Custom addition to file name (optional; applicable only to some save types)
	\return True if the file exists
	*/
	bool FileExists(ESaveType type, string customName = string.Empty)
	{
		SCR_DSSessionCallback callback = FindCallback(type);
		if (!callback)
		{
			Print(string.Format("SCR_SaveManagerCore: Cannot check if file exists, no rules found for save type %1! Check configuration of SCR_SaveLoadComponent on game mode.", typename.EnumToString(ESaveType, type)), LogLevel.WARNING);
			return false;
		}
		
		return callback.FileExists(m_sMissionSaveFileName, customName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	\return True if save files can be saved online.
	*/
	bool CanSaveToCloud()
	{
		return RplSession.Mode() == RplMode.Dedicated;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Open file name and read its meta header.
	\param Save file name
	\return Meta header
	*/
	SCR_MetaStruct GetMeta(string fileName)
	{
		SCR_DSSessionCallback callback = FindCallback(fileName);
		if (!callback)
		{
			Print(string.Format("SCR_SaveManagerCore: Cannot load meta of save file '%1', no rules found for it!", fileName), LogLevel.WARNING);
			return null;
		}
		
		return callback.GetMeta(fileName);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	///@name Lists
	///@{
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/*!
	Get save file names of given save type.
	\param[out] outLocalSaves Array to be filled with save file names
	\param type Save type
	\param currentMissionOnly When true, return onlyu save files belonging to currently loaded mission
	\return Number of save file names
	*/
	int GetLocalSaveFiles(out notnull array<string> outLocalSaves, ESaveType saveTypes, bool currentMissionOnly)
	{
		string missionFileName;
		if (currentMissionOnly)
			missionFileName = m_sMissionSaveFileName;
		
		return GetLocalSaveFiles(outLocalSaves, saveTypes, missionFileName);
	}
	/*!
	Get save file names of given save type.
	\param[out] outLocalSaves Array to be filled with save file names
	\param type Save type
	\param missionFileName When not an empty string, return only save files belonging to this mission
	\return Number of save file names
	*/
	int GetLocalSaveFiles(out notnull array<string> outLocalSaves, ESaveType saveTypes, string missionFileName = string.Empty)
	{
		for (int i = GetGame().GetBackendApi().GetStorage().AvailableSaves(outLocalSaves) - 1; i >= 0; i--)
		{
			SCR_DSSessionCallback callback = FindCallback(outLocalSaves[i]);
			if (!callback || !(saveTypes & callback.GetSaveType()) || (missionFileName && callback.GetMissionFileName(outLocalSaves[i]) != missionFileName))
				outLocalSaves.Remove(i);
		}
		return outLocalSaves.Count();
	}
	///@}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	///@name Save Types
	///@{
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/*!
	Check if saving is allowed at this moment.
	\param type Save type
	\return True if saving is allowed
	*/
	bool CanSave(ESaveType type)
	{
		if (!m_sMissionSaveFileName || !FindCallback(type))
			return false;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		return !gameMode || gameMode.GetState() == SCR_EGameModeState.GAME;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Assign JSON struct to specific save type.
	Multiple save types can reuse the same struct.
	\param type Save type
	\param struct JSON mission struct to be assigned.
	*/
	void SetStruct(ESaveType type, SCR_MissionStruct struct)
	{
		foreach (SCR_DSSessionCallback callback: m_aCallbacks)
		{
			if (callback.GetSaveType() == type)
			{
				callback.SetStruct(struct);
				return;
			}
		}
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Print out JSON struct of given save type.
	\param type Save type
	*/
	void Log(ESaveType type)
	{
		SCR_DSSessionCallback callback = FindCallback(type);
		if (callback)
			callback.Log();
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Extract mission file name from save file name.
	\param fileName Save file name
	\return Mission file name
	*/
	string GetMissionFileName(string fileName)
	{
		SCR_DSSessionCallback callback = FindCallback(fileName);
		if (callback)
			return callback.GetMissionFileName(fileName);
		else
			return string.Empty;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Extract custom name from save file name.
	\param fileName Save file name
	\return Custom name
	*/
	string GetCustomName(string fileName)
	{
		SCR_DSSessionCallback callback = FindCallback(fileName);
		if (callback)
			return callback.GetCustomName(fileName);
		else
			return string.Empty;
	}
	///@}
	
	//----------------------------------------------------------------------------------------
	protected SCR_DSSessionCallback FindCallback(ESaveType type)
	{
		foreach (SCR_DSSessionCallback callback: m_aCallbacks)
		{
			if (callback.GetSaveType() == type && callback.IsConfigured())
				return callback;
		}
		return null;
	}
	
	//----------------------------------------------------------------------------------------
	protected SCR_DSSessionCallback FindCallback(string fileName)
	{
		foreach (SCR_DSSessionCallback callback: m_aCallbacks)
		{
			if (callback.IsCompatible(fileName))
				return callback;
		}
		return null;
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Workshop WIP
	//////////////////////////////////////////////////////////////////////////////////////////	
	void UploadToWorkshop(string fileName)
	{
		SCR_DSSessionCallback callback = FindCallback(fileName);
		if (!callback)
			return;
		
		Print("UploadToWorkshop: " + fileName);
		
		m_UploadCallback = new SCR_ServerSaveSessionCallback(fileName, callback.GetStruct());
	}
	
	//----------------------------------------------------------------------------------------
	void DownloadFromWorkshop()
	{
		m_DownloadCallback = new SCR_SaveManager_BackendCallback();
		
		m_DownloadPageParams = new SCR_SaveManager_PageParams();
		m_DownloadPageParams.limit = 50;

		GetGame().GetBackendApi().GetWorldSaveApi().RequestPage(m_DownloadCallback, m_DownloadPageParams, true);
	}
	
	//----------------------------------------------------------------------------------------
	void OnDownloadFromWorkshop()
	{
		m_DownloadCallback = null;
		m_DownloadPageParams = null;
		
		Print("GetPageCount() = " + GetGame().GetBackendApi().GetWorldSaveApi().GetPageCount());
		Print("GetPageItemCount() = " + GetGame().GetBackendApi().GetWorldSaveApi().GetPageItemCount());
		Print("GetTotalItemCount() = " + GetGame().GetBackendApi().GetWorldSaveApi().GetTotalItemCount());
		
		array<WorldSaveItem> items = {};
		int count = GetGame().GetBackendApi().GetWorldSaveApi().GetPageItems(items);
		Print(count);
		foreach (int i, WorldSaveItem item: items)
		{
			PrintFormat("%1: %2: %3", i, item.Id(), item.Name());
			
			if (m_bDebugDelete)
				item.DeleteOnline(null);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	///@name File Name To Load
	///@{
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/*!
	Set the latest save file of given mission as the save that should be loaded upon mission start.
	\param missionHeader Mission header
	\return True if save file exists and was marked for load
	*/
	bool SetFileNameToLoad(SCR_MissionHeader missionHeader)
	{
		if (!missionHeader)
			return false;
		
		//--- Find latest save for the mission
		string latestSaveFileName;
		if (!FindLatestSave(missionHeader.GetSaveFileName(), latestSaveFileName))
			return false;
		
		return SetFileNameToLoad(latestSaveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Set which save file should be loaded upon mission start.
	\param fileName Save file name
	\return True if save file exists and was marked for load
	*/
	bool SetFileNameToLoad(string fileName)
	{
		if (!GetGame().GetBackendApi().GetStorage().CheckFileID(fileName))
			return false;
		
		GameSessionStorage.s_Data.Insert(GAME_SESSION_STORAGE_NAME, fileName);
		Print(string.Format("'%1' set as a save file name to load after world start.", fileName), LogLevel.VERBOSE);
		return true;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Remove save file name marked to be loaded after mission start.
	Does not remove the file itself.
	*/
	void ResetFileNameToLoad()
	{
		GameSessionStorage.s_Data.Remove(GAME_SESSION_STORAGE_NAME);
		Print("Save file name to load after world start removed.", LogLevel.VERBOSE);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check which save file should be loaded after mission start.
	\param[out] fileNameToLoad Save file name
	\return True if some save file is marked to be loaded after mission start
	*/
	bool FindFileNameToLoad(out string fileNameToLoad)
	{
		return GameSessionStorage.s_Data.Find(GAME_SESSION_STORAGE_NAME, fileNameToLoad);
	}
	///@}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	///@name Latest Save
	///@{
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/*!
	Set the latest save for the currently running mission.
	\param saveFileName Save file name
	*/
	void SetCurrentMissionLatestSave(string saveFileName)
	{
		SetLatestSave(m_sMissionSaveFileName, saveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Remove the latest save for the currently running mission.
	\param saveFileName Save file name
	*/
	void RemoveCurrentMissionLatestSave()
	{
		RemoveLatestSave(m_sMissionSaveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Find the latest save for the currently running mission.
	\param[out] outSaveFileName String to be filled with the latest save file name
	*/
	bool FindCurrentMissionLatestSave(out string outSaveFileName)
	{
		return FindLatestSave(m_sMissionSaveFileName, outSaveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Set the latest save for the given mission.
	\param missionFileName Mission save file name
	\param saveFileName Save file name
	*/
	void SetLatestSave(string missionFileName, string saveFileName)
	{
		m_LatestSaveSettings.SetFileName(missionFileName, saveFileName);
		WriteLatestSaves();
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Remove the latest save for the given mission.
	\param missionFileName Mission save file name
	\param saveFileName Save file name
	*/
	void RemoveLatestSave(string missionFileName)
	{
		m_LatestSaveSettings.RemoveFileName(missionFileName);
		WriteLatestSaves();
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Find the latest save for the given mission.
	\param missionFileName Mission save file name
	\param saveFileName Save file name
	*/
	bool FindLatestSave(string missionFileName, out string outSaveFileName)
	{
		return m_LatestSaveSettings.FindFileName(missionFileName, outSaveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Set the latest save for the given mission.
	\param missionFileName Mission header
	\param saveFileName Save file name
	*/
	bool FindLatestSave(SCR_MissionHeader missionHeader, out string outSaveFileName)
	{
		return missionHeader && FindLatestSave(missionHeader.GetSaveFileName(), outSaveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check if the mission has a latest save.
	\param missionFileName Mission save file name
	*/
	bool HasLatestSave(string missionFileName)
	{
		string saveFileName;
		return FindLatestSave(missionFileName, saveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check if the mission has a latest save.
	\param missionFileName Mission header
	*/
	bool HasLatestSave(SCR_MissionHeader missionHeader)
	{
		string saveFileName;
		return missionHeader && FindLatestSave(missionHeader, saveFileName);
	}
	///@}
	
	//----------------------------------------------------------------------------------------
	protected void ReadLatestSaves()
	{
		m_LatestSaveContainer = GetGame().GetGameUserSettings().GetModule("SCR_LatestSaveSettings");
		m_LatestSaveSettings = new SCR_LatestSaveSettings();
		BaseContainerTools.WriteToInstance(m_LatestSaveSettings, m_LatestSaveContainer);
	}
	
	//----------------------------------------------------------------------------------------
	protected void WriteLatestSaves()
	{
		BaseContainerTools.ReadFromInstance(m_LatestSaveSettings, m_LatestSaveContainer);
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Init
	//////////////////////////////////////////////////////////////////////////////////////////
	protected void Init()
	{		
		//--- Initialize latest saves
		ReadLatestSaves();
		
		//--- Stop if mission save file is not defined (e.g., when mission header is missing) or when on client
		if (!m_sMissionSaveFileName || !Replication.IsServer())
			return;
		
		//--- Find save file to be loaded on start
		string fileNameToLoad;
		if (System.IsConsoleApp())
		{
			//--- DEDICATED SERVER - Check file name defined in CLI param
			if (!System.GetCLIParam("loadSaveFile", fileNameToLoad))
 				return;
			
			//--- If the param is present without the file name specified, use the latest save
			if (fileNameToLoad.IsEmpty() && !FindLatestSave(m_sMissionSaveFileName, fileNameToLoad))
				return;
			
			//if (System.IsCLIParam("disableLoadingLatestSave") || !FindLatestSave(m_sMissionSaveFileName, fileNameToLoad))
			//	return;
		}
		else
		{
			//--- STANDARD GAME - Check if requested save file exists
			if (!FindFileNameToLoad(fileNameToLoad))
				return;
		}
		
		//--- Load the file and unmark it, so restarting from pause menu won't load it again
		Load(fileNameToLoad);
		ResetFileNameToLoad();
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Default functions
	//////////////////////////////////////////////////////////////////////////////////////////
	override void OnUpdate(float timeSlice)
	{
		if (m_sMissionSaveFileName && !System.IsConsoleApp() && GetGame().IsDev())
		{
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_SAVE))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_SAVE, false);
				
				ESaveType saveType = 1 << DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_SAVING_TYPE);
				
				array<string> customNames = {"Alpha", "Kilo", "Zulu"};
				string customName = customNames.GetRandomElement();
				Save(saveType, customName);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_LOG))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_LOG, false);
				
				ESaveType saveType = 1 << DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_SAVING_TYPE);
				Log(saveType);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_LOAD_LATEST))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_LOAD_LATEST, false);
				
				string fileName;
				if (FindLatestSave(m_sMissionSaveFileName, fileName))
					Load(fileName);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_RESTART_AND_LOAD_LATEST))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_RESTART_AND_LOAD_LATEST, false);
				RestartAndLoad();
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_LOG_LATEST))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_LOG_LATEST, false);
				
				string latestSaveFileName;
				if (FindLatestSave(m_sMissionSaveFileName, latestSaveFileName))
					PrintFormat("The latest save file name for mission '%1' is '%2'", m_sMissionSaveFileName, latestSaveFileName);
				else
					PrintFormat("There is no latest save file name for mission '%1'", latestSaveFileName);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_UPLOAD_LATEST))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_UPLOAD_LATEST, false);
				
				string fileName;
				if (FindLatestSave(m_sMissionSaveFileName, fileName))
					UploadToWorkshop(fileName);
				else
					Print("SCR_SaveManagerCore: Cannot upload, latest save not found!", LogLevel.WARNING);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_DOWNLOAD))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_DOWNLOAD, false);
				
				m_bDebugDelete = false;
				DownloadFromWorkshop();
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_DELETE))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_DELETE, false);
				
				m_bDebugDelete = true;
				DownloadFromWorkshop();
			}
		}
	}
	
	//----------------------------------------------------------------------------------------
	override void OnWorldPostProcess(World world)
	{
		SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		
#ifdef WORKBENCH
		//--- Mission header not found, create a debug one (play mode in World Editor never has a mission header, even when one for the world exists)
		if (!missionHeader && SCR_SaveLoadComponent.GetInstance())
		{
			m_WorkbenchMissionHeader = new SCR_MissionHeader();
			m_WorkbenchMissionHeader.m_sSaveFileName = FilePath.StripPath(FilePath.StripExtension(GetGame().GetWorldFile()));
			m_WorkbenchMissionHeader.m_bIsSavingEnabled = true;
			missionHeader = m_WorkbenchMissionHeader;
		}	
#endif
		//--- Set mission save file name, but only if saving is enabled
		if (missionHeader && missionHeader.m_bIsSavingEnabled)
			m_sMissionSaveFileName = missionHeader.GetSaveFileName();
		
		//--- Initialize save manager and load marked save file
		Init();
	}
	
	//----------------------------------------------------------------------------------------
	override void OnGameStart()
	{
		if (GetGame().IsDev() && Replication.IsServer() && !System.IsConsoleApp())
		{
			typename enumType = ESaveType;
			string categoryName = "Save Manager";
			DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_SAVING, categoryName, "Game");
			DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_SAVING_TYPE, "", "Type", categoryName, string.Format("0,%1,0,1", enumType.GetVariableCount() - 1));
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_LOG, "", "Log Struct By Type", categoryName);
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_SAVE, "", "Save By Type", categoryName);
			
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_LOAD_LATEST, "", "Load Latest Save", categoryName);
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_RESTART_AND_LOAD_LATEST, "", "Restart and Load Latest Save", categoryName);
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_LOG_LATEST, "", "Log Latest Save File Name", categoryName);
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_UPLOAD_LATEST, "", "Upload Latest Save", categoryName);
			
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_DOWNLOAD, "", "Download Workshop Saves", categoryName);
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_DELETE, "", "Delete Workshop Saves", categoryName);
		}
	}
	
	//----------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		m_sMissionSaveFileName = string.Empty;
#ifdef WORKBENCH
		m_WorkbenchMissionHeader = null;
#endif
		
		if (GetGame().IsDev() && Replication.IsServer() && !System.IsConsoleApp())
		{
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_TYPE);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_SAVE);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_LOG);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_LOAD_LATEST);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_RESTART_AND_LOAD_LATEST);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_LOG_LATEST);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_UPLOAD_LATEST);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_DOWNLOAD);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_DELETE);
		}
	}
};

class SCR_SaveManager_BackendCallback: BackendCallback
{
	protected ref SCR_SaveManager_PageParams m_DownloadPageParams;
	
	override void OnError( int code, int restCode, int apiCode )
	{
		PrintFormat("[BackendCallback] OnError: code=%1 ('%4'), restCode=%2, apiCode=%3", code, restCode, apiCode, GetGame().GetBackendApi().GetErrorCode(code));
	}
	override void OnSuccess( int code )
	{
		PrintFormat("[BackendCallback] OnSuccess(): code=%1", code);
		GetGame().GetSaveManager().OnDownloadFromWorkshop();
	}
	override void OnTimeout()
	{
		Print("[BackendCallback] OnTimeout");
	}
};
class SCR_SaveManager_PageParams: PageParams
{
	override void OnPack()
	{
		StoreBoolean("owned", true);
	}
};