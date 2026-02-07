[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_SaveLoadComponentClass: SCR_BaseGameModeComponentClass
{
};
/*!
Manager for saving and loading game mode data.

ToDo:
* Load when starting the world if it's requested (i.e., 'Continue' save)
* Refactor Conflict save to use this system
*/
class SCR_SaveLoadComponent: SCR_BaseGameModeComponent
{
	[Attribute(desc: "Struct object which manages saved data. Must be defined, without it saving won't work.")]
	protected ref SCR_MissionStruct m_Struct;
	
	[Attribute(defvalue: "1", desc: "When enabled, save the state when exiting the world.")]
	protected bool m_SaveOnExit;
	
	[Attribute(defvalue: "0", desc: "0 = disabled. 60 seconds is the lowest accepted value otherwise.")]
	protected int m_iAutosavePeriod;
	
	protected ref SCR_DSSessionCallback m_Callback;
	protected string m_sFileName;
	protected float m_fTimer;
	
	protected ref SCR_SaveLoadComponent_BackendCallback m_DownloadCallback;
	protected ref PageParams m_DownloadPageParams;
	protected ref SCR_MissionHeader m_DebugMissionHeader;
	
	protected static const string GAME_SESSION_STORAGE_NAME = "SCR_SaveLoadComponent_FileNameToLoad";
	protected static const int MINIMUM_AUTOSAVE_PERIOD = 60;
	
	/////////////////////////////////////////////////////////////////////////////
	// Static
	/////////////////////////////////////////////////////////////////////////////
	/*!
	Check if given mission has a save file
	\param missionHeader Mission header
	\return True if the mission has a save file
	*/
	static bool HasSaveFile(SCR_MissionHeader missionHeader)
	{
		return GetGame().GetBackendApi().GetStorage().CheckFileID(missionHeader.GetSaveFileName());
	}
	/*!
	Mark the mission to be loaded from a save file upon start.
	\param missionHeader Header of the misison that should be loaded on start. Use null to reset the value.
	*/
	static void LoadOnStart(SCR_MissionHeader missionHeader = null)
	{
		if (missionHeader)
			GameSessionStorage.s_Data.Insert(GAME_SESSION_STORAGE_NAME, missionHeader.GetSaveFileName());
		else
			GameSessionStorage.s_Data.Remove(GAME_SESSION_STORAGE_NAME)
	}
	/*!
	Check if the mission should be loaded from a save file upon start.
	\return missionHeader Queried mission
	*/
	static bool IsLoadOnStart(SCR_MissionHeader missionHeader)
	{
		string saveFileName = GetSaveFileName(missionHeader);
		if (!saveFileName)
			return false;
		
		//--- Load from CLI param
		string fileNameCLI;
		if (System.GetCLIParam("loadSaveFile", fileNameCLI))
		{
			array<string> fileNamesCLI = {};
			fileNameCLI.Split(",", fileNamesCLI, false);
			if (fileNamesCLI.Contains(saveFileName))
				return true;
		}
		
		string fileNameToLoad;
		return GameSessionStorage.s_Data.Find(GAME_SESSION_STORAGE_NAME, fileNameToLoad) && fileNameToLoad == saveFileName;
	}
	
	protected static string GetSaveFileName(SCR_MissionHeader missionHeader)
	{
		if (missionHeader)
			return missionHeader.GetSaveFileName();
		else
			return string.Empty;
	}
	
	/////////////////////////////////////////////////////////////////////////////
	// Public
	/////////////////////////////////////////////////////////////////////////////
	/*!
	\return Local instance of the possession manager
	*/
	static SCR_SaveLoadComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return SCR_SaveLoadComponent.Cast(gameMode.FindComponent(SCR_SaveLoadComponent));
		else
			return null;
	}
	
	/*!
	Save all configured structs.
	*/
	void Save()
	{
		if (m_Callback != null)
			m_Callback.SaveSession(m_sFileName);
	}
	/*!
	Restart the world and load saved state afterwards.
	*/
	void RestartAndLoad()
	{
		SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(GetMissionHeader());
		if (missionHeader)
		{
			LoadOnStart(missionHeader);
			GameStateTransitions.RequestServerReload();
		}
		else
		{
			Print("Cannot load mission, no MissionHeader defined. Happens for example when playing the world from World Editor.", LogLevel.WARNING);
		}
	}
	/*!
	\return True if the world should be saved on exit.
	*/
	bool CanSaveOnExit()
	{
		return m_SaveOnExit;
	}	
	
	//--- For testing only
	void UploadToWorkshop()
	{
		Print(m_sFileName);
		
		WorldSaveManifest manifest = new WorldSaveManifest();
		manifest.m_sName = m_sFileName;
		manifest.m_sSummary = "Test save";
		manifest.m_sPreview = "UI/Textures/ScalingTest/Sources/pattern_900x900.jpg";
		manifest.m_aFiles = {m_Struct};
		manifest.m_aFileNames = {m_sFileName};
		
		GetGame().GetBackendApi().GetWorldSaveApi().UploadWorldSave(manifest, null, null);
	}
	//--- For testing only
	void DownloadFromWorkshop()
	{
		m_DownloadCallback = new SCR_SaveLoadComponent_BackendCallback();
		m_DownloadCallback.m_SaveLoadComponent = this;
		
		m_DownloadPageParams = new PageParams();
		m_DownloadPageParams.limit = 42;
		
		GetGame().GetBackendApi().GetWorldSaveApi().RequestPage(m_DownloadCallback, m_DownloadPageParams, false);
		
	}
	//--- For testing only
	void OnDownloadFromWorkshop()
	{
		Print(m_DownloadPageParams);
		Print(GetGame().GetBackendApi().GetWorldSaveApi().GetPageCount());
		Print(GetGame().GetBackendApi().GetWorldSaveApi().GetPageItemCount());
		
		array<WorldSaveItem> items = {};
		GetGame().GetBackendApi().GetWorldSaveApi().GetPageItems(items);
		foreach (int i, WorldSaveItem item: items)
		{
			PrintFormat("%1: %2", i, item.Id());
		}
	}
	/*!
	Log the most recently saved structs.
	*/
	void Log()
	{
		if (m_Callback != null)
			m_Callback.LogSession();
	}
	
	/////////////////////////////////////////////////////////////////////////////
	// Protected
	/////////////////////////////////////////////////////////////////////////////
	protected void Load()
	{
		if (m_Callback != null)
			m_Callback.LoadSession(m_sFileName);
	}
	protected MissionHeader GetMissionHeader()
	{
		if (m_DebugMissionHeader)
			return m_DebugMissionHeader;
		else
			return GetGame().GetMissionHeader();
	}
	
	/////////////////////////////////////////////////////////////////////////////
	// Overrides
	/////////////////////////////////////////////////////////////////////////////
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (GetGame().IsDev() && !System.IsConsoleApp())
		{
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_SAVE))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVELOAD_SAVE, false);
				Save();
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOAD))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOAD, false);
				Load();
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_RESTART_AND_LOAD))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVELOAD_RESTART_AND_LOAD, false);
				RestartAndLoad();
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_UPLOAD))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVELOAD_UPLOAD, false);
				UploadToWorkshop();
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_DOWNLOAD))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVELOAD_DOWNLOAD, false);
				DownloadFromWorkshop();
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOG))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOG, false);
				Log();
			}
		}
		
		// Autosave
		if (m_iAutosavePeriod > 0)
		{
			m_fTimer += timeSlice;
			
			if (m_fTimer >= m_iAutosavePeriod)
			{
				m_fTimer = 0;
				Save();
			}
		}
	}
	/* //--- Disabled, not reliable. Some variables are already cleared at this moment.
	override void OnGameEnd()
	{
		if (m_SaveOnExit)
			Save();
	}
	*/
	override void OnWorldPostProcess(World world)
	{
		if (Replication.IsServer())
		{
			SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
			
#ifdef WORKBENCH
			if (!missionHeader)
			{
				m_DebugMissionHeader = new SCR_MissionHeader();
				m_DebugMissionHeader.m_sSaveFileName = "WB_" + FilePath.StripPath(FilePath.StripExtension(GetGame().GetWorldFile()));
				m_DebugMissionHeader.m_bIsSavingEnabled = true;
				missionHeader = m_DebugMissionHeader;
			}	
#endif
			if (missionHeader && !missionHeader.m_bIsSavingEnabled)
				return;
			
			m_sFileName = GetSaveFileName(missionHeader);
			if (m_sFileName && m_Struct)
			{			
				m_Callback = new SCR_DSSessionCallback(m_Struct);
				
				if (IsLoadOnStart(missionHeader))
				{
					m_Callback.LoadSession(m_sFileName);
					LoadOnStart(); //--- Unmark for load so restarting from pause menu won't load it again
				}
			}
		}
	}
	override void OnPostInit(IEntity owner)
	{
		if (Replication.IsServer())
		{
			SetEventMask(owner, EntityEvent.FRAME);
			
			if (m_iAutosavePeriod > 0 && m_iAutosavePeriod < MINIMUM_AUTOSAVE_PERIOD)
			{
				Print("SCR_SaveLoadComponent: Autosave period set too low (" + m_iAutosavePeriod + "), setting to " + MINIMUM_AUTOSAVE_PERIOD, LogLevel.WARNING);
				m_iAutosavePeriod = MINIMUM_AUTOSAVE_PERIOD;
			}
		}
		
		if (GetGame().IsDev() && Replication.IsServer() && !System.IsConsoleApp())
		{
			DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_SAVELOAD, "Save/Load", "Game");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_SAVE, "", "Save Session", "Save/Load");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOAD, "", "Load Session", "Save/Load");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_RESTART_AND_LOAD, "", "Restart and Load Session", "Save/Load");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_UPLOAD, "", "Upload Saves", "Save/Load");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_DOWNLOAD, "", "Download Saves", "Save/Load");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOG, "", "Log Session Save", "Save/Load");
		}
	}
};
class SCR_SaveLoadComponent_BackendCallback: BackendCallback
{
	SCR_SaveLoadComponent m_SaveLoadComponent;
	
	override void OnError( int code, int restCode, int apiCode )
	{
		PrintFormat("[BackendCallback] OnError: code=%1 ('%4'), restCode=%2, apiCode=%3", code, restCode, apiCode, GetGame().GetBackendApi().GetErrorCode(code));
	}
	override void OnSuccess( int code )
	{
		Print("[BackendCallback] OnSuccess()");
		m_SaveLoadComponent.OnDownloadFromWorkshop();
	}
	override void OnTimeout()
	{
		Print("[BackendCallback] OnTimeout");
	}
};