[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditorVONControllerComponentClass : SCR_BaseEditorComponentClass
{
}

//! Manages interaction with Voice Over Network IEntityComponentSource
class SCR_EditorVONControllerComponent : SCR_BaseEditorComponent
{
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpen()
	{
		SCR_EditorManagerEntity instance = SCR_EditorManagerEntity.GetInstance();
		
		if (instance)
			instance.GetOnModeChange().Insert(OnEditorModeChange);
		
		OnEditorModeChange();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorClose()
	{
		SCR_EditorManagerEntity instance = SCR_EditorManagerEntity.GetInstance();
		
		if (instance)
			instance.GetOnModeChange().Remove(OnEditorModeChange);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorModeChange()
	{
		EEditorMode mode = SCR_EditorManagerEntity.GetInstance().GetCurrentMode();
		
		// Prevent von menu using
		SCR_VONController vonController = SCR_VONController.Cast(GetGame().GetPlayerController().FindComponent(SCR_VONController));
		if (vonController && vonController.GetVONMenu())
			vonController.GetVONMenu().SetMenuDisabled(mode == EEditorMode.PHOTO);
	}
}
