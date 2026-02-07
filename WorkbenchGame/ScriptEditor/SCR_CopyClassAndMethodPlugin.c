#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Copy Class and Method Name", description: "Copy class and method name based on the current line.\nWhen it is inside of a method, the format is 'MyClass.MyMethod()', otherwise the result is just 'MyClass'", shortcut: "Ctrl+Shift+C", wbModules: { "ScriptEditor" }, awesomeFontCode: 0xF0C5)]
class SCR_CopyClassAndMethodPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);

		string fileName;
		scriptEditor.GetCurrentFile(fileName);

		const string bracketOpen = "{";
		const string bracketClose = "}";
		const string commentOpen = "/" + "*";
		const string commentClose = "*" + "/";

		//--- Get scope hierarchy
		string text;
		array<int> scopes = {};
		bool isComment;
		bool extended;
		for (int l = 0, lineCount = scriptEditor.GetCurrentLine(); l < lineCount; l++)
		{
			scriptEditor.GetLineText(text, l);
			text.Replace(" ", string.Empty);
			text.Replace("\t", string.Empty);

			if (!isComment)
			{
				if (text.StartsWith(bracketOpen))
					scopes.Insert(l - 1);
				else if (text.StartsWith(bracketClose))
					scopes.Resize(scopes.Count() - 1);
				else if (text.StartsWith(commentOpen))
					isComment = true;
			}
			else if (text.StartsWith(commentClose))
			{
				isComment = false;
			}

			//--- When a method is on the current line or on the line above, extend the search to include it
			if (!extended && l == lineCount - 1 && scopes.Count() == 1)
			{
				lineCount += 2;
				extended = true;
			}
		}

		if (scopes.IsEmpty())
			scopes.Insert(scriptEditor.GetCurrentLine());

		string result;
		string className;
		scriptEditor.GetLineText(className, scopes[0]);

		array<string> lineArray = {};
		className.Split(" ", lineArray, false);
		if (lineArray.Count() < 2)
		{
			Print("Nothing copied to clipboard, selected line is not inside of a class.", LogLevel.DEBUG);
			return;
		}

		className = lineArray[1];
		className.Replace(":", string.Empty);

		if (scopes.Count() == 1)
			scopes.Insert(scriptEditor.GetCurrentLine());

		string methodName;
		scriptEditor.GetLineText(methodName, scopes[1]);
		methodName.Split("(", lineArray, false);
		if (lineArray.Count() >= 2)
		{
			methodName = lineArray[0];
			methodName.Split(" ", lineArray, false);
			if (lineArray.Count() >= 2)
				result = string.Format("%1.%2()", className, lineArray[lineArray.Count() - 1]);
		}

		if (result.IsEmpty())
			result = className;

		Print(string.Format("Copied to clipboard: \"%1\"", result), LogLevel.DEBUG);
		System.ExportToClipboard(result);
	}
}
#endif // WORKBENCH
