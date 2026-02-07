[WorkbenchPluginAttribute(name: "Settings", category: "In-game Editor", wbModules: {"WorldEditor"})]
class EditorPlugin : WorldEditorPlugin
{
	override void OnGameModeStarted(string worldName, string gameMode, bool playFromCameraPos, vector cameraPosition, vector cameraAngles)
	{
		//--- Player is spawned at camera position; initialize, but don't open the editor
		//--- Disabled, was showing error and editor was initialized either way
		if (playFromCameraPos)
		{
#ifdef WORKBENCH
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (core) core.OnPlayFromCameraPos();
#endif
		}
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager) return;
		
		ArmaReforgerScripted game = GetGame();
		if (!game) return;
		
		BaseWorld world = game.GetWorld();
		if (!world) return;
		
		//--- Exit when the editor is already opened, a player is present or a camera is already initalized
		vector matrix[4];
		world.GetCurrentCamera(matrix);
		if (editorManager.IsOpened() || game.GetPlayerController() || matrix[3] != vector.Zero) return;
		
		//--- Open the editor
		editorManager.Open();
		if (!editorManager.IsOpened()) return; //--- Failed to open for some reason, exit
		Print("No player present in the world, starting Editor instead.", LogLevel.DEBUG);
		
		//--- Move editor camera to World Editor camera position
		vector angles = Vector(cameraAngles[1], cameraAngles[0], cameraAngles[2]);
		world.SetCamera(world.GetCurrentCameraId(), cameraPosition, angles);
	}
	
	//--- Play the editor directly
	//--- Example: -plugin=EditorPlugin
	override void RunCommandline() 
	{
		Workbench.OpenModule(WorldEditor);
		WorldEditor we = Workbench.GetModule(WorldEditor);
		if (!we) return;
		
		we.SetOpenedResource("{25E6D4AEC3F45872}worlds/Editor/Test/TestGameMaster.ent");
		we.SwitchToGameMode();
	}
};