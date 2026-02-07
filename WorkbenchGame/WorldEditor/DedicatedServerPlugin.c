[WorkbenchPluginAttribute(name: "Dedicated Server Tool", shortcut: "Ctrl+Shift+D", wbModules: {"WorldEditor"}, awesomeFontCode: 0xf233)]
class DedicatedServerPlugin: WorldEditorPlugin
{
	[Attribute(desc: "Enable to start play mode and automatically connect to dedicated server.", category: "Server")]
	protected bool m_bPlayAndConnect;
	
	[Attribute(desc: "Server configuration. Dedicated server cannot be launched without this!", category: "Server")]
	protected ref DedicatedServerPluginCLI m_Config;
	
	[Attribute("ArmaReforgerServer.exe", UIWidgets.FileNamePicker, desc: "Executable file used to start the server.", params: "exe FileNameFormat=absolute", category: "Server")]
	protected string m_sExecutable;
	
	[Attribute("", desc: "Profile used on the server.", category: "Server")]
	protected string m_sProfile;
	
	[Attribute("0", desc: "Suppresses all sorts of errors (like asserts), so the game continues without stalling and disconnecting clients.", category: "Server")]
	protected bool m_bNoThrow;
	
	[Attribute("0", desc: "Logs connection information to output.", category: "Server")]
	protected bool m_bLogRPL;
	
	[Attribute("", desc: "Basic backend logging. Can have one of following values:\n  \"Http\" - turn on logging of Http Response & Requests\n  <filename> - save data to profile\.backend folder", category: "Server")]
	protected string m_sLogBackend;
	
	[Attribute("30", desc: "Maximum FPS on the server. When 0, no limit will be set.", category: "Server")]
	protected int m_iMaxFPS;
	
	[Attribute(desc: "Additional command line params.", category: "Server")]
	protected string m_sParams;
	
	[Attribute(desc: "Run peers defined by the PeerTool.", category: "Peers")]
	protected bool m_bRunPeers;
	
	[Attribute("ArmaReforger.exe", UIWidgets.FileNamePicker, desc: "Peer exe path", params: "exe FileNameFormat=absolute", category: "Peers")]
	string m_PeerExecutable;

	[Attribute(desc: "Peers configuration.", category: "Peers")]
	ref array<ref PeerConfig> m_PeersConfig;
	
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
		string gproj = Workbench.GetCurrentGameProjectFile();
		string process = string.Format("%1 -gproj=\"%2\"", m_sExecutable, gproj);
		
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
		if (m_bRunPeers)
		{
			int profileIndex = 0;
			
			foreach(PeerConfig conf : m_PeersConfig)
			{
				if (!conf.Enabled)
					continue;
				
				profileIndex++; // start indexing peers from 1
				string sCmd = m_PeerExecutable + " -gproj \"" + gproj + "\" ";
							
				if (!addonsDir.IsEmpty())
					sCmd += "-addonsDir " + addonsDir + " ";				
				if (!addonIDs.IsEmpty())
					sCmd += "-addons " + addonIDs + " ";
				if(conf.Windowed)
					sCmd += "-window ";
				if(conf.X != -1)
					sCmd += "-posX " + conf.X + " ";
				if(conf.Y != -1)
					sCmd += "-posY " + conf.Y + " ";
				if(conf.Width != -1)
					sCmd += "-screenWidth " + conf.Width + " ";
				if(conf.Height != -1)
					sCmd += "-screenHeight " + conf.Height + " ";
				if(conf.MaxFPS > 0)
					sCmd += "-maxFPS " + conf.MaxFPS + " ";
				if(conf.ForceUpdate)
					sCmd += "-forceupdate ";
				if(conf.NoFocus)
					sCmd += "-nofocus ";
				if(conf.RplAutoJoin)
					sCmd += "-client ";
				if(conf.RplAutoReconnect)
					sCmd += "-rpl-reconnect ";
				if(conf.RplDisableTimeout)
					sCmd += "-rpl-timeout-disable ";
				if(conf.Profile)
					sCmd += "-profile " + conf.Profile + profileIndex.ToString() + " ";
				if (conf.Params)
					sCmd += conf.Params + " ";

				Print("Running command: " + sCmd);
				conf.Handle = Workbench.RunProcess(sCmd);
				if (!conf.Handle)
					Print("Peer #" + profileIndex + " couldn't run. Check if your PeerExecutable or other settings are correct", LogLevel.ERROR);
			}
		}		
			
		// Also make the WB join the server as a peer
		if (m_bPlayAndConnect)
			worldEditor.SwitchToGameMode();
	}
	override void OnGameModeStarted(string worldName, string gameMode, bool playFromCameraPos, vector cameraPosition, vector cameraAngles)
	{
		if (!m_bPlayAndConnect || !m_ServerHandle)
			return;
		
		GameStateTransitions.RequestServerConnectTransition("127.0.0.1");
	}
	override void OnGameModeEnded()
	{
		// Close any connected peers
		foreach(PeerConfig conf : m_PeersConfig)
		{			
			if (!conf.Handle) // what if the game mode wasn't the PeerTool?
				continue;
			
			Workbench.KillProcess(conf.Handle);
			conf.Handle = null;
		}

		// Close the server when returning from play mode
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
	
	[Attribute(desc: "Optional password for logging-in as administrator (in game, type '#login <password>' in chat)")]
	protected string m_sAdminPassword;
	
	override bool GetCLI(out string outParam, WorldEditorAPI api)
	{
		ResourceName worldPath = m_World;
		if (!worldPath)
			api.GetWorldPath(worldPath);
		
		if (m_MissionHeader)
			outParam = string.Format("-server %1 -MissionHeader %2", worldPath.GetPath(), m_MissionHeader.GetPath());
		else
			outParam = string.Format("-server %1", worldPath.GetPath());
		
		if (m_sAdminPassword)
			outParam += string.Format(" -adminPassword %1", m_sAdminPassword);
		
		return !worldPath.IsEmpty();
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
