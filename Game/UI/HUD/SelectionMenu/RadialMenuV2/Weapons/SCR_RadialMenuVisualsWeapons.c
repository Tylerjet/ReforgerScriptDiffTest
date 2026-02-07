class SCR_RadialMenuVisualsWeapons : SCR_RadialMenuVisuals
{
	static private ResourceName m_ItemPreviewManagerPrefab = "{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et";

	// Entries widget elements references 
	protected const string RADIALMENU_ICON = "Icon";
	protected const string RADIALMENU_ICON_RENDER = "IconRender";
	
	// Localized string formats 
	protected const string EMPTY_SLOT_FORMAT = "#AR-WeaponMenu_SlotEmptyFormat";
	protected const string COUNT_FORMAT = "%1x";
	
	// Image references for empty slots - TODO: currently obsolete,add this to icon once we have ready empty icons 
	[Attribute(SCR_RadialMenuIcons.RADIALMENU_ICON_EMPTY, UIWidgets.ResourceNamePicker, "Layout", "edds")]
	protected ResourceName m_rMissingIconPrimary;
	
	[Attribute(SCR_RadialMenuIcons.RADIALMENU_ICON_EMPTY, UIWidgets.ResourceNamePicker, "Layout", "edds")]
	protected ResourceName m_rMissingIconSecondary;
	
	[Attribute(SCR_RadialMenuIcons.RADIALMENU_ICON_EMPTY, UIWidgets.ResourceNamePicker, "Layout", "edds")]
	protected ResourceName m_rMissingIconGrenade;
	
	[Attribute(SCR_RadialMenuIcons.RADIALMENU_ICON_EMPTY, UIWidgets.ResourceNamePicker, "Layout", "edds")]
	protected ResourceName m_rIconBandage;
	
	// Source 
	protected IEntity m_SourceEntity;
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnOpen(IEntity owner)
	{	
		super.OnOpen(owner);

		if (m_RadialMenuHandler && !m_SourceEntity)
			m_SourceEntity = m_RadialMenuHandler.GetSource();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetContent(array<BaseSelectionMenuEntry> allEntries, array<BaseSelectionMenuEntry> disabledEntries, bool clearData = false)
	{
		// Create renderer - this is global
		if (!GetGame().GetItemPreviewManager())
		{
			Resource rsc = Resource.Load(m_ItemPreviewManagerPrefab);
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
		if (!entry)
			return;
		
		entry.UpdateVisuals();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set icon names by type of the entry  
	override protected string SetEmptyIconByType(ScriptedSelectionMenuEntry entry)
	{
		if (!entry)
			return m_sEmptyIconName;
		
		//protected string m 
		
		ImageWidget widgetIcon = null;
		float entrySize = 64;
		
		SCR_SelectionEntryWidgetComponent entryWidget = entry.GetEntryComponent();
		if (entryWidget)
			entryWidget.SetIconSize(entrySize, entrySize);
		
		// Weapons  
		if ( SCR_WeaponSwitchSelectionMenuEntry.Cast(entry) )
		{
			SCR_WeaponSwitchSelectionMenuEntry entryWepon = SCR_WeaponSwitchSelectionMenuEntry.Cast(entry);
			
			switch (entryWepon.GetTargetSlot().GetWeaponSlotType())
			{
				// Primary
				case "primary":
				if (entryWidget)
					entryWidget.SetIconSize(entrySize * 2, entrySize * 2);
				return "weapon_primary";
				
				// Secondary 
				case "secondary":
				return "weapon_secondary";
				
				// Grenade 
				case "grenade":
				return "weapon_grenade";
			}
			
			return m_sEmptyIconName;
		}
			
		// Bandage 
		if ( SCR_ConsumableSelectionMenuEntry.Cast(entry) )
		{
			return "gadget_bandage";
		}
		
		return m_sEmptyIconName;
	}
};