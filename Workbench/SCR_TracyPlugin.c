[WorkbenchPluginAttribute(
	name: "Run Tracy Profiler",
	description: "Run Tracy Profiler",
	wbModules: { "ResourceManager", "ScriptEditor", "WorldEditor" },
	awesomeFontCode: 0x0054)]
class SCR_TracyPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		string binDir;
		Workbench.GetCwd(binDir);
		Workbench.RunProcess(binDir + "/tracy/Tracy.exe");
	}
}
