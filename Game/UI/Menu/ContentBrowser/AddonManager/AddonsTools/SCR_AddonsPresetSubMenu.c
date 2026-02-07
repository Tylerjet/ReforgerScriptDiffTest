/*!
Class for displaying, saving and editing mod presets
*/

class SCR_AddonsPresetsSubMenu: SCR_SubMenuBase
{
	protected ref SCR_ListAddonsPresetsWidgets m_Widgets = new SCR_ListAddonsPresetsWidgets();
	
	//------------------------------------------------------------------------------------------------
	// Overrided widget api
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_Widgets.Init(w);
		
		// List 
		UpdatePresetsList();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		
		// Buttons and actions 
		CreateNavigationButton("MenuBack", "test");
	}
	
	//------------------------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	//! Show current list of created presets 
	protected void UpdatePresetsList()
	{
		SCR_ListBoxComponent lb = m_Widgets.m_ScrollPresetsComponent;
		
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
	}
	
	//---------------------------------------------------------------------------------------------------
	protected string GetPresetDisplayName(notnull SCR_WorkshopAddonPreset preset)
	{
		int count  = preset.GetAddonCount();
		string strMods;
		if (count == 1)
			strMods = "1 mod";
		else
			strMods = string.Format("%1 mods", count);
		
		return string.Format("%1  -  %2", preset.GetName(), strMods);
	}
	
	//------------------------------------------------------------------------------------------------\
	//! Create new preset from given mods items list
	protected void CreateNewPreset(array<SCR_WorkshopItem> items)
	{
		
	}
};