[WorkbenchPluginAttribute("Re-Save Meta Tool", "Saves all meta files with given extension", "", "", {"ResourceManager"},"",0xf0c7)]
class ResaveMetaPlugin: WorkbenchPlugin
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "File extensions (without .meta)" )]
	ref array<string> Extensions;
	ResourceManager m_module;
	ref array<string> m_files = {};
	
	void Find(ResourceName resName, string file)
	{
		m_files.Insert(file + ".meta");
	}
	
	void Resave()
	{
		WBProgressDialog progress = new WBProgressDialog("Resaving...", m_module);
		int cnt = m_files.Count();						
		foreach (int i, string file: m_files)
		{
			ResourceName resName = Workbench.GetResourceName(file);
			Resource res = BaseContainerTools.LoadContainer(resName);
			if (res && res.IsValid())
			{
				Print("Resaving: " + file);
				BaseContainer cont = res.GetResource().ToBaseContainer();
				BaseContainerTools.SaveContainer(cont, "", file);
			}
			
			progress.SetProgress(i / cnt);
		}
		m_files.Clear();
	}
	
	override void Run()
	{
		if (Workbench.ScriptDialog("Resave", "Which files metafiles you want to resave?", this))
		{
			m_module = Workbench.GetModule(ResourceManager);
			
			SearchResourcesFilter filter = new SearchResourcesFilter();
			filter.fileExtensions = Extensions;
			ResourceDatabase.SearchResources(filter, Find);
			
			Resave();			
		}
	}
	
	override void RunCommandline() 
	{
		m_module = Workbench.GetModule(ResourceManager);
		
		string ext;
		if (m_module.GetCmdLine("-extension", ext))
		{
			ext.Replace("\"", "");
			ext.Replace(".", "");
			Extensions = {ext};
		}
		
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = Extensions;
		ResourceDatabase.SearchResources(filter, Find);
		
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