class SVNPluginBase: WorkbenchPlugin
{
	string GetCommandLine()
	{
		return "";
	}
	override void Run()
	{
		ScriptEditor mod = Workbench.GetModule(ScriptEditor);
		if (mod)
		{
			string file;
			string absPath;
			if (mod.GetCurrentFile(file) && Workbench.GetAbsolutePath(file, absPath))
			{
				int line = mod.GetCurrentLine();
				string command = GetCommandLine();
				command.Replace("$path", "\""+ absPath + "\"");
				Workbench.RunCmd(command);
			}
		}
	}
};
[WorkbenchPluginAttribute("SVN Log", "Show SVN log of the selected file", "alt+shift+l", "", {"ScriptEditor"}, "SVN", 0xf1de)]
class SVNLogPlugin: SVNPluginBase
{
	[Attribute("TortoiseProc /command:log /path:$path", UIWidgets.EditBox)]
	string CommandLine;
	
	override string GetCommandLine()
	{
		return CommandLine;
	}
	override void Configure()
	{
		Workbench.ScriptDialog("Configure SVN Log", "Usage: \n$path - will be replaced with file name", this);
	}
	
	[ButtonAttribute("OK")]
	void OkButton() {}
};

[WorkbenchPluginAttribute("SVN Log", "Show SVN log of the selected file", "alt+shift+l", "", {"ResourceManager"}, "SVN", 0xf1de)]
class SVNLogRMPlugin: SVNLogPlugin
{
	override void Run()
	{
		ResourceManager mod = Workbench.GetModule(ResourceManager);
		if (mod)
		{
			array<ResourceName> selection = new array<ResourceName>;
			SCR_WorkbenchSearchResourcesCallbackArray context = new SCR_WorkbenchSearchResourcesCallbackArray(selection);
			mod.GetResourceBrowserSelection(context.Insert, true);
			
			string absPath;
			foreach (ResourceName resource : selection)
			{
				string command = CommandLine;
				Workbench.GetAbsolutePath(resource.GetPath(), absPath);
				command.Replace("$path", "\""+ absPath + "\"");
				Workbench.RunCmd(command);
			}
		}
	}
};
[WorkbenchPluginAttribute("SVN Diff", "Show local SVN difference of the selected file", "alt+shift+i", "", {"ScriptEditor"}, "SVN", 0xf002)]
class SVNDiffPlugin: SVNPluginBase
{
	[Attribute("TortoiseProc /command:diff /path:$path", UIWidgets.EditBox)]
	string CommandLine;
	
	override string GetCommandLine()
	{
		return CommandLine;
	}
	
	override void Configure()
	{
		Workbench.ScriptDialog("Configure SVN Diff", "Usage: \n$path - will be replaced with file name", this);
	}
};

[WorkbenchPluginAttribute("SVN Blame", "Show local SVN difference of the selected file", "alt+shift+b", "", {"ScriptEditor"},"SVN", 0xf4fc)]
class SVNBlamePlugin: SVNPluginBase
{
	[Attribute("TortoiseProc /command:blame /path:$path /startrev:1 /endrev:-1 /ignoreeol /ignoreallspaces /line:$line", UIWidgets.EditBox)]
	string CommandLine;
	
	override string GetCommandLine()
	{
		return CommandLine;
	}
	override void Configure()
	{
		Workbench.ScriptDialog("Configure SVN Blame", "Usage: \n$path - will be replaced with file name\n$line - will be replaced with current line number", this);
	}
	
};