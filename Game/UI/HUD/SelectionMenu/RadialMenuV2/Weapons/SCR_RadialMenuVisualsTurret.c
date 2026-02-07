class SCR_RadialMenuVisualsTurret : SCR_RadialMenuVisualsWeapons
{
	//------------------------------------------------------------------------------------------------
	//! Update magazine count for wepaons icons
	override protected void SetElementData(SCR_RadialMenuPair<Widget, BaseSelectionMenuEntry> element, bool canBePerformed, SCR_SelectionEntryWidgetComponent widgetComp)
	{
		//super.SetElementData(element, canBePerformed);
		/*Widget ammoLayout = element.m_pWidget.FindAnyWidget(ENTRY_AMMO_LAYOUT);
		if(!ammoLayout)
			return;
		
		SetVisibleSafe(ammoLayout, false);
		
		// Get datat for rendering
		SCR_TurretWeaponMenuEntry weaponEntry = SCR_TurretWeaponMenuEntry.Cast(element.m_pEntry);
		if(!weaponEntry)
			return;
		
		WeaponSlotComponent slot = weaponEntry.GetTargetSlot();
		if(!slot)
			return;
		
		// find widgets 
		ImageWidget icon = ImageWidget.Cast(element.m_pWidget.FindAnyWidget("Icon"));
		RenderTargetWidget previewImage = RenderTargetWidget.Cast(icon.GetParent().FindAnyWidget("IconRender"));
		if(!previewImage || !icon)
			return;
		
		// Render Weapon item
		RenderWeaponIcon(previewImage, slot.GetWeaponEntity());
		SetVisibleSafe(icon, false);*/
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuVisualsTurret(IEntity owner)
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_RadialMenuVisualsTurret()
	{
		
	}
};