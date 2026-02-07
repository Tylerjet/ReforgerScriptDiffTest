[WorkbenchPluginAttribute(name: "EBT: Send To Blender", wbModules: { "ResourceManager" }, resourceTypes: { "fbx", "xob", "et" }, awesomeFontCode: 0xf1d8)]
class SendToBlenderPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void OnResourceContextMenu(notnull array<ResourceName> resources)
	{
		if (!EBTConfigPlugin.HasBlenderRegistered())
			return;
		
		
		
		if (resources.IsEmpty())
		{
			Print("No selected resources", LogLevel.WARNING);
			return;
		}
		
		BlenderOperatorDescription operatorDescription = new BlenderOperatorDescription("core");
		
		operatorDescription.blIDName = "scene.ebt_import_fbx";
		if (resources[0].EndsWith(".et"))
			operatorDescription.blIDName = "scene.ebt_import_prefab";		
		
		string absPath; 
		
		if (GetPathToResource(resources[0], absPath))
		{	
			operatorDescription.AddParam("file_path_override", absPath); // ImportHelper is using self.file_path
			operatorDescription.AddParam("remove_all_objects", true);
			
			StartBlenderWithOperator(operatorDescription, false);
			return;	
		}
		
		Print("Resource couldn't be imported to blender because requested resource file is missing or is locked", LogLevel.WARNING);
		
	}
	
	protected bool GetPathToResource(ResourceName resource, inout string absPath)
	{
		string path = resource.GetPath();
		Workbench.GetAbsolutePath(path, absPath);
		
		
		if (!FileIO.FileExists(absPath) || absPath == string.Empty)
			return false;
		
		if (resource.EndsWith(".et"))
			return true;
		
		if (absPath.EndsWith(".xob"))
			absPath.Replace(".xob", ".fbx");
		
		if (!FileIO.FileExists(absPath))
			return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Send")]
	protected bool ButtonOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}
