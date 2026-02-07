//------------------------------------------------------------------------------------------------
/*class AddonsToolsUI : SCR_SuperMenuBase
{
	//protected SCR_AddonsTools_PresetDialogWidgets m_Widgets = new SCR_AddonsTools_PresetDialogWidgets();
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		// List 
		UpdatePresetsList();
		
		// Buttons and actions 
		
	}
	
	//------------------------------------------------------------------------------------------------\
	//! Show current list of created presets 
	protected void UpdatePresetsList()
	{
		/*
		SCR_ListBoxComponent lb = widgets.m_PresetNamesListboxComponent;
		
		// Remove all items
		while (lb.GetItemCount() > 0)
			lb.RemoveItem(0);
		
		SCR_WorkshopAddonManagerPresetStorage presetStorage = SCR_AddonManager.GetInstance().GetPresetStorage();
		array<ref SCR_WorkshopAddonPreset> presets = presetStorage.GetAllPresets();
		
		foreach (SCR_WorkshopAddonPreset preset : presets)
		{
			SCR_WorkshopAddonPresetListItemData data = new SCR_WorkshopAddonPresetListItemData(preset.GetName());
			lb.AddItem(GetPresetDisplayName(preset), data);
		}
		*/
	//}
	
	//------------------------------------------------------------------------------------------------\
	//! Create new preset from given mods items list
	/*protected void CreateNewPreset(array<SCR_WorkshopItem> items)
	{
		
	}
};