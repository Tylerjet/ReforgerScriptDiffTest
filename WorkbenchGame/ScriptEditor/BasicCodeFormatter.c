[WorkbenchPluginAttribute(
	name: "Basic Code Formatter",
	shortcut: "Ctrl+Shift+K",
	wbModules: { "ScriptEditor" },
	awesomeFontCode: 0xF036)]
class BasicCodeFormatterPlugin : WorkbenchPlugin
{
	/*
		Category: Formatting
	*/

	[Attribute(defvalue: "1", desc: "Fixes e.g 'for( int' → 'for (int', ';;' → ';', ') ;' → ');', 'NULL' → 'null', etc.", category: "Formatting")]
	protected bool m_bGeneralFormatting;

	[Attribute(defvalue: "1", desc: "Remove empty spaces at the end of code lines", category: "Formatting")]
	protected bool m_bTrimLineEnds;

	[Attribute(defvalue: "1", desc: "Replace four-spaces tabs with the tab character and removes spaces mixed with tabs", category: "Formatting")]
	protected bool m_bFixTabs;

	[Attribute(defvalue: "1", desc: "Fix method separators (//---)", category: "Formatting")]
	protected bool m_bFixMethodSeparators;

	[Attribute(defvalue: "1", desc: "Add one final line return if the file is missing one", category: "Formatting")]
	protected bool m_bAddFinalLineReturn;

	/*
		Category: Options
	*/

	[Attribute(defvalue: "1", desc: "Write fixes in console log", category: "Options")]
	protected bool m_bLogFixes;

	[Attribute(defvalue: "1", desc: "Show potential improvements report", category: "Options")]
	protected bool m_bReportFindings;

	protected static const string SPACE = " ";
	protected static const string FOUR_SPACES = " ";
	protected static const string TAB = "	";
	protected static const string METHOD_SEPARATOR = "//------------------------------------------------------------------------------------------------";

	//------------------------------------------------------------------------------------------------
	//! Running method
	// ###############
	// It is recommended to NOT use breakpoints in its root scope
	// otherwise GetCurrentFile, GetLinesCount, GetLineText etc get confused
	// ###############
	override void Run()
	{
		array<string> reportArray;
		if (m_bLogFixes)
			reportArray = {};

		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		string filePath;
		scriptEditor.GetCurrentFile(filePath);

		bool isPluginFile = filePath == __FILE__;

		bool reportFindings = !isPluginFile && m_bReportFindings;
		bool doGeneralFormatting = !isPluginFile && m_bGeneralFormatting;

		string lineContent;
		int endSpacesRemoved, endSpacesRemovedTotal;
		int spaceInTabsReplaced, spaceInTabsReplacedTotal;
		int fourSpacesReplaced, fourSpacesReplacedTotal;
		int methodSeparatorFixedTotal;
		int generalFormatting, generalFormattingTotal;
		int linesEdited;
		bool replaceLine;
		bool isPreviousLineEmptyLine;

		array<string> forbiddenDivisions = {};
		array<int> forbiddenDivisors = { 2, 4, 5, 8, 10, 20, 50, 100, 1000, 10000, 100000, 1000000 };
		foreach (int forbiddenDivisor : forbiddenDivisors)
		{
			forbiddenDivisions.Insert(" / " + forbiddenDivisor + ";");
			forbiddenDivisions.Insert(" / " + forbiddenDivisor + ")");
			forbiddenDivisions.Insert(" / " + forbiddenDivisor + " ");
		}

		array<int> doubleEmptyLineFound, newRefFound, newArrayFound, autoKeywordFound, divideByXFound, unscopedLoopFound, oneLinerFound;
		if (reportFindings)
		{
			doubleEmptyLineFound = {};
			newRefFound = {};
			newArrayFound = {};
			autoKeywordFound = {};
			divideByXFound = {};
			unscopedLoopFound = {};
			oneLinerFound = {};
		}

		/*
			Start
		*/
		int startTick = System.GetTickCount();

		for (int lineNumber = 0, linesCount = scriptEditor.GetLinesCount(); lineNumber < linesCount; lineNumber++)
		{
			replaceLine = false;
			scriptEditor.GetLineText(lineContent, lineNumber);

			if (m_bTrimLineEnds)
			{
				endSpacesRemoved = RemoveTrailingSpaces(lineContent);
				if (endSpacesRemoved > 0)
				{
					endSpacesRemovedTotal += endSpacesRemoved;
					replaceLine = true;
				}
			}

			if (m_bFixTabs)
			{
				string indentation, content;
				GetIndentAndLineContent(lineContent, indentation, content);
				bool hasSpace = indentation.Contains(SPACE);

				fourSpacesReplaced = ReplaceFourSpacesIndentByTab(lineContent);
				if (fourSpacesReplaced > 0)
				{
					fourSpacesReplacedTotal += fourSpacesReplaced;
					replaceLine = true;
				}

				spaceInTabsReplaced = ReplaceSpaceInTabIndent(lineContent);
				if (spaceInTabsReplaced > 0)
				{
					spaceInTabsReplacedTotal += spaceInTabsReplaced;
					replaceLine = true;
				}
			}

			if (m_bFixMethodSeparators)
			{
				if (FixMethodSeparator(lineContent))
				{
					methodSeparatorFixedTotal++;
					replaceLine = true;
				}
			}

			if (doGeneralFormatting)
			{
				generalFormatting = GeneralFormatting(lineContent);
				if (generalFormatting > 0)
				{
					generalFormattingTotal += generalFormatting;
					replaceLine = true;
				}
			}

			if (reportFindings)
			{
				string trimmedLineContent = lineContent.Trim();

				if (trimmedLineContent.Contains("new array<") || trimmedLineContent.Contains("new ref array<"))
					newArrayFound.Insert(lineNumber + 1);
				else // to not have both 'new ref array' and 'new ref'
				if (trimmedLineContent.Contains("new ref "))
					newRefFound.Insert(lineNumber + 1);

				if (isPreviousLineEmptyLine && trimmedLineContent.IsEmpty())
					doubleEmptyLineFound.Insert(lineNumber /* -1 */);

				isPreviousLineEmptyLine = trimmedLineContent.IsEmpty();

				if (trimmedLineContent.Contains("auto "))
					autoKeywordFound.Insert(lineNumber + 1);

				foreach (string forbiddenDivision : forbiddenDivisions)
				{
					if (trimmedLineContent.Contains(forbiddenDivision))
					{
						divideByXFound.Insert(lineNumber + 1);
						break;
					}
				}

				if (SCR_StringHelper.StartsWithAny(trimmedLineContent, { "for ", "foreach ", "while " }) && lineNumber < linesCount - 1)
				{
					string nextLine;
					scriptEditor.GetLineText(nextLine, lineNumber + 1);
					if (!nextLine.Trim().StartsWith("{"))
						unscopedLoopFound.Insert(lineNumber + 1);
					"}"; // fix indent
				}

				if (SCR_StringHelper.StartsWithAny(trimmedLineContent, { "if ", "for ", "foreach ", "while " }) && SCR_StringHelper.EndsWithAny(trimmedLineContent, { "}", ";" }))
					oneLinerFound.Insert(lineNumber + 1);
				"{"; // fix indent
			}

			if (!replaceLine)
				continue;

			scriptEditor.SetLineText(lineContent, lineNumber);
			linesEdited++;
		}
		/*
			Finish
		*/

		bool hasAddedFinalLineReturn = m_bAddFinalLineReturn && AddFinalLineReturn(scriptEditor);

		// later, with proper line insertion/deletion
		// RemoveEmptyLines();

		if (m_bLogFixes)
		{
			if (linesEdited > 0)
				reportArray.Insert(string.Format("%1 lines changed", linesEdited));
			if (generalFormattingTotal > 0)
				reportArray.Insert(string.Format("%1 formattings", generalFormattingTotal));
			if (endSpacesRemovedTotal > 0)
				reportArray.Insert(string.Format("%1 end spaces trimming", endSpacesRemovedTotal));
			if (fourSpacesReplacedTotal > 0)
				reportArray.Insert(string.Format("%1 4-spaces indent -> tabs replaced", fourSpacesReplacedTotal));
			if (spaceInTabsReplacedTotal > 0)
				reportArray.Insert(string.Format("%1 space(s) in indentation removed", spaceInTabsReplacedTotal));
			if (methodSeparatorFixedTotal > 0)
				reportArray.Insert(string.Format("%1 method separators fixed", methodSeparatorFixedTotal));

			if (m_bAddFinalLineReturn && hasAddedFinalLineReturn)
				reportArray.Insert("added final newline");

			string report = FilePath.StripPath(filePath) + " - ";
			if (reportArray.IsEmpty())
				report += "no fixes to report";
			else
				report += SCR_StringHelper.Join(", ", reportArray, false);

			// ' "" + ' prevents 'string report = ' Print output format
			Print("" + report + " (" + (System.GetTickCount() - startTick) + " ms)", LogLevel.NORMAL);
		}

		if (reportFindings)
		{
			if (!newRefFound.IsEmpty())
				PrintFinding("'new ref'", newRefFound, "no 'ref' is needed on creation, only on declaration");

			if (!newArrayFound.IsEmpty())
				PrintFinding("'new (ref) array<>'", newArrayFound, "replace by {} whenever possible");

			if (!doubleEmptyLineFound.IsEmpty())
				PrintFinding("double empty line", doubleEmptyLineFound, "leave only one");

			if (!autoKeywordFound.IsEmpty())
				PrintFinding("'auto' keyword", autoKeywordFound, "use the direct type instead");

			if (!divideByXFound.IsEmpty())
				PrintFinding("potentially avoidable division", divideByXFound, "use ' * 0.5' instead of ' / 2', ' * 0.1' instead of ' / 10' etc whenever possible");

			if (!unscopedLoopFound.IsEmpty())
				PrintFinding("loop without brackets", unscopedLoopFound, "always use brackets {} with for/foreach/while loops");

			if (!oneLinerFound.IsEmpty())
				PrintFinding("one-liner", oneLinerFound, "use a line return after an if/for/foreach/while condition");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Trim line ends (removes spaces and tabs)
	//! \return number of removed characters
	protected int RemoveTrailingSpaces(out string lineContent)
	{
		int lineLength = lineContent.Length();
		int originalLineLength = lineLength;

		while (lineContent.EndsWith(TAB) || lineContent.EndsWith(SPACE))
		{
			lineContent = lineContent.Substring(0, --lineLength);
		}

		return originalLineLength - lineLength;
	}

	//------------------------------------------------------------------------------------------------
	//! Replace four spaces by one tab
	//! \return 4space-to-tab replacement count (multiple per line possible)
	protected int ReplaceFourSpacesIndentByTab(out string lineContent)
	{
		int lineLength = lineContent.Length();
		if (lineLength < 1)
			return 0;

		string indentation, content;
		GetIndentAndLineContent(lineContent, indentation, content);

		if (indentation.IsEmpty())
			return 0;

		if (content.IsEmpty())
		{
			lineContent = "";
			return 1;
		}

		int replacements = indentation.Replace(FOUR_SPACES, TAB);
		lineContent = indentation + content;

		return replacements;
	}

	//------------------------------------------------------------------------------------------------
	//! Remove spaces in indentation
	//! \return deleted spaces count (multiple per line possible)
	protected int ReplaceSpaceInTabIndent(out string lineContent)
	{
		if (lineContent.IsEmpty())
			return 0;

		string indentation, content;
		GetIndentAndLineContent(lineContent, indentation, content);

		if (indentation.IsEmpty())
			return 0;

		if (content.IsEmpty())
		{
			lineContent = "";
			return 1;
		}

		int replacements = indentation.Replace(SPACE, "");
		lineContent = indentation + content;

		return replacements;
	}

	//------------------------------------------------------------------------------------------------
	//! Fix method separator's length (type "//---", format, boom: "//------------------------------------------------------------------------------------------------")
	protected bool FixMethodSeparator(out string lineContent)
	{
		string indentation, content;
		GetIndentAndLineContent(lineContent, indentation, content);

		if (content.StartsWith("//---") && content.EndsWith("---") && content.Length() != METHOD_SEPARATOR.Length())
		{
			lineContent = indentation + METHOD_SEPARATOR;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! General formatting like spacing, "NULL" → "null", ";;" → ";", etc
	protected int GeneralFormatting(out string lineContent)
	{
		if (lineContent == "}")
		{
			lineContent = "};";
			return 1;
		}
		"{{"; // fix indent

		string indentation, content;
		GetIndentAndLineContent(lineContent, indentation, content);

		if (content.StartsWith("//")) // don't format (inline) comments
			return 0;

		int replacements;
		array<ref array<string>> format = {};

		// line starts
		format.Insert({ "if(",			"if (" });
		format.Insert({ "if ( ",		"if (" });
		format.Insert({ "for(",			"for (" });
		format.Insert({ "for ( ",		"for (" });
		format.Insert({ "foreach(",		"foreach (" });
		format.Insert({ "foreach ( ",	"foreach (" });
		format.Insert({ "while(",		"while (" });
		format.Insert({ "while ( ",		"while (" });
		format.Insert({ "switch(",		"switch (" });
		format.Insert({ "switch ( ",	"switch (" });
		// format.Insert({ "{ ",			"{" }); // weird one-lined scopes will take a hit - arrays too?
		for (int i, cnt = format.Count(); i < cnt; i++)
		{
			if (content.StartsWith(format[i][0]))
				replacements++;

			while (lineContent.Trim().StartsWith(format[i][0]))
			{
				lineContent = SCR_StringHelper.ReplaceTimes(lineContent, format[i][0], format[i][1], 1);
			}
		}

		// class abc" : "parent spacing
		// foreach " : " spacing
		if ((content.StartsWith("class") || content.StartsWith("foreach (")) && content.IndexOf(":") > -1)
		{
			bool replacement = false;
			int colonIndex = lineContent.IndexOf(":");
			if (lineContent[colonIndex - 1] != SPACE)
			{
				lineContent = SCR_StringHelper.InsertAt(lineContent, SPACE, colonIndex);
				replacement = true;
			}

			colonIndex = lineContent.IndexOf(":");
			if (colonIndex < lineContent.Length() -1 && lineContent[colonIndex + 1] != SPACE)
			{
				lineContent = SCR_StringHelper.InsertAt(lineContent, SPACE, colonIndex + 1);
				replacement = true;
			}

			if (replacement)
				replacements++;
		}

		// line middles (beware of strings)
		format.Clear();
		format.Insert({ "  ",	" " }); // double spaces
		format.Insert({ ";;",	";" }); // double semi-colon (so, colon?)
		format.Insert({ " , ",	", " });
		format.Insert({ " ; ",	"; " });
		format.Insert({ "NULL",	"null" });
		format.Insert({ "{ }",	"{}" });
		format.Insert({ "array <",	"array<" });
		if (!lineContent.Contains("\""))
		{
			format.Insert({ "( ", "(" });
			format.Insert({ " )", ")" });
		}
		for (int i, cnt = format.Count(); i < cnt; i++)
		{
			if (lineContent.Contains(format[i][0]))
				replacements++;

			while (lineContent.Contains(format[i][0]))
			{
				lineContent.Replace(format[i][0], format[i][1]);
			}
		}

		// now line ends
		format.Clear();
		format.Insert({ " )",	")" });
		// format.Insert({ " }",	"}" }); // weird one-lined scopes will take a hit e.g 'if (condition) { DoSomething(); return; }'
		format.Insert({ " ,",	"," });
		format.Insert({ " ;",	";" });
		format.Insert({ ")];",	")]" });
		format.Insert({ ", )]",	")]" });
		format.Insert({ ",)]",	")]" });
		// format.Insert({ "++i)",	"i++)" }); // veeery tempted
		// format.Insert({ "--i)",	"i--)" }); // to fix both :D
		for (int i, cnt = format.Count(); i < cnt; i++)
		{
			if (lineContent.EndsWith(format[i][0]))
				replacements++;

			while (lineContent.EndsWith(format[i][0]))
			{
				lineContent = lineContent.Substring(0, lineContent.Length() - format[i][0].Length()) + format[i][1];
			}
		}

		return replacements;
	}

/*
	//------------------------------------------------------------------------------------------------
	protected void RemoveEmptyLines()
	{
		// remove double empty lines
		int actualLine;
		bool isCurrentLineEmpty;
		bool wasPreviousLineEmpty;
		for (int i, cnt = scriptEditor.GetLinesCount(); i < cnt; i++)
		{
			scriptEditor.GetLineText(lineContent, i);
			isCurrentLineEmpty = lineContent.IsEmpty();

			if (!isCurrentLineEmpty || !wasPreviousLineEmpty)
			{
				if (i != actualLine)
					scriptEditor.SetLineText(lineContent, actualLine);
				actualLine++;
			}

			wasPreviousLineEmpty = isCurrentLineEmpty;
		}

		// Print("last line with actual content = " + actualLine);

		// delete trailing lines
		for (int cnt = scriptEditor.GetLinesCount() - 1; actualLine <= cnt; cnt--)
		{
			scriptEditor.RemoveLine(cnt);
		}

		// remove top empty lines
		scriptEditor.GetLineText(lineContent, 0);
		for (int i = 0; i < 50; i++)
		{
			if (!lineContent.IsEmpty())
				break;

			scriptEditor.RemoveLine(0);
			scriptEditor.GetLineText(lineContent, 0);
		}

		// remove bottom empty lines
		for (int i; i < 500; i++)
		{
			scriptEditor.RemoveLine(actualLine);
		}
	}
// */

	//------------------------------------------------------------------------------------------------
	//! Add the final line return to a file (to end with a line return instead of the usual "};"
	//! \return true if an empty line has been added, false otherwise
	protected bool AddFinalLineReturn(ScriptEditor scriptEditor)
	{
		int lastLineNumber = scriptEditor.GetLinesCount() - 1;
		string lineContent;
		scriptEditor.GetLineText(lineContent, lastLineNumber);
		if (lineContent.Trim().IsEmpty())
			return false;

		// haxx
		scriptEditor.InsertLine(string.Empty, lastLineNumber);
		scriptEditor.SetLineText(lineContent, lastLineNumber);
		scriptEditor.SetLineText(string.Empty, lastLineNumber + 1);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Split line between indentation and content (trailing spaces included if any)
	//! \param lineContent the line's content
	//! \param[out] indentation - gets the left spacing (tabs and spaces)
	//! \param[out] content - gets the text
	protected void GetIndentAndLineContent(string lineContent, out string indentation, out string content)
	{
		int lineLength = lineContent.Length();
		if (lineLength < 1)
		{
			indentation = string.Empty;
			content = string.Empty;
			return;
		}

		int firstCharIndex = -1;
		for (int i; i < lineLength; i++)
		{
			if (lineContent[i] != SPACE && lineContent[i] != TAB)
			{
				firstCharIndex = i;
				break;
			}
		}

		indentation = lineContent.Substring(0, firstCharIndex);
		content = lineContent.Substring(firstCharIndex, lineLength - firstCharIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Joins the ints together with commas and ampersand for the last element
	//! examples:
	//! {} = ""
	//! { 1 } = "1"
	//! { 1, 2 } = "1 & 2"
	//! { 1, 2, 3 } = "1, 2 & 3"
	//! { 1, 2, 3, 4 } = "1, 2, 3 & 4"
	protected string JoinLineNumbers(notnull array<int> lineNumbers)
	{
		if (lineNumbers.IsEmpty())
			return string.Empty;

		string result = lineNumbers[0].ToString();
		int count = lineNumbers.Count();
		for (int i = 1; i < count - 1; i++)
		{
			result += ", " + lineNumbers[i];
		}

		if (count > 1)
			result += " & " + lineNumbers[count - 1];

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Print the finding with line numbers
	//! examples:
	//! "int(s)", {}, ""							= "No int(s) found"
	//! "int(s)", {}, "a tip"						= "No int(s) found"
	//! "int(s)", { 1, 2, 3 }, ""					= "3 int(s) found at line(s) 1, 2 & 3"
	//! "int(s)", { 1, 2, 3 }, "use longs instead"	= "3 int(s) found at line(s) 1, 2 & 3 - use longs instead"
	//! \param description the finding(s)' description
	//! \param lineNumbers the lines where the findings have been found
	//! \param tip a suggestion to fix the finding
	protected void PrintFinding(string description, notnull array<int> lineNumbers, string tip = string.Empty)
	{
		description.TrimInPlace();
		tip.TrimInPlace();
		if (lineNumbers.IsEmpty())
		{
			Print("No " + description + " found", LogLevel.NORMAL);
			return;
		}

		if (!tip.IsEmpty())
			tip = " - " + tip;

		Print(lineNumbers.Count().ToString() + "× " + description + " found at line(s) " + JoinLineNumbers(lineNumbers) + tip, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Configure 'Basic Code formatter' plugin", "Formats code, basically.", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close", true)]
	protected bool ButtonClose()
	{
		return true;
	}
};
