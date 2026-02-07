class SCR_RadialMenuVisualsItems : SCR_RadialMenuVisuals
{
	// Entries widget elements references 
	protected const string RADIALMENU_ICON = "Icon";
	protected const string RADIALMENU_ICON_RENDER = "IconRender";
	
	// Localized string formats 
	protected const string EMPTY_SLOT_FORMAT = "#AR-WeaponMenu_SlotEmptyFormat";
	protected const string COUNT_FORMAT = "%1x";
	
	// Data templates 
	protected static ResourceName s_sItemPreviewManagerPrefab = "{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et";
	
	//------------------------------------------------------------------------------------------------
	override void SetContent(array<BaseSelectionMenuEntry> allEntries, array<BaseSelectionMenuEntry> disabledEntries, bool clearData = false)
	{
		// Create renderer - this is global
		if (!GetGame().GetItemPreviewManager())
		{
			Resource rsc = Resource.Load(s_sItemPreviewManagerPrefab);
			if (rsc.IsValid())
				GetGame().SpawnEntityPrefabLocal(rsc);
		}
		
		super.SetContent(allEntries, disabledEntries, clearData);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update magazine count for wepaons icons
	override protected void SetElementData(SCR_RadialMenuPair<Widget, BaseSelectionMenuEntry> element, bool canBePerformed, SCR_SelectionEntryWidgetComponent widgetComp)
	{
		super.SetElementData(element, canBePerformed, widgetComp);
		
		// Update entry visuals 
		ScriptedSelectionMenuEntry entry = ScriptedSelectionMenuEntry.Cast(element.m_pEntry);
		if (entry)
			entry.UpdateVisuals();
	}
};