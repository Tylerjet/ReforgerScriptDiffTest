[WorkbenchPluginAttribute(name: "Start Dedicated Server", shortcut: "Ctrl+Shift+D", wbModules: {"WorldEditor"}, awesomeFontCode: 0xf233)]
class DedicatedServerPlugin: WorldEditorPlugin
{
	[Attribute(desc: "Enable to start play mode and automatically connect to dedicated server.")]
	protected bool m_bPlayAndConnect;
	
	[Attribute(desc: "Server configuration. Dedicated server cannot be launched without this!")]
	protected ref DedicatedServerPluginCLI m_Config;
	
	[Attribute("ArmaReforgerServer_Internal.exe", desc: "Executable file used to start the server.")]
	protected string m_sExecutable;
	
	[Attribute("", desc: "Profile used on the server.")]
	protected string m_sProfile;
	
	[Attribute("0", desc: "Suppresses all sorts of errors (like asserts), so the game continues without stalling and disconnecting clients.")]
	protected bool m_bNoThrow;
	
	[Attribute("0", desc: "Logs connection information to output.")]
	protected bool m_bLogRPL;
	
	[Attribute("", desc: "Basic backend logging. Can have one of following values:\n  \"Http\" - turn on logging of Http Response & Requests\n  <filename> - save data to profile\.backend folder")]
	protected string m_sLogBackend;
	
	[Attribute("30", desc: "Maximum FPS on the server. When 0, no limit will be set.")]
	protected int m_iMaxFPS;
	
	[Attribute(desc: "Additional command line params.")]
	protected string m_sParams;
	
	protected ProcessHandle m_ServerHandle = null;
	
	private string GetAddonsDirCLI()
	{
		string addonsDir;
		System.GetCLIParam("addonsDir", addonsDir);
		return addonsDir;
	}

	private string GetAddonsCLI()
	{
		string addonIDs;
		
		array<string> addonsGUIDs = {};
		GameProject.GetLoadedAddons(addonsGUIDs);
		
		foreach (string GUID: addonsGUIDs)
		{
			if (!GameProject.IsVanillaAddon(GUID))
			{
				if (!addonIDs.IsEmpty())
					addonIDs += ",";
				
				addonIDs += GameProject.GetAddonID(GUID);
			}
		}
		
		return addonIDs;
	}

	override void Run()
	{
		if (!Workbench.ScriptDialog("Start Dedicated Server", "Configure dedicated server before running it.                                             ", this))
			return;
		
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = worldEditor.GetApi();
		
		//--- Generate run command
		string process = string.Format("%1 -gproj=\"%2\"", m_sExecutable, Workbench.GetCurrentGameProjectFile());
		
		string configParam;
		if (m_Config && m_Config.GetCLI(configParam, api))
		{
			process += " " + configParam;
		}
		else
		{
			Print("Cannot start dedicated server, 'Config' setting is incomplete!", LogLevel.WARNING);
			return;
		}

		string addonsDir = GetAddonsDirCLI();
		if (!addonsDir.IsEmpty())
			process += " -addonsDir " + addonsDir;

		string addonIDs = GetAddonsCLI();
		if (!addonIDs.IsEmpty())
			process += " -addons " + addonIDs;
		
		if (m_sProfile)
			process += string.Format(" -profile %1", m_sProfile);
		
		if (m_bNoThrow)
			process += " -nothrow";
		
		if (m_bLogRPL)
			process += " -rpl-log-con";
		
		if (m_sLogBackend)
			process += string.Format(" -backendLog %1", m_sLogBackend);

		if (m_iMaxFPS > 0)
			process += string.Format(" -maxFPS %1", m_iMaxFPS);
		
		if (m_sParams)
			process += " " + m_sParams;
		
		//--- Start dedicated server
		m_ServerHandle = Workbench.RunProcess(process);
		PrintFormat("Starting dedicated server: %1", process);
		
		//--- Connect to the server
		if (m_bPlayAndConnect)
			worldEditor.SwitchToGameMode();
	}
	override void OnGameModeStarted(string worldName, string gameMode, bool playFromCameraPos, vector cameraPosition, vector cameraAngles)
	{
		if (!m_bPlayAndConnect || !m_ServerHandle)
			return;
		
		GameStateTransitions.RequestServerConnectTransition(new ClientSessionParams("127.0.0.1"));
	}
	override void OnGameModeEnded()
	{
		//--- Close the server when returning from play mode
		if (m_ServerHandle)
			Workbench.KillProcess(m_ServerHandle);
	}
	
	[ButtonAttribute("Run", true)]
	bool ButtonRun()
	{
		return true;
	}
	[ButtonAttribute("Close")]
	bool ButtonClose()
	{
		return false;
	}
};

[BaseContainerProps()]
class DedicatedServerPluginCLI
{
	bool GetCLI(out string outParam, WorldEditorAPI api);
};
[BaseContainerProps()]
class DedicatedServerPluginCLI_Server: DedicatedServerPluginCLI
{
	[Attribute(desc: "World which the server will start.\nWhen empty, currently opened world will be used.", uiwidget: UIWidgets.ResourceNamePicker, params: "ent")]
	protected ResourceName m_World;
	
	[Attribute(desc: "Mission configuration, can enable settings like metabolism / vehicles, set loading screen image, etc.", uiwidget: UIWidgets.ResourceNamePicker, params: "conf class=ServerConfig")]
	protected ResourceName m_MissionHeader;
	
	override bool GetCLI(out string outParam, WorldEditorAPI api)
	{
		ResourceName worldPath = m_World;
		if (!worldPath)
			api.GetWorldPath(worldPath);
		
		if (m_MissionHeader)
			outParam = string.Format("-server %1 -MissionHeader %2", worldPath.GetPath(), m_MissionHeader.GetPath());
		else
			outParam = string.Format("-server %1", worldPath.GetPath());
		
		return !worldPath.IsEmpty();
	}
};
[BaseContainerProps()]
class DedicatedServerPluginCLI_DServer: DedicatedServerPluginCLI
{
	[Attribute(desc: "Server configuration file.", uiwidget: UIWidgets.ResourceNamePicker, params: "conf class=ServerConfig")]
	protected ResourceName m_DServer;
	
	override bool GetCLI(out string outParam, WorldEditorAPI api)
	{
		outParam = string.Format("-dserver %1", m_DServer.GetPath());
		return !m_DServer.IsEmpty();
	}
};
[BaseContainerProps()]
class DedicatedServerPluginCLI_Config: DedicatedServerPluginCLI
{
	[Attribute(desc: "JSON server configuration file.", uiwidget: UIWidgets.ResourceNamePicker, params: "json")]
	protected ResourceName m_Config;
	
	override bool GetCLI(out string outParam, WorldEditorAPI api)
	{
		outParam = string.Format("-config %1", m_Config.GetPath());
		return !m_Config.IsEmpty();
	}
};