void OnDeployMenuOpenDelegate();
typedef func OnDeployMenuOpenDelegate;
typedef ScriptInvokerBase<OnDeployMenuOpenDelegate> OnDeployMenuOpenInvoker;

void OnDeployMenuCloseDelegate();
typedef func OnDeployMenuCloseDelegate;
typedef ScriptInvokerBase<OnDeployMenuCloseDelegate> OnDeployMenuCloseInvoker;

//------------------------------------------------------------------------------------------------
class SCR_PlayerDeployMenuHandlerComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
/*!
	Component responsible for deploy menu management.
*/
class SCR_PlayerDeployMenuHandlerComponent : ScriptComponent
{
	private SCR_RespawnComponent m_RespawnComponent;
	private SCR_PlayerController m_PlayerController;
	private SCR_PlayerFactionAffiliationComponent m_PlyFactionAffil;
	private bool m_bReady = false;
	private bool m_bFirstOpen = true;
	private bool m_bWelcomeOpened;
	private bool m_bWelcomeClosed;
	
	private static ref OnDeployMenuCloseInvoker s_OnMenuClose;

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.FRAME);

		m_RespawnComponent = SCR_RespawnComponent.Cast(owner.FindComponent(SCR_RespawnComponent));
		if (!m_RespawnComponent)
		{
			Print(string.Format("%1 could not find %2!",
				Type().ToString(), SCR_RespawnComponent),
				LogLevel.ERROR);		
		}

		m_PlayerController = SCR_PlayerController.Cast(owner);
		if (!m_PlayerController)
		{
			Print(string.Format("%1 is not attached in %2 hierarchy! (%1 should be a child of %3!)",
				Type().ToString(), SCR_PlayerController, SCR_RespawnComponent),
				LogLevel.ERROR);
		}
		
		m_PlyFactionAffil = SCR_PlayerFactionAffiliationComponent.Cast(owner.FindComponent(SCR_PlayerFactionAffiliationComponent));

		m_RespawnComponent.GetOnRespawnReadyInvoker_O().Insert(SetReady);
		m_RespawnComponent.GetOnRespawnResponseInvoker_O().Insert(SetNotReady);

		SCR_WelcomeScreenComponent welcomeScreenComp = SCR_WelcomeScreenComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_WelcomeScreenComponent));
		if (!welcomeScreenComp)
			m_bWelcomeClosed = true;
	}

	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

#ifdef ENABLE_DIAG
		if (SCR_RespawnComponent.Diag_IsCLISpawnEnabled())
			return;
#endif
		if (!m_bReady)
			return;

		if (CanOpenWelcomeScreen())
		{
			if (!IsWelcomeScreenOpen())
				GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.WelcomeScreenMenu);
			return;

		}
		else if (IsWelcomeScreenOpen())
		{
			CloseWelcomeScreen();
			return;
		}

		if (CanOpenMenu())
		{
			if (!m_bFirstOpen && HasFaction() && !SCR_RoleSelectionMenu.GetRoleSelectionMenu())
			{
				if (!SCR_DeployMenuMain.GetDeployMenu())
					SCR_DeployMenuMain.OpenDeployMenu();
			}
			else
			{
				if (!SCR_RoleSelectionMenu.GetRoleSelectionMenu())
					SCR_RoleSelectionMenu.OpenRoleSelectionMenu();
				
				m_bFirstOpen = false;
			}
		}
		else if (IsMenuOpen())
			CloseMenu();
	}
	
	protected bool IsMenuOpen()
	{
		return SCR_DeployMenuMain.GetDeployMenu() || SCR_RoleSelectionMenu.GetRoleSelectionMenu();
	}

	protected bool IsWelcomeScreenOpen()
	{
		return GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.WelcomeScreenMenu);
	}

	protected void CloseWelcomeScreen()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.WelcomeScreenMenu);
	}

	protected void CloseMenu()
	{
		if (m_PlayerController.GetPlayerId() != GetGame().GetPlayerController().GetPlayerId())
			return;
		
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.WelcomeScreenMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.RespawnSuperMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.RoleSelectionDialog);

		if (s_OnMenuClose)
			s_OnMenuClose.Invoke();
	}
	
	protected bool HasFaction()
	{
		return (m_PlyFactionAffil && m_PlyFactionAffil.GetAffiliatedFaction());
	}

	protected bool CanOpenMenu()
	{
		if (!m_bWelcomeClosed)
			return false;

		bool hasEntity = (m_PlayerController.IsPossessing() || m_PlayerController.GetControlledEntity());
		bool hasDeadEntity = false;
		ChimeraCharacter character = ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());
		if (character)
			hasDeadEntity = character.GetCharacterController().IsDead();
		
		bool canOpen = (
			!SCR_EditorManagerEntity.IsOpenedInstance() && 
			(!hasEntity || hasDeadEntity) && 
			(SCR_BaseGameMode.Cast(GetGame().GetGameMode()).GetState() != SCR_EGameModeState.POSTGAME)
		);

		return canOpen;
	}
	
	protected bool CanOpenWelcomeScreen()
	{
		if (m_bWelcomeClosed)
			return false;
		bool hasEntity = (m_PlayerController.IsPossessing() || m_PlayerController.GetControlledEntity());
		bool hasDeadEntity = false;

		ChimeraCharacter character = ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());
		if (character)
			hasDeadEntity = character.GetCharacterController().IsDead();
		
		bool canOpen = (
			!SCR_EditorManagerEntity.IsOpenedInstance() && 
			(!hasEntity || hasDeadEntity) && 
			(SCR_BaseGameMode.Cast(GetGame().GetGameMode()).GetState() != SCR_EGameModeState.POSTGAME)
		);

		return canOpen;		
	}

	protected void SetReady()
	{
		m_bReady = true;
	}
	
	void SetWelcomeClosed()
	{
		m_bWelcomeClosed = true;
	}

	protected void SetNotReady(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response)
	{
		if (response == SCR_ESpawnResult.OK)
		{
			SCR_DeployMenuMain.CloseDeployMenu();
			m_bReady = false;
		}
	}

	static OnDeployMenuCloseInvoker SGetOnMenuClosed()
	{
		if (!s_OnMenuClose)
			s_OnMenuClose = new OnDeployMenuCloseInvoker();
		
		return s_OnMenuClose;
	}
	
	void ~SCR_PlayerDeployMenuHandlerComponent()
	{
		if (s_OnMenuClose)
			delete s_OnMenuClose;
	}
};