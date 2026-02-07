class ArmaReforgerServerParams : ServerBrowserParams
{
	// This place can combine values obtained from mission header and server
	// config, or wherever else they would come from.
	ref SCR_MissionHeader m_MissionHeader;
	string m_sServerName = "Arma Reforger Prototype Server";
	string m_sServerIP = "";
	string m_sClientConnectIP = "";
	int m_iClientConnectPort = 0;

	//------------------------------------------------------------------------------------------------
	void ArmaReforgerServerParams(SCR_MissionHeader header)
	{
		m_MissionHeader = header;
		m_sServerName = string.Format("%1 (%2)", header.m_sName, SCR_Global.GetProfileName());
	}
	
	//------------------------------------------------------------------------------------------------
	void ~ArmaReforgerServerParams()
	{
		m_MissionHeader = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create new instance of ArmaReforgerServerParams from provided server config
	static ArmaReforgerServerParams CreateInstance(notnull SCR_ServerConfig serverConfig)
	{
		ref SCR_MissionHeader mission = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(serverConfig.GetServerMissionHeader()));
		ref ArmaReforgerServerParams params = new ArmaReforgerServerParams( mission );
		
		string serverName = serverConfig.GetServerName();
		if (serverName.Contains("%profilename"))
			serverName.Replace("%profilename", SCR_Global.GetProfileName());
		
		string serverIP = serverConfig.GetServerAddress();
		if (serverIP != string.Empty)
			params.m_sServerIP = serverIP;
		
		params.m_sServerName = serverName;
		params.m_sClientConnectIP = serverConfig.GetClientConnectAddress();
		params.m_iClientConnectPort = serverConfig.GetClientConnectPort();
		return params;
	}

	// Implement accessors to data. Unimplemented methods will still provide default values.

	override string GetName() { return m_sServerName; }
	override string GetScenarioName() { return m_MissionHeader.m_sName; }
	override string GetScenarioModId() { return m_MissionHeader.m_sOwner; }
	override string GetHostModId() { return m_MissionHeader.m_sOwner; }
	override string GetDescription() { return m_MissionHeader.m_sDescription; }
	override int GetMaxPlayers() { return m_MissionHeader.m_iPlayerCount; }
	// override int GetGameMode() { return m_MissionHeader.m_sGameMode; } // int vs. string mismatch

	override string GetHostIp()
	{
		string clientConnectIp = m_sClientConnectIP;
		if (clientConnectIp != string.Empty)
			return clientConnectIp;

		return super.GetHostIp();
	}

	override int GetHostPort()
	{
		int clientConnectPort = m_iClientConnectPort;
		if (clientConnectPort != 0)
			return clientConnectPort;

		return super.GetHostPort();
	}
};

class HeaderFileCallback
{
	array<ref SCR_MissionHeader> m_aHeaders;
	ref array<string> m_aPaths = {};
	WorkshopApi api;
	
	void FindFilesCallback(string fileName, FileAttribute attributes = 0, string filesystem = string.Empty)
	{
		ref SCR_MissionHeader header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader( fileName ));
		header.m_sOwner = filesystem;
		m_aHeaders.Insert(header);
		m_aPaths.Insert(string.Format(fileName));
	}
};

class HeaderSearchCallback
{
	string addon;
	string scenario;
	WorkshopApi api;
	ref SCR_MissionHeader output;
	
	void FindFilesCallback(string fileName, FileAttribute attributes = 0, string filesystem = string.Empty)
	{
		if (filesystem == addon)
		{
			ref SCR_MissionHeader header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader( fileName ));
			if (header.m_sName == scenario)
				output = header;
		}
	}
};

// TODO: Solve situation with addons: What are they doing there?
// 

//------------------------------------------------------------------------------------------------
class WorldSelectionMenuBase: ChimeraMenuBase
{
	[MenuBindAttribute()]
	ButtonWidget Play;
	
	[MenuBindAttribute()]
	ButtonWidget Back;

	[MenuBindAttribute()]
	ButtonWidget Host; // TODO: Remove
	
	[MenuBindAttribute()]
	ButtonWidget Configure; // TODO: Remove

	protected float m_fPadding = 4;
	protected float m_fGridElementHeight = 300;
	protected int m_iColumnCount;

	protected ResourceName m_GridElementLayout = "{7348E372C62DFD8E}UI/layouts/Menus/WorldSelection/WorldSelectionElement.layout";
	protected string m_sSectionName = "PLAY";

	// Names of elements in layout
	protected string m_sElementTitle = "MissionTitle";
	protected string m_sElementDescription = "MissionDescription";
	protected string m_sElementIcon = "BackgroundImage";
	protected string m_sWorldHeadersFolder = "Missions";
	protected string m_sWorldHeadersPattern = ".conf";

	protected Widget m_wRoot;
	protected Widget m_LastFocused;
	protected WorkspaceWidget m_Workspace;
	protected EGameFlags m_eFocusedGameflags;
	protected SCR_MissionHeader m_FocusedMissionHeader;
	protected int m_iFocusedAddon;
		
	protected ref array<Widget> m_aGridButtons = new array<Widget>();
	protected ref array<Widget> m_aAddonButtons = new array<Widget>();
	protected ref array<string> m_aAddons = new array<string>();

		protected ref array<ref SCR_MissionHeader> m_aGridHeaders = new array<ref SCR_MissionHeader>(); // TODO: Unify worlds, missions, addons, workshop items to the same base class
	protected ref array<string> m_aMissionFolderPaths = new ref array<string>;
	protected ref array<string> m_aMissionPaths = new ref array<string>;
	protected bool m_bListAddons = false;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_Workspace = GetGame().GetWorkspace();
		
		m_wRoot = GetRootWidget();
		if (!m_wRoot)
			return;
		
		TextWidget menuName = TextWidget.Cast(m_wRoot.FindAnyWidget("HeaderText"));
		if (menuName)
			menuName.SetText(m_sSectionName);
		
		if (m_aGridHeaders)
		{
			m_aGridHeaders.Clear();
			
			foreach (string path : m_aMissionPaths)
			{
				LoadMissionFile(path);
			}
			
			foreach (string path : m_aMissionFolderPaths)
			{
				LoadMissionFolder(path);
			}
		}
		
		GameProject.GetAvailableAddons(m_aAddons);
		
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		
		inputManager.AddActionListener("MenuSelect",EActionTrigger.UP, Action_OnMenuSelect);
		inputManager.AddActionListener("MenuBack",EActionTrigger.PRESSED, Action_OnMenuBack);
	}
	
	// TODO: Reimplement once there is need for it again
	//------------------------------------------------------------------------------------------------
	protected void SetLastFocus()
	{
		m_LastFocused = ButtonWidget.Cast(GetGame().GetWorkspace().GetFocusedWidget());
	}
	
	static MenuBase s_InvalidMissionDialog;
	
	//------------------------------------------------------------------------------------------------
	static void InvalidMissionConfirm()
	{
		if (s_InvalidMissionDialog)
		{
			s_InvalidMissionDialog.Close();
			s_InvalidMissionDialog = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowInvalidMissionDialog()
	{
		auto dialog = GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.SimpleDialog);		
		s_InvalidMissionDialog = dialog;
		if (dialog)
		{
			dialog.SetLabel("Title", "ERROR");
			dialog.SetLabel("Text", "Selected mission could not be loaded.");
			dialog.SetLabel("ConfirmText", "OK");
			dialog.BindItem("Confirm", InvalidMissionConfirm);
		}
	}

	//------------------------------------------------------------------------------------------------
	[MenuBindAttribute()]
	void Play()
	{
		if (m_FocusedMissionHeader)
		{
			GetGame().SetGameFlags(m_eFocusedGameflags, false);
			
			SCR_UISoundEntity.SoundEvent(UISounds.CLICK);
			
			if (!GameStateTransitions.RequestMissionChangeTransition(m_FocusedMissionHeader))
			{
				// Show error dialog?
				MenuManager menuManager = GetGame().GetMenuManager();
				if (menuManager)
				{
					//menuManager.OpenMenu(ChimeraMenuPreset.WorkshopSelectionMenu);
					ShowInvalidMissionDialog();
				}
			}
			else
			{
				GetGame().GetMenuManager().CloseAllMenus();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[MenuBindAttribute()]
	void Host()
	{
		if (m_FocusedMissionHeader)
		{
			GetGame().SetGameFlags(m_eFocusedGameflags, false);

			bool res = GameStateTransitions.RequestPublicServerTransition(m_FocusedMissionHeader, new ArmaReforgerServerParams(m_FocusedMissionHeader));
			if (res)
				GetGame().GetMenuManager().CloseAllMenus();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[MenuBindAttribute()]
	void Back()
	{
		if (IsFocused())
			Close();
	}
	
	//------------------------------------------------------------------------------------------------
	void Action_OnMenuSelect()
	{
		if (!Play.IsEnabled())
		{
			SCR_UISoundEntity.SoundEvent(UISounds.CLICK_CANCEL);
			return;
		}
		
		SCR_UISoundEntity.SoundEvent(UISounds.CLICK);
		if (m_iFocusedAddon != -1)
			PlayAddon(m_iFocusedAddon);
		else
			Play();
	}
	
	//------------------------------------------------------------------------------------------------
	void Action_OnMenuBack()
	{
		Back();
	}
	
	// Placeholder
	//------------------------------------------------------------------------------------------------
	protected void CreateGridWidgets(array<ref SCR_MissionHeader> headers, array<string> addons)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DestroyGridWidgets()
	{
		if (m_aGridButtons)
		{
			int cnt = m_aGridButtons.Count();
			for (int i = cnt-1; i >= 0; i--)
			{
				m_aGridButtons[i].RemoveFromHierarchy();
				m_aGridButtons[i] = null;
			}
			
			m_aGridButtons.Clear();
		}
		
		if (m_aAddonButtons)
		{
			int cnt = m_aAddonButtons.Count();
			for (int i = cnt-1; i >= 0; i--)
			{
				m_aAddonButtons[i].RemoveFromHierarchy();
				m_aAddonButtons[i] = null;
			}
			
			m_aAddonButtons.Clear();
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DestroyWorldHeaders()
	{
		int count = m_aGridHeaders.Count();
		for (int i = 0; i < count; i++)
		{
			m_aGridHeaders[i] = null;
		}
		
		m_aGridHeaders.Clear();
	}

	//------------------------------------------------------------------------------------------------
	bool LoadMissionFile(string missionPath)
	{
		ref SCR_MissionHeader header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(missionPath));
		if (header && m_aGridHeaders.Find(header) == -1)
		{
			m_aGridHeaders.Insert(header);
			return true;
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadMissionFolder(string folderPath)
	{
		array<string> files = {};
		HeaderFileCallback callback = new HeaderFileCallback;
		callback.m_aHeaders = m_aGridHeaders;
		callback.api = GetGame().GetBackendApi().GetWorkshop();
		System.FindFiles(callback.FindFilesCallback, folderPath, m_sWorldHeadersPattern);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		DestroyGridWidgets();
		DestroyWorldHeaders();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		Action_OnMenuSelect();
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (!m_aGridHeaders || !m_aGridButtons)
			return super.OnFocus(w, x, y);
		
		// TODO: volatile, unhack
		int selectedIndex = m_aGridButtons.Find(w);
		
		// If valid, set found header as 'm_FocusedMissionHeader'
		// else set that variable to null
		// Toggle 'Play', 'Host' and 'Back' availibility based on if header is valid
		int headerCount = m_aGridHeaders.Count();
		if (selectedIndex < m_aGridHeaders.Count() && selectedIndex >= 0)
		{
			m_FocusedMissionHeader 	= m_aGridHeaders[selectedIndex];
			m_eFocusedGameflags 	= m_FocusedMissionHeader.m_eDefaultGameFlags;
			
			if (m_FocusedMissionHeader.IsMultiplayer())
			{
				Host.SetEnabled(true);
				WidgetAnimator.PlayAnimation(Host,WidgetAnimationType.Opacity,1,WidgetAnimator.FADE_RATE_DEFAULT);
			}
			else
			{
				Host.SetEnabled(false);
				WidgetAnimator.PlayAnimation(Host,WidgetAnimationType.Opacity,0,WidgetAnimator.FADE_RATE_DEFAULT);
			}

			string path = m_FocusedMissionHeader.GetWorldPath();
			path.ToLower();
			if (path == "worlds/sandbox/sandbox_arland.ent")
			{
				Configure.SetEnabled(true);
				WidgetAnimator.PlayAnimation(Configure,WidgetAnimationType.Opacity,1,WidgetAnimator.FADE_RATE_DEFAULT);
			}
			else
			{
				Configure.SetEnabled(false);
				WidgetAnimator.PlayAnimation(Configure,WidgetAnimationType.Opacity,0,WidgetAnimator.FADE_RATE_DEFAULT);
			}
		}
		else
		{
			m_FocusedMissionHeader = null;
		}
		
		m_iFocusedAddon = m_aAddonButtons.Find(w);
		
		return super.OnFocus(w, x, y);
	}
	
	
	//------------------------------------------------------------------------------------------------
	void PlayAddon(int idx)
	{
		string addon = m_aAddons[idx];			
			
		MenuBase dialog = GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.SimpleDialog);
		dialog.SetLabel("Title", "Please wait");
		dialog.SetLabel("Text", "Loading addon '" + addon + "'");
		dialog.GetItemWidget("Confirm").SetVisible(false);
		GetGame().GetCallqueue().CallLater(LoadAddon, 100, false, addon);
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadAddon(string addon)
	{
		Print("Switch Addon " + addon, LogLevel.DEBUG);
		GetGame().RequestReload({addon});
	}
};
