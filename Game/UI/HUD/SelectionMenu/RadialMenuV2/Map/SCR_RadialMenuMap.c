//------------------------------------------------------------------------------------------------
class SCR_RadialMenuMap : SCR_RadialMenuVisuals
{
	//------------------------------------------------------------------------------------------------
	void InitMenuVisuals(SCR_RadialMenuHandler handler, Widget root)
	{
		SetMenuHandler(handler);
		
		m_wRoot = root;
		
		m_LayoutPath = "{C967377E3D96107D}UI/layouts/Common/RadialMenu/RadialMenuVisuals.layout";
		m_rElementLayoutPath = "{95123068DC2C2ECF}UI/layouts/Common/RadialMenu/radialElementIcon.layout";
		m_rEmptyElementIconPath = SCR_RadialMenuIcons.RADIALMENU_ICON_EMPTY;
		m_rBorderLinePath = "{639717DA6610F1C1}UI/layouts/Common/RadialMenu/RadialMenuBorderLine.layout";
		m_sPageIndicationLayout = "{595FE46A06BE9DB3}UI/layouts/Common/RadialMenu/MenuPageIndication.layout";
		m_sPageIconsSet = "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset";
		m_sLayoutWrap = WIDGET_LAYOUT_WRAP;
		m_sLayoutBase = WIDGET_LAYOUT_BASE;
		m_sLayoutContent = WIDGET_LAYOUT_CONTET;
		m_sLayoutInfo = WIDGET_LAYOUT_INFO;
		m_sTxtAction = "TxtAction";
		m_sTxtItemName = "TxtItemName";
		m_sImageSelector = "ImgSelector";
		m_sSelectLast = "SelectLast";
		m_sSelectCurrent = WIDGET_SELECT_CURRENT;
		m_sEmptyIconName = "cancel";
		m_fSizeMenu = 660.000;
		m_fSizeEntries = 750;
		m_fRadiusEntries = 256;
		m_fSelectorDistance = 185;
		m_fFadeInSpeed = 10;
		m_fFadeOutSpeed = 10;
		
		OnCreate(GetGame().GetPlayerController());
		DisplayInit(GetGame().GetPlayerController());
	}
	
	//------------------------------------------------------------------------------------------------
	override Widget CreateElementWidget(Widget root, ScriptedSelectionMenuEntry entry)
	{
		Widget wEntry = super.CreateElementWidget(root, entry);
		
		SCR_MapMenuEntry mEntry = SCR_MapMenuEntry.Cast(entry);
		if (mEntry) 
			mEntry.UpdateVisuals();
		
		SCR_MapMenuCategory mCategory = SCR_MapMenuCategory.Cast(entry);
		if (mCategory) 
			mCategory.UpdateVisuals();
		
		return wEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Applies providede element data (from entry) to the widget (value)
	override void SetElementData(SCR_RadialMenuPair<Widget, BaseSelectionMenuEntry> element, bool canBePerformed, SCR_SelectionEntryWidgetComponent widgetComp)
	{
		if (!element || element.IsEmpty())
			return;
		
		// Set entry default icon 
		ScriptedSelectionMenuEntry entryGrouped = ScriptedSelectionMenuEntry.Cast(element.m_pEntry);
		if (!entryGrouped)
			return;
		
		SCR_SelectionEntryWidgetComponent entryWidget = entryGrouped.GetEntryComponent();
		if (!entryWidget)
			return;
		
		entryWidget.SetIcon(entryWidget.GetImageTexture(), entryWidget.GetImageName());
	}
};
