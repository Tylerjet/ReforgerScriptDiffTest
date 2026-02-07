[WorkbenchPluginAttribute("Re-Save Tool", "Saves all files with given extension", "", "", {"ResourceManager"},"",0xf0c7)]
class ResavePlugin: WorkbenchPlugin
{
	[Attribute(uiwidget: UIWidgets.FileNamePicker, desc: "Root directory to search", params:"unregFolders" )]
	string RootPath;
	
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "File extensions" )]
	ref array<string> Extensions;
	
	ref array<ResourceName> m_files = new array<ResourceName>;
	
	void Find(ResourceName resName)
	{
		m_files.Insert(resName);
	}
	
	void Resave()
	{
		WBProgressDialog progress = new WBProgressDialog("Resaving...", null);
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		int cnt = m_files.Count();	
		
		foreach (int i, ResourceName file: m_files)
		{
			Print("Resaving: " + file);
			Resource res = BaseContainerTools.LoadContainer(file);
			if (res && res.IsValid())
			{
				BaseContainer cont = res.GetResource().ToBaseContainer();
				BaseContainerTools.SaveContainer(cont, file);
			}
			else
			{
				rm.RebuildResourceFile(file.GetPath(), "PC", false);
			}
			
			progress.SetProgress(i / cnt);
		}
		m_files.Clear();
	}
	
	override void Run()
	{
		if (Workbench.ScriptDialog("Resave", "Which files you want to resave?", this))
		{
			Workbench.SearchResources(Find, Extensions, null, RootPath);
			Resave();			
		}
	}
	
	override void RunCommandline() 
	{
		ResourceManager module = Workbench.GetModule(ResourceManager);
		module = Workbench.GetModule(ResourceManager);
		
		string ext;
		if (module.GetCmdLine("-extension", ext))
		{
			ext.Replace("\"", "");
			ext.Replace(".", "");
			Extensions = {ext};
		}
		
		Workbench.SearchResources(Find, Extensions);
		Resave();
		Workbench.Exit(0);
	}
	
	[ButtonAttribute("Re-Save")]
	bool OK()
	{
		return true;
	}
	
	[ButtonAttribute("Cancel")]
	bool Cancel()
	{
		return false;
	}
}