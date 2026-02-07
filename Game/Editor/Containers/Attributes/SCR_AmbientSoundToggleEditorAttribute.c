/**
Toggle Ambient Sound Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_AmbientSoundToggleEditorAttribute : SCR_BaseEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		SCR_MusicManager musicManager = SCR_MusicManager.GetInstance();
		if (!musicManager)
			return null;
	
		return SCR_BaseEditorAttributeVar.CreateBool(musicManager.IsAmbientMusicAuthorizedByServer());
	}	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		SCR_MusicManager musicManager = SCR_MusicManager.GetInstance();
		if (!musicManager) 
			return;
		
		musicManager.AuthorizeAmbientMusicForClients(var.GetBool(), playerID);
	}
};