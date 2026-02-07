[BaseContainerProps()]
class PeerConfig
{
	[Attribute("true", UIWidgets.CheckBox)]
	bool Enabled;
	
	[Attribute("-1", UIWidgets.EditBox, "Horizontal coordinate on the screen if running in window mode. Default if -1.")]
	int	X;

	[Attribute("-1", UIWidgets.EditBox, "Vertical coordinate on the screen if running in window mode. Default if -1.")]
	int	Y;

	[Attribute("-1", UIWidgets.EditBox, "Width of the peer window if running in window mode. Default if -1.")]
	int	Width;

	[Attribute("-1", UIWidgets.EditBox, "Height of the peer window if running in window mode. Default if -1.")]
	int	Height;

	[Attribute("true", UIWidgets.CheckBox, "If true the peer will run in window mode.")]
	bool Windowed;

	[Attribute("true", UIWidgets.CheckBox, "If true the peer will run at full speed even when its window doesn't have focus.")]
	bool ForceUpdate;

	[Attribute("true", UIWidgets.CheckBox, "If true the peer won't steal focus from the current active window.")]
	bool NoFocus;

	[Attribute("-1", UIWidgets.EditBox, "Maximum allowed FPS. No limit if -1.")]
	int MaxFps;

	[Attribute("true", UIWidgets.CheckBox, "If true, the peer joins the running server immediately.")]
	bool RplAutoJoin;
	[Attribute("true", UIWidgets.CheckBox, "If true, any time the connection is droppped an attempt to reconnect is made.")]
	bool RplAutoReconnect;	
	[Attribute("true", UIWidgets.CheckBox, "If true, no network-related timeout will result in disconnect. Necessary for debugging. Don't use for anything else.")]
	bool RplDisableTimeout;
	
	[Attribute("PeerPlugin", UIWidgets.EditBox, "Profile name used for the peer. A suffix of <PeerIndex> is added to the name.")]
	string Profile;
	
	[Attribute("", UIWidgets.EditBox, "Custom parameters.")]
	string Params;

	ProcessHandle	Handle = null;
}

[WorkbenchPluginAttribute("PeerTool", "Runs peer clients that connect to the WB instance.", "", "", {"WorldEditor"})]
class PeerPlugin : WorldEditorPlugin
{
	[Attribute("game.exe", UIWidgets.EditBox)]
	string Executable;

	[Attribute("")]
	ref array<ref PeerConfig> PeersWindows;

	ProcessHandle Handle = null;
	
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

	void Start()
	{
		int profileIndex = 0;
		
		string addonsDir = GetAddonsDirCLI();
		string addonIDs = GetAddonsCLI();
		
		foreach(PeerConfig conf : PeersWindows)
		{
			if (!conf.Enabled)
				continue;
			
			profileIndex++; // start indexing peers from 1
			string gproj = Workbench.GetCurrentGameProjectFile();
			string sCmd = Executable + " -gproj \"" + gproj + "\" ";
						
			if (!addonsDir.IsEmpty())
			{
				sCmd += "-addonsDir " + addonsDir + " ";
			}
			
			if (!addonIDs.IsEmpty())
			{
				sCmd += "-addons " + addonIDs + " ";
			}

			if(conf.Windowed)
			{
				sCmd += "-window ";
			}

			if(conf.X != -1)
			{
				sCmd += "-posX " + conf.X + " ";
			}

			if(conf.Y != -1)
			{
				sCmd += "-posY " + conf.Y + " ";
			}

			if(conf.Width != -1)
			{
				sCmd += "-screenWidth " + conf.Width + " ";
			}

			if(conf.Height != -1)
			{
				sCmd += "-screenHeight " + conf.Height + " ";
			}

			if(conf.MaxFps > 0)
			{
				sCmd += "-maxFPS " + conf.MaxFps + " ";
			}

			if(conf.ForceUpdate)
			{
				sCmd += "-forceupdate ";
			}

			if(conf.NoFocus)
			{
				sCmd += "-nofocus ";
			}
			
			if(conf.RplAutoJoin)
			{
				sCmd += "-client ";
			}
			
			if(conf.RplAutoReconnect)
			{
				sCmd += "-rpl-reconnect ";
			}

			if(conf.RplDisableTimeout)
			{
				sCmd += "-rpl-timeout-disable ";
			}

			if(conf.Profile)
			{			
				sCmd += "-profile " + conf.Profile + profileIndex.ToString() + " ";
			}
		
			if (conf.Params)
			{
				sCmd += conf.Params + " ";
			}

			Print("Running command: " + sCmd);
			conf.Handle = Workbench.RunProcess(sCmd);
		}
	}

	void End()
	{
		foreach(PeerConfig conf : PeersWindows)
		{
			if (!conf.Handle) // what if the game mode wasn't the PeerTool?
				continue;
			
			Workbench.KillProcess(conf.Handle);
			conf.Handle = null;
		}
	}

	override void OnGameModeStarted(string worldName,
									string gameMode,
									bool	 playFromCameraPos,
									vector cameraPosition,
									vector cameraAngles)
	{
		if(gameMode == "Server localhost + PeerTool") // would consider different (if possible) solution for this. if this is changed on the C++ side, it will stop working
		{
			Start();
		}
	}

	override void OnGameModeEnded()
	{
		End();
	}

	override void Configure()
	{
		Workbench.ScriptDialog("Configure Peer tool", "Settings:", this);
	}

	[ButtonAttribute("OK")]
	void OkButton()	{}
}