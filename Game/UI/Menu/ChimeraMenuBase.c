//------------------------------------------------------------------------------------------------
//! Menu presets
enum ChimeraMenuPreset : ScriptMenuPresetEnum
{
	MainMenu,
	PlayMenu,
	EditorSelectionMenu,
	CareerMenu,
	PauseMenu,
	MultiplayerDialog,
	LoginDialog,
	LoginDialogConsole,
	ServerBrowserMenu,
	ProfileDialog,
	ErrorDialog,
	MapMenu,
	WorldEditorIngameMenu,
	EditorMenu,
	EditorModesDialog,
	EditorActionListDialog,
	EditorEntityListDialog,
	EditorPlayerListDialog,
	EditorFactionListDialog,
	EditorPlacingMenuDialog,
	EditorAttributesDialog,
	EditorBrowserDialog,
	EditorSaveDialog,
	EditorLoadDialog,
	FeedbackDialog,
	Inventory20Menu,
	ReportItemDialog,
	ContentBrowser,
	ContentBrowserDetailsMenu,
	RespawnSuperMenu,
	WelcomeScreenMenu,
	DebriefingScreenMenu,
	SettingsSuperMenu,
	PlayerListMenu,
	FieldManualDialog,
	WidgetLibraryMenu,
	PickAssignee,
	TaskDetail,
	EndgameScreen,
	StartScenarioDialog,
	ScenarioMenu,
	ProfileSuperMenu,
	NewsDialog,
	GalleryDialog,
	ConfigurableDialog,
	ScrollTest,
	LoadingOverlay,
	CreateAccountDialog,
	CreditsMenu,
	CreditsLicensesMenu,
	WelcomeDialog,
	GroupSettingsDialog,
	CareerProfileMenu,
	CampaignBuildingPlacingMenuDialog,
	GroupMenu,
	GroupFlagDialog,
	RoleSelectionDialog,
	GamepadRemovalDialog
};

//------------------------------------------------------------------------------------------------
//! Constant variables used in various menus.
// #define MPTEST // TODO: remove

//------------------------------------------------------------------------------------------------
class ChimeraMenuBase : MenuBase
{
	// Editbox type check
	protected const string INPUT_CONTEXT_EDIT = "MenuTextEditContext";
	protected bool m_bTextEditActive = false;
	ref ScriptInvoker m_OnTextEditContextChange = new ScriptInvoker;
	ref ScriptInvoker m_OnUpdate = new ScriptInvoker; // (float tDelta)

	protected static ChimeraMenuBase m_ThisMenu;

	//------------------------------------------------------------------------------------------------
	static ChimeraMenuBase CurrentChimeraMenu()
	{
		return m_ThisMenu;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuInit()
	{
		m_ThisMenu = this;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		// Check edit box context active
		bool ctxActive = GetGame().GetInputManager().IsContextActive(INPUT_CONTEXT_EDIT);
		m_OnTextEditContextChange.Invoke(ctxActive);

		// Invoke OnUpdate
		m_OnUpdate.Invoke(tDelta);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpened();
		SCR_MenuHelper.OnMenuOpen(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpened()
	{
		super.OnMenuOpened();
		SCR_MenuHelper.OnMenuOpened(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		SCR_MenuHelper.OnMenuClose(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();
		SCR_MenuHelper.OnMenuFocusGained(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();
		SCR_MenuHelper.OnMenuFocusLost(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		SCR_MenuHelper.OnMenuShow(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();
		SCR_MenuHelper.OnMenuHide(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns parent menu of a widget
	static ChimeraMenuBase GetOwnerMenu(Widget w)
	{
		auto game = GetGame();

		if (!game)
			return null;

		auto menuManager = game.GetMenuManager();

		if (!menuManager)
			return null;

		MenuBase menuBase = menuManager.GetOwnerMenu(w);

		if (!menuBase)
			return null;

		ChimeraMenuBase m = ChimeraMenuBase.Cast(menuBase);

		if (!m)
			return null;

		return m;
	}
};
