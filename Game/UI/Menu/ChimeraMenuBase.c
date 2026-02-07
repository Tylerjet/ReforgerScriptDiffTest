//------------------------------------------------------------------------------------------------
//! Menu presets
enum ChimeraMenuPreset : ScriptMenuPresetEnum
{
	MainMenu,
	ConfirmationDialog,
	PlayMenu,
	EditorSelectionMenu,
	CareerMenu,
	PauseMenu,
	MultiplayerDialog,
	LoginDialog,
	LoginDialogConsole,
	SettingsMenu,
	InventoryMenu,
	ServerBrowserMenu,
	ServerBrowserDetailsUI,
	ServerBrowserJoinDialog,
	ServerHostingDialog,
	SimpleDialog,
	ProfileDialog,
	ErrorDialog,
	InventoryPrototype,
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
	BindingMenu,
	AssetCardMenu,
	ServicesStatusDialog,
	FeedbackDialog,
	Inventory20Menu,
	SelectLoadoutMenu,
	SelectFactionMenu,
	SelectSpawnPointMenu,
	BetaWarningDialog,
	ReportItemDialog,
	ContentBrowser,
	PauseSuperMenu,
	ContentBrowserDetailsMenu,
	AddonsToolsMenu,
	RespawnSuperMenu,
	SettingsSuperMenu,
	PlayerListMenu,
	LogoutDialog,
	FieldManualDialog,
	WidgetLibraryMenu,
	PickAssignee,
	TaskDetail,
	DownloadManagerDialog,
	EndgameScreen,
	StartScenarioDialog,
	ScenarioMenu,
	ProfileSuperMenu,
	NewsDialog,
	GalleryDialog,
	ConfigurableDialog,
	KeybindChangeDialog,
	NewsletterDialog,
	ScrollTest,
	LoadingOverlay,
	CreateAccountDialog,
	TutorialDialog,
	CreditsMenu,
	WelcomeDialog,
	AddonPresetDialog,
	AddonExportDialog,
	GroupSettingsDialog,
	CareerProfileMenu
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
	static MenuBase OpenFeedbackDialog()
	{
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return null;

		MenuManager menuManager = game.GetMenuManager();
		if (!menuManager)
			return null;

		MenuBase dialog = menuManager.FindMenuByPreset(ChimeraMenuPreset.FeedbackDialog);
		if (!dialog)
			dialog = menuManager.OpenDialog(ChimeraMenuPreset.FeedbackDialog, DialogPriority.INFORMATIVE, 0, true);

		return dialog;
	}

	//------------------------------------------------------------------------------------------------
	static ChimeraMenuBase CurrentChimeraMenu()
	{
		return m_ThisMenu;
	}

	//------------------------------------------------------------------------------------------------
	static void ReloadCurrentWorld()
	{
		MissionHeader header = GetGame().GetMissionHeader();
		if (header)
		{
			GameStateTransitions.RequestMissionChangeTransition(header);
		}
		else
		{
			// In case of running the world from WB and entering the game menu (F10 ATM) there is no mission header to reload
			GameStateTransitions.RequestWorldChangeTransition(GetGame().GetWorldFile());
		}
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
