#ifdef WORKBENCH
class SCR_VCSBasePlugin : WorkbenchPlugin
{
	[Attribute(defvalue: TORTOISESVN_BLAME, desc: "Find out whodunit")]
	protected string m_sBlameCommandLine;

	[Attribute(defvalue: TORTOISESVN_DIFF, desc: "Find out local changes")]
	protected string m_sDiffCommandLine;

	[Attribute(defvalue: TORTOISESVN_LOG, desc: "Find out the file history")]
	protected string m_sLogCommandLine;

	protected static const string TORTOISESVN_BLAME = "TortoiseProc /command:blame /path:\"$path\" /startrev:1 /endrev:-1 /ignoreeol /ignoreallspaces /line:$line";
	protected static const string TORTOISESVN_DIFF = "TortoiseProc /command:diff /path:\"$path\"";
	protected static const string TORTOISESVN_LOG = "TortoiseProc /command:log /path:\"$path\"";

	protected static const string GITEXT_BLAME = "gitex blame \"$path\"";
	protected static const string GITEXT_DIFF = "gitex difftool \"$path\"";
	protected static const string GITEXT_LOG = "gitex filehistory \"$path\"";

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Set TortoiseSVN")]
	protected void ButtonTortoiseSVN()
	{
		m_sBlameCommandLine = TORTOISESVN_BLAME;
		m_sDiffCommandLine = TORTOISESVN_DIFF;
		m_sLogCommandLine = TORTOISESVN_LOG;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Set GitExtensions")]
	protected void ButtonGitExtensions()
	{
		m_sBlameCommandLine = GITEXT_BLAME;
		m_sDiffCommandLine = GITEXT_DIFF;
		m_sLogCommandLine = GITEXT_LOG;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close", true)]
	protected void ButtonClose();
}

[WorkbenchPluginAttribute("VCS Configuration", "Configure Blame/Diff/Log commands", "", "", { "ScriptEditor" }, "VCS", 0xF1DE)]
class SCR_VCSSettingsPlugin : SCR_VCSBasePlugin
{
	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		Workbench.ScriptDialog("Configure Blame/Diff/Log commands", "Parameters:\n- $path: the absolute file path (unquoted)\n- $line: the current line number", this);
	}
}

class SCR_VCSRootPlugin : SCR_VCSBasePlugin
{
	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		if (!scriptEditor)
			return;

		string file;
		string absPath;
		if (!scriptEditor.GetCurrentFile(file) || !Workbench.GetAbsolutePath(file, absPath))
		{
			Print("File " + file + " cannot be opened", LogLevel.WARNING);
			return;
		}

		int lineNumber = scriptEditor.GetCurrentLine() + 1; // 0-based

		string command = GetCommandLine();
		command.Replace("$path", absPath);
		command.Replace("$line", lineNumber.ToString());
		Workbench.RunCmd(command);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetCommandLine();
}

[WorkbenchPluginAttribute("VCS Log", "Show current file's changelog", "Alt+Shift+L", "", { "ScriptEditor" }, "VCS", 0xF1DE)]
class SCR_VCSLogScriptEditorPlugin : SCR_VCSRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return m_sLogCommandLine;
	}
}

[WorkbenchPluginAttribute("File(s) VCS Changelog", "Show Resource Browser-selected file(s) changelog or (whenever possible) the currently opened file if no files are selected", "Alt+Shift+L", "", { "ResourceManager" }, string.Empty, 0xF1DE)]
class SCR_VCSLogResourceManagerPlugin : SCR_VCSLogScriptEditorPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		if (!resourceManager)
			return;

		array<ResourceName> selection = {};
		resourceManager.GetResourceBrowserSelection(selection.Insert, true);

		if (selection.IsEmpty())
		{
			BaseContainer currentContainer = resourceManager.GetContainer(0);
			if (!currentContainer)
				return; // image, script or something

			BaseContainer ancestor = currentContainer.GetAncestor();
			if (!ancestor)
				return;

			selection.Insert(ancestor.GetResourceName());
		}

		foreach (ResourceName resourceName : selection)
		{
			string absPath;
			if (!Workbench.GetAbsolutePath(resourceName.GetPath(), absPath))
				continue;

			string command = m_sLogCommandLine;
			command.Replace("$path", absPath);
			Workbench.RunCmd(command);
		}
	}
}

[WorkbenchPluginAttribute("VCS Diff", "Show current file's local changes", "Alt+Shift+I", "", { "ScriptEditor" }, "VCS", 0xF002)]
class SCR_VCSDiffPlugin : SCR_VCSRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return m_sDiffCommandLine;
	}
}

[WorkbenchPluginAttribute("VCS Blame", "Show current file's line authors", "Alt+Shift+B", "", { "ScriptEditor" }, "VCS", 0xF4FC)]
class SCR_VCSBlamePlugin : SCR_VCSRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return m_sBlameCommandLine;
	}
}
#endif // WORKBENCH
