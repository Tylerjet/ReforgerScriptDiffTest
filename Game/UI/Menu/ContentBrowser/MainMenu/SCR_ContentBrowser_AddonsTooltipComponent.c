/*!
Component which must be attached to tooltip which lists dependencies or dependent mods.
*/
class SCR_ContentBrowser_AddonsTooltipComponent : ScriptedWidgetComponent
{
	[Attribute("{30907DAA2D89E065}UI/layouts/Menus/ContentBrowser/Tile/AddonsTooltipLineNoBG.layout")]
	protected ResourceName m_sAddonLineLayout;
	
	VerticalLayoutWidget m_wAddonsList;
	
	void Init(array<ref SCR_WorkshopItem> addons)
	{
		foreach (SCR_WorkshopItem addon : addons)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sAddonLineLayout, m_wAddonsList);
			TextWidget wText = TextWidget.Cast(w.FindAnyWidget("Text"));
			if (wText)
				wText.SetText(addon.GetName());
		}
	}
	
	protected override void HandlerAttached(Widget w)
	{
		m_wAddonsList = VerticalLayoutWidget.Cast(w.FindAnyWidget("AddonsList"));
	}
};