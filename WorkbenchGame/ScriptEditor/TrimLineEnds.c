[WorkbenchPluginAttribute(name: "Trim Line Ends", description: "", shortcut: "Ctrl+Shift+K", wbModules: { "ScriptEditor" }, awesomeFontCode: 0xf036)]
class RemoveEmptySpacesPlugin : WorkbenchPlugin
{
	override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);

		int removedCharacters = 0;
		int cleanedLines = 0;
		int linesCount = scriptEditor.GetLinesCount();

		int lineLength;
		string lineContent;
		for (int lineNumber = 0; lineNumber < linesCount; lineNumber++)
		{
			scriptEditor.GetLineText(lineContent, lineNumber);
			if (lineContent.EndsWith("	") || lineContent.EndsWith(" "))
			{
				cleanedLines++;
				lineLength = lineContent.Length() -1;
				while (lineContent.EndsWith("	") || lineContent.EndsWith(" "))
				{
					lineContent = lineContent.Substring(0, lineLength--);
					removedCharacters++;
				}
				scriptEditor.SetLineText(lineContent, lineNumber);
			}
		}

		string filePath;
		scriptEditor.GetCurrentFile(filePath);
		array<string> splits = {};
		filePath.Split("/", splits, true);
		PrintFormat("%1 cleaned up - %2 line(s) amended, %3 character(s) removed [%4]", splits[splits.Count() -1], cleanedLines, removedCharacters, filePath);
	}
};
