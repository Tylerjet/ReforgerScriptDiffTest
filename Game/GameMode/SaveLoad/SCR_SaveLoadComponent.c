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
	
	protected static string m_sFileNameToLoad;
	
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
			m_sFileNameToLoad = missionHeader.GetSaveFileName();
		else
			m_sFileNameToLoad = string.Empty;
	}
	/*!
	Check if the mission should be loaded from a save file upon start.
	\return missionHeader Queried mission
	*/
	static bool IsLoadOnStart(SCR_MissionHeader missionHeader)
	{
		// TODO: Solve for Listen servers
		if (RplSession.Mode() == RplMode.Dedicated)
			return !System.IsCLIParam("backendFreshSession");
		else
			return missionHeader && missionHeader.GetSaveFileName() == m_sFileNameToLoad;
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
		SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (missionHeader)
		{
			LoadOnStart(missionHeader);
			GetGame().PlayMission(missionHeader);
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
	/*!
	Log the most recently saved structs.
	*/
	void Log()
	{
		if (m_Callback != null)
			m_Callback.LogSession();
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
				Save();
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVELOAD_SAVE, false);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOAD))
			{
				RestartAndLoad();
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOAD, false);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOG))
			{
				Log();
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOG, false);
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
			if (missionHeader)
			{
				m_sFileName = missionHeader.GetSaveFileName();
			
				//--- Saving is disabled, terminate
				if (!missionHeader.IsSavingEnabled())
					return;
			}
#ifdef WORKBENCH
			else
			{
				m_sFileName = "WB_" + FilePath.StripPath(FilePath.StripExtension(GetGame().GetWorldFile()));
			}
#endif
			
			if (m_sFileName)
			{			
				m_Callback = new SCR_DSSessionCallback(m_Struct);
				
				if (IsLoadOnStart(missionHeader))
					m_Callback.LoadSession(m_sFileName);
			}
		}
	}
	override void OnPostInit(IEntity owner)
	{
		if (Replication.IsServer())
		{
			owner.SetFlags(EntityFlags.ACTIVE, false);
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
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOAD, "", "Restart and Load Session", "Save/Load");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOG, "", "Log Session Save", "Save/Load");
		}
	}
};