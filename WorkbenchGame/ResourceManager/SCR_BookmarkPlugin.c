#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Bookmark 1", category: "Bookmarks", shortcut: "Ctrl+1", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin1 : WorkbenchPlugin
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_sFile;

	//------------------------------------------------------------------------------------------------
	protected int GetBookmarkIndex()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);

		array<ResourceName> selection = {};
		resourceManager.GetResourceBrowserSelection(selection.Insert, true);

		int action = -1;
		if (selection.IsEmpty() || m_sFile == selection[0])
			action = 0;
		else if (m_sFile)
			action = Workbench.ScriptDialog("Open or Replace", string.Format("Do you want to open existing bookmark '%1'\nor replace it by selected file '%2'?", FilePath.StripPath(m_sFile), FilePath.StripPath(selection[0])), new SCR_BookmarkPluginPrompt);
		else
			action = 1;

		switch (action)
		{
			case -1:
				return;

			case 0:
				if (m_sFile)
					resourceManager.SetOpenedResource(m_sFile);
				break;

			case 1:
				m_sFile = selection[0];
				Print(string.Format("Bookmark %1 now points to @\"%2\"", GetBookmarkIndex(), m_sFile.GetPath()), LogLevel.NORMAL);
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog(string.Format("Configure 'Bookmark %1' plugin", GetBookmarkIndex()), "Select which file will be opened upon pressing bookmark's shortcut.", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected bool Close()
	{
		return true;
	}
}

[WorkbenchPluginAttribute(name: "Bookmark 2", category: "Bookmarks", shortcut: "Ctrl+2", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin2 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 2; } }

[WorkbenchPluginAttribute(name: "Bookmark 3", category: "Bookmarks", shortcut: "Ctrl+3", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin3 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 3; } }

[WorkbenchPluginAttribute(name: "Bookmark 4", category: "Bookmarks", shortcut: "Ctrl+4", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin4 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 4; } }

[WorkbenchPluginAttribute(name: "Bookmark 5", category: "Bookmarks", shortcut: "Ctrl+5", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin5 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 5; } }

[WorkbenchPluginAttribute(name: "Bookmark 6", category: "Bookmarks", shortcut: "Ctrl+6", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin6 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 6; } }

[WorkbenchPluginAttribute(name: "Bookmark 7", category: "Bookmarks", shortcut: "Ctrl+7", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin7 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 7; } }

[WorkbenchPluginAttribute(name: "Bookmark 8", category: "Bookmarks", shortcut: "Ctrl+8", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin8 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 8; } }

[WorkbenchPluginAttribute(name: "Bookmark 9", category: "Bookmarks", shortcut: "Ctrl+9", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin9 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 9; } }

[WorkbenchPluginAttribute(name: "Bookmark 10", category: "Bookmarks", shortcut: "Ctrl+0", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin0 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 10; } }

class SCR_BookmarkPluginPrompt
{
	//------------------------------------------------------------------------------------------------
	/*
	[ButtonAttribute("Cancel")]
	int Cancel()
	{
		return -1;
	}
	*/

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Replace")]
	protected int Replace()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Open", true)]
	protected int Open()
	{
		return 0;
	}
}
#endif // WORKBENCH
