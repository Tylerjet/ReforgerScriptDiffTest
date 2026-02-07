/*
===========================================
Do not modify, this script is generated
===========================================
*/

class GameWorldEditor: Managed
{
	proto external WorldEditorAPI GetEditorAPI();
	proto external bool LoadWorld(string worldPath);
	proto external bool SaveWorld();
	//! If overrideWorld is true, and if the path already exists it will be overridden
	proto external bool SaveWorldAs(string savePath, bool overridePath = false);
	proto external bool CreateNewSubsceneWorld();
	proto external void Undo();
	proto external void Redo();
	proto external bool SwitchToGameMode();
	proto external void SwitchToEditMode();
}
