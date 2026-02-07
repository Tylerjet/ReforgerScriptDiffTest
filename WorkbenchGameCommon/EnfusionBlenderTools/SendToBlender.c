[WorkbenchPluginAttribute(name: "Send To Blender", wbModules: { "ResourceManager" }, resourceTypes: { "fbx", "xob", "et" }, awesomeFontCode: 0xF00C)]
class SendToBlenderPlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "C:\\Program Files\\Blender Foundation\\Blender 3.6\\blender.exe", uiwidget: UIWidgets.FileNamePicker, desc: "Absolute path to your blender.exe", params: "exe FileNameFormat=absolute", category: "Path")]
	protected string m_sBlenderPath;

	//------------------------------------------------------------------------------------------------
	override void OnResourceContextMenu(notnull array<ResourceName> resources)
	{
		if (resources.IsEmpty())
		{
			Print("No selected resources", LogLevel.WARNING);
			return;
		}

		if (!Workbench.ScriptDialog("Send To Blender", "Make sure you have supported version of Blender and newest EBT installed.", this))
			return;

		if (!FileIO.FileExists(m_sBlenderPath))
		{
			Print(m_sBlenderPath + " does not exist! Please select a valid path.", LogLevel.WARNING);
			return;
		}

		if (resources[0].EndsWith(".et"))
			ImportET(resources[0]);
		else
			ImportXOB(resources[0]);
	}

	//------------------------------------------------------------------------------------------------
	protected void ImportXOB(ResourceName resource)
	{
		// Get path to FBX
		string path = resource.GetPath();
		string absPath;
		Workbench.GetAbsolutePath(path, absPath);
		if (absPath.EndsWith(".xob"))
			absPath.Replace(".xob", ".fbx");

		// call Blender via console with python expression that imports the FBX
		if (!FileIO.FileExists(absPath))
		{
			Print("Resource couldn't be imported to blender because .fbx file is missing", LogLevel.WARNING);
			return;
		}

		string command = string.Format("\"%1\" --python-expr \"import bpy;bpy.ops.scene.ebt_import_fbx()\" -- -filePath \"%2\" -remove %3", m_sBlenderPath, absPath, true);
		Workbench.RunProcess(command);
	}

	//------------------------------------------------------------------------------------------------
	protected void ImportET(ResourceName resource)
	{
		// Get absolute path to .et
		string path = resource.GetPath();
		string absPath;
		Workbench.GetAbsolutePath(path, absPath);

		// call Blender via console with python expression that imports the FBX
		if (!FileIO.FileExists(absPath))
		{
			Print("Resource couldn't be imported to blender because .et file is missing", LogLevel.WARNING);
			return;
		}

		string command = string.Format("\"%1\" --python-expr \"import bpy;bpy.ops.scene.ebt_import_prefab()\" -- -filePath \"%2\" -remove %3", m_sBlenderPath, absPath, true);

		Workbench.RunProcess(command);
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
