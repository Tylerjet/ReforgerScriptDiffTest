[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_SaveLoadComponentClass: SCR_BaseGameModeComponentClass
{
};
/*!
Game mode-specific settings for session saving.
*/
class SCR_SaveLoadComponent: SCR_BaseGameModeComponent
{
	[Attribute(desc: "Struct object which manages saved data. Must be defined, without it saving won't work.")]
	protected ref SCR_MissionStruct m_Struct;
	
	[Attribute(defvalue: "1", desc: "When enabled, save the state when exiting the world.")]
	protected bool m_SaveOnExit;
	
	[Attribute(defvalue: "0", desc: "0 = disabled. 60 seconds is the lowest accepted value otherwise.")]
	protected int m_iAutosavePeriod;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "conf class=SCR_MissionHeader", desc: "Mission header used for saving/loading in Workbench (where standard mission header is not loaded)")]
	protected ResourceName m_DebugHeaderResourceName;
	
	protected float m_fTimer;
	
	protected static const int MINIMUM_AUTOSAVE_PERIOD = 60;
	
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
	
	//----------------------------------------------------------------------------------------
	/*!
	\return True if the world should be saved on exit.
	*/
	bool CanSaveOnExit()
	{
		return m_SaveOnExit;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	\return Mission header used for debugging in World Editor (where mission headers are otherwise unavailable)
	*/
	ResourceName GetDebugHeaderResourceName()
	{
		return m_DebugHeaderResourceName;
	}
	
	/////////////////////////////////////////////////////////////////////////////
	// Overrides
	/////////////////////////////////////////////////////////////////////////////
	override void EOnFrame(IEntity owner, float timeSlice)
	{		
		// Autosave
		if (m_iAutosavePeriod > 0)
		{
			m_fTimer += timeSlice;
			
			if (m_fTimer >= m_iAutosavePeriod)
			{
				m_fTimer = 0;
				GetGame().GetSaveManager().Save(ESaveType.AUTO);
			}
		}
	}
	
	//----------------------------------------------------------------------------------------
	override void OnGameModeEnd(SCR_GameModeEndData data)
	{
		GetGame().GetSaveManager().RemoveCurrentMissionLatestSave();
	}
	
	//----------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (Replication.IsServer() && !owner.GetWorld().IsEditMode())
		{
			SCR_SaveManagerCore saveManager = GetGame().GetSaveManager();
			saveManager.SetStruct(ESaveType.USER, m_Struct);
			saveManager.SetStruct(ESaveType.AUTO, m_Struct);
			saveManager.SetStruct(ESaveType.EDITOR, m_Struct);
			
			SetEventMask(owner, EntityEvent.FRAME);
			
			if (m_iAutosavePeriod > 0 && m_iAutosavePeriod < MINIMUM_AUTOSAVE_PERIOD)
			{
				Print("SCR_SaveLoadComponent: Autosave period set too low (" + m_iAutosavePeriod + "), setting to " + MINIMUM_AUTOSAVE_PERIOD, LogLevel.WARNING);
				m_iAutosavePeriod = MINIMUM_AUTOSAVE_PERIOD;
			}
		}
	}
};