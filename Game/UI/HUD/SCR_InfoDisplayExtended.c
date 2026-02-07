//#define DISABLE_HUD
//#define DEBUG_INFODISPLAY_CALLS

enum EShowGUI
{
	IN_ADS = 1,
	IN_1ST_PERSON = 2,
	IN_3RD_PERSON = 4,
	IN_PAUSE_MENU = 8,
	WITHOUT_ENTITY = 16
};

//------------------------------------------------------------------------------------------------
class SCR_InfoDisplayExtended : SCR_InfoDisplay
{
	[Attribute("1", UIWidgets.CheckBox, "Toggles ON/OFF info display.")]
	protected bool m_bIsEnabled;
	
	[Attribute("7", UIWidgets.Flags, "Defines when the GUI element is allowed to show.", "", ParamEnumArray.FromEnum(EShowGUI) )]
	private EShowGUI m_eShow;	
	
	protected IEntity m_ControlledEntity;
	protected SCR_PlayerController m_PlayerController;
	protected SCR_CharacterCameraHandlerComponent m_CameraHandler;
	protected MenuManager m_MenuManager;
	protected EventHandlerManagerComponent m_EventHandlerManager;
	
	protected bool m_bAttachedToController;
	
	protected bool m_bInThirdPerson;
	protected bool m_bInADS;
	protected bool m_bInPauseMenu;
	protected bool m_bShowGlobal;						// Global GUI visibility flag
	protected bool m_bShowLocal = true;					// Local/custom "per-GUI-handling" flag
	protected bool m_bShowInAllCameras = true;
	
	private ScriptInvoker<IEntity, IEntity> m_OnControlledEntityChanged;
	private ScriptInvoker m_OnThirdPersonChange;
	private ScriptInvoker m_OnPauseMenuOpen;
	private ScriptInvoker m_OnPauseMenuClose;
	
	//------------------------------------------------------------------------------------------------		
	// Interface methods for the extended InfoDisplay class.
	//------------------------------------------------------------------------------------------------		
	bool DisplayStartDrawInit(IEntity owner)
	{
		return true;
	}	

	void DisplayStartDraw(IEntity owner)
	{
	}	

	void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
	}	
			
	void DisplayStopDraw(IEntity owner)
	{
	}	
	
	void DisplayInit(IEntity owner)
	{
	}	

	void DisplayUpdate(IEntity owner, float timeSlice)
	{
	}	
	
	//------------------------------------------------------------------------------------------------
	// InfoDisplay events blocked for overriding.
	// The interface methods above should be used instead.		
	//------------------------------------------------------------------------------------------------
	private override event void OnStartDraw(IEntity owner)
	{
		#ifdef DEBUG_INFODISPLAY_CALLS
		PrintFormat(">> OnStartDraw: %1 | %2", this, owner);
		#endif
		
		#ifdef DISABLE_HUD
		m_bIsEnabled = false;	
		#endif
		
		if (!m_bIsEnabled)
			return;

		m_MenuManager = GetGame().GetMenuManager();
		
		if (!m_MenuManager)
		{
			m_bIsEnabled = false;
			return;
		}
		
		m_EventHandlerManager = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
		
		if (m_EventHandlerManager)
			m_EventHandlerManager.RegisterScriptHandler("OnADSChanged", this, OnADSSwitched);
	
		m_HUDManager = SCR_HUDManagerComponent.GetHUDManager();
		
		if (!m_HUDManager)
		{
			m_bIsEnabled = false;
			return;
		}		
		
		// Terminate if UI is not enabled for any of the camera modes (1PV, 3PV, ADS) && requires a controlled entity
		if (m_eShow & (EShowGUI.IN_ADS | EShowGUI.IN_1ST_PERSON | EShowGUI.IN_3RD_PERSON | EShowGUI.WITHOUT_ENTITY) == 0)
		{
			m_bIsEnabled = false;
			return;
		}		
		
		m_bShowInAllCameras = (m_eShow & (EShowGUI.IN_ADS | EShowGUI.IN_1ST_PERSON | EShowGUI.IN_3RD_PERSON)) == (EShowGUI.IN_ADS | EShowGUI.IN_1ST_PERSON | EShowGUI.IN_3RD_PERSON);
			
		m_PlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());

		if (!m_PlayerController)
		{
			m_bIsEnabled = false;
			return;
		}
		
		m_ControlledEntity = m_PlayerController.GetControlledEntity();
		
		m_bIsEnabled = DisplayStartDrawInit(owner);
		
		if (!m_bIsEnabled)
			return;
		
		super.OnStartDraw(owner);
		
		if (!m_wRoot)
		{
			m_bIsEnabled = false;
			return;
		}
		
		DisplayStartDraw(owner);

		// Init PauseMenu handling
		if ((m_eShow & EShowGUI.IN_PAUSE_MENU) == 0)
		{
			m_OnPauseMenuOpen = PauseMenuUI.m_OnPauseMenuOpened;
			m_OnPauseMenuOpen.Insert(OnPauseMenuOpen);
			
			m_OnPauseMenuClose = PauseMenuUI.m_OnPauseMenuClosed;
			m_OnPauseMenuClose.Insert(OnPauseMenuClose);
		}
		
		// Init monitor of controlled entity and setup initial GUI visibility
		if (m_bAttachedToController)
		{
			m_OnControlledEntityChanged = m_PlayerController.m_OnControlledEntityChanged;
			m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		}
		OnControlledEntityChanged(null, m_ControlledEntity);				
	}
	
	private void OnADSSwitched(BaseWeaponComponent weapon, bool inADS)
	{
		m_bInADS = inADS;
		
		UpdateVisibility();
	}

	private void OnPauseMenuOpen()
	{
		m_bInPauseMenu = true;
		
		UpdateVisibility();
	}	

	private void OnPauseMenuClose()
	{
		m_bInPauseMenu = false;
		
		UpdateVisibility();
	}	
			
	private void UpdateVisibility()
	{
		#ifdef DEBUG_INFODISPLAY_CALLS
		PrintFormat(">> UpdateVisibility: %1 | m_CameraHandler: %2", this, m_CameraHandler);
		#endif

		bool menuPassed = !m_bInPauseMenu || (m_bInPauseMenu && m_eShow & EShowGUI.IN_PAUSE_MENU);		
		bool cameraPassed;
		
		if (m_CameraHandler)		
		{
			m_bInThirdPerson = !m_bInADS && m_CameraHandler.Is3rdPersonView();
			
			cameraPassed = (m_bInADS && m_eShow & EShowGUI.IN_ADS) || (m_bInThirdPerson && m_eShow & EShowGUI.IN_3RD_PERSON) || (!m_bInThirdPerson && !m_bInADS && m_eShow & EShowGUI.IN_1ST_PERSON);
		}
		else
		{
			cameraPassed = m_eShow & EShowGUI.WITHOUT_ENTITY;
		}
			
		m_bShowGlobal = cameraPassed && menuPassed;
			
		if (m_wRoot)
			m_wRoot.SetVisible(m_bShowGlobal && m_bShowLocal);
	}
	
	private override event void OnStopDraw(IEntity owner)
	{
		#ifdef DEBUG_INFODISPLAY_CALLS
		PrintFormat(">> OnStopDraw: %1", this);
		#endif
		
		#ifdef DISABLE_HUD
		m_bIsEnabled = false;	
		#endif
	
		if (!m_bIsEnabled)
			return;

		if (m_OnControlledEntityChanged)
			m_OnControlledEntityChanged.Remove(OnControlledEntityChanged);		
		
		if (m_OnThirdPersonChange)
			m_OnThirdPersonChange.Remove(UpdateVisibility);	
		
		if (m_OnPauseMenuOpen)
			m_OnPauseMenuOpen.Remove(OnPauseMenuOpen);

		if (m_OnPauseMenuClose)
			m_OnPauseMenuClose.Remove(OnPauseMenuClose);
				
		if (m_EventHandlerManager)
			m_EventHandlerManager.RemoveScriptHandler("OnADSChanged", this, OnADSSwitched);						
				
		super.OnStopDraw(owner);
		
		DisplayStopDraw(owner);		
	}	
	
	private override event void OnInit(IEntity owner)
	{
		#ifdef DEBUG_INFODISPLAY_CALLS
		PrintFormat(">> OnInit: %1", this);
		#endif
		
		#ifdef DISABLE_HUD
		m_bIsEnabled = false;	
		#endif

		if (!m_bIsEnabled)
			return;

		m_bAttachedToController = PlayerController.Cast(owner);
		
		super.OnInit(owner);
		
		DisplayInit(owner);
	}
	
	private override event void UpdateValues(IEntity owner, float timeSlice)
	{
		if (!m_bIsEnabled || !m_bShowGlobal)
			return;
		
		super.UpdateValues(owner, timeSlice);		
		
		DisplayUpdate(owner, timeSlice);
	}	
	
	private void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		#ifdef DEBUG_INFODISPLAY_CALLS
		PrintFormat(">> OnControlledEntityChanged: %1 | %2 -> %3", this, from, to);
		#endif
		
		// Unregister old & potentionally obsolete callback
		if (m_OnThirdPersonChange)
			m_OnThirdPersonChange.Remove(UpdateVisibility);
		
		m_ControlledEntity = to;	
		m_CameraHandler = null;
		
		if (m_ControlledEntity)
		{
			m_CameraHandler = SCR_CharacterCameraHandlerComponent.Cast(m_ControlledEntity.FindComponent(SCR_CharacterCameraHandlerComponent));			

			// Listen to camera changed invoker, if UI is enabled only for some of the camera modes - 1PV, ADS or 3PV
			if (m_CameraHandler && !m_bShowInAllCameras)
			{			
				m_OnThirdPersonChange = m_CameraHandler.GetThirdPersonSwitchInvoker();
				m_OnThirdPersonChange.Insert(UpdateVisibility);
			}			
		}	

		// ADS seems to cancel when the entity changes		
		m_bInADS = false;
				
		DisplayControlledEntityChanged(from, to);
		
		// Update visibility after entity changed
		UpdateVisibility();		
	}	
};