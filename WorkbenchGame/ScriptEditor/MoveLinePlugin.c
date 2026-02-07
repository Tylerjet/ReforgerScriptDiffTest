[WorkbenchPluginAttribute(name: "Move Line Up", description: "", shortcut: "Alt+Up", wbModules: { "ScriptEditor" }, category: "Move Line", awesomeFontCode: 0xf062)]
class MoveLineUpPlugin : WorkbenchPlugin
{
	// option idea: try to auto indent?

	override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		int currentLineIndex = scriptEditor.GetCurrentLine();
		if (currentLineIndex == 0)
		{
			return;
		}
		int aboveLineIndex = currentLineIndex - 1;

		string aboveLine;
		string currentLine;

		scriptEditor.GetLineText(aboveLine, aboveLineIndex);
		scriptEditor.GetLineText(currentLine, currentLineIndex);

		scriptEditor.SetLineText(currentLine, aboveLineIndex);
		scriptEditor.SetLineText(aboveLine, currentLineIndex);
	}
};

[WorkbenchPluginAttribute(name: "Move Line Down", description: "", shortcut: "Alt+Down", wbModules: { "ScriptEditor" }, category: "Move Line", awesomeFontCode: 0xf063)]
class MoveLineDownPlugin : WorkbenchPlugin
{
	// option idea: try to auto indent?

	override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		int currentLineIndex = scriptEditor.GetCurrentLine();
		if (currentLineIndex == scriptEditor.GetLinesCount() - 1)
		{
			return;
		}
		int belowLineIndex = currentLineIndex + 1;

		string currentLine;
		string belowLine;

		scriptEditor.GetLineText(currentLine, currentLineIndex);
		scriptEditor.GetLineText(belowLine, belowLineIndex);

		scriptEditor.SetLineText(currentLine, belowLineIndex);
		scriptEditor.SetLineText(belowLine, currentLineIndex);
	}
};
